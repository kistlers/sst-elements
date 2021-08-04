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

#include "mpi/embermpigen.h"
#include "pspin.h"

namespace SST {
namespace Ember {

class EmberPspinSendGenerator : public EmberMessagePassingGenerator {
   public:
    SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(EmberPspinSendGenerator, "ember", "PspinSendMotif",
                                          SST_ELI_ELEMENT_VERSION(1, 0, 0), "Performs a Pspin Send Motif",
                                          SST::Ember::EmberGenerator)

    SST_ELI_DOCUMENT_PARAMS({"arg.count", "Sets the number of elements in the send operation", "112"},
                            {"arg.iterations", "Sets the number of send operations to perform", "1"},
                            {"arg.rank2", "Sets the 2nd rank to send to (0 is the 1st)", "1"}, )
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
    EmberPspinSendGenerator(SST::ComponentId_t, Params& params);
    bool generate(std::queue<EmberEvent*>& evQ);

    uint32_t otherRank() { return rank() == m_rank2 ? 0 : m_rank2; }

   private:
    MessageRequest m_req;
    MessageResponse m_resp;
    void* m_sendBuf;
    void* m_recvBuf;

    uint32_t m_count;
    uint32_t m_messageSize;
    uint32_t m_iterations;
    uint64_t m_startTime;
    uint64_t m_stopTime;
    uint32_t m_loopIndex;

    int m_rank2;
    bool m_verify;
};

}  // namespace Ember
}  // namespace SST

#endif
