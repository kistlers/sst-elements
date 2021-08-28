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

#include "emberpspinpingpong.h"

using namespace SST::Ember;

#define TAG_PING 0xBEEFCAFE
#define TAG_PONG 0xFEFEFEFE
#define TAG_SYNC 0x0000CAFE

EmberPspinPingPongGenerator::EmberPspinPingPongGenerator(SST::ComponentId_t id, Params &params)
    : EmberMessagePassingGenerator(id, params, "PspinPingPong"), m_loopIndex(0), m_rank2(1) {
    m_count = (uint32_t)params.find("arg.count", 128);
    m_iterations = (uint32_t)params.find("arg.iterations", 1);
    m_rank2 = (uint32_t)params.find("arg.rank2", 1);
    m_verify = params.find<bool>("arg.verify", false);

    m_messageSize = sizeof(pspin_pingpong_header_t) + m_count * sizeof(uint32_t);
    memSetBackedZeroed();
    m_sendBuf = (uint8_t *)memAlloc(m_messageSize);
    m_recvBuf = (uint8_t *)memAlloc(m_messageSize);

    output("rank: %d, count: %u, messageSize: %u\n", rank(), m_count, m_messageSize);

    PAYLOAD_DATATYPE *sendBufElements = (PAYLOAD_DATATYPE *)(m_sendBuf + sizeof(pspin_pingpong_header_t));
    for (int i = 0; i < m_count; i++) {
        sendBufElements[i] = 100 * rank() + i;
    }
}

bool EmberPspinPingPongGenerator::generate(std::queue<EmberEvent *> &evQ) {
    if (0 != rank() && m_rank2 != rank()) {
        return true;
    }

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

        if (m_verify) {
            std::function<uint64_t()> verify = [&]() {
                pspin_pingpong_header_t *header = (pspin_pingpong_header_t *)m_sendBuf;

                PAYLOAD_DATATYPE *recvBufElements = (PAYLOAD_DATATYPE *)(m_recvBuf + sizeof(pspin_pingpong_header_t));
                for (int i = 0; i < m_count; i++) {
                    int32_t shouldEqual = 100 * 0 + i;
                    if (shouldEqual != recvBufElements[i]) {
                        printf("Error: Rank %d recvBufElements[%d] failed: got=%d shouldEqual=%d\n", rank(), i,
                               recvBufElements[i], shouldEqual);
                    }
                }
                return 0;
            };
            enQ_compute(evQ, verify);
        }

        return true;
    }

    uint32_t synch_msg_size = 64;

    if (0 == rank()) {
        // get ready to recv the pong
        enQ_irecv(evQ, m_recvBuf, m_messageSize, CHAR, m_rank2, TAG_PONG, GroupWorld, &m_req);

        // first recv the sync message (to make sure there is a recv on the other side)
        enQ_recv(evQ, m_recvBuf, synch_msg_size, CHAR, m_rank2, TAG_SYNC, GroupWorld, &m_resp);

        // get the time after the sync
        if (m_loopIndex == 0) {
            enQ_getTime(evQ, &m_startTime);
        }

        // then send
        auto pingTag = pspinTag(TAG_PING);
        ((pspin_pingpong_header_t *)m_sendBuf)->source = rank();
        ((pspin_pingpong_header_t *)m_sendBuf)->destination = m_rank2;
        enQ_send(evQ, m_sendBuf, m_messageSize, CHAR, m_rank2, pingTag, GroupWorld);

        // then wait for the pong
        enQ_wait(evQ, &m_req);

    } else if (m_rank2 == rank()) {
        // get ready to recv the ping
        auto pingTag = pspinTag(TAG_PING);
        enQ_irecv(evQ, m_recvBuf, m_messageSize, CHAR, 0, pingTag, GroupWorld, &m_req);

        // send the ready-to-recv
        enQ_send(evQ, NULL, synch_msg_size, CHAR, 0, TAG_SYNC, GroupWorld);

        // wait for the recv to complete (the sending of the pong is done by PsPIN)
        enQ_wait(evQ, &m_req);
    }

    if (++m_loopIndex == m_iterations) {
        enQ_getTime(evQ, &m_stopTime);
    }
    return false;
}
