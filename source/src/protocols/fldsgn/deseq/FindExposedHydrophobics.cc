// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// Copyright in the Rosetta software belongs to the developers and their institutions.
// For more information, see www.rosettacommons.org.

/// @file ./src/protocols/fldsgn/deseq/FindExposedHydrophobics.cc
/// @brief
/// @author Nobuyasu Koga ( nkoga@ims.ac.jp )

// unit headers
#include <protocols/fldsgn/deseq/FindExposedHydrophobics.hh>
#include <protocols/fldsgn/topology/util.hh>

#include <core/types.hh>
#include <core/chemical/AtomType.hh>
#include <core/chemical/ResidueType.hh>
#include <core/chemical/ResidueTypeSet.hh>
#include <core/chemical/AA.hh>
#include <core/chemical/ResidueType.hh>
#include <core/conformation/Conformation.hh>
#include <core/conformation/Residue.hh>
#include <core/pack/task/PackerTask.hh>
#include <core/pack/task/operation/TaskOperation.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/sasa/SasaCalc.hh>

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

static basic::Tracer TR( "protocols.fldsn.design.FindExposedHydrophobics" );

using namespace core;

namespace protocols {
namespace fldsgn {
namespace deseq {

/// @brief default constructor
FindExposedHydrophobics::FindExposedHydrophobics():
    hydrophobic_aas_( "VILFMWY" )
{
    /*
    boost::assign::insert( threshold_sasa_aa_ )
    ( core::chemical::aa_val, 25.0 )
    ( core::chemical::aa_ile, 30.0 )
    ( core::chemical::aa_leu, 30.0 )
    ( core::chemical::aa_phe, 35.0 )
    ( core::chemical::aa_met, 35.0 )
    ( core::chemical::aa_tyr, 35.0 )
    ( core::chemical::aa_trp, 70.0 );
    */
        
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


/// @brief IO Operator
std::ostream & operator<<(std::ostream & out, const FindExposedHydrophobics & s )
{
    out << s.hydrophobic_aas_;
    return out;
}

/// @brief 
utility::vector1< core::chemical::AA >
FindExposedHydrophobics::find ( Pose const & pose )
{
              
    utility::vector1< Real > sc_sasas = protocols::fldsgn::topology::calc_sasa_sidechains_using_fullatoms ( pose );

    utility::vector1< core::chemical::AA > exposed_aas( pose.total_residue(), core::chemical::aa_none );
    for ( Size iaa=1; iaa<=pose.size(); ++iaa ) {
        
        if ( ! pose.residue( iaa ).is_protein() ) continue;
        if( hydrophobic_aas_.find( pose.residue( iaa ).name1() ) != std::string::npos ) {
            if( sc_sasas[ iaa ] > threshold_sasa_aa_[ pose.residue( iaa ).aa() ] ) {
                exposed_aas[ iaa ] = pose.residue( iaa ).aa();
            }
        }
    }
 
    return exposed_aas;
    
}

/// @brief
void
FindExposedHydrophobics::find_and_append_exposed_aas ( Pose const & pose, utility::vector1< std::list< core::chemical::AA > > & exposed_aas )
{
    
    utility::vector1< core::chemical::AA > aas ( find( pose ) );
    for ( Size iaa=1; iaa<=pose.total_residue(); ++iaa ) {
        
        if ( !pose.residue( iaa ).is_protein() ) continue;
        
        if ( aas[ iaa ] != core::chemical::aa_none ) {
            core::chemical::AA aa = aas[ iaa ];
            std::list< core::chemical::AA >::iterator ite = std::find( exposed_aas[ iaa ].begin(), exposed_aas[ iaa ].end(), aa );
            if( ite == exposed_aas[ iaa ].end() ) {
                exposed_aas[ iaa ].push_back( aa );
            }
        }
        
    } // for iaa
    
}

void
FindExposedHydrophobics::find_exposed_hydrophobics(
    core::pose::Pose const & pose,
    core::scoring::ScoreFunctionCOP scorefxn,
    utility::vector1< bool > & exposed_hydrophobics
) {
    exposed_hydrophobics.resize(pose.size(), false);

    // Calculate SASA
    core::scoring::sasa::SasaCalc sasa_calc;
    utility::vector1< core::Real > residue_sasa;
    sasa_calc.calculate(pose, residue_sasa);

    // Check each residue
    for (core::Size i = 1; i <= pose.size(); ++i) {
        if (!pose.residue(i).is_protein()) continue;

        // Check if residue is exposed
        if (residue_sasa[i] < 20.0) continue;

        // Check if residue is hydrophobic
        bool is_hydrophobic = false;
        for (core::Size j = 1; j <= pose.residue(i).natoms(); ++j) {
            if (pose.residue(i).atom_type(j).is_hydrophobic()) {
                is_hydrophobic = true;
                break;
            }
        }
        if (is_hydrophobic) {
            exposed_hydrophobics[i] = true;
        }
    }
}

} // namespace deseq
} // namespace fldsgn
} // namespace protocols

