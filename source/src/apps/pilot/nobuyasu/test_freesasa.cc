// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file apps/pilot/nobuyasu/test_freesasa.cc
/// @brief Test application for FreeSASA integration
/// @author Nobuyasu Koga (nobuyasu@uw.edu)

// Core headers
#include <core/init/init.hh>
#include <core/pose/Pose.hh>
#include <core/import_pose/import_pose.hh>
#include <core/conformation/Residue.hh>
#include <core/id/AtomID.hh>
#include <core/types.hh>
#include <core/pose/PDBInfo.hh>

// Basic headers
#include <basic/options/option.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/Tracer.hh>

// Utility headers
#include <utility/excn/Exceptions.hh>
#include <utility/vector1.hh>

// External headers
#include <freesasa.h>

// C++ headers
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

static basic::Tracer TR( "test_freesasa" );

using namespace core;

// Helper function to convert Rosetta pose to FreeSASA structure
freesasa_structure* 
pose_to_freesasa_structure( pose::Pose const & pose ) {
	// Create empty structure
	freesasa_structure* structure = freesasa_structure_new();
	
	if ( structure == NULL ) {
		TR.Error << "Failed to create empty FreeSASA structure" << std::endl;
		return NULL;
	}
	
	// Process all atoms in the pose
	for ( Size res_idx = 1; res_idx <= pose.size(); ++res_idx ) {
		conformation::Residue const & rsd = pose.residue(res_idx);
		std::string res_name = rsd.name3();
		
		// Get chain and residue info from PDB info if available
		char chain_id = ' ';
		int res_number = res_idx;
		if ( pose.pdb_info() ) {
			chain_id = pose.pdb_info()->chain(res_idx);
			res_number = pose.pdb_info()->number(res_idx);
		}
		
		// Use 'A' as default chain if empty
		if ( chain_id == ' ' || chain_id == '\0' ) {
			chain_id = 'A';
		}
		
		for ( Size atom_idx = 1; atom_idx <= rsd.natoms(); ++atom_idx ) {
			std::string atom_name = rsd.atom_name(atom_idx);
			
			// Skip hydrogen atoms (FreeSASA typically ignores them)
			if ( atom_name.size() > 0 && atom_name[0] == 'H' ) continue;
			
			// Also check for hydrogens starting with digit (e.g., "1H", "2H")
			if ( atom_name.find("1H") == 0 || atom_name.find("2H") == 0 ) continue;
			
			// Trim spaces from atom name to match FreeSASA's expectations
			atom_name.erase(std::remove_if(atom_name.begin(), atom_name.end(), ::isspace), atom_name.end());
			
			// Get atom coordinates
			numeric::xyzVector<Real> const & coords = rsd.xyz(atom_idx);
			
			// Add atom to FreeSASA structure
			if ( freesasa_structure_add_atom(
				structure,
				atom_name.c_str(),
				res_name.c_str(),
				std::to_string(res_number).c_str(),
				chain_id,
				coords.x(),
				coords.y(),
				coords.z()
			) == FREESASA_FAIL ) {
				TR.Warning << "Failed to add atom " << atom_name 
				          << " from residue " << res_name << " " << res_number 
				          << " chain " << chain_id << std::endl;
			}
		}
	}
	
	return structure;
}

int main( int argc, char * argv[] ) {
	try {
		// Initialize Rosetta
		core::init::init( argc, argv );
		
		using namespace basic::options;
		using namespace basic::options::OptionKeys;
		
		// Check if input PDB file was provided
		if ( !option[in::file::s].user() ) {
			TR.Error << "No input PDB file provided. Use -in:file:s to specify." << std::endl;
			return 1;
		}
		
		// Load pose from PDB file
		pose::Pose pose;
		core::import_pose::pose_from_file( pose, option[in::file::s]()[1] );
		TR.Info << "Loaded pose with " << pose.size() << " residues." << std::endl;
		
		// Convert pose to FreeSASA structure
		freesasa_structure* structure = pose_to_freesasa_structure( pose );
		
		if ( structure == NULL ) {
			TR.Error << "Failed to create FreeSASA structure from pose." << std::endl;
			return 1;
		}
		
		// Set up FreeSASA parameters
		freesasa_parameters parameters = freesasa_default_parameters;
		parameters.alg = FREESASA_LEE_RICHARDS;
		parameters.probe_radius = 1.4;
		parameters.lee_richards_n_slices = 20;
		
		// Calculate SASA
		freesasa_result* result = freesasa_calc_structure( structure, &parameters );
		
		if ( result == NULL ) {
			TR.Error << "FreeSASA calculation failed." << std::endl;
			freesasa_structure_free( structure );
			return 1;
		}
		
		// Output results
		std::ofstream out_file( "sasa_results.txt" );
		
		// Total SASA
		Real total_sasa = result->total;
		out_file << "# Total SASA: " << total_sasa << " Å²" << std::endl;
		TR.Info << "Total SASA: " << total_sasa << " Å²" << std::endl;
		
		// Per-atom SASA
		out_file << "# Per-atom SASA values:" << std::endl;
		out_file << "#   Atom#   SASA (Å²)" << std::endl;
		
		Size atom_index = 0;
		for ( Size res_idx = 1; res_idx <= pose.size(); ++res_idx ) {
			conformation::Residue const & rsd = pose.residue(res_idx);
			for ( Size atom_idx = 1; atom_idx <= rsd.natoms(); ++atom_idx ) {
				std::string atom_name = rsd.atom_name(atom_idx);
				
				// Skip hydrogen atoms
				if ( atom_name.size() > 0 && atom_name[0] == 'H' ) continue;
				if ( atom_name.find("1H") == 0 || atom_name.find("2H") == 0 ) continue;
				
				out_file << std::setw(8) << atom_index+1 
				         << std::setw(12) << std::fixed << std::setprecision(2) 
				         << result->sasa[atom_index] << std::endl;
				atom_index++;
			}
		}
		
		// Test different algorithms
		TR.Info << "Testing Lee-Richards algorithm with different resolutions:" << std::endl;
		for ( int res = 10; res <= 40; res += 10 ) {
			parameters.lee_richards_n_slices = res;
			freesasa_result* res_result = freesasa_calc_structure( structure, &parameters );
			if ( res_result ) {
				TR.Info << "  Resolution " << res << ": " << res_result->total << " Å²" << std::endl;
				freesasa_result_free( res_result );
			}
		}
		
		// Test Shrake-Rupley algorithm
		TR.Info << "Testing Shrake-Rupley algorithm:" << std::endl;
		parameters.alg = FREESASA_SHRAKE_RUPLEY;
		parameters.shrake_rupley_n_points = 100;
		freesasa_result* sr_result = freesasa_calc_structure( structure, &parameters );
		if ( sr_result ) {
			TR.Info << "  Shrake-Rupley: " << sr_result->total << " Å²" << std::endl;
			TR.Info << "  Difference from Lee-Richards: " << (sr_result->total - total_sasa) << " Å²" << std::endl;
			freesasa_result_free( sr_result );
		}
		
		// Clean up
		freesasa_result_free( result );
		freesasa_structure_free( structure );
		
		out_file.close();
		
		return 0;
		
	} catch ( utility::excn::Exception const & e ) {
		std::cerr << "Exception caught: " << e.msg() << std::endl;
		return 1;
	} catch ( std::exception const & e ) {
		std::cerr << "STL exception caught: " << e.what() << std::endl;
		return 1;
	} catch ( ... ) {
		std::cerr << "Unknown exception caught" << std::endl;
		return 1;
	}
}