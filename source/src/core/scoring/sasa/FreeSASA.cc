// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file core/scoring/sasa/FreeSASA.cc
/// @brief FreeSASA wrapper implementation
/// @author Nobuyasu Koga (nobuyasu@uw.edu)

// Unit headers
#include <core/scoring/sasa/FreeSASA.hh>

// Package headers
#include <core/pose/Pose.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/Atom.hh>
#include <core/chemical/AtomType.hh>
#include <core/chemical/AtomTypeSet.hh>
#include <core/id/AtomID.hh>
#include <core/id/AtomID_Map.hh>

// Basic headers
#include <basic/Tracer.hh>

// Utility headers
#include <utility/exit.hh>

// External headers
#include <freesasa.h>

// C++ headers
#include <string>
#include <vector>
#include <cstring>

#ifdef    SERIALIZATION
// Utility serialization headers
#include <utility/serialization/serialization.hh>

// Cereal headers
#include <cereal/access.hpp>
#include <cereal/types/polymorphic.hpp>
#endif // SERIALIZATION

static basic::Tracer TR( "core.scoring.sasa.FreeSASA" );

namespace core {
namespace scoring {
namespace sasa {

/// @brief Constructor
FreeSASA::FreeSASA() :
	SasaMethod(),
	parameters_( nullptr ),
	probe_radius_( 1.4 ), // Default water probe radius
	algorithm_( "LeeRichards" ),
	n_slices_( 100 ),
	n_points_( 100 ),
	n_threads_( 1 )
{
	init_parameters();
}

/// @brief Constructor with probe radius
FreeSASA::FreeSASA( Real probe_radius ) :
	SasaMethod(),
	parameters_( nullptr ),
	probe_radius_( probe_radius ),
	algorithm_( "LeeRichards" ),
	n_slices_( 100 ),
	n_points_( 100 ),
	n_threads_( 1 )
{
	init_parameters();
}

/// @brief Destructor
FreeSASA::~FreeSASA()
{
	if ( parameters_ != nullptr ) {
		delete parameters_;
		parameters_ = nullptr;
	}
}

/// @brief Copy constructor
FreeSASA::FreeSASA( FreeSASA const & src ) :
	SasaMethod( src ),
	parameters_( nullptr ),
	probe_radius_( src.probe_radius_ ),
	algorithm_( src.algorithm_ ),
	n_slices_( src.n_slices_ ),
	n_points_( src.n_points_ ),
	n_threads_( src.n_threads_ )
{
	init_parameters();
}

/// @brief Clone method
SasaMethodOP
FreeSASA::clone() const
{
	return utility::pointer::make_shared< FreeSASA >( *this );
}

/// @brief Initialize FreeSASA parameters
void
FreeSASA::init_parameters()
{
	if ( parameters_ != nullptr ) {
		delete parameters_;
	}

	parameters_ = new freesasa_parameters();
	*parameters_ = freesasa_default_parameters;
	parameters_->probe_radius = probe_radius_;
	parameters_->n_threads = n_threads_;

	if ( algorithm_ == "LeeRichards" ) {
		parameters_->alg = FREESASA_LEE_RICHARDS;
		parameters_->lee_richards_n_slices = n_slices_;
	} else if ( algorithm_ == "ShrakeRupley" ) {
		parameters_->alg = FREESASA_SHRAKE_RUPLEY;
		parameters_->shrake_rupley_n_points = n_points_;
	} else {
		utility_exit_with_message( "Unknown FreeSASA algorithm: " + algorithm_ );
	}
}

/// @brief Calculate SASA for the whole pose
Real
FreeSASA::calculate( pose::Pose const & pose )
{
	// Convert pose to FreeSASA structure
	freesasa_structure* structure = pose_to_freesasa_structure( pose );
	if ( structure == nullptr ) {
		TR.Error << "Failed to convert pose to FreeSASA structure" << std::endl;
		return 0.0;
	}

	// Calculate SASA
	freesasa_result* result = freesasa_calc_structure( structure, parameters_ );
	if ( result == nullptr ) {
		freesasa_structure_free( structure );
		TR.Error << "FreeSASA calculation failed" << std::endl;
		return 0.0;
	}

	Real total_sasa = result->total;

	// Clean up
	cleanup_freesasa_structures( structure, result );

	return total_sasa;
}

/// @brief Get per-atom SASA values
void
FreeSASA::get_atom_sasa(
	pose::Pose const & pose,
	id::AtomID_Map< Real > & atom_sasa,
	bool const exclude_virtual /* = true */
) const
{
	// Initialize atom_sasa map
	atom_sasa.resize( pose.size() );
	for ( Size i = 1; i <= pose.size(); ++i ) {
		atom_sasa.resize( i, pose.residue( i ).natoms(), 0.0 );
	}

	// Convert pose to FreeSASA structure
	freesasa_structure* structure = pose_to_freesasa_structure( pose );
	if ( structure == nullptr ) {
		TR.Error << "Failed to convert pose to FreeSASA structure" << std::endl;
		return;
	}

	// Calculate SASA
	freesasa_result* result = freesasa_calc_structure( structure, parameters_ );
	if ( result == nullptr ) {
		freesasa_structure_free( structure );
		TR.Error << "FreeSASA calculation failed" << std::endl;
		return;
	}

	// Extract per-atom SASA
	Size atom_index = 0;
	for ( Size res_num = 1; res_num <= pose.size(); ++res_num ) {
		conformation::Residue const & res = pose.residue( res_num );
		for ( Size atom_num = 1; atom_num <= res.natoms(); ++atom_num ) {
			if ( exclude_virtual && res.atom_type( atom_num ).is_virtual() ) {
				continue;
			}
			if ( atom_index < result->n_atoms ) {
				atom_sasa[ id::AtomID( atom_num, res_num ) ] = result->sasa[ atom_index ];
				++atom_index;
			}
		}
	}

	// Clean up
	cleanup_freesasa_structures( structure, result );
}

/// @brief Get per-residue SASA values
utility::vector1< Real >
FreeSASA::get_residue_sasa(
	pose::Pose const & pose,
	bool const exclude_virtual /* = true */
) const
{
	utility::vector1< Real > residue_sasa( pose.size(), 0.0 );

	// Get per-atom SASA
	id::AtomID_Map< Real > atom_sasa;
	get_atom_sasa( pose, atom_sasa, exclude_virtual );

	// Sum up per-residue SASA
	for ( Size res_num = 1; res_num <= pose.size(); ++res_num ) {
		conformation::Residue const & res = pose.residue( res_num );
		Real res_sasa = 0.0;
		for ( Size atom_num = 1; atom_num <= res.natoms(); ++atom_num ) {
			if ( exclude_virtual && res.atom_type( atom_num ).is_virtual() ) {
				continue;
			}
			res_sasa += atom_sasa[ id::AtomID( atom_num, res_num ) ];
		}
		residue_sasa[ res_num ] = res_sasa;
	}

	return residue_sasa;
}

/// @brief Calculate SASA for specific atom subset
Real
FreeSASA::calculate_atom_subset(
	pose::Pose const & pose,
	utility::vector1< bool > const & atom_subset
)
{
	// Get per-atom SASA
	id::AtomID_Map< Real > atom_sasa;
	get_atom_sasa( pose, atom_sasa );

	// Sum up SASA for selected atoms
	Real subset_sasa = 0.0;
	Size atom_index = 1;
	for ( Size res_num = 1; res_num <= pose.size(); ++res_num ) {
		conformation::Residue const & res = pose.residue( res_num );
		for ( Size atom_num = 1; atom_num <= res.natoms(); ++atom_num ) {
			if ( atom_index <= atom_subset.size() && atom_subset[ atom_index ] ) {
				subset_sasa += atom_sasa[ id::AtomID( atom_num, res_num ) ];
			}
			++atom_index;
		}
	}

	return subset_sasa;
}

/// @brief Convert Rosetta pose to FreeSASA structure
freesasa_structure*
FreeSASA::pose_to_freesasa_structure( pose::Pose const & pose ) const
{
	// Count non-virtual atoms
	Size n_atoms = 0;
	for ( Size res_num = 1; res_num <= pose.size(); ++res_num ) {
		conformation::Residue const & res = pose.residue( res_num );
		for ( Size atom_num = 1; atom_num <= res.natoms(); ++atom_num ) {
			if ( !res.atom_type( atom_num ).is_virtual() ) {
				++n_atoms;
			}
		}
	}

	if ( n_atoms == 0 ) {
		TR.Warning << "No non-virtual atoms found in pose" << std::endl;
		return nullptr;
	}

	// Allocate arrays
	std::vector< double > x_coords( n_atoms );
	std::vector< double > y_coords( n_atoms );
	std::vector< double > z_coords( n_atoms );
	std::vector< double > radii( n_atoms );

	// Fill arrays
	Size atom_index = 0;
	for ( Size res_num = 1; res_num <= pose.size(); ++res_num ) {
		conformation::Residue const & res = pose.residue( res_num );
		for ( Size atom_num = 1; atom_num <= res.natoms(); ++atom_num ) {
			if ( res.atom_type( atom_num ).is_virtual() ) {
				continue;
			}

			conformation::Atom const & atom = res.atom( atom_num );
			x_coords[ atom_index ] = atom.xyz().x();
			y_coords[ atom_index ] = atom.xyz().y();
			z_coords[ atom_index ] = atom.xyz().z();

			// Get van der Waals radius
			// Note: FreeSASA expects radii in Angstroms
			radii[ atom_index ] = res.atom_type( atom_num ).lj_radius();

			++atom_index;
		}
	}

	// Create FreeSASA structure
	freesasa_structure* structure = freesasa_structure_new();
	if ( structure == nullptr ) {
		TR.Error << "Failed to allocate FreeSASA structure" << std::endl;
		return nullptr;
	}

	// Add atoms to structure
	int result = freesasa_structure_add_atom_wopt(
		structure,
		x_coords.data(),
		y_coords.data(),
		z_coords.data(),
		radii.data(),
		n_atoms
	);

	if ( result != FREESASA_SUCCESS ) {
		freesasa_structure_free( structure );
		TR.Error << "Failed to add atoms to FreeSASA structure" << std::endl;
		return nullptr;
	}

	return structure;
}

/// @brief Clean up FreeSASA structures
void
FreeSASA::cleanup_freesasa_structures(
	freesasa_structure* structure,
	freesasa_result* result
) const
{
	if ( structure != nullptr ) {
		freesasa_structure_free( structure );
	}
	if ( result != nullptr ) {
		freesasa_result_free( result );
	}
}

/// @brief Set probe radius
void
FreeSASA::set_probe_radius( Real probe_radius )
{
	probe_radius_ = probe_radius;
	init_parameters();
}

/// @brief Get probe radius
Real
FreeSASA::get_probe_radius() const
{
	return probe_radius_;
}

/// @brief Set algorithm type
void
FreeSASA::set_algorithm( std::string const & algorithm )
{
	if ( algorithm != "LeeRichards" && algorithm != "ShrakeRupley" ) {
		utility_exit_with_message( "Unknown FreeSASA algorithm: " + algorithm +
			". Valid options are: LeeRichards, ShrakeRupley" );
	}
	algorithm_ = algorithm;
	init_parameters();
}

/// @brief Get algorithm type
std::string
FreeSASA::get_algorithm() const
{
	return algorithm_;
}

/// @brief Set number of slices per atom
void
FreeSASA::set_n_slices( Size n_slices )
{
	n_slices_ = n_slices;
	if ( algorithm_ == "LeeRichards" ) {
		init_parameters();
	}
}

/// @brief Get number of slices per atom
Size
FreeSASA::get_n_slices() const
{
	return n_slices_;
}

/// @brief Set number of test points
void
FreeSASA::set_n_points( Size n_points )
{
	n_points_ = n_points;
	if ( algorithm_ == "ShrakeRupley" ) {
		init_parameters();
	}
}

/// @brief Get number of test points
Size
FreeSASA::get_n_points() const
{
	return n_points_;
}

/// @brief Set number of threads for calculation
void
FreeSASA::set_n_threads( Size n_threads )
{
	n_threads_ = n_threads;
	init_parameters();
}

/// @brief Get number of threads
Size
FreeSASA::get_n_threads() const
{
	return n_threads_;
}

#ifdef    SERIALIZATION

/// @brief Automatically generated serialization method
template< class Archive >
void
FreeSASA::save( Archive & arc ) const {
	arc( cereal::base_class< SasaMethod >( this ) );
	arc( CEREAL_NVP( probe_radius_ ) );
	arc( CEREAL_NVP( algorithm_ ) );
	arc( CEREAL_NVP( n_slices_ ) );
	arc( CEREAL_NVP( n_points_ ) );
	arc( CEREAL_NVP( n_threads_ ) );
}

/// @brief Automatically generated deserialization method
template< class Archive >
void
FreeSASA::load( Archive & arc ) {
	arc( cereal::base_class< SasaMethod >( this ) );
	arc( probe_radius_ );
	arc( algorithm_ );
	arc( n_slices_ );
	arc( n_points_ );
	arc( n_threads_ );

	// Reinitialize parameters after loading
	init_parameters();
}

SAVE_AND_LOAD_SERIALIZABLE( FreeSASA );
CEREAL_REGISTER_TYPE( ::core::scoring::sasa::FreeSASA )

CEREAL_REGISTER_DYNAMIC_INIT( core_scoring_sasa_FreeSASA )
#endif // SERIALIZATION

} // namespace sasa
} // namespace scoring
} // namespace core