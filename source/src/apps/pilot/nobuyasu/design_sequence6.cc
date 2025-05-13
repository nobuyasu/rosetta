// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file apps/pilot/nobuyasu/design_sequence5.cc
/// @brief
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

// Unit headers
#include <core/io/pdb/build_pose_as_is.hh>
#include <core/conformation/Conformation.fwd.hh>
#include <core/conformation/Residue.hh>
#include <core/chemical/ResidueType.hh>
#include <core/pose/Pose.hh> 
#include <core/pack/task/PackerTask.hh>
#include <core/pack/task/TaskFactory.hh>
#include <core/pack/task/ResfileReader.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/sequence/Sequence.hh>
#include <core/sequence/SequenceAlignment.hh>
#include <core/pack/task/PackerTask.hh>
#include <core/pack/task/ResidueLevelTask_.hh>
#include <core/util/SwitchResidueTypeSet.hh>
#include <protocols/fldsgn/deseq/DesSeqOperator2.hh>
#include <protocols/fldsgn/topology/util.hh>
#include <protocols/viewer/viewers.hh>

#include <protocols/symmetry/DetectSymmetryMover.hh>

#include <protocols/jd2/JobDistributor.hh>
#include <protocols/jd2/JobOutputter.hh>
#include <protocols/jd2/Job.hh>

#include <core/pose/datacache/CacheableDataType.hh>
#include <basic/datacache/BasicDataCache.hh>
#include <basic/datacache/CacheableString.hh>
#include <basic/datacache/CacheableStringFloatMap.hh>
#include <basic/datacache/CacheableStringMap.hh>

// Utility Headers
#include <basic/options/option.hh>
#include <basic/options/option_macros.hh>
#include <basic/options/keys/OptionKeys.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/options/keys/out.OptionKeys.gen.hh>
#include <basic/options/keys/deseq.OptionKeys.gen.hh>

#include <devel/init.hh>
#include <basic/Tracer.hh>
#include <utility/excn/Exceptions.hh>
#include <utility/io/ozstream.hh>
#include <utility/file/FileName.hh>

#include <ObjexxFCL/format.hh>

static basic::Tracer TR( "sequence_design.v6" );

typedef core::Size Size;
typedef std::string String;
using namespace ObjexxFCL::format;
using namespace core;
using namespace basic::options;
using namespace basic::options::OptionKeys;

typedef core::chemical::AA AA;
typedef core::chemical::ResidueTypeCOP ResidueTypeCOP;
typedef core::sequence::Sequence Sequence;
typedef core::sequence::SequenceAlignment SequenceAlignment;
typedef core::pose::Pose Pose;
typedef core::pose::PoseOP PoseOP;
typedef core::pack::task::PackerTask PackerTask;
typedef core::pack::task::PackerTaskOP PackerTaskOP;
typedef core::pack::task::TaskFactory TaskFactory;
typedef core::pack::task::TaskFactoryOP TaskFactoryOP;

typedef protocols::fldsgn::deseq::DesSeqOperator2 DesSeqOperator2;
typedef protocols::fldsgn::deseq::DesSeqOperator2OP DesSeqOperator2OP;

typedef protocols::symmetry::DetectSymmetry DetectSymmetry;
typedef protocols::symmetry::DetectSymmetryOP DetectSymmetryOP;

typedef std::list<core::chemical::AA> ListAA;
typedef std::list< core::chemical::ResidueTypeCOP > ResidueTypeCOPList;
typedef std::list< core::chemical::ResidueTypeCOP >::iterator ResidueTypeCOPListIter;
typedef std::list< core::chemical::ResidueTypeCOP >::const_iterator ResidueTypeCOPListConstIter;


/// @brief
/// read sequences to be favored to design each position
static utility::vector1< ListAA >
read_favseqs ( Size const chain, String const filename, Pose const & pose )
{
    
    Real prob_using_aa( 0.8 );
        
    TaskFactoryOP tf = TaskFactoryOP( new TaskFactory );
    PackerTaskOP ptask = tf->create_packer_task( pose );

    SequenceAlignment sa;
    sa.read_from_file2( filename );
    
    Size n_residue( pose.chain_end( chain ) - pose.chain_begin( chain ) + 1 );
    runtime_assert( n_residue == sa.length() );

    utility::vector1< ListAA > allowed_aas( pose.total_residue() );
    for( Size iaa=1; iaa<=sa.length(); ++iaa ) {
        std::map< AA, Real > aaprob =  sa.calculate_per_position_aaprob( iaa );
        utility::vector1< std::pair< Real, AA > > v;
        for ( std::map< AA, Real >::iterator it=aaprob.begin(); it!=aaprob.end(); it++ ) {
            v.push_back( std::pair< Real, AA > ( it->second, it->first ) );
        }
        Real prob( 0.0 );
        std::sort( v.rbegin(), v.rend() );
        for ( Size ii=1; ii<=v.size(); ++ii ) {
            prob += v[ ii ].first;
            allowed_aas[ iaa ].push_back( v[ ii ].second );
            // std::cout << iaa << " " << v[ ii ].first << " " << v[ ii ].second << std::endl;
            if( prob > prob_using_aa ) break;
        }
    }

    return allowed_aas;
    
}

/// @brief
/// process fav_seqs for all chains
static std::map< core::Size, utility::vector1< ListAA > >
process_fav_seqs_per_chain (
    Pose const & pose,
    utility::vector1<utility::file::FileName> fav_files )
{
    
    std::map< Size, utility::vector1< ListAA > > allowed_aas_for_allchains;
    TR << "Processing " << fav_files.size() << " fav_seqs file(s)..." << std::endl;
    
    Size const n_chains( pose.num_chains() );
            
    if ( fav_files.size() > n_chains ) {
        TR.Error << "More fav_seqs files provided (" << fav_files.size()
                 << ") than chains in pose (" << n_chains
                 << "). Extra files will be ignored." << std::endl;
    }

    for ( Size chain_idx = 1; chain_idx <= fav_files.size(); ++chain_idx ) {

        Size length = pose.chain_begin(chain_idx) - pose.chain_end(chain_idx) + 1;
                
        try {
            
            TR << "Calling user's read_favseqs for file " << fav_files[chain_idx].name()
               << " (chain index " << chain_idx << ")" << std::endl;
            
            utility::vector1< ListAA > result_from_read_favseqs =
                read_favseqs( chain_idx, fav_files[chain_idx].name(), pose );

            allowed_aas_for_allchains[chain_idx] = result_from_read_favseqs;

        } catch ( utility::excn::Exception & e ) {
            TR.Error << "Error calling read_favseqs for file " << fav_files[chain_idx].name()
                     << " for chain index " << chain_idx << ": " << e.msg() << std::endl;
            allowed_aas_for_allchains[chain_idx] = utility::vector1<ListAA>();
        }
    }

    TR << "Finished processing fav_seqs files using user's function. Data stored for "
       << allowed_aas_for_allchains.size() << " chain(s)." << std::endl;

    return allowed_aas_for_allchains;
  
}


static void
dump_pose_extra_score (
    Pose const & pose,
    utility::io::ozstream & out )
{
    
    // ARBITRARY_STRING_DATA
    if ( pose.data().has( core::pose::datacache::CacheableDataType::ARBITRARY_STRING_DATA ) ) {
        basic::datacache::CacheableStringMapCOP data
            = utility::pointer::dynamic_pointer_cast< basic::datacache::CacheableStringMap const >
            ( pose.data().get_const_ptr( core::pose::datacache::CacheableDataType::ARBITRARY_STRING_DATA ) );
        assert( data.get() != 0 );

        for ( std::map< std::string, std::string >::const_iterator it( data->map().begin() ), end( data->map().end() );
                it != end;
                ++it ) {
            // TR << it->first << " " << it->second << std::endl;
            out << "REMARK " << RJ( 8, it->first ) << ": " << it->second << std::endl;
        }
    }

    // ARBITRARY_FLOAT_DATA
    if ( pose.data().has( core::pose::datacache::CacheableDataType::ARBITRARY_FLOAT_DATA ) ) {
        basic::datacache::CacheableStringFloatMapCOP data
            = utility::pointer::dynamic_pointer_cast< basic::datacache::CacheableStringFloatMap const >
            ( pose.data().get_const_ptr( core::pose::datacache::CacheableDataType::ARBITRARY_FLOAT_DATA ) );
        assert( data.get() != 0 );

        for ( std::map< std::string, float >::const_iterator it( data->map().begin() ), end( data->map().end() );
                it != end;
                ++it ) {
            //TR << it->first << " " << it->second << std::endl;
            out << "REMARK " << RJ( 8, it->first ) << ": " << F( 8, 2, it->second ) << std::endl;
        }
    }
    
}


int
main( int argc, char * argv [] )
{
        
    try {
         
        devel::init(argc, argv);
        
        String input_pdb( option[ in::file::s ].value().at( 1 ) );
        
        Pose pose;
        core::io::pdb::build_pose_from_pdb_as_is( pose, input_pdb );
                        
        if ( !pose.is_fullatom() ) {
            core::util::switch_to_residue_type_set ( pose, core::chemical::FA_STANDARD );
        }

        // read msa files predicted by ProteinMPNN or LigandMPNN to specify favorite aa for each position
        std::map< Size, utility::vector1< ListAA > > allowed_aas_for_allchains;
        if ( option[ deseq::fav_seqs ].user() ) {
            utility::vector1<utility::file::FileName> fav_files( option[ deseq::fav_seqs ]() );
            allowed_aas_for_allchains = process_fav_seqs_per_chain ( pose, fav_files );
        }
        
        // read resfile to specify PIKAA or NATRO, usually for functional sites
        TaskFactoryOP tf = TaskFactoryOP( new TaskFactory() );
        PackerTaskOP resfile_task = tf->create_packer_task( pose );
        if ( option[ deseq::resfile ].user() ) {
            core::pack::task::parse_resfile( pose, *resfile_task, option[ deseq::resfile ]() );
        }
        
        if ( option[ deseq::symmetry ].user() ) {
            Real subunit_tolerance( 0.25 );
            Real plane_tolerance( 0.001 );
            DetectSymmetryOP dm = DetectSymmetryOP( new DetectSymmetry( subunit_tolerance, plane_tolerance ) );
            dm->apply( pose );
        }

        //
        Size nstruct( option[ deseq::nstruct ]() );
        for ( Size ii=1; ii<=nstruct; ++ii ) {
            
            DesSeqOperator2OP mover = DesSeqOperator2OP( new DesSeqOperator2() );
            mover->apply( pose );
            
            // read pdb file
            // set packer task
            
            
            
            // mover->set_input_ptask( ptask );
            // mover->dump_trj( option[ dump_trj ] );
                                    
            // define output filename
            Size dotpos = input_pdb.find_last_of('/');
            input_pdb = input_pdb.substr( dotpos+1, input_pdb.length() );

            String pdbout ( input_pdb ), trjout( input_pdb );
            dotpos = input_pdb.find_last_of('.');
            std::ostringstream num;
            num << std::setfill('0') << std::setw(4) << ii;
            if ( dotpos == std::string::npos ) {
                pdbout += "." + num.str() + ".ds5.pdb";
                trjout += "." + num.str() + ".ds5.traj";
            } else {
                pdbout = pdbout.substr( 0, dotpos ) + "." + num.str() + ".ds5.pdb";
                trjout = trjout.substr( 0, dotpos ) + "." + num.str() + ".ds5.traj";
            }
            
            String outdir( option[ deseq::outdir ]() );
            if ( outdir.substr( outdir.length()-1, 1 ) != "/" ) {
                outdir += "/";
            }
            pdbout = outdir + pdbout;
            trjout = outdir + trjout;
            
            TR << pdbout << " " << trjout<< std::endl;
            
            Pose work_pose( pose );
            // mover->trj_filename( trjout );
            // mover->apply( work_pose );
            work_pose.dump_pdb( pdbout );
            
            utility::io::ozstream file ( pdbout, std::ios::app );
            if ( !file ) {
                basic::Error() << "FileData::dump_pdb: Unable to open file:" << file << " for writing!!!" << std::endl;
                return -1;
            }
            dump_pose_extra_score( work_pose, file );
            file.close();
                                            
        }
        
    } catch ( utility::excn::Exception const & e ) {
        
		std::cout << "caught exception " << e.msg() << std::endl;
		return -1;
        
	}
    
    return 1;
}


