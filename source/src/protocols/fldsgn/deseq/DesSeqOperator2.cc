// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/deseq/DesSeqOperator.cc
/// @brief
/// @author Nobuyasu Koga ( nkoga@protein.osaka-u.ac.jp )

// unit headers
#include <protocols/fldsgn/deseq/DesSeqOperator2.hh>
#include <core/pack/task/ResfileReader.hh>

// Project headers
#include <basic/Tracer.hh>

#include <core/conformation/Conformation.hh>
#include <core/conformation/symmetry/SymmetryInfo.hh>
#include <core/id/AtomID_Mask.fwd.hh>
#include <core/kinematics/MoveMap.hh>
#include <core/pack/task/TaskFactory.hh>
#include <core/pack/task/operation/TaskOperations.hh>
#include <core/pack/task/PackerTask_.hh>
#include <core/pose/Pose.hh>
#include <core/pose/util.hh>
#include <core/pose/symmetry/util.hh>
#include <core/scoring/dssp/Dssp.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <core/scoring/rms_util.hh>
#include <core/util/SwitchResidueTypeSet.hh>

#include <protocols/fldsgn/topology/SS_Info2.hh>
#include <protocols/fldsgn/topology/util.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <protocols/minimization_packing/PackRotamersMover.hh>
#include <protocols/minimization_packing/symmetry/SymMinMover.hh>
#include <protocols/pose_creation/MakePolyXMover.hh>
#include <protocols/relax/FastRelax.hh>
#include <protocols/simple_task_operations/RestrictToInterface.hh>
#include <protocols/simple_filters/PackStatFilter.hh>
#include <protocols/simple_filters/HolesFilter.hh>
#include <protocols/task_operations/LimitAromaChi2Operation.hh>

#include <protocols/jd2/Job.hh>
#include <protocols/jd2/JobDistributor.hh>
#include <protocols/jd2/JobOutputter.hh>

// utility headers
#include <boost/lexical_cast.hpp>
#include <utility/exit.hh>
#include <iostream>
#include <fstream>
#include <utility/vector1.hh>


static basic::Tracer TR( "protocols.fldsgn.deseq.DesSeqOperator2" );

namespace protocols {
namespace fldsgn {
namespace deseq {

using namespace core;
using namespace core::pack;
using namespace core::pack::task;
using namespace core::chemical;
using core::chemical::AA;
using core::chemical::aa_none;
using core::chemical::num_canonical_aas;


/// @brief constructor
DesSeqOperator2::DesSeqOperator2():
    protocols::moves::Mover("DesSeqOperator2"),
    scorefxn_design_( new ScoreFunction ),
    scorefxn_relax_( new ScoreFunction ),
    tf_design_( new TaskFactory ),
    tf_relax_( new TaskFactory ),
    relax_structure_( true ),
    only_interface_( false ),
    dump_trajectory_( false )
{
    initialize();
}


/// @brief copy constructor
DesSeqOperator2::DesSeqOperator2( DesSeqOperator2 const & r ) :
    protocols::moves::Mover( r ),
    scorefxn_design_( r.scorefxn_design_ ),
    scorefxn_relax_( r.scorefxn_relax_ ),
    tf_design_( r.tf_design_ ),
    tf_relax_( r.tf_relax_ ),
    movemap_( r.movemap_ ),
    resfile_( r.resfile_ ),
    relax_structure_( r.relax_structure_ ),
    only_interface_( r.only_interface_ ),
    dump_trajectory_( r.dump_trajectory_ ),
    history_poses_( r.history_poses_ )
{}


/// @brief
void
DesSeqOperator2::initialize()
{

    history_poses_.clear();
    history_.clear();

    ScoreFunctionOP sfxn_design = core::scoring::get_score_function();
    this->scorefxn_design(sfxn_design); 

    ScoreFunctionOP sfxn_relax = core::scoring::get_score_function();
    this->scorefxn_relax(sfxn_relax);
            
    using core::pack::task::operation::InitializeFromCommandline;
    using core::pack::task::operation::RestrictToRepacking;
    using protocols::task_operations::LimitAromaChi2Operation;
    using protocols::task_operations::LimitAromaChi2OperationOP;
    using protocols::simple_task_operations::RestrictToInterface;

    // setup task factory for relax        
    tf_relax_->push_back ( TaskOperationOP( new InitializeFromCommandline ));
    tf_relax_->push_back ( TaskOperationOP( new RestrictToRepacking ));
        
    LimitAromaChi2OperationOP lac2 = LimitAromaChi2OperationOP( new LimitAromaChi2Operation );
    lac2->include_trp( true );
    tf_relax_->push_back ( TaskOperationOP( lac2 ) );

    // setup task factory for design
    tf_design_->push_back( TaskOperationOP( new InitializeFromCommandline ) );
    tf_design_->push_back( TaskOperationOP( new LimitAromaChi2Operation   ) );
    if ( only_interface() ) {   
        tf_design_->push_back( TaskOperationOP( new RestrictToInterface ) );
    }

}
    

/// @brief clone
protocols::moves::MoverOP
DesSeqOperator2::clone() const {
    return utility::pointer::make_shared< DesSeqOperator2 >( *this );
}


/// @brief make fresh instance
protocols::moves::MoverOP
DesSeqOperator2::fresh_instance() const {
    return utility::pointer::make_shared< DesSeqOperator2 >();
}

/// @brief scorefxn for design
DesSeqOperator2::ScoreFunctionOP const
DesSeqOperator2::scorefxn_design() const
{
    return scorefxn_design_;
}

/// @brief set scorefxn for design
void
DesSeqOperator2::scorefxn_design( ScoreFunctionOP const sfx )
{
    scorefxn_design_ = sfx->clone();
}

/// @brief scorefxn for design
DesSeqOperator2::ScoreFunctionOP const
DesSeqOperator2::scorefxn_relax() const
{
    return scorefxn_relax_;
}

/// @brief set scorefxn for design
void
DesSeqOperator2::scorefxn_relax( ScoreFunctionOP const sfx )
{
    scorefxn_relax_ = sfx->clone();
}

/// @brief add pose into history pose vector
void
DesSeqOperator2::add_history_pose( PoseOP const pose )
{
    history_poses_.push_back( pose );
}

/// @brief
void
DesSeqOperator2::set_resfile_ctrls(
    Pose const & pose,
    String const & resfile )
{
    VecDesignCtrl resfile_ctrls;
    ListAAs allowed_aas;

    TaskFactoryOP tf = TaskFactoryOP( new TaskFactory() );
    PackerTaskOP ptask = tf->create_packer_task( pose );
    core::pack::task::parse_resfile( pose, *ptask, resfile );

    resfile_ctrls.resize( pose.total_residue(), fldsgn::deseq::vanilla );
    allowed_aas.resize( pose.total_residue() );

    for ( Size iaa=1; iaa<=pose.total_residue(); ++iaa ) {
        String command = ptask->residue_task( iaa ).command_string();
        if ( command.find( "NATAA" ) != std::string::npos ) {
            resfile_ctrls[ iaa ] = fldsgn::deseq::nataa;
            allowed_aas[ iaa ].clear();
            allowed_aas[ iaa ].push_back(pose.residue( iaa ).aa());
        } else if ( command.find( "NATRO" ) != std::string::npos ) {
            resfile_ctrls[ iaa ] = fldsgn::deseq::natrot;
            allowed_aas[ iaa ].clear();
            allowed_aas[ iaa ].push_back(pose.residue( iaa ).aa());
        } else if ( command.find( "PIKAA" ) != std::string::npos ) {
            resfile_ctrls[ iaa ] = fldsgn::deseq::force_aa;            
            ResidueTypeCOPs rtypes = ptask->residue_task( iaa ).allowed_residue_types();
            allowed_aas[ iaa ].clear();
            for ( auto const & rtype : rtypes ) {
                allowed_aas[ iaa ].push_back(rtype->aa());
            }
        } else {
            TR << "The command: " << command << " is ignored." << std::endl;
        }
    }

    resfile_ctrls_ = resfile_ctrls;
    allowed_aas_ = allowed_aas;

}


/// @brief
PackerTaskOP
DesSeqOperator2::set_design_ptask (
    PoseOP const pose,
    DesignCtrlLists const & des_ctrl_lists,
    ListAAs const & allowed_aas,
    DesignCtrl const selected_des_ctrl,
    ListAAs const & selected_aas,
    PackerTaskOP ptask )
{
            
    PoseOP asym_pose;
    if ( core::pose::symmetry::is_symmetric( *pose ) ) {
        core::pose::symmetry::extract_asymmetric_unit( *pose, *asym_pose , false );
    } else {
        asym_pose = pose->clone();
    }
    
    for ( Size iaa=1; iaa<=asym_pose->total_residue(); ++iaa ) {
    
        DesignCtrlList des_ctrl_list = des_ctrl_lists[ iaa ];
        DesignCtrlList::iterator i1 = std::find ( des_ctrl_list.begin(), des_ctrl_list.end(), fldsgn::deseq::nataa  );
        DesignCtrlList::iterator i2 = std::find ( des_ctrl_list.begin(), des_ctrl_list.end(), fldsgn::deseq::natrot );
        DesignCtrlList::iterator i3 = std::find ( des_ctrl_list.begin(), des_ctrl_list.end(), fldsgn::deseq::force_aa );
        DesignCtrlList::iterator i4 = std::find ( des_ctrl_list.begin(), des_ctrl_list.end(), selected_des_ctrl );

        if ( i1 == des_ctrl_list.end() && i2 == des_ctrl_list.end() && i3 == des_ctrl_list.end() ) {

            if ( i4 != des_ctrl_list.end() ) {
                ListAA const aas( selected_aas[ iaa ] );                
                if ( !aas.empty() ) {
                    utility::vector1< bool > restrict_aa( num_canonical_aas, false );
                    for ( ListAA::const_iterator it = aas.begin(); it != aas.end(); ++it ) {
                        restrict_aa[ *it ] = true;
                    }
                    ptask->nonconst_residue_task( iaa ).restrict_absent_canonical_aas( restrict_aa );                   
                } else {
                    ptask->nonconst_residue_task( iaa ).restrict_to_repacking();
                }

            } else {
                ptask->nonconst_residue_task( iaa ).restrict_to_repacking();
            }

        } else {

            if ( i1 != des_ctrl_list.end() ) {
                ptask->nonconst_residue_task( iaa ).prevent_repacking();
                continue;
            } else if ( i2 != des_ctrl_list.end() ) {
                ListAA const aas( allowed_aas[ iaa ] );
                utility::vector1< bool > restrict_aa( num_canonical_aas, false );
                for ( ListAA::const_iterator itr=aas.begin(); itr != aas.end(); ++itr ) {
                    restrict_aa[ *itr ] = true;
                }
                ptask->nonconst_residue_task( iaa ).restrict_absent_canonical_aas( restrict_aa );
                continue;
            } else if ( i3 != des_ctrl_list.end() ) {
                ptask->nonconst_residue_task( iaa ).restrict_to_repacking();
            } else {
                runtime_assert_msg( false, "Unknown design control found for residue " + boost::lexical_cast<std::string>(iaa) );
            }

        }
        
    }
        
    return ptask;
    
}

/// @brief
/// @brief General design packer task setup
PackerTaskOP
DesSeqOperator2::set_design_general_ptask( PoseOP const pose )
{
    TaskFactoryOP tf = task_factory_design()->clone();
    PackerTaskOP ptask = tf->create_packer_task( *pose );
    set_design_ptask( pose, des_ctrl_lists(), allowed_aas(), fldsgn::deseq::favaa, selected_aas(), ptask );
    return ptask;
}

/// @brief Removes exposed hydrophobic residues by replacing them with polar residues
PackerTaskOP
DesSeqOperator2::remove_exposed_hydrophobics (
    PoseOP const pose,
    VecDesignCtrl const & resfile_ctrls,
    ListAAs const & allowed_aas,
    DesignCtrlLists & des_ctrl_lists )
{
    // List of polar amino acids to replace exposed hydrophobics with
    ListAA saa;
    saa.push_back( core::chemical::aa_asn );
    saa.push_back( core::chemical::aa_arg );
    saa.push_back( core::chemical::aa_gln );
    saa.push_back( core::chemical::aa_glu );
    saa.push_back( core::chemical::aa_lys );
    saa.push_back( core::chemical::aa_ser );
    saa.push_back( core::chemical::aa_thr );

		// Use the new FindExposedHydrophobics class for more controlled identification

    // Configure the hydrophobic detection settings (optional)
    // exposed_hp.set_hydrophobic_set_mode(FineExposeHydrophobics::MODERATE);
    // exposed_hp.set_threshold_multiplier(0.9); // Can lower thresholds to be more aggressive

    // Find exposed hydrophobic residues
    //utility::vector1< AA > exposed_aa = exposed_hp.find(*pose);
		utility::vector1< AA > exposed_aa;

    // Prepare selected amino acids for each position
    ListAAs selected_aas(pose->total_residue());
    for (Size iaa=1; iaa<=pose->total_residue(); ++iaa) {
        // Skip residues with specific design controls from resfile
        if (resfile_ctrls[iaa] != fldsgn::deseq::vanilla) {
            des_ctrl_lists[iaa].push_back(resfile_ctrls[iaa]);
            continue;
        } 
        
        // If this is an exposed hydrophobic, add it to the design list with polar amino acids
        if (exposed_aa[iaa] != aa_none) {
            selected_aas[iaa] = saa;
            des_ctrl_lists[iaa].push_back(fldsgn::deseq::exhp);
            TR.Debug << "Marking residue " << iaa << " (" << pose->residue(iaa).name1() 
                     << ") as exposed hydrophobic for redesign" << std::endl;
        }
    }                   

    // Create and configure the packer task
    TaskFactoryOP tf = task_factory_design()->clone();
    PackerTaskOP ptask = tf->create_packer_task(*pose);
    set_design_ptask(pose, des_ctrl_lists, allowed_aas, fldsgn::deseq::exhp, selected_aas, ptask);
    
    return ptask;
}

/// @brief
PackerTaskOP
DesSeqOperator2::remove_buried_polars(
    PoseOP const pose,
    VecDesignCtrl const & resfile_ctrls,
    ListAAs const & allowed_aas,
    DesignCtrlLists & des_ctrl_lists )
{
        
    ListAA erk, avilm;
    erk.push_back( core::chemical::aa_glu );
    erk.push_back( core::chemical::aa_arg );
    erk.push_back( core::chemical::aa_lys );
    avilm.push_back( core::chemical::aa_ala );
    avilm.push_back( core::chemical::aa_val );
    avilm.push_back( core::chemical::aa_ile );
    avilm.push_back( core::chemical::aa_leu );
    avilm.push_back( core::chemical::aa_met );

    //FindBuriedUnsatisfiedPolars find_bunsat;
    //find_bunsat.find( pose );
    //utility::vector1< Size > bunsat_res = find_bunsat.residues_bunsathb_sc();
    utility::vector1< Size > bunsat_res( pose->total_residue() );

    // identify surface regions
    //using protocols::fldsgn::topology::calc_sasa_mainchain_w_cb;
    //utility::vector1< Real > rsd_sasa = calc_sasa_mainchain_w_cb( *pose, 3.5 );
    //Real sasa_core ( 10.0 );

    ListAAs selected_aas( pose->total_residue() );
    for( Size ii=1; ii<=bunsat_res.size(); ++ii ) {        
        Size res = bunsat_res[ ii ];

        if ( !pose->residue( res ).is_protein() ) continue;

        if ( resfile_ctrls[ res ] != fldsgn::deseq::vanilla ) {
            des_ctrl_lists[ res ].push_back( resfile_ctrls[ res ] );
            continue;
        } 
       
        // Use default sasa and threshold values for decision
        Real sasa_core = 10.0; // Default threshold value
        Real rsd_sasa = 15.0;  // Assume a default value for testing

        if ( rsd_sasa > sasa_core ) { // for boundary or surface
            selected_aas[ res ] = erk;
        } else { // for core
            selected_aas[ res ] = avilm;
        }
    }
        
    TaskFactoryOP tf = task_factory_design()->clone();
    PackerTaskOP ptask = tf->create_packer_task( *pose );
    set_design_ptask( pose, des_ctrl_lists, allowed_aas, fldsgn::deseq::bpolar, selected_aas, ptask );
    
    return ptask;
}   

// @brief function for running packrotamer mover followed by SymMinimize or FastRelax
void
DesSeqOperator2::pack_and_min(    
    Size const num_iteration,
    PackerTaskOP const design_task,
    MoveMapOP const movemap,
    Pose & pose )
{
        
    // runtime_assert_msg( scorefxn_design_, "Design score function not set!" );
    // runtime_assert_msg( scorefxn_relax_, "Relax score function not set!" );
    // runtime_assert_msg( task_factory_, "TaskFactory not set!" );
    // runtime_assert_msg( movemap_, "MoveMap not set!" );

    bool const is_symmetric = core::pose::symmetry::is_symmetric( pose );
    
    for ( Size ii=1; ii<=num_iteration; ++ii ) {
        TR << "--- Design Cycle " << ii << " ---" << std::endl;
        
        // --- 1. Packing Step ---
        TR << "Applying PackRotamersMover..." << std::endl;
        PackRotamersMoverOP pack( new PackRotamersMover() );
        pack->score_function( scorefxn_design() );
        pack->task( design_task );
        pack->apply( pose );

        Real const score_after_pack = scorefxn_design()->score( pose );
        
        // --- 2. Relaxation Step ---
        Real score_after_relax( 0.0 );
        if ( relax_structure() ) {
            if( is_symmetric ) {
                TR << "Applying SymMinimizeMover..." << std::endl;
                SymMinMoverOP sym_min_mover( new SymMinMover() );
                sym_min_mover->score_function( scorefxn_relax() );                
                sym_min_mover->movemap( movemap->clone() );
                sym_min_mover->apply( pose );

            } else {
                TR << "Applying FastRelax..." << std::endl;
                FastRelaxOP frlx( new FastRelax( scorefxn_relax_ ) );
                frlx->set_task_factory( tf_relax_->clone() );
                frlx->set_movemap( movemap->clone() );
                frlx->apply( pose );
            }
            score_after_relax = scorefxn_relax()->score( pose );
        } else {
            TR << "Skipping relaxation/minimization step." << std::endl;
            score_after_relax = scorefxn_relax()->score( pose );
        }

        // --- History and Output ---
        history_ << "# === Cycle " << ii << " ===" << std::endl;
        history_ << "# Score after packing (Design Sfxn): " << score_after_pack << std::endl;
        history_ << "# Score after relaxation (Relax Sfxn): " << score_after_relax << std::endl;
        history_ << "# Detailed Scores (Design Sfxn):" << std::endl;
        scorefxn_design()->show( history_, pose );
        history_ << "# Detailed Scores (Relax Sfxn):" << std::endl;
        scorefxn_relax()->show( history_, pose );
        history_ << std::endl;
        
        // add pose into history_poses_
        add_history_pose( PoseOP( new Pose( pose ) ) );
        
    }
    
}



// @brief apply
void
DesSeqOperator2::apply( Pose & pose )
{

        
    protocols::jd2::JobOP job_me( protocols::jd2::JobDistributor::get_instance()->current_job() );
    String me( protocols::jd2::JobDistributor::get_instance()->job_outputter()->output_name( job_me ) );
    String job_output_name = "output"; // Default name
    if (job_me) {
        job_output_name = protocols::jd2::JobDistributor::get_instance()->job_outputter()->output_name( job_me );
    } else {
        TR.Warning << "Could not get current job from JobDistributor. Using default output name 'output'." << std::endl;
    }
            
    // Saves a copy of the original pose.
    Pose original_pose( pose );
    // If the pose is not in fullatom representation, switches it to fullatom.
    if ( !pose.is_fullatom() ) {
        TR << "Switching pose to full-atom representation." << std::endl;
        core::util::switch_to_residue_type_set ( pose, core::chemical::FA_STANDARD );
    }
    
    // Generates secondary structure information using DSSP and inserts it into the pose.
    core::scoring::dssp::Dssp dssp( pose );
    dssp.dssp_reduced();
    dssp.insert_ss_into_pose( pose, false );
      
    /* Sequence design started */
    
    Size design_cycles( 3 );
    
    // --- Initialize History ---
    history_.clear();
    history_ << "# ---------------------------------------------------------------- # " << std::endl;
    history_ << "# Protein Design Protocol: DesSeqOperator2" << std::endl;
    history_ << "# Output Job Name: " << job_output_name << std::endl;
    history_ << "# Initial Score (Design Sfxn): " << scorefxn_design()->score(pose) << std::endl;
    history_ << "# Initial Score (Relax Sfxn):  " << scorefxn_relax_->score(pose)   << std::endl;
    history_ << "# Number of design cycles: " << design_cycles << std::endl;
    history_ << "# Relaxation step enabled: " << (relax_structure_ ? "Yes" : "No") << std::endl;
    history_ << "# ---------------------------------------------------------------- # " << std::endl;
    history_ << std::endl;
    
    TR << "# ---------------------------------------------------------------- # " << std::endl;
    TR << "# Starting Protein Design Protocol: DesSeqOperator2" << std::endl;
    TR << "# Output Job Name: " << job_output_name << std::endl;
    TR << "# Initial Score (Design Sfxn): " << scorefxn_design()->score(pose) << std::endl;
    TR << "# Initial Score (Relax Sfxn):  " << scorefxn_relax_->score(pose)   << std::endl;
    TR << "# Number of design cycles: " << design_cycles << std::endl;
    TR << "# Relaxation step enabled: " << (relax_structure_ ? "Yes" : "No") << std::endl;
    TR << "# ---------------------------------------------------------------- # " << std::endl;
    TR << std::endl;
        
    if( core::pose::symmetry::is_symmetric( pose ) ) {
        TR << "# Symmetry mode: num_of_chains = " << pose.num_chains() << std::endl;
        history_ << "# Symmetry mode: num_of_chains = " << pose.num_chains() << std::endl;
    } else {
        TR << "# Asymmetry mode. " << std::endl;
        history_ << "# Asymmetry mode. " << std::endl;
        runtime_assert_msg( false, "Currently, monomer design was not supporetd." );
    }
    
    // --- Run Packing and Relaxation Cycles ---

    // --- 1. Packing Step ---
    // set packer task    
    if ( !resfile().empty() ) {
        set_resfile_ctrls( pose, resfile() );
    }
    

    PackerTaskOP design_task = tf_design_->create_task_and_apply_taskoperations( pose );
    MoveMapOP movemap( new MoveMap() );
    movemap->set_bb(true);
    movemap->set_chi(true);
    pack_and_min( design_cycles, design_task, movemap, pose );

    TR << "Design protocol finished for job: " << job_output_name << std::endl;
            
} // apply

} // namespace design
} // namespace fldsgn
} // namespace protocols

