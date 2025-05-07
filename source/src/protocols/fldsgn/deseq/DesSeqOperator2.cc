// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/design/DesSeqOperator.cc
/// @brief
/// @author Nobuyasu Koga ( nkoga@protein.osaka-u.ac.jp )

// unit headers
#include <protocols/fldsgn/deseq/DesSeqOperator2.hh>

// Project headers
#include <basic/Tracer.hh>

#include <core/conformation/Conformation.hh>
#include <core/conformation/symmetry/SymmetryInfo.hh>
#include <core/id/AtomID_Mask.fwd.hh>
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

#include <protocols/moves/Mover.hh>
#include <protocols/minimization_packing/symmetry/SymPackRotamersMover.hh>
#include <protocols/minimization_packing/symmetry/SymMinMover.hh>

#include <protocols/fldsgn/topology/SS_Info2.hh>
#include <protocols/fldsgn/topology/util.hh>

#include <protocols/simple_filters/PackStatFilter.hh>
#include <protocols/simple_filters/HolesFilter.hh>
#include <protocols/minimization_packing/PackRotamersMover.hh>
#include <protocols/pose_creation/MakePolyXMover.hh>

#include <protocols/relax/FastRelax.hh>

#include <protocols/jd2/Job.hh>
#include <protocols/jd2/JobDistributor.hh>
#include <protocols/jd2/JobOutputter.hh>

// utility headers
#include <boost/lexical_cast.hpp>
#include <utility/exit.hh>
#include <iostream>
#include <fstream>
#include <utility/vector1.hh>


static basic::Tracer TR( "protocols.fldsgn.design.DesSeqOperator2" );

using namespace core;

namespace protocols {
namespace fldsgn {
namespace deseq {


/// @brief constructor
DesSeqOperator2::DesSeqOperator2():
    Mover("DesSeqOperator2"),
    scorefxn_design_( new ScoreFunction ),
    scorefxn_relax_( new ScoreFunction ),
    relax_structure_( true )
{
    initialize();
}

/// @brief copy constructor
DesSeqOperator2::DesSeqOperator2( DesSeqOperator2 const & r ):
    Mover("DesSeqOperator2"),
    scorefxn_design_( r.scorefxn_design_ ),
    scorefxn_relax_( r.scorefxn_relax_ ),
    relax_structure_( r.relax_structure_ ),
    history_poses_( r.history_poses_ )
{}

/// @brief
void
DesSeqOperator2::initialize()
{
    history_poses_.clear();
    history_.clear();
}
    

/// @brief clone
protocols::moves::MoverOP
DesSeqOperator2::clone() const {
    return protocols::moves::MoverOP( new DesSeqOperator2( *this ) );
}


/// @brief make fresh instance
protocols::moves::MoverOP
DesSeqOperator2::fresh_instance() const {
    return protocols::moves::MoverOP( new DesSeqOperator2() );
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

// @brief function for running packrotamer mover followed by SymMinimize or FastRelax
void
DesSeqOperator2::pack_and_min(
    Size const num_iteration,
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
        if( is_symmetric ) {
            TR << "Applying SymPackRotamersMover..." << std::endl;
            SymPackRotamerMoverOP sympack = SymPackRotamerMoverOP( new SymPackRotamerMover() );
            sympack->score_function( scorefxn_design() );
            // sympack->task  ( ptask );
            sympack->apply( pose );
        } else {
            TR << "Applying PackRotamersMover..." << std::endl;
            PackRotamersMoverOP pack( new PackRotamersMover() );
            pack->score_function( scorefxn_design() );
            // pack->task( task );
            pack->apply( pose );
        }
        Real const score_after_pack = scorefxn_design()->score( pose );
        
        // --- 2. Relaxation Step ---
        Real score_after_relax( 0.0 );
        if ( relax_structure() ) {
            if( is_symmetric ) {
                TR << "Applying SymMinimizeMover..." << std::endl;
                SymMinMoverOP sym_min_mover( new SymMinMover() );
                sym_min_mover->score_function( scorefxn_relax() );
                // sym_min_mover->movemap( movemap_->clone() );
                sym_min_mover->apply( pose );

            } else {
                TR << "Applying FastRelax..." << std::endl;
                FastRelaxOP frlx( new FastRelax( scorefxn_relax() ) );
                // frlx->set_movemap( movemap_->clone() ); // MoveMap を設定
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
    pack_and_min( design_cycles, pose );

    TR << "Design protocol finished for job: " << job_output_name << std::endl;
            
} // apply

} // namespace design
} // namespace fldsgn
} // namespace protocols

