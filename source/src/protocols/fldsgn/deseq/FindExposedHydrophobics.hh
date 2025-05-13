// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/deseq/FindExposedHydrophobics.hh
/// @brief
/// @author Nobuyasu Koga ( nkoga@ims.ac.jp )

#ifndef INCLUDED_protocols_fldsgn_deseq_FindExposedHydrophobics_hh
#define INCLUDED_protocols_fldsgn_deseq_FindExposedHydrophobics_hh

// unit headers
#include <protocols/fldsgn/deseq/FindExposedHydrophobics.fwd.hh>

// project headers
#include <core/chemical/AA.hh>
#include <core/chemical/ResidueType.fwd.hh>
#include <core/chemical/ResidueTypeSet.fwd.hh>
#include <core/pack/task/operation/TaskOperation.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>
#include <core/scoring/ScoreFunction.fwd.hh>

#include <utility/vector1.hh>
#include <utility/VirtualBase.hh>

#include <map>
#include <list>
#include <iterator>

namespace protocols {
namespace fldsgn {
namespace deseq {


class FindExposedHydrophobics : public utility::VirtualBase
{
public:

    
    typedef core::Size Size;
    typedef core::Real Real;
    typedef std::string String;        
    typedef core::pose::Pose Pose;
        

public:


    /// @brief default constructor
    FindExposedHydrophobics ();

    
    /// @brief copy constructor
    FindExposedHydrophobics ( FindExposedHydrophobics const & src );

    
    /// @brief destructor
    ~FindExposedHydrophobics ();

    
    /// @brief make clone
    FindExposedHydrophobicsOP clone ();
    

public:
    
    
    /// @brief IO Operator
    friend std::ostream & operator<<(std::ostream & out, const FindExposedHydrophobics & s );

    
public:


    /// @brief find exposed hydorphobic resiudes and add to the second input vector
    void
    find_and_append_exposed_aas ( Pose const & pose, utility::vector1< std::list<core::chemical::AA > > & exposed_aas );
    
    /// @brief find exposed hydorphobic resiudes
    utility::vector1< core::chemical::AA >
    find ( Pose const & pose );

    void find_exposed_hydrophobics(
        core::pose::Pose const & pose,
        core::scoring::ScoreFunctionCOP scorefxn,
        utility::vector1< bool > & exposed_hydrophobics
    );

        
private:

    
    /// @brief list for hydrophobic amino acids
    String hydrophobic_aas_;
    
    /// @brief threshold to determine residues that exposed to solvent
    std::map< core::chemical::AA, Real > threshold_sasa_aa_;
        
};

} // namespace deseq
} // namespace fldsgn
} // namespace protocols

#endif
