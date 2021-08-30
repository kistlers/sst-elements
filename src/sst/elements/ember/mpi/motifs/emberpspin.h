#ifndef _H_EMBER_PSPIN
#define _H_EMBER_PSPIN

#include <pspin_sst.h>

#define MPI_ANY_SOURCE -1
#define MPI_ANY_TAG -1

class EmberPspinGenerator {
   public:
    EmberPspinGenerator();
    ~EmberPspinGenerator();

    uint32_t pspinTag(uint32_t tag);
};

#endif
