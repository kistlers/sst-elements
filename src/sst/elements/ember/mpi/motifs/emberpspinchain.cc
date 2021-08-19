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

#include "emberpspinchain.h"

using namespace SST::Ember;

#define TAG 0xDEADBEEF

EmberPspinChainGenerator::EmberPspinChainGenerator(SST::ComponentId_t id, Params &params)
    : EmberMessagePassingGenerator(id, params, "PspinChain"), m_loopIndex(0) {
    m_count = (uint32_t)params.find("arg.count", 128);
    m_iterations = (uint32_t)params.find("arg.iterations", 1);

    memSetBackedZeroed();
    m_messageSize = sizeof(sst_handle_t) + sizeof(pspin_chain_header_t) + m_count * sizeofDataType(INT);
    m_sendBuf = (uint8_t *)memAlloc(m_messageSize);
    m_recvBuf = (uint8_t *)memAlloc(m_messageSize);

    if (rank() == 0) {
        output(
            "m_messageSize=%lu sizeof(sst_handle_t)+sizeof(pspin_chain_header_t)=%lu "
            "m_count*sizeofDataType(INT)=%lu\n",
            m_messageSize, sizeof(sst_handle_t) + sizeof(pspin_chain_header_t), m_count * sizeofDataType(INT));
    }

    if (rank() < size() - 1) {
        pspin_chain_header_t *chain_header = (pspin_chain_header_t *)(m_sendBuf + sizeof(sst_handle_t));
        chain_header->chain_target = size() - 1;
        output("comm_rank %u: comm_size: %u chain_target=%d\n", rank(), size(), chain_header->chain_target);
    }

    PAYLOAD_DATATYPE *sendBufElements =
        (PAYLOAD_DATATYPE *)(m_sendBuf + sizeof(sst_handle_t) + sizeof(pspin_chain_header_t));
    for (int i = 0; i < m_count; i++) {
        sendBufElements[i] = 100 * rank() + i;
    }
}

bool EmberPspinChainGenerator::generate(std::queue<EmberEvent *> &evQ) {
    if (m_loopIndex == m_iterations) {
        if (0 == rank()) {
            double totalTime = (double)(m_stopTime - m_startTime) / 1000000000.0;

            double latency = ((totalTime / m_iterations) / 2);
            double bandwidth = (double)m_messageSize / latency;

            output(
                "%s: size %d, total time %.3f us, loop %d, bufLen %d"
                ", latency %.3f us. bandwidth %f GB/s\n",
                getMotifName().c_str(), size(), totalTime * 1000000.0, m_iterations, m_messageSize, latency * 1000000.0,
                bandwidth / 1000000000.0);
        }

        // if (rank() > 0 && m_verify) {
        //     std::function<uint64_t()> verify = [&]() {
        //         pspin_pkt_header_t *header = (pspin_pkt_header_t *)m_sendBuf;

        //         if (header->destination != rank()) {
        //             printf("Error: Rank %d header->destination failed  got=%d shouldEqual=%d\n", rank(),
        //             header->destination,
        //                    rank());
        //         }

        //         int32_t *recvBufElements =
        //             (int32_t *)((char *)m_recvBuf + ROUND_UP_DMA_WIDTH(sizeof(pspin_pkt_header_t)));
        //         ;
        //         for (int i = 0; i < m_count; i++) {
        //             int32_t shouldEqual = 100 * 0 + i;
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
            output("rank=%d messageSize=%d iterations=%d\n", rank(), m_messageSize, m_iterations);
            enQ_getTime(evQ, &m_startTime);
        }
    }

    if (rank() == 0) {
        auto pspinSendTag = pspinTag(nextRank());
        output("send %d->%d tag=0x%x pspinSendTag=0x%x messageSize=%d iterations=%d\n", rank(), nextRank(), nextRank(),
               pspinSendTag, m_messageSize, m_iterations);
        enQ_send(evQ, m_sendBuf, m_messageSize, CHAR, nextRank(), pspinSendTag, GroupWorld);
    }

    if (rank() > 0) {
        auto pspinRecvTag = pspinTag(rank());
        output("recv %d->%d tag=0x%x pspinRecvTag=0x%x messageSize=%d iterations=%d\n", prevRank(), rank(), rank(),
               pspinRecvTag, m_messageSize, m_iterations);
        enQ_recv(evQ, m_recvBuf, m_messageSize, CHAR, prevRank(), pspinRecvTag, GroupWorld, &m_resp);
    }

    if (++m_loopIndex == m_iterations) {
        enQ_getTime(evQ, &m_stopTime);
    }
    return false;
}
