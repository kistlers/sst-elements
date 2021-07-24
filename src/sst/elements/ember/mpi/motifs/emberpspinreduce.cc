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

#include "emberpspinreduce.h"

using namespace SST::Ember;

#define TAG 0xDEADBEEF

EmberPspinReduceGenerator::EmberPspinReduceGenerator(SST::ComponentId_t id, Params& params)
    : EmberMessagePassingGenerator(id, params, "PspinReduce"),
      m_loopIndex(0) {
    m_count = (uint32_t)params.find("arg.count", 1024);
    m_iterations = (uint32_t)params.find("arg.iterations", 1);

    m_verify = params.find<bool>("arg.verify", true);

    memSetBacked();
    m_messageSize = m_count * sizeofDataType(INT);
    m_sendBuf = memAlloc(m_messageSize);
    m_recvBuf = memAlloc(m_messageSize);

    for (int i = 0; i < m_count; i++) {
        ((int*)m_sendBuf)[i] = 1000 * rank() + i;
    }
}

bool EmberPspinReduceGenerator::generate(std::queue<EmberEvent*>& evQ) { return true; }
