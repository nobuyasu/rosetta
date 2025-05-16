// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/design/FindExposedHydrophobics.hh
/// @brief Class for identifying hydrophobic residues that are exposed to solvent
/// @details This class helps identify hydrophobic residues (V, I, L, F, M, W, Y) that
///          have solvent accessible surface areas (SASA) above amino-acid specific thresholds.
///          Such residues can be problematic for protein stability and are often targets
///          for redesign in protein engineering efforts.
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

#include <utility/vector1.hh>
#include <utility/pointer/owning_ptr.hh>

#include <map>
#include <list>
#include <iterator>

namespace protocols {
namespace fldsgn {
namespace deseq {


/// @brief Class for identifying exposed hydrophobic residues in protein structures
/// 
/// This class detects amino acid residues that:
/// 1. Are hydrophobic in nature (V, I, L, F, M, W, Y)
/// 2. Have solvent accessible surface areas (SASA) that exceed amino-acid specific thresholds
///
/// These exposed hydrophobic residues are often targets for redesign in protein engineering
/// to improve stability and solubility.
class FindExposedHydrophobics
{
public:
    // Type definitions for convenience
    typedef core::Size Size;
    typedef core::Real Real;
    typedef std::string String;        
    typedef core::pose::Pose Pose;
        
public:
    /// @brief Default constructor
    /// Initializes the list of hydrophobic amino acids and their SASA thresholds
    FindExposedHydrophobics ();
    
    /// @brief Copy constructor
    /// @param src Source object to copy from
    FindExposedHydrophobics ( FindExposedHydrophobics const & src );
    
    /// @brief Destructor
    ~FindExposedHydrophobics ();
    
    /// @brief Create a new copy of this object
    /// @return Owning pointer to the new copy
    FindExposedHydrophobicsOP clone ();
    
public:
    /// @brief Output operator for debugging
    /// @param out Output stream
    /// @param s FindExposedHydrophobics object to output
    /// @return Modified output stream
    friend std::ostream & operator<<(std::ostream & out, const FindExposedHydrophobics & s );
    
public:
    /// @brief Find exposed hydrophobic residues and append them to the provided list
    /// @param pose The protein structure to analyze
    /// @param exposed_aas Output parameter: a vector of lists where each position corresponds to a residue in the pose.
    ///                    Hydrophobic residues that exceed their SASA threshold are appended to the list at their position.
    void
    find_and_append_exposed_aas ( Pose const & pose, utility::vector1< std::list<core::chemical::AA > > & exposed_aas );
    
    /// @brief Find exposed hydrophobic residues in a protein structure
    /// @param pose The protein structure to analyze
    /// @return A vector where each position corresponds to a residue in the pose.
    ///         Positions with hydrophobic residues that exceed their SASA threshold
    ///         contain the amino acid type; other positions contain aa_none.
    utility::vector1< core::chemical::AA >
    find ( Pose const & pose );

        
private:
    /// @brief String containing the single-letter codes of hydrophobic amino acids (VILFMWY)
    /// Used to quickly check if a residue is hydrophobic
    String hydrophobic_aas_;
    
    /// @brief Map of SASA thresholds for each hydrophobic amino acid type
    /// Residues with SASA values greater than these thresholds are considered "exposed"
    /// - Val: 30.0
    /// - Ile: 35.0  
    /// - Leu: 35.0
    /// - Phe: 40.0
    /// - Met: 40.0
    /// - Tyr: 40.0
    /// - Trp: 70.0
    std::map< core::chemical::AA, Real > threshold_sasa_aa_;
        
};

} // namespace deseq
} // namespace fldsgn
} // namespace protocols

#endif
