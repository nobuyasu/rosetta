// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file ./src/protocols/fldsgn/deseq/DesSeqOperator.fwd.hh
/// @brief
/// @author Nobuyasu Koga ( nkoga@protein.osaka-u.ac.jp )

#ifndef INCLUDED_protocols_fldsgn_deseq_DesSeqOperator2_fwd_hh
#define INCLUDED_protocols_fldsgn_deseq_DesSeqOperator2_fwd_hh

#include <utility/pointer/owning_ptr.hh>
#include <utility/vector1.fwd.hh>


namespace protocols {
namespace fldsgn {
namespace deseq {

class DesSeqOperator2;

typedef utility::pointer::shared_ptr< DesSeqOperator2 > DesSeqOperator2OP;
typedef utility::pointer::shared_ptr< DesSeqOperator2 const > DesSeqOperator2COP;

} // namespace deseq
} // namespace fldsgn
} // namespace protocol

#endif
