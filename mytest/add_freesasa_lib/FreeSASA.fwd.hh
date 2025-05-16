// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the
// (c) Rosetta Commons. For more information, see http://www.rosettacommons.org.
// (c) Questions about this can be addressed to University of Washington UW TechTransfer,
// (c) email: license@u.washington.edu.

/// @file core/scoring/sasa/FreeSASA.fwd.hh
/// @brief Forward declarations for FreeSASA class
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

#ifndef INCLUDED_core_scoring_sasa_FreeSASA_fwd_hh
#define INCLUDED_core_scoring_sasa_FreeSASA_fwd_hh

// Utility Headers
#include <utility/pointer/owning_ptr.hh>

namespace core {
namespace scoring {
namespace sasa {

// Forward declaration of FreeSASA class
class FreeSASA;

// Owning pointer typedef
typedef utility::pointer::shared_ptr< FreeSASA > FreeSASAOP;
typedef utility::pointer::shared_ptr< FreeSASA const > FreeSASACOP;

} // namespace sasa
} // namespace scoring
} // namespace core

#endif // INCLUDED_core_scoring_sasa_FreeSASA_fwd_hh