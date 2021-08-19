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

#include <sst_config.h>

// sst_config must be at top

#include "emberpspinsend.h"

using namespace SST::Ember;

#define TAG 0xDEADBEEF

EmberPspinSendGenerator::EmberPspinSendGenerator(SST::ComponentId_t id, Params &params)
    : EmberMessagePassingGenerator(id, params, "PspinSend"), m_loopIndex(0), m_rank2(1) {
    m_count = (uint32_t)params.find("arg.count", 112);
    m_iterations = (uint32_t)params.find("arg.iterations", 1);
    m_rank2 = (uint32_t)params.find("arg.rank2", 1);

    memSetBackedZeroed();
    m_messageSize = sizeof(pspin_first_pkt_header_t) + m_count * sizeofDataType(INT);
    m_sendBuf = (uint8_t *)memAlloc(m_messageSize);
    m_recvBuf = (uint8_t *)memAlloc(m_messageSize);

    PAYLOAD_DATATYPE *sendBufElements = (PAYLOAD_DATATYPE *)(m_sendBuf + sizeof(pspin_first_pkt_header_t));
    for (int i = 0; i < m_count; i++) {
        sendBufElements[i] = 100 * rank() + i;
    }
}

bool EmberPspinSendGenerator::generate(std::queue<EmberEvent *> &evQ) {
    if (m_loopIndex == m_iterations || !(0 == rank() || m_rank2 == rank())) {
        if (0 == rank()) {
            double totalTime = (double)(m_stopTime - m_startTime) / 1000000000.0;

            double latency = ((totalTime / m_iterations) / 2);
            double bandwidth = (double)m_messageSize / latency;

            output(
                "%s: otherRank %d, total time %.3f us, loop %d, bufLen %d"
                ", latency %.3f us. bandwidth %f GB/s\n",
                getMotifName().c_str(), m_rank2, totalTime * 1000000.0, m_iterations, m_messageSize,
                latency * 1000000.0, bandwidth / 1000000000.0);
        }

        // if (m_rank2 == rank() && m_verify) {
        //     std::function<uint64_t()> verify = [&]() {
        //         pspin_pkt_header_t *header = (pspin_pkt_header_t *)m_sendBuf;

        //         if (header->destination != rank()) {
        //             printf("Error: Rank %d header->                if (header->destination != rank()) {
        //                    failed got = % d shouldEqual = % d\n ", rank(), header->destination,
        //                                                   rank());
        //         }

        //         int32_t *recvBufElements =
        //             (int32_t *)((char *)m_recvBuf + sizeof(pspin_pkt_header_t));
        //         ;
        //         for (int i = 0; i < m_count; i++) {
        //             int32_t shouldEqual = 100 * otherRank() + i;
        //             if (shouldEqual != recvBufElements[i]) {
        //                 printf("Error: Rank %d recvBufElements[%d] failed  got=%d shouldEqual=%d\n", rank(), i,
        //                        recvBufElements[i], shouldEqual);
        //             }
        //         }
        //         return 0;
        //     };
        //     enQ_compute(evQ, verify);
        // }

        return true;
    }

    if (0 == m_loopIndex) {
        verbose(CALL_INFO, 1, 0, "rank=%d size=%d\n", rank(), size());

        if (0 == rank()) {
            output("rank2=%d messageSize=%d iterations=%d\n", m_rank2, m_messageSize, m_iterations);
            enQ_getTime(evQ, &m_startTime);
        }
    }

    if (0 == rank()) {
        enQ_send(evQ, m_sendBuf, m_messageSize, CHAR, otherRank(), m_rank2, GroupWorld);
        // enQ_recv(evQ, m_recvBuf, m_messageSize, CHAR, otherRank(), m_rank2, GroupWorld, &m_resp);
    } else if (m_rank2 == rank()) {
        enQ_recv(evQ, m_recvBuf, m_messageSize, CHAR, otherRank(), m_rank2, GroupWorld, &m_resp);
        // enQ_send(evQ, m_sendBuf, m_messageSize, CHAR, otherRank(), m_rank2, GroupWorld);
    }

    if (++m_loopIndex == m_iterations) {
        enQ_getTime(evQ, &m_stopTime);
    }
    return false;
}
