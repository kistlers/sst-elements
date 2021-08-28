#ifndef _H_EMBER_PSPIN
#define _H_EMBER_PSPIN

#include <pspin_sst.h>

class EmberPspinGenerator {
   public:
    EmberPspinGenerator();
    ~EmberPspinGenerator();

    uint32_t pspinTag(uint32_t tag);
};

#endif
