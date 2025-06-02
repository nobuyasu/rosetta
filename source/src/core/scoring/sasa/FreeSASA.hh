// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file core/scoring/sasa/FreeSASA.hh
/// @brief FreeSASA wrapper for Rosetta
/// @author Nobuyasu Koga (nobuyasu@uw.edu)

#ifndef INCLUDED_core_scoring_sasa_FreeSASA_HH
#define INCLUDED_core_scoring_sasa_FreeSASA_HH

// Unit headers
#include <core/scoring/sasa/FreeSASA.fwd.hh>
#include <core/scoring/sasa/SasaMethod.hh>

// Package headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/conformation/Residue.fwd.hh>
#include <core/id/AtomID_Map.fwd.hh>

// Utility headers
#include <utility/vector1.hh>

// C++ headers
#include <string>
#include <vector>

// Forward declaration for FreeSASA C structures
struct freesasa_structure;
struct freesasa_result;
struct freesasa_parameters;

namespace core {
namespace scoring {
namespace sasa {

/// @brief Wrapper class for FreeSASA library
/// @details This class provides a Rosetta interface to the FreeSASA library
///          for calculating solvent accessible surface area
class FreeSASA : public SasaMethod {

public:
	/// @brief Constructor
	FreeSASA();

	/// @brief Constructor with probe radius
	FreeSASA( Real probe_radius );

	/// @brief Destructor
	~FreeSASA() override;

	/// @brief Copy constructor
	FreeSASA( FreeSASA const & src );

	/// @brief Clone method
	SasaMethodOP
	clone() const override;

	/// @brief Calculate SASA for the whole pose
	Real
	calculate( pose::Pose const & pose ) override;

	/// @brief Get per-atom SASA values
	void
	get_atom_sasa(
		pose::Pose const & pose,
		id::AtomID_Map< Real > & atom_sasa,
		bool const exclude_virtual = true
	) const override;

	/// @brief Get per-residue SASA values
	utility::vector1< Real >
	get_residue_sasa(
		pose::Pose const & pose,
		bool const exclude_virtual = true
	) const override;

	/// @brief Calculate SASA for specific atom subset
	Real
	calculate_atom_subset(
		pose::Pose const & pose,
		utility::vector1< bool > const & atom_subset
	);

	/// @brief Set probe radius
	void
	set_probe_radius( Real probe_radius );

	/// @brief Get probe radius
	Real
	get_probe_radius() const;

	/// @brief Set algorithm type
	/// @details Options: "LeeRichards", "ShrakeRupley"
	void
	set_algorithm( std::string const & algorithm );

	/// @brief Get algorithm type
	std::string
	get_algorithm() const;

	/// @brief Set number of slices per atom (for Lee-Richards algorithm)
	void
	set_n_slices( Size n_slices );

	/// @brief Get number of slices per atom
	Size
	get_n_slices() const;

	/// @brief Set number of test points (for Shrake-Rupley algorithm)
	void
	set_n_points( Size n_points );

	/// @brief Get number of test points
	Size
	get_n_points() const;

	/// @brief Set number of threads for calculation
	void
	set_n_threads( Size n_threads );

	/// @brief Get number of threads
	Size
	get_n_threads() const;

protected:
	/// @brief Initialize FreeSASA parameters
	void
	init_parameters();

	/// @brief Convert Rosetta pose to FreeSASA structure
	freesasa_structure*
	pose_to_freesasa_structure( pose::Pose const & pose ) const;

	/// @brief Clean up FreeSASA structures
	void
	cleanup_freesasa_structures(
		freesasa_structure* structure,
		freesasa_result* result
	) const;

private:
	/// @brief FreeSASA parameters
	freesasa_parameters* parameters_;

	/// @brief Probe radius (Angstroms)
	Real probe_radius_;

	/// @brief Algorithm type
	std::string algorithm_;

	/// @brief Number of slices per atom (Lee-Richards)
	Size n_slices_;

	/// @brief Number of test points (Shrake-Rupley)
	Size n_points_;

	/// @brief Number of threads
	Size n_threads_;

#ifdef    SERIALIZATION
public:
	template< class Archive > void save( Archive & arc ) const;
	template< class Archive > void load( Archive & arc );
#endif // SERIALIZATION

}; // FreeSASA

} // namespace sasa
} // namespace scoring
} // namespace core

#ifdef    SERIALIZATION
CEREAL_FORCE_DYNAMIC_INIT( core_scoring_sasa_FreeSASA )
#endif // SERIALIZATION

#endif // INCLUDED_core_scoring_sasa_FreeSASA_HH