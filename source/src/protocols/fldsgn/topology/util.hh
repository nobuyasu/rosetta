// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file ./src/protocols/src/fldsgn/topology/util.cc
/// @brief
/// @author Nobuyasu Koga ( nobuyasu@u.washington.edu )

#ifndef INCLUDED_protocols_fldsgn_topology_util_hh
#define INCLUDED_protocols_fldsgn_topology_util_hh

// unit header
#include <protocols/fldsgn/topology/StrandPairing.fwd.hh>
#include <protocols/fldsgn/topology/SS_Info2.fwd.hh>
#include <core/scoring/hbonds/HBondSet.fwd.hh>
#include <protocols/forge/build/Interval.fwd.hh>

#include <core/pose/Pose.hh>
#include <core/id/AtomID_Map.fwd.hh>
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>


#include <utility/vector1.hh>
#include <map>
#include <string>

namespace protocols {
namespace fldsgn {
namespace topology {


/// @brief convert StrandParingSet of dssp to fldsgn::topology::StrandPairingSet
protocols::fldsgn::topology::StrandPairingSet
calc_strand_pairing_set(
	core::pose::Pose const & pose,
	protocols::fldsgn::topology::SS_Info2_COP const ssinfo,
	core::Size minimum_pair_length = 1
);

/// @brief calc Helix Order
core::Real
calc_HelixOrder( utility::vector1< core::Vector > const & helix_unit_vectors );

/// @brief calc delta sasa, when a molecule is splited to 2parts.
core::Real
calc_delta_sasa(
	core::pose::Pose const & pose,
	utility::vector1< protocols::forge::build::Interval > intervals,
	core::Real const pore_radius
);


/// @brief check kink of helix, return number of loosen hydrogen
core::Size
check_kink_helix(
	core::pose::Pose const & pose,
	core::Size const begin,
	core::Size const end );

/// @brief
utility::vector1< core::scoring::hbonds::HBond >
check_internal_hbonds(
	core::pose::Pose const & pose,
	core::Size const begin,
	core::Size const end );

/// @brief
void
append_terminal_glyres(
    core::Size chain,                
    core::pose::Pose & pose );

/// @brief check chain break
core::pose::Pose
build_blank_pose (
    core::Size const target_size,
    bool const centroid = true,
    bool const add_terminal = true,
    core::Real const phi=-60.0,
    core::Real const psi=-45.0,
    core::Real const omega=180.0,
    core::pose::PoseOP const appended_pose = NULL,
    char const which_side = 'N',
    bool const as_different_chain = true );


/// @brief
bool
check_chain_break(
    core::pose::Pose const & pose,
    core::Size const begin,
    core::Size const end,
    bool const verbose = false );


/// @brief calc sasa using mainchain and cbeta atoms
utility::vector1< core::Real >
calc_sasa_mainchain_w_cb(
    core::pose::Pose const & pose,
    core::Real const probe_radius );


/// @brief calc sasa for sidechains
void
calc_sasa_sidechains_using_fullatoms (
    core::pose::Pose const & pose,
    core::Real const probe_radius,
    utility::vector1< core::Real > & rsd_sasa,
    utility::vector1< core::Real > & sc_sasa,
    core::id::AtomID_Map< core::Real > atom_sasa );
    

/// @brief calc sasa for sidechains
utility::vector1< core::Real >
calc_sasa_sidechains_using_fullatoms (
    core::pose::Pose const & pose,
    core::Real const probe_radius = 1.4 );


/// @brief calc number of consecutive buried and exposed residues
std::map< std::string, core::Size >
max_consective_buried_exposed(
    char const & sstype,
    protocols::fldsgn::topology::SS_Info2_OP const ssinfo,
    utility::vector1< core::Real > const rsd_sasa,
    core::Real core_cutoff = 10.0,
    core::Real surface_cutoff = 20.0,
    bool ignore_terminals_for_exposed_res = true );


/// @brief get helix vectors
utility::vector1< core::Vector >
get_helix_vectors( SS_Info2_OP const ssinfo );

/// @brief get helix vectors
utility::vector1< core::Vector >
get_helix_vectors ( core::pose::PoseOP const pose, utility::vector1< HelixOP > const & helices );

/// @brief
utility::vector1< utility::vector1< core::Size > >
get_contacts( core::pose::Pose const & pose, protocols::fldsgn::topology::SS_Info2_COP const ssinfo );

/// @brief
utility::vector1< core::Vector >
struct_axes( core::pose::PoseOP const & pose, utility::vector1< core::Vector > const & helix_vectors );

/// @brief Calculate angle between a strand pair and helix
/// @details This function calculates geometric relationships between a strand pair and a helix
/// @param pose The protein pose
/// @param ssinfo Secondary structure information
/// @param strand_id1 ID of the first strand
/// @param strand_id2 ID of the second strand
/// @param helix_id ID of the helix
/// @param metric Type of measurement to return: "dist", "ortho_angle", or "plane_angle"
/// @return The requested measurement between the strand pair and helix
core::Real
calc_strand_helix_angle(
    core::pose::Pose const & pose,
    protocols::fldsgn::topology::SS_Info2_COP const ssinfo,
    core::Size const strand_id1,
    core::Size const strand_id2,
    core::Size const helix_id,
    std::string const & metric
);

} // namespace topology
} // namespace fldsgn
} // namespace protocols

#endif
