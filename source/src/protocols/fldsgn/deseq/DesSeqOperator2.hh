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

// project headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/pack/task/TaskFactory.fwd.hh>
#include <core/pack/task/operation/TaskOperation.fwd.hh>
#include <core/pack/task/PackerTask.fwd.hh>

#include <core/scoring/ScoreType.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <protocols/fldsgn/topology/SS_Info2.fwd.hh>

#include <protocols/minimization_packing/PackRotamersMover.fwd.hh>
#include <protocols/relax/FastRelax.fwd.hh>

#include <protocols/minimization_packing/symmetry/SymPackRotamersMover.fwd.hh>
#include <protocols/minimization_packing/symmetry/SymMinMover.fwd.hh>


#include <protocols/moves/Mover.hh>

#include <utility/vector1.hh>
#include <sstream>

namespace protocols {
namespace fldsgn {
namespace deseq {

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
    
    typedef protocols::minimization_packing::symmetry::SymPackRotamersMover SymPackRotamerMover;
    typedef protocols::minimization_packing::symmetry::SymPackRotamersMoverOP SymPackRotamerMoverOP;
    typedef protocols::minimization_packing::symmetry::SymMinMover SymMinMover;
    typedef protocols::minimization_packing::symmetry::SymMinMoverOP SymMinMoverOP;
    

public:

    
    /// @brief
    DesSeqOperator2();
    
    /// @brief
    DesSeqOperator2( DesSeqOperator2 const & src );

    /// @brief
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
    
    /// @brief do relax/minimize structure after design
    bool
    relax_structure() const { return relax_structure_; }
    
    /// @brief
    void
    relax_structure( bool const b ) { relax_structure_ = b; }
    
    
    
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
    
    
    void pack_and_min( Size const num_iteration, Pose & pose );
    
    void add_history_pose( PoseOP const pose );
    
                        
private:
    
    
    /// @brief scorefxn for design
    ScoreFunctionOP scorefxn_design_;
    
    /// @brief scorefxn for design
    ScoreFunctionOP scorefxn_relax_;

    /// @brief scorefxn for design
    bool relax_structure_;
        
    /// @brief poses for storgae
    utility::vector1< PoseOP > history_poses_;
    
    /// @brief storage for job running history
    std::ostringstream history_;
            
};


} // namespace design
} // namespace fldsgn
} // namespace protocol

#endif
