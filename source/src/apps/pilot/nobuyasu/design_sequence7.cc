// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file apps/pilot/nobuyasu/design_sequence7.cc
/// @brief Protein sequence design application using DesSeqOperator2
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

// Protocol headers
#include <protocols/fldsgn/deseq/DesSeqOperator2.hh>
#include <protocols/fldsgn/topology/util.hh>
#include <protocols/viewer/viewers.hh>
#include <protocols/symmetry/DetectSymmetryMover.hh>
#include <protocols/jd2/JobDistributor.hh>
#include <protocols/jd2/JobOutputter.hh>
#include <protocols/jd2/Job.hh>

// Data cache
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
#include <basic/options/keys/run.OptionKeys.gen.hh>
#include <devel/init.hh>
#include <basic/Tracer.hh>
#include <utility/excn/Exceptions.hh>
#include <utility/io/ozstream.hh>
#include <utility/file/FileName.hh>
#include <ObjexxFCL/format.hh>

// Standard library includes
#include <iomanip>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

static basic::Tracer TR("sequence_design.v7");

// Type aliases for cleaner code
using namespace ObjexxFCL::format;
using namespace core;
using namespace basic::options;
using namespace basic::options::OptionKeys;

typedef core::Size Size;
typedef std::string String;
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
typedef std::list<core::chemical::ResidueTypeCOP> ResidueTypeCOPList;
typedef std::map<Size, utility::vector1<ListAA>> ChainAminoAcidMap;

/**
 * @brief Read sequences to be favored for design at each position
 * 
 * Parses a sequence alignment file to extract amino acid preferences
 * for each position in the specified chain.
 * 
 * @param chain_idx The chain index in the pose
 * @param filename Path to the sequence alignment file
 * @param pose The protein pose
 * @param prob_threshold Probability threshold for amino acid inclusion (default: 0.8)
 * @return Vector of allowed amino acids for each position
 */
static utility::vector1<ListAA>
read_favseqs(
    Size const chain_idx,
    String const& filename,
    Pose const& pose,
    Real prob_threshold = 0.8)
{
    TR << "Reading favored sequences from file " << filename << " for chain " << chain_idx << std::endl;
    
    // Create a TaskFactory and PackerTask for the pose
    TaskFactoryOP tf = TaskFactoryOP(new TaskFactory);
    PackerTaskOP ptask = tf->create_packer_task(pose);
    
    // Read the sequence alignment from the file
    SequenceAlignment sa;
    sa.read_from_file2(filename);
    
    // Check that the sequence length matches the chain length
    Size chain_begin = pose.chain_begin(chain_idx);
    Size chain_end = pose.chain_end(chain_idx);
    Size n_residue = chain_end - chain_begin + 1;
    
    if (n_residue != sa.length()) {
        TR.Error << "Chain length (" << n_residue << 
                  ") doesn't match alignment length (" << 
                  sa.length() << ")" << std::endl;
        return utility::vector1<ListAA>();
    }
    
    // Initialize the vector of allowed amino acids for the entire pose
    utility::vector1<ListAA> allowed_aas(pose.total_residue());
    
    // Process each position in the alignment
    for (Size aln_pos = 1; aln_pos <= sa.length(); ++aln_pos) {
        // Calculate amino acid probabilities at this position
        std::map<AA, Real> aa_probs = sa.calculate_per_position_aaprob(aln_pos);
        
        // Convert to a sortable vector of (probability, amino acid) pairs
        utility::vector1<std::pair<Real, AA>> prob_aa_pairs;
        for (auto const& aa_prob : aa_probs) {
            prob_aa_pairs.push_back(std::make_pair(aa_prob.second, aa_prob.first));
        }
        
        // Sort by probability in descending order
        std::sort(prob_aa_pairs.rbegin(), prob_aa_pairs.rend());
        
        // Map alignment position to pose position
        Size pose_pos = chain_begin + aln_pos - 1;
        
        // Add amino acids to the allowed list until we reach the probability threshold
        Real cumulative_prob = 0.0;
        for (auto const& prob_aa : prob_aa_pairs) {
            cumulative_prob += prob_aa.first;
            allowed_aas[pose_pos].push_back(prob_aa.second);
            
            if (cumulative_prob > prob_threshold) {
                break;
            }
        }
        
        // Log the selected amino acids for this position
        if (TR.visible()) {
            std::ostringstream aa_list;
            for (auto const& aa : allowed_aas[pose_pos]) {
                aa_list << core::chemical::name_from_aa(aa) << " ";
            }
            TR << "Position " << pose_pos << " (chain " << chain_idx 
                       << ", alignment pos " << aln_pos << "): " << aa_list.str() << std::endl;
        }
    }
    
    return allowed_aas;
}

/**
 * @brief Process favored sequence files for all chains
 * 
 * @param pose The protein pose
 * @param fav_files Vector of filenames containing favored sequences
 * @return Map of chain index to allowed amino acids
 */
static ChainAminoAcidMap
process_fav_seqs_per_chain(
    Pose const& pose,
    utility::vector1<utility::file::FileName> const& fav_files)
{
    ChainAminoAcidMap allowed_aas_for_allchains;
    TR << "Processing " << fav_files.size() << " favored sequence file(s)..." << std::endl;
    
    Size const n_chains = pose.num_chains();
    
    if (fav_files.size() > n_chains) {
        TR.Warning << "More fav_seqs files provided (" << fav_files.size()
                   << ") than chains in pose (" << n_chains
                   << "). Extra files will be ignored." << std::endl;
    }
    
    for (Size chain_idx = 1; chain_idx <= std::min(fav_files.size(), n_chains); ++chain_idx) {
        try {
            TR << "Processing favored sequences for chain " << chain_idx 
               << " from file " << fav_files[chain_idx].name() << std::endl;
            
            utility::vector1<ListAA> chain_allowed_aas = read_favseqs(
                chain_idx, 
                fav_files[chain_idx].name(), 
                pose
            );
            
            allowed_aas_for_allchains[chain_idx] = chain_allowed_aas;
        } 
        catch (std::exception& e) {
            TR.Error << "Error processing favored sequences for chain " << chain_idx 
                     << " from file " << fav_files[chain_idx].name() 
                     << ": " << e.what() << std::endl;
            // Initialize with empty list for this chain
            allowed_aas_for_allchains[chain_idx] = utility::vector1<ListAA>();
        }
    }
    
    TR << "Finished processing favored sequence files for " 
       << allowed_aas_for_allchains.size() << " chain(s)." << std::endl;
    
    return allowed_aas_for_allchains;
}

/**
 * @brief Write additional pose score data to the output file
 * 
 * @param pose The protein pose with score data
 * @param out Output stream to write to
 */
static void
write_pose_extra_scores(
    Pose const& pose,
    utility::io::ozstream& out)
{
    // Write string data from pose
    if (pose.data().has(core::pose::datacache::CacheableDataType::ARBITRARY_STRING_DATA)) {
        basic::datacache::CacheableStringMapCOP data =
            utility::pointer::dynamic_pointer_cast<basic::datacache::CacheableStringMap const>(
                pose.data().get_const_ptr(core::pose::datacache::CacheableDataType::ARBITRARY_STRING_DATA)
            );
        
        if (data) {
            for (auto const& item : data->map()) {
                out << "REMARK " << RJ(8, item.first) << ": " << item.second << std::endl;
            }
        }
    }
    
    // Write float data from pose
    if (pose.data().has(core::pose::datacache::CacheableDataType::ARBITRARY_FLOAT_DATA)) {
        basic::datacache::CacheableStringFloatMapCOP data =
            utility::pointer::dynamic_pointer_cast<basic::datacache::CacheableStringFloatMap const>(
                pose.data().get_const_ptr(core::pose::datacache::CacheableDataType::ARBITRARY_FLOAT_DATA)
            );
        
        if (data) {
            for (auto const& item : data->map()) {
                out << "REMARK " << RJ(8, item.first) << ": " << F(8, 2, item.second) << std::endl;
            }
        }
    }
}

/**
 * @brief Generate output filenames based on input filename and iteration number
 * 
 * @param input_pdb Input PDB filename
 * @param iteration Current iteration number
 * @param outdir Output directory
 * @return Pair of PDB output filename and trajectory output filename
 */
static std::pair<String, String>
generate_output_filenames(
    String const& input_pdb,
    Size const iteration,
    String const& outdir)
{
    // Extract base filename from path
    Size path_sep_pos = input_pdb.find_last_of('/');
    String base_filename = (path_sep_pos == String::npos) ? 
                           input_pdb : 
                           input_pdb.substr(path_sep_pos + 1);
    
    // Format iteration number with leading zeros
    std::ostringstream num;
    num << std::setfill('0') << std::setw(4) << iteration;
    String iter_str = num.str();
    
    // Find file extension position
    Size ext_pos = base_filename.find_last_of('.');
    String base_name = (ext_pos == String::npos) ? 
                       base_filename : 
                       base_filename.substr(0, ext_pos);
    
    // Create output filenames
    String pdb_filename = base_name + "." + iter_str + ".ds7.pdb";
    String traj_filename = base_name + "." + iter_str + ".ds7.traj";
    
    // Ensure output directory has trailing slash
    String output_dir = outdir;
    if (!output_dir.empty() && output_dir.back() != '/') {
        output_dir += '/';
    }
    
    // Prepend output directory
    pdb_filename = output_dir + pdb_filename;
    traj_filename = output_dir + traj_filename;
    
    return std::make_pair(pdb_filename, traj_filename);
}

/**
 * @brief Initialize a pose from a PDB file
 * 
 * @param pdb_path Path to the PDB file
 * @return Initialized pose
 */
static Pose
initialize_pose_from_pdb(String const& pdb_path)
{
    TR << "Initializing pose from PDB file: " << pdb_path << std::endl;
    
    Pose pose;
    try {
        core::io::pdb::build_pose_from_pdb_as_is(pose, pdb_path);
    }
    catch (std::exception const& e) {
        TR.Error << "Failed to build pose from PDB: " << e.what() << std::endl;
        // Return an empty pose, caller should check pose.total_residue() > 0
    }
    
    // Ensure the pose is in full-atom representation
    if (!pose.is_fullatom()) {
        TR << "Converting pose to full-atom representation" << std::endl;
        core::util::switch_to_residue_type_set(pose, core::chemical::FA_STANDARD);
    }
    
    return pose;
}

/**
 * @brief Main function for the sequence design application
 */
int
main(int argc, char* argv[])
{
    try {
        // Initialize Rosetta with command-line options
        devel::init(argc, argv);
        
        // Check required input
        if (!option[in::file::s].user()) {
            TR.Error << "Input PDB file required (-s)" << std::endl;
            return 1;
        }
        
        // Load the input PDB file
        String input_pdb = option[in::file::s].value().at(1);
        Pose pose = initialize_pose_from_pdb(input_pdb);
        
        TR << "Loaded pose with " << pose.total_residue() << " residues and " 
           << pose.num_chains() << " chains" << std::endl;
        
        // Read MSA files for favored amino acids
        ChainAminoAcidMap allowed_aas_for_allchains;
        if (option[deseq::fav_seqs].user()) {
            utility::vector1<utility::file::FileName> fav_files(option[deseq::fav_seqs]());
            allowed_aas_for_allchains = process_fav_seqs_per_chain(pose, fav_files);
        }
        
        // Set up packer task from resfile if provided
        TaskFactoryOP tf = TaskFactoryOP(new TaskFactory());
        PackerTaskOP resfile_task = tf->create_packer_task(pose);
        if (option[deseq::resfile].user()) {
            TR << "Reading resfile: " << option[deseq::resfile]() << std::endl;
            core::pack::task::parse_resfile(pose, *resfile_task, option[deseq::resfile]());
        }
        
        // Set up symmetry if requested
        if (option[deseq::symmetry].user()) {
            TR << "Detecting symmetry in input structure" << std::endl;
            Real subunit_tolerance = 0.25;
            Real plane_tolerance = 0.001;
            DetectSymmetryOP symmetry_detector = DetectSymmetryOP(
                new DetectSymmetry(subunit_tolerance, plane_tolerance)
            );
            symmetry_detector->apply(pose);
        }
        
        // Get output directory
        String outdir = option[deseq::outdir].user() ? 
                        option[deseq::outdir]() : 
                        "./";
        
        // Run multiple design trajectories
        Size nstruct = option[deseq::nstruct]();
        TR << "Running " << nstruct << " design trajectory/trajectories" << std::endl;
        
        for (Size iteration = 1; iteration <= nstruct; ++iteration) {
            TR << "Starting design iteration " << iteration << " of " << nstruct << std::endl;
            
            // Create and configure the design mover
            DesSeqOperator2OP design_mover = DesSeqOperator2OP(new DesSeqOperator2());
            
            // Apply design to a copy of the input pose
            Pose design_pose(pose);
            design_mover->apply(design_pose);
            
            // Generate output filenames
            std::pair<String, String> output_files = generate_output_filenames(input_pdb, iteration, outdir);
            String pdb_out = output_files.first;
            String traj_out = output_files.second;
            TR << "Writing output to " << pdb_out << std::endl;
            
            // Save the designed structure
            design_pose.dump_pdb(pdb_out);
            
            // Append extra score information to the PDB
            utility::io::ozstream out_file(pdb_out, std::ios::app);
            if (!out_file) {
                TR.Error << "Unable to open file " << pdb_out << " for writing extra scores" << std::endl;
                continue;
            }
            
            write_pose_extra_scores(design_pose, out_file);
            out_file.close();
            
            TR << "Completed design iteration " << iteration << std::endl;
        }
        
        TR << "Design protocol completed successfully" << std::endl;
        return 0;
        
    } catch (utility::excn::Exception const& e) {
        TR.Error << "Caught exception from Rosetta" << std::endl;
        return 1;
    } catch (std::exception const& e) {
        TR.Error << "Caught standard exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        TR.Error << "Caught unknown exception" << std::endl;
        return 1;
    }
}