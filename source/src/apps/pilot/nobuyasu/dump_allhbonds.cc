// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file
/// @brief Application to dump all hydrogen bonds in a structure
/// @author Nobuyasu Koga

// Project headers
#include <protocols/moves/Mover.hh>
#include <protocols/jd2/JobDistributor.hh>
#include <protocols/jd2/JobOutputter.hh>
#include <protocols/jd2/Job.hh>

#include <core/pose/Pose.hh>
#include <core/scoring/hbonds/HBondSet.hh>
#include <core/scoring/hbonds/hbonds.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>

// type headers
#include <core/types.hh>

// Utility Headers
#include <devel/init.hh>
#include <basic/Tracer.hh>
#include <basic/options/option.hh>
#include <basic/options/option_macros.hh>
#include <basic/options/keys/OptionKeys.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/options/keys/out.OptionKeys.gen.hh>

// C++ header
#include <fstream>
#include <iostream>
#include <utility/excn/Exceptions.hh>

typedef core::Size Size;
typedef std::string String;

using namespace core;
using namespace basic::options;
using namespace basic::options::OptionKeys;

static basic::Tracer TR( "dump_allhbonds" );

class ThisApplication  {
public:
	ThisApplication(){};
	static void register_options();
};

OPT_KEY( File, output )
OPT_KEY( Boolean, verbose )

void ThisApplication::register_options() {
	using namespace basic::options;
	using namespace basic::options::OptionKeys;
	NEW_OPT( output, "output file for hydrogen bond information", "hbonds.out" );
	NEW_OPT( verbose, "output detailed hydrogen bond geometry", false );
}

/// local mover for dumping hydrogen bonds
class DumpAllHBonds : public protocols::moves::Mover {
public:
	DumpAllHBonds():
		verbose_( option[ verbose ]() )
	{
		std::ostringstream filename;
		filename << option[ output ]();
		output_.open( filename.str().c_str(), std::ios::out );
		
		if ( !output_.is_open() ) {
			throw CREATE_EXCEPTION(utility::excn::BadInput, "Unable to open output file: " + filename.str());
		}
		
		// Write header
		output_ << "# Hydrogen bonds dump" << std::endl;
		output_ << "# Structure Donor_Res Donor_Atom Acceptor_Res Acceptor_Atom";
		if ( verbose_ ) {
			output_ << " Distance Angle Energy";
		}
		output_ << std::endl;
	}

	virtual ~DumpAllHBonds(){
		if ( output_.is_open() ) {
			output_.close();
		}
	}

	virtual std::string get_name() const { return "DumpAllHBonds"; }

	virtual
	void
	apply( core::pose::Pose & pose ){
		using namespace protocols::jd2;
		using namespace core::scoring::hbonds;
		
		// Get current job name
		JobOP job_me( JobDistributor::get_instance()->current_job() );
		String me( JobDistributor::get_instance()->job_outputter()->output_name( job_me ) );
		
		// Score the pose to ensure energies are up to date
		core::scoring::ScoreFunctionOP sfxn = core::scoring::get_score_function();
		(*sfxn)(pose);
		
		// Create HBondSet and fill it with all hydrogen bonds
		HBondSetOP hbond_set( new HBondSet() );
		fill_hbond_set( pose, false /*calc_deriv*/, *hbond_set );
		
		// Get total number of hydrogen bonds
		Size n_hbonds = hbond_set->nhbonds();
		
		TR << "Found " << n_hbonds << " hydrogen bonds in " << me << std::endl;
		
		// Iterate through all hydrogen bonds
		for ( Size i = 1; i <= n_hbonds; ++i ) {
			HBond const & hbond = hbond_set->hbond(i);
			
			// Get donor and acceptor information
			Size don_res = hbond.don_res();
			Size don_hatm = hbond.don_hatm();
			Size acc_res = hbond.acc_res();
			Size acc_atm = hbond.acc_atm();
			
			// Get atom names
			String don_atom_name = pose.residue(don_res).atom_name(don_hatm);
			String acc_atom_name = pose.residue(acc_res).atom_name(acc_atm);
			
			// Get residue names
			String don_res_name = pose.residue(don_res).name3() + utility::to_string(don_res);
			String acc_res_name = pose.residue(acc_res).name3() + utility::to_string(acc_res);
			
			// Output basic information
			output_ << me << " ";
			output_ << don_res_name << " " << don_atom_name << " ";
			output_ << acc_res_name << " " << acc_atom_name;
			
			// Output detailed geometry if requested
			if ( verbose_ ) {
				Real distance = hbond.get_HAdist();
				Real angle = hbond.get_AHDangle();
				Real energy = hbond.energy();
				
				output_ << " " << distance;
				output_ << " " << angle;
				output_ << " " << energy;
			}
			
			output_ << std::endl;
		}
		
		// Also output summary to tracer
		output_ << "# Total hydrogen bonds: " << n_hbonds << std::endl;
	}

private:
	std::ofstream output_;
	bool verbose_;
};

typedef utility::pointer::shared_ptr< DumpAllHBonds > DumpAllHBondsOP;

/////////////////////////////////////////////////////////////////////////////
int main( int argc, char * argv [] ) {
	try {
		// Initialize options
		ThisApplication::register_options();
		devel::init( argc, argv );
		
		// Create and run the mover through job distributor
		DumpAllHBondsOP dump_hbonds( new DumpAllHBonds );
		protocols::jd2::JobDistributor::get_instance()->go( dump_hbonds );
		
		TR << "Successfully completed hydrogen bond analysis" << std::endl;
	} catch (utility::excn::Exception const & e ) {
		std::cerr << "caught exception " << e.msg() << std::endl;
		return -1;
	}
	return 0;
}