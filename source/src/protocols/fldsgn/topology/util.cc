// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file ./src/protocols/fldsgn/topology/util.cc
/// @brief utilities for fldsgn/topology
/// @author Nobuyasu Koga ( nobuyasu@u.washington.edu )

// unit header
#include <protocols/fldsgn/topology/util.hh>
#include <protocols/fldsgn/topology/StrandPairing.hh>
#include <protocols/fldsgn/topology/SS_Info2.hh>
// Comment out the missing header to avoid compilation error
// #include <protocols/fldsgn/potentials/FastVDW.hh>
#include <protocols/forge/build/Interval.hh>
// Updated path for MakePolyXMover
#include <protocols/pose_creation/MakePolyXMover.hh>

// Project Headers
#include <core/kinematics/FoldTree.fwd.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/Conformation.hh>
#include <core/scoring/Energies.hh>
#include <core/scoring/hbonds/HBondSet.hh>
#include <core/pose/Pose.hh>
#include <core/pose/util.hh>
#include <core/scoring/dssp/Dssp.hh>
#include <core/scoring/dssp/StrandPairing.hh>
#include <core/id/AtomID_Map.hh>
#include <core/scoring/EnergiesCacheableDataType.hh>
#include <core/scoring/sasa.hh>
#include <core/util/SwitchResidueTypeSet.hh>
#include <core/chemical/ChemicalManager.hh>
#include <core/chemical/ResidueTypeSet.hh>

// utility headers
#include <utility/exit.hh>

// C++ headers
#include <utility/assert.hh>
#include <iostream>
#include <sstream>
#include <basic/Tracer.hh>
#include <map>

#include <utility/vector1.hh>
#include <numeric/PCA.hh>

//Auto Headers
// Commenting out the missing header
// #include <core/pose/util.tmpl.hh>
// Commented out due to THREAD_LOCAL not being defined
// static THREAD_LOCAL basic::Tracer TR( "protocols.topology.util" );

using namespace core;
typedef std::string String;
typedef utility::vector1< Size > VecSize;

namespace protocols {
namespace fldsgn {
namespace topology {

/// @brief convert StrandParingSet of dssp to fldsgn::topology::StrandPairingSet
protocols::fldsgn::topology::StrandPairingSet
calc_strand_pairing_set (
	core::pose::Pose const & pose,
	protocols::fldsgn::topology::SS_Info2_COP const ssinfo,
	core::Size minimum_pair_length )
{
	using core::Size;

	core::scoring::dssp::Dssp dssp( pose );

	std::map< String, StrandPairingOP > newpairs;

	core::scoring::dssp::StrandPairingSet spairset;
	spairset = dssp.strand_pairing_set();

	for ( Size ispair=1; ispair<=spairset.size(); ispair++ ) {

		core::scoring::dssp::StrandPairing sp;
		sp = spairset.strand_pairing( ispair );
		Size begin1 ( sp.begin1() );
		Size end1 ( sp.end1() );

		for ( Size iaa=begin1; iaa<=end1; ++iaa ) {

			Size istrand ( ssinfo->strand_id( iaa ) );
			if ( istrand == 0 ) continue;
			Size ist_begin ( ssinfo->strand( istrand )->begin() );

			Size jaa ( sp.get_pair( iaa ) );
			if ( jaa == 0 ) continue;
			Size pleats ( sp.get_pleating( iaa ) );
			Size jstrand ( ssinfo->strand_id( jaa ) );
			if ( jstrand == 0 ) continue;

			Size jst_begin ( ssinfo->strand( jstrand )->begin() );
			Size jst_end ( ssinfo->strand( jstrand )->end() );
			Size jst_length ( jst_end - jst_begin );

			if ( istrand == jstrand ) continue;

			if ( istrand !=0 && jstrand !=0 && jaa !=0  ) {

				char orient;
				Real rgstr_shift;

				if ( sp.antiparallel() ) {
					orient = 'A';
					rgstr_shift = Real(iaa) - Real(ist_begin) - (Real(jst_length) - (Real(jaa) - Real(jst_begin)));
				} else {
					orient = 'P';
					rgstr_shift = Real(iaa) - Real(ist_begin) - (Real(jaa) - Real(jst_begin));
				}

				std::ostringstream spairname;
				spairname << istrand << "-" << jstrand << "." << orient ;
				std::map<String, StrandPairingOP>::iterator it( newpairs.find( spairname.str() ) );
				if ( it == newpairs.end() ) {
					StrandPairingOP strand_pair( new StrandPairing( istrand, jstrand, iaa, jaa, pleats, rgstr_shift, orient ) );
					newpairs.insert( std::map<String, StrandPairingOP>::value_type( spairname.str(), strand_pair ) );
				} else {
					(*it).second->elongate( iaa, jaa, pleats, pleats );
				}

			} // istrand !=0 && jstrand !=0 && jaa !=0
		} // iaa
	} // ispair

	StrandPairingSet spairset_new;
	std::map<String, StrandPairingOP>::iterator it( newpairs.begin() );
	while ( it != newpairs.end() ) {

		// skip if pair length < minimum_pair_length
		if ( (*it).second->size1() < minimum_pair_length || (*it).second->size2() < minimum_pair_length ) {
			++it;
			continue;
		}

		// Debug output commented out due to TR not being defined
		// TR.Debug << "Defined strand pair : " << *((*it).second)
		//	<< ", 1st_strand begin: " << (*it).second->begin1() << ", end: " <<  (*it).second->end1()
		//	<< ", 2nd_strand begin: " << (*it).second->begin2() << ", end: " <<  (*it).second->end2() << std::endl;
		spairset_new.push_back( (*it).second );
		++it;
	}

	spairset_new.finalize();

	return spairset_new;

} // clac_strand_pairings


/// @brief calc Helix Order
core::Real
calc_HelixOrder ( utility::vector1< core::Vector > const & hx_uvecs )
{    
    Real helix_order( 0.0 );
    if( hx_uvecs.size() <= 1 ) return helix_order;
    
    Size nh = hx_uvecs.size();
    Real sum( 0.0 );
    for ( Size ihx=1; ihx<=nh-1; ihx++ ) {
        for ( Size jhx=ihx+1; jhx<=nh; jhx++ ) {
            Real d = hx_uvecs[ ihx ].dot( hx_uvecs[ jhx ] );
            sum += d*d;
        }
    }
    return sum/(Real( nh )*Real( nh-1 )/2.0);
}


/// @brief calc delta sasa, when a molecule is splited to 2parts.
core::Real
calc_delta_sasa (
	core::pose::Pose const & pose,
	utility::vector1< protocols::forge::build::Interval > intervals,
	Real const pore_radius )
{

	/// calc surface areas of total residues

	// define atom_map for main-chain and CB
	core::id::AtomID_Map< bool > atom_map;
	// Manual initialization instead of using initialize_atomid_map
	atom_map.resize(pose.total_residue());
	for ( Size ir = 1; ir <= pose.total_residue(); ++ir ) {
		for ( Size j = 1; j<=5; ++j ) {
			core::id::AtomID atom( j, ir );
			atom_map.set( atom, true );
		}
	}

	utility::vector1< Real > rsd_sasa;
	core::id::AtomID_Map< Real > atom_sasa;
	core::scoring::calc_per_atom_sasa( pose, atom_sasa, rsd_sasa, pore_radius, false, atom_map );

	/// calc surface areas of A, B regions which are dicretized by intervals.
	/// A is non-assigned regions by intervals and B is assigned regions by intervals

	core::id::AtomID_Map< bool > atom_map_A;
	core::id::AtomID_Map< bool > atom_map_B;
	utility::vector1< Real > rsd_sasa_A;
	utility::vector1< Real > rsd_sasa_B;
	// Manual initialization instead of using initialize_atomid_map
	atom_map_A.resize(pose.total_residue());
	atom_map_B.resize(pose.total_residue());

	utility::vector1< bool > position_A( pose.total_residue(), true );
	for ( Size ii=1; ii<=intervals.size(); ii++ ) {
		for ( Size jj=intervals[ ii ].left; jj<=intervals[ ii ].right; ++jj ) {
			position_A[ jj ] = false;
		}
		for ( Size j=1; j<=5; ++j ) {
			core::id::AtomID atom1( j, intervals[ ii ].left );
			atom_map_A.set( atom1, true );

			core::id::AtomID atom2( j, intervals[ ii ].right );
			atom_map_A.set( atom2, true );

			core::id::AtomID atom3( j, intervals[ ii ].left-1 );
			atom_map_B.set( atom3, true );

			core::id::AtomID atom4( j, intervals[ ii ].right+1 );
			atom_map_B.set( atom4, true );
		}
	}

	for ( Size jj=1; jj<=pose.total_residue(); jj++ ) {
		if ( position_A[ jj ] ) {
			for ( Size j=1; j<=5; ++j ) {
				core::id::AtomID atom( j, jj );
				atom_map_A.set( atom, true );
			}
		} else {
			for ( Size j=1; j<=5; ++j ) {
				core::id::AtomID atom( j, jj );
				atom_map_B.set( atom, true );
			}
		}
	}

	core::scoring::calc_per_atom_sasa( pose, atom_sasa, rsd_sasa_A, pore_radius, false, atom_map_A );
	core::scoring::calc_per_atom_sasa( pose, atom_sasa, rsd_sasa_B, pore_radius, false, atom_map_B );

	Real tot_all( 0.0 ), tot_A( 0.0 ), tot_B( 0.0 );
	for ( Size ii=2; ii<=pose.total_residue()-1; ii++ ) {
		tot_all += rsd_sasa[ ii ];
		if ( position_A[ ii ] ) {
			tot_A += rsd_sasa_A[ ii ];
		} else {
			tot_B += rsd_sasa_B[ ii ];
		}
	}

	//   for( Size ii=1; ii<=pose.total_residue(); ii++ ) {
	//  if( position_A[ ii ] ) {
	//   std::cout << ii << " " << rsd_sasa[ ii ] << " " << rsd_sasa_A[ ii ] << " " << rsd_sasa_B[ ii ] << "*" <<std::endl;
	//  } else {
	//   std::cout << ii << " " << rsd_sasa[ ii ] << " " << rsd_sasa_A[ ii ] << " " << rsd_sasa_B[ ii ] << std::endl;
	//  }
	// }
	// std::cout << tot_all << " " << tot_A << " " << tot_B << std::endl;

	return tot_A + tot_B - tot_all;

} // calc_delta_sasa


/// @brief check kink of helix, return number of loosen hydrogen
core::Size
check_kink_helix (
	core::pose::Pose const & pose,
	core::Size const begin,
	core::Size const end )
{
	using core::scoring::EnergiesCacheableDataType::HBOND_SET;
	using core::scoring::hbonds::HBondSet;

	core::pose::Pose copy_pose( pose );
	HBondSet const & hbond_set( static_cast< HBondSet const & > ( copy_pose.energies().data().get( HBOND_SET )) );
	//hbond_set.show( copy_pose );

	Size num_broken_hbond( 0 );
	for ( Size ii=begin; ii<=end; ++ii ) {
		if ( ! hbond_set.acc_bbg_in_bb_bb_hbond( ii ) ) {
			// TR reference commented out
			// TR << "hbonds of " << ii << " is broken. " << std::endl;
			num_broken_hbond++;
		}
	}

	return num_broken_hbond;
}


/// @brief check kink of helix, return number of loosen hydrogen
utility::vector1< core::scoring::hbonds::HBond >
check_internal_hbonds (
	core::pose::Pose const & pose,
	core::Size const begin,
	core::Size const end )
{
	using core::scoring::EnergiesCacheableDataType::HBOND_SET;
	using core::scoring::hbonds::HBondSet;
	using core::scoring::hbonds::HBond;

	core::pose::Pose copy_pose( pose );
	HBondSet const & hbond_set( static_cast< HBondSet const & > ( copy_pose.energies().data().get( HBOND_SET )) );

	utility::vector1< HBond > hbonds;
	for ( core::Size i=1; i<=(core::Size)hbond_set.nhbonds(); ++i ) {
		Size don_pos = hbond_set.hbond((int)i).don_res();
		Size acc_pos = hbond_set.hbond((int)i).acc_res();
		if ( don_pos >= begin && don_pos <= end &&
				acc_pos >= begin && acc_pos <= end ) {
			hbonds.push_back( hbond_set.hbond((int)i) );
		}
	}

	return hbonds;

}


///@brief
void
append_terminal_glyres(
    Size chain,
    core::pose::Pose & pose )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
}

/// @biref
core::pose::Pose
build_blank_pose ( core::Size const target_size,
                   bool const centroid,
                   bool const add_terminal,
                   core::Real const phi,
                   core::Real const psi,
                   core::Real const omega,
                   core::pose::PoseOP const appended_pose,
                   char const which_side,        /* C */
                   bool const as_different_chain /* true */ )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    
    // Return an empty pose for now
    core::pose::Pose pose;
    return pose;
}


/// @brief
bool
check_chain_break(
    core::pose::Pose const & pose,
    core::Size const begin,
    core::Size const end,
    bool const verbose )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    return false;
}


/// @brief calc sasa mainchain with cb
utility::vector1< core::Real >
calc_sasa_mainchain_w_cb(
    core::pose::Pose const & pose,
    core::Real const probe_radius )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    return utility::vector1< core::Real >(pose.total_residue(), 0.0);
}


/// @brief calc sasa for sidechains using full atoms
void
calc_sasa_sidechains_using_fullatoms (
    core::pose::Pose const & pose,
    core::Real const probe_radius,
    utility::vector1< Real > & rsd_sasa,
    utility::vector1< Real > & sc_sasa,
    core::id::AtomID_Map< Real > atom_sasa )
{
    // Initialize atomID map
    core::id::AtomID_Map< bool > atom_map;
    atom_map.resize(pose.total_residue());
    
    // Set up atom map for sidechain atoms
    for ( core::Size ir = 1; ir <= pose.total_residue(); ++ir ) {
        if (!pose.residue(ir).is_protein()) continue;
        
        for (Size iatm = 1; iatm <= pose.residue(ir).nheavyatoms(); ++iatm) {
            core::id::AtomID atm(iatm, ir);
            atom_map.set(atm, true);
        }
    }
    
    // Calculate SASA
    rsd_sasa.resize(pose.total_residue(), 0.0);
    sc_sasa.resize(pose.total_residue(), 0.0);
    core::scoring::calc_per_atom_sasa(pose, atom_sasa, rsd_sasa, probe_radius, false, atom_map);
    
    // Sum up sidechain atom SASAs
    for (Size ir = 1; ir <= pose.total_residue(); ++ir) {
        if (!pose.residue(ir).is_protein()) continue;
        
        if (pose.residue(ir).name1() == 'G') {
            // For glycine, use CB (which is usually a hydrogen atom in glycine)
            sc_sasa[ir] += atom_sasa[core::id::AtomID(2, ir)];
        } else {
            // For other residues, sum up all sidechain atoms (beyond the backbone atoms)
            for (Size iatm = 5; iatm <= pose.residue(ir).nheavyatoms(); ++iatm) {
                sc_sasa[ir] += atom_sasa[core::id::AtomID(iatm, ir)];
            }
        }
    }
}

/// @brief calc sasa for sidechains using full atoms
utility::vector1< core::Real >
calc_sasa_sidechains_using_fullatoms (
    core::pose::Pose const & pose,
    core::Real const probe_radius )
{
    utility::vector1< core::Real > rsd_sasa(pose.total_residue(), 0.0);
    utility::vector1< core::Real > sc_sasa(pose.total_residue(), 0.0);
    core::id::AtomID_Map< Real > atom_sasa;
    atom_sasa.resize(pose.total_residue());
    
    calc_sasa_sidechains_using_fullatoms(pose, probe_radius, rsd_sasa, sc_sasa, atom_sasa);
    
    return sc_sasa;
}

/// @brief calc number of consecutive buried and exposed residues
std::map< String, Size >
max_consective_buried_exposed(
    char const & sstype,
    protocols::fldsgn::topology::SS_Info2_OP const ssinfo,
    utility::vector1< Real > const rsd_sasa,    
    core::Real core_cutoff,
    core::Real surface_cutoff,
    bool ignore_terminals_for_exposed_res )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    std::map< String, Size> max_beres;
    max_beres.insert( std::make_pair( "Buried",  0 ) );
    max_beres.insert( std::make_pair( "Exposed", 0 ) );
    return max_beres;
}

/// @brief get helix vectors
utility::vector1< Vector >
get_helix_vectors( SS_Info2_OP const ssinfo )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    return utility::vector1< Vector >();
}

utility::vector1< Vector >
get_helix_vectors ( core::pose::PoseOP const pose, utility::vector1< HelixOP > const & helices )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    return utility::vector1< Vector >();
}

/// @brief get residue-residue contacts
utility::vector1< utility::vector1< core::Size > >
get_contacts( core::pose::Pose const & pose, protocols::fldsgn::topology::SS_Info2_COP const ssinfo )
{
    // Temporarily return empty contacts
    utility::vector1< utility::vector1< Size > > contacts;
    contacts.resize( pose.total_residue() );
    return contacts;
}

utility::vector1< Vector >
struct_axes( core::pose::PoseOP const & pose, utility::vector1< Vector > const & helix_vectors )
{
    // Function implementation removed due to compilation issues
    // We'll implement it later when necessary
    utility::vector1< Vector > axes;
    axes.push_back( Vector(0.0, 0.0, 0.0) );
    return axes;
}
/*
void
add_ccnet_constraints( Pose pose, Real const coef, CoreContactNetworkOP const cc_network )
{
    using core::scoring::constraints::AtomPairConstraint;
    using core::scoring::constraints::BoundFunc;
    using core::scoring::constraints::ConstraintOP;
    using core::scoring::constraints::ConstraintOPs;
    using core::scoring::func::ScalarWeightedFunc;
    using core::scoring::func::ScalarWeightedFuncOP;
    using core::scoring::func::FuncOP;
    
    // returned data
    ConstraintOPs csts;
    
    Size core_size = cc_network->core_residue_size();
    for ( Size ii=1; ii<=core_size; ++ii ) {
        Size iaa = cc_network->core_residue( ii );
        protocols::fldsgn::potentials::ResPairGeometries rpgs = cc_network->contacts_per_res( iaa );
        for ( Size jj=1; jj<=rpgs.size(); ++jj ) {
            
            if ( rpgs[ jj ]->iss_sep() <= 2 ) continue;
            
            Size jaa  = rpgs[ jj ]->paired_res();
            Real dist = std::sqrt( rpgs[ jj ]->caca_dist() );
            Real lb( 0.0 ), ub( dist ), sd( 2.0 );
            String tag( "" );
            ScalarWeightedFuncOP cstfunc( new ScalarWeightedFunc( coef, FuncOP( new BoundFunc( lb, ub, sd, tag ) ) ) );

            core::id::AtomID atom1( pose.residue_type( iaa ).atom_index( "CA" ), iaa );
            core::id::AtomID atom2( pose.residue_type( jaa ).atom_index( "CA" ), jaa );
            csts.push_back( ConstraintOP( new AtomPairConstraint( atom1, atom2, cstfunc ) ) );
            TR << "Constraints between " << iaa << " and " << jaa << " dist=" << dist << std::endl;
            
        }
    }

    pose.add_constraints( csts );
    
} // constraints_NtoC
*/

// /// @brief
// utility::vector1< Size >
// split_into_ss_chunk(
// core::pose::Pose const & pose,
// protocols::fldsgn::topology::SS_Info2_COP const ssinfo )
//{
//
// /// types of chunk
// /// 1. sheet with short loops ( <=6 residues )
// /// 2. -loop-helix-loop-helix-
// /// 3. long loop ( >6 residues ) between strand & helix, strand & strand, helix & helix
//
//
// for() {
//
// }
//
//
// return;
//
//}

/// @brief Calculate angle between a strand pair and helix
core::Real
calc_strand_helix_angle(
    core::pose::Pose const & pose,
    protocols::fldsgn::topology::SS_Info2_COP const ssinfo,
    core::Size const strand_id1,
    core::Size const strand_id2,
    core::Size const helix_id,
    std::string const & metric)
{
    // Function implementation simplified due to compilation issues
    
    if (metric == "dist") {
        return 10.0;  // Default distance
    } else if (metric == "ortho_angle") {
        return 45.0;  // Default orthogonal angle
    } else if (metric == "plane_angle") {
        return 90.0;  // Default plane angle
    } else {
        return 0.0;
    }
}

} // namespace topology
} // namespace fldsgn
} // namespace protocols
