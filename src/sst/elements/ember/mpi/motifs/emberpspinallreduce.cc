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

#include "emberpspinallreduce.h"

using namespace SST::Ember;

#define TAG_REDUCE 0x0000CECE
#define TAG_BROADCAST 0x0000BCBC

EmberPspinAllReduceGenerator::EmberPspinAllReduceGenerator(SST::ComponentId_t id, Params &params)
    : EmberMessagePassingGenerator(id, params, "PspinAllReduce"), m_loopIndex(0) {
    m_count = (uint32_t)params.find("arg.count", 128);
    m_iterations = (uint32_t)params.find("arg.iterations", 1);
    m_verify = params.find<bool>("arg.verify", false);

    m_messageSize = m_count * sizeof(uint32_t);
    memSetBackedZeroed();
    m_sendBuf = (uint8_t *)memAlloc(m_messageSize);
    m_recvBuf = (uint8_t *)memAlloc(m_messageSize);

    m_req_children = (MessageRequest *)memAlloc((REDUCTION_FACTOR + 1) * sizeof(MessageRequest));

    output("rank: %d, size: %d, count: %u, messageSize: %u\n", rank(), size(), m_count, m_messageSize);

    PAYLOAD_DATATYPE *sendBufElements = (PAYLOAD_DATATYPE *)m_sendBuf;
    for (int i = 0; i < m_count; i++) {
        sendBufElements[i] = 100 * rank() + i;
    }
}

bool EmberPspinAllReduceGenerator::generate(std::queue<EmberEvent *> &evQ) {
    if (m_loopIndex == m_iterations) {
        std::function<uint64_t()> rankDone = [&]() {
            output("rank %d done\n", rank());
            return 0;
        };
        enQ_compute(evQ, rankDone);

        if (0 == rank()) {
            double totalTime = (double)(m_stopTime - m_startTime) / 1000000000.0;

            double latency = ((totalTime / m_iterations) / 2);
            double bandwidth = (double)m_messageSize / latency;

            output(
                "%s: total time %.3f us, loop %d, bufLen %d"
                ", latency %.3f us. bandwidth %f GB/s\n",
                getMotifName().c_str(), totalTime * 1000000.0, m_iterations, m_messageSize, latency * 1000000.0,
                bandwidth / 1000000000.0);
        }

        if (m_verify && rank() == 0) {
            std::function<uint64_t()> verify = [&]() {
                PAYLOAD_DATATYPE *recvBufElements = (PAYLOAD_DATATYPE *)m_recvBuf;
                for (int i = 0; i < m_count; i++) {
                    int32_t shouldEqual = 100 * 0 + i;
                    if (shouldEqual != recvBufElements[i]) {
                        output("Error: Rank %d recvBufElements[%d] failed: got=%d shouldEqual=%d\n", rank(), i,
                               recvBufElements[i], shouldEqual);
                    }
                }
                return 0;
            };
            enQ_compute(evQ, verify);
        }

        return true;
    }

    // assertNumChildren();

    const auto reduceTag = pspinTag(TAG_REDUCE);
    // const auto broadcastTag = pspinTag(TAG_BROADCAST);
    const auto parent = PARENT(rank());
    const auto children = getChildren();

    // recv from children during reduce (one for each child)
    for (size_t i = 0; i < children.size(); i++) {
        output("rank %u starting irecv from child %u\n", rank(), children[i]);
        enQ_irecv(evQ, m_recvBuf, m_messageSize, CHAR, children[i], reduceTag, GroupWorld, &m_req_children[i]);
    }

    // if (rank() != 0) {
    // recv from parent during broadcast
    // enQ_irecv(evQ, m_recvBuf, m_messageSize, CHAR, parent, broadcastTag, GroupWorld, &m_req_parent);
    // }

    // get the time after the sync
    if (m_loopIndex == 0) {
        enQ_getTime(evQ, &m_startTime);
    }

    // if (hasChildren()) {
    //     // send to self to start reduction at current node
    //     output("rank %u sending to self\n", rank());
    //     enQ_send(evQ, m_sendBuf, m_messageSize, CHAR, rank(), reduceTag, GroupWorld);
    //     // enQ_isend(evQ, m_sendBuf, m_messageSize, CHAR, rank(), reduceTag, GroupWorld, &m_req_self_send);
    // }
    if (!hasChildren()) {
        // start at leaves and send to parent to start reduction
        output("rank %u sending to parent %u\n", rank(), parent);
        enQ_send(evQ, m_sendBuf, m_messageSize, CHAR, parent, reduceTag, GroupWorld);
        // enQ_isend(evQ, m_sendBuf, m_messageSize, CHAR, parent, reduceTag, GroupWorld, &m_req_parent_send);
    }

    // if (hasChildren()) {
    //     output("rank %u waiting for send to self\n", rank());
    //     enQ_wait(evQ, &m_req_self_send);
    // } else {
    //     output("rank %u waiting for send to parent %u\n", rank(), parent);
    //     enQ_wait(evQ, &m_req_parent_send);
    // }

    // then wait for the recvs
    for (size_t i = 0; i < children.size(); i++) {
        output("rank %u waiting for recv from child %u\n", rank(), children[i]);
        enQ_wait(evQ, &m_req_children[i]);
    }

    if (++m_loopIndex == m_iterations) {
        enQ_getTime(evQ, &m_stopTime);
    }
    return false;
}
