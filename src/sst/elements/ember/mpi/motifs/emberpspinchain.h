// Copyright 2009-2021 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2021, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_EMBER_PSPIN_SEND
#define _H_EMBER_PSPIN_SEND

#include <cassert>

#include "emberpspin.h"
#include "mpi/embermpigen.h"
#include "pspin_chain.h"

namespace SST {
namespace Ember {

class EmberPspinChainGenerator : public EmberMessagePassingGenerator {
   public:
    SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(EmberPspinChainGenerator, "ember", "PspinChainMotif",
                                          SST_ELI_ELEMENT_VERSION(1, 0, 0), "Performs a Pspin Chain Motif",
                                          SST::Ember::EmberGenerator)

    SST_ELI_DOCUMENT_PARAMS({"arg.count", "Sets the number of elements in the send operation", "112"},
                            {"arg.iterations", "Sets the number of send operations to perform", "1"}, )
    SST_ELI_DOCUMENT_STATISTICS(
        {"time-Init", "Time spent in Init event", "ns", 0}, {"time-Finalize", "Time spent in Finalize event", "ns", 0},
        {"time-Rank", "Time spent in Rank event", "ns", 0}, {"time-Size", "Time spent in Size event", "ns", 0},
        {"time-Send", "Time spent in Recv event", "ns", 0}, {"time-Recv", "Time spent in Recv event", "ns", 0},
        {"time-Irecv", "Time spent in Irecv event", "ns", 0}, {"time-Isend", "Time spent in Isend event", "ns", 0},
        {"time-Wait", "Time spent in Wait event", "ns", 0}, {"time-Waitall", "Time spent in Waitall event", "ns", 0},
        {"time-Waitany", "Time spent in Waitany event", "ns", 0},
        {"time-Compute", "Time spent in Compute event", "ns", 0},
        {"time-Barrier", "Time spent in Barrier event", "ns", 0},
        {"time-Alltoallv", "Time spent in Alltoallv event", "ns", 0},
        {"time-Alltoall", "Time spent in Alltoall event", "ns", 0},
        {"time-Allreduce", "Time spent in Allreduce event", "ns", 0},
        {"time-Reduce", "Time spent in Reduce event", "ns", 0}, {"time-Bcast", "Time spent in Bcast event", "ns", 0},
        {"time-Gettime", "Time spent in Gettime event", "ns", 0},
        {"time-Commsplit", "Time spent in Commsplit event", "ns", 0},
        {"time-Commcreate", "Time spent in Commcreate event", "ns", 0}, )

   public:
    EmberPspinChainGenerator(SST::ComponentId_t, Params& params);
    bool generate(std::queue<EmberEvent*>& evQ);

    uint32_t nextRank() { return rank() < size() - 1 ? rank() + 1 : -1; }

    uint32_t prevRank() { return rank() > 0 ? rank() - 1 : -1; }

    uint32_t pspinTag(uint32_t tag) {
        assert((tag & (uint32_t)0xffff0000) == 0x0);
        return tag | (uint32_t)0xbeef0000;
    }

   private:
    MessageRequest m_req;
    MessageResponse m_resp;
    uint8_t* m_sendBuf;
    uint8_t* m_recvBuf;

    uint32_t m_count;
    uint32_t m_messageSize;
    uint32_t m_iterations;
    uint64_t m_startTime;
    uint64_t m_stopTime;
    uint32_t m_loopIndex;
};

}  // namespace Ember
}  // namespace SST

#endif
