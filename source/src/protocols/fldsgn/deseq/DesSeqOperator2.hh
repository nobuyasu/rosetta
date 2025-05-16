// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/deseq/DesSeqOperator.hh
/// @brief
/// @author Nobuyasu Koga ( nkoga@protein.osaka-u.ac.jp )

#ifndef INCLUDED_protocols_fldsgn_deseq_DesSeqOperator2_hh
#define INCLUDED_protocols_fldsgn_deseq_DesSeqOperator2_hh

// unit headers
#include <protocols/fldsgn/deseq/DesSeqOperator2.fwd.hh>
#include <protocols/fldsgn/deseq/FindExposedHydrophobics.hh>
#include <protocols/fldsgn/deseq/FineExposeHydrophobics.hh>

// project headers
#include <core/chemical/AA.hh>
#include <core/chemical/ResidueType.fwd.hh>
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/pack/task/TaskFactory.fwd.hh>
#include <core/pack/task/operation/TaskOperation.fwd.hh>
#include <core/pack/task/PackerTask.fwd.hh>
#include <core/kinematics/MoveMap.fwd.hh>
#include <core/scoring/ScoreType.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <protocols/fldsgn/topology/SS_Info2.fwd.hh>

#include <protocols/minimization_packing/PackRotamersMover.fwd.hh>
#include <protocols/relax/FastRelax.fwd.hh>
#include <protocols/minimization_packing/symmetry/SymMinMover.fwd.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <protocols/moves/Mover.hh>

#include <utility/vector1.hh>
#include <sstream>
#include <list>
#include <string>

#include <utility/tag/Tag.fwd.hh>

namespace protocols {
namespace fldsgn {
namespace deseq {

enum DesignCtrl
{
    vanilla  = 1,
    bpolar   = 2,
    exhp     = 3,
    favaa    = 4,
    force_aa = 5,
    nataa    = 6,
    natrot   = 7,
    num_des_ctrl = natrot        
};

class DesSeqOperator2 : public protocols::moves::Mover
{
public:
    
    typedef core::Size Size;
    typedef core::Real Real;
    typedef std::string String;
    typedef core::Vector Vector;
    typedef utility::vector1< Size > VecSize;
    typedef utility::vector1< Real > VecReal;
            
    typedef core::pose::Pose Pose;
    typedef core::pose::PoseOP PoseOP;

    typedef core::kinematics::MoveMap MoveMap;
    typedef core::kinematics::MoveMapOP MoveMapOP;

    typedef core::scoring::ScoreType ScoreType;
    typedef core::scoring::ScoreFunction ScoreFunction;
    typedef core::scoring::ScoreFunctionOP ScoreFunctionOP;
    
    typedef core::pack::task::TaskFactory TaskFactory;
    typedef core::pack::task::TaskFactoryOP TaskFactoryOP;
    typedef core::pack::task::operation::TaskOperation TaskOperation;
    typedef core::pack::task::operation::TaskOperationOP TaskOperationOP;
    
    typedef core::pack::task::PackerTask PackerTask;
    typedef core::pack::task::PackerTaskOP PackerTaskOP;
    
    typedef protocols::fldsgn::topology::SS_Info2 SS_Info2;
    typedef protocols::fldsgn::topology::SS_Info2_OP SS_Info2_OP;
    typedef protocols::fldsgn::topology::SS_Info2_COP SS_Info2_COP;
        
    typedef protocols::relax::FastRelax FastRelax;
    typedef protocols::relax::FastRelaxOP FastRelaxOP;
    typedef protocols::minimization_packing::PackRotamersMover PackRotamersMover;
    typedef protocols::minimization_packing::PackRotamersMoverOP PackRotamersMoverOP;
    
    typedef protocols::minimization_packing::symmetry::SymMinMover SymMinMover;
    typedef protocols::minimization_packing::symmetry::SymMinMoverOP SymMinMoverOP;
    
    typedef std::list<core::chemical::AA> ListAA;
    typedef utility::vector1< ListAA > ListAAs;

    typedef std::list< DesignCtrl > DesignCtrlList;
    typedef utility::vector1< DesignCtrlList > DesignCtrlLists;
    
    typedef std::list<core::chemical::ResidueTypeCOP> ResidueTypeCOPs;
    typedef utility::vector1< DesignCtrl > VecDesignCtrl;
    typedef utility::vector1< PoseOP > PoseOPs;

public:

    
    /// @brief constructor
    DesSeqOperator2();
    
    /// @brief copy constructor
    DesSeqOperator2( DesSeqOperator2 const & src );

    /// @brief destrctor
    virtual ~DesSeqOperator2(){};

    /// @brief 
    virtual std::string get_name() const { return "DesSeqOperator2"; }
    
    
private:
    
    
    /// @brief initialize
    void
    initialize();
    
    
public:
    
    
    /// @brief scorefxn for design
    ScoreFunctionOP const
    scorefxn_design() const;
    
    /// @brief set scorefxn for design
    void
    scorefxn_design( ScoreFunctionOP const sfx );
    
    /// @brief scorefxn for relax
    ScoreFunctionOP const
    scorefxn_relax() const;
    
    /// @brief set scorefxn for relax
    void
    scorefxn_relax( ScoreFunctionOP const sfx );
    
    /// @brief
    TaskFactoryOP
    task_factory_design() const { return tf_design_; }
    
    /// @brief
    TaskFactoryOP
    task_factory_relax() const { return tf_relax_; }
    
    
    /// @brief do relax/minimize structure after design
    bool
    relax_structure() const { return relax_structure_; }
    
    /// @brief perform relax/minimize structure after design if true
    void
    relax_structure( bool const b ) { relax_structure_ = b; }

    /// @brief set resfile  
    void
    set_resfile(const std::string& resfile) { resfile_ = resfile; }

    /// @brief get resfile
    const
    std::string& resfile() const { return resfile_; }

    /// @brief get resfile controls
    VecDesignCtrl const & resfile_ctrls() const { return resfile_ctrls_; }

    /// @brief get allowed aas
    ListAAs const & allowed_aas() const { return allowed_aas_; }

    /// @brief get flag for design only interface or not
    bool
    only_interface() const { return only_interface_; }
    
    /// @brief set flag for design only interface or not
    void
    only_interface( bool const b ) { only_interface_ = b; }

    /// @brief get flag for dump trajectory
    bool
    dump_trajectory() const { return dump_trajectory_; }
    
    /// @brief set flag for dump trajectory
    void
    dump_trajectory( bool const b ) { dump_trajectory_ = b; }
    
    
public:

    
    virtual protocols::moves::MoverOP clone() const;

    virtual protocols::moves::MoverOP fresh_instance() const;

    virtual void parse_my_tag(
        utility::tag::TagCOP,
        basic::datacache::DataMap & ){};

    
public:


    virtual
    void apply( Pose & pose );
    
    

private:
    
   
    void
    pack_and_min(
                 Size const num_iteration,
                 PackerTaskOP const design_task,
                 MoveMapOP const movemap,
                 Pose & pose );


public:


    void
    set_resfile_ctrls(
        Pose const & pose,
        String const & resfile );        

    PackerTaskOP
    remove_exposed_hydrophobics(
        PoseOP const pose,
        VecDesignCtrl const & resfile_ctrls,
        ListAAs const & allowed_aas,
        DesignCtrlLists & des_ctrl_lists );
        
    /// @brief General design packer task setup
    PackerTaskOP
    set_design_general_ptask( PoseOP const pose );
    
    /// @brief Return design control lists
    DesignCtrlLists const & 
    des_ctrl_lists() const { return des_ctrl_lists_; }
    
    /// @brief Return selected amino acids
    ListAAs const & 
    selected_aas() const { return selected_aas_; }
    
    PackerTaskOP
    remove_buried_polars(
        PoseOP const pose,
        VecDesignCtrl const & resfile_ctrls,
        ListAAs const & allowed_aas,
        DesignCtrlLists & des_ctrl_lists );

    PackerTaskOP
    set_design_ptask (
        PoseOP const pose,
        DesignCtrlLists const & des_ctrl_lists,
        ListAAs const & allowed_aas,        
        DesignCtrl const selected_des_ctrl,
        ListAAs const & selected_aas,
        PackerTaskOP ptask );


    void
    add_history_pose( PoseOP const pose );
    
                        
private:
    
    
    /// @brief scorefxn for design
    ScoreFunctionOP scorefxn_design_;
        
    /// @brief scorefxn for design
    ScoreFunctionOP scorefxn_relax_;

    /// @brief task factory for design
    TaskFactoryOP tf_design_;

    /// @brief task factory for relax
    TaskFactoryOP tf_relax_;

    /// @brief movemap for relax
    MoveMapOP movemap_;

    /// @brief resfile
    std::string resfile_;

    /// @brief resfile controls
    VecDesignCtrl resfile_ctrls_;

    /// @brief allowed aas
    ListAAs allowed_aas_;

    /// @brief do relax/minimize structure after design
    bool relax_structure_;            

    /// @brief flag for design only interface
    bool only_interface_;

    /// @brief flag for dump trajectory
    bool dump_trajectory_;

    /// @brief poses for storgae
    PoseOPs history_poses_;
        
    /// @brief storage for job running history
    std::ostringstream history_;
    
    /// @brief control lists for design
    DesignCtrlLists des_ctrl_lists_;
    
    /// @brief selected amino acids
    ListAAs selected_aas_;
};


} // namespace design
} // namespace fldsgn
} // namespace protocol

#endif
