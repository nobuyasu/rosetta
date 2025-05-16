// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/deseq/FindExposedHydrophobics.cc
/// @brief Identifies exposed hydrophobic residues in protein structures
/// @author Nobuyasu Koga ( nkoga@ims.ac.jp )

// unit headers
#include <protocols/fldsgn/deseq/FindExposedHydrophobics.hh>
#include <protocols/fldsgn/topology/util.hh>

// core headers
#include <core/types.hh>
#include <core/chemical/ResidueType.hh>
#include <core/chemical/ResidueTypeSet.hh>
#include <core/chemical/AA.hh>
#include <core/chemical/ResidueType.hh>
#include <core/conformation/Conformation.hh>
#include <core/conformation/Residue.hh>
#include <core/pack/task/PackerTask.hh>
#include <core/pack/task/ResidueLevelTask.hh>
#include <core/pack/task/operation/TaskOperation.hh>
#include <core/pose/Pose.hh>

// Project headers
#include <basic/Tracer.hh>

// utility headers
#include <utility/exit.hh>
#include <iostream>
#include <utility/vector1.hh>

#include <boost/assign/list_inserter.hpp>
#include <boost/lexical_cast.hpp>
#include <ObjexxFCL/format.hh>

#include <list>
#include <map>
#include <algorithm>

static basic::Tracer TR( "protocols.fldsgn.deseq.FindExposedHydrophobics" );

using namespace core;

namespace protocols {
namespace fldsgn {
namespace deseq {





/// @brief default constructor
FindExposedHydrophobics::FindExposedHydrophobics():
    hydrophobic_aas_( "VILFMWY" )
{
        
    boost::assign::insert( threshold_sasa_aa_ )
    ( core::chemical::aa_val, 30.0 )
    ( core::chemical::aa_ile, 35.0 )
    ( core::chemical::aa_leu, 35.0 )
    ( core::chemical::aa_phe, 40.0 )
    ( core::chemical::aa_met, 40.0 )
    ( core::chemical::aa_tyr, 40.0 )
    ( core::chemical::aa_trp, 70.0 );
}


/// @Brief copy constructor
FindExposedHydrophobics::FindExposedHydrophobics( FindExposedHydrophobics const & r ):
    hydrophobic_aas_( r.hydrophobic_aas_ ),
    threshold_sasa_aa_( r.threshold_sasa_aa_ )
{}


/// @brief default destructor
FindExposedHydrophobics::~FindExposedHydrophobics() {}


/// @brief make clone
FindExposedHydrophobicsOP 
FindExposedHydrophobics::clone() 
{
    return FindExposedHydrophobicsOP( new FindExposedHydrophobics(*this) );
}


/// @brief IO Operator
std::ostream & operator<<(std::ostream & out, const FindExposedHydrophobics & s )
{
    out << s.hydrophobic_aas_;
    return out;
}

/// @brief Find exposed hydrophobic residues in a protein structure
/// @param pose The protein structure to analyze
/// @return A vector where each position corresponds to a residue in the pose.
///         Positions with hydrophobic residues that exceed their SASA threshold
///         contain the amino acid type; other positions contain aa_none.
utility::vector1< core::chemical::AA >
FindExposedHydrophobics::find ( Pose const & pose )
{
    // Calculate SASA values for all sidechains in the pose
    utility::vector1< Real > sc_sasas = protocols::fldsgn::topology::calc_sasa_sidechains_using_fullatoms ( pose );
    
    // Initialize vector with aa_none for all residues
    utility::vector1< core::chemical::AA > exposed_aas( pose.total_residue(), core::chemical::aa_none );
    
    // Counter for statistics
    Size num_exposed_hydrophobics = 0;
    
    // Iterate through all residues in the pose
    for ( Size iaa=1; iaa<=pose.total_residue(); ++iaa ) {
        // Skip non-protein residues
        if ( ! pose.residue( iaa ).is_protein() ) continue;
        
        // Check if this residue is one of the hydrophobic amino acids we're tracking
        if( hydrophobic_aas_.find( pose.residue( iaa ).name1() ) != std::string::npos ) {
            // Get the SASA threshold for this amino acid type
            Real threshold = threshold_sasa_aa_[ pose.residue( iaa ).aa() ];
            
            // Check if the SASA exceeds the threshold for this amino acid type
            if( sc_sasas[ iaa ] > threshold ) {
                // Mark as exposed hydrophobic
                exposed_aas[ iaa ] = pose.residue( iaa ).aa();
                num_exposed_hydrophobics++;
                
                // Log the exposed hydrophobic for debugging
                TR.Debug << "Residue " << iaa << " (" << pose.residue( iaa ).name1() 
                         << ") has SASA " << sc_sasas[ iaa ] << " > threshold " 
                         << threshold << std::endl;
            }
        }
    }
    
    TR.Info << "Found " << num_exposed_hydrophobics << " exposed hydrophobic residues" << std::endl;
    return exposed_aas;
}

/// @brief Find exposed hydrophobic residues and append them to the provided list
/// @param pose The protein structure to analyze
/// @param exposed_aas Output parameter: a vector of lists where each position corresponds to a residue in the pose.
///                    Hydrophobic residues that exceed their SASA threshold are appended to the list at their position
void
FindExposedHydrophobics::find_and_append_exposed_aas ( Pose const & pose, utility::vector1< std::list< core::chemical::AA > > & exposed_aas )
{
    // First find all exposed hydrophobic residues
    utility::vector1< core::chemical::AA > aas ( find( pose ) );
    
    // Make sure the output vector is the right size
    if (exposed_aas.size() < pose.total_residue()) {
        TR.Warning << "Resizing exposed_aas vector from " << exposed_aas.size() 
                   << " to " << pose.total_residue() << std::endl;
        exposed_aas.resize(pose.total_residue());
    }
    
    // Iterate through all residues in the pose
    for ( Size iaa=1; iaa<=pose.total_residue(); ++iaa ) {
        // Skip non-protein residues
        if ( !pose.residue( iaa ).is_protein() ) continue;
        
        // Check if this residue was identified as an exposed hydrophobic
        if ( aas[ iaa ] != core::chemical::aa_none ) {
            core::chemical::AA aa = aas[ iaa ];
            
            // Check if this amino acid is already in the list
            std::list< core::chemical::AA >::iterator ite = 
                std::find( exposed_aas[ iaa ].begin(), exposed_aas[ iaa ].end(), aa );
            
            // Add it to the list if not already present
            if( ite == exposed_aas[ iaa ].end() ) {
                exposed_aas[ iaa ].push_back( aa );
                TR.Debug << "Appending " << aa << " to exposed_aas at position " << iaa << std::endl;
            }
        }
    }
}



} // namespace deseq
} // namespace fldsgn
} // namespace protocols

