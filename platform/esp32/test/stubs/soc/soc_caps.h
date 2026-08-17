/* What the IDF's own soc_caps.h says about the part this backend targets, reduced to the one
   capability the firmware reads: how many symbols one RMT memory block holds.
   Pixels.cpp sizes its channel from it, so a host build has to know it too.

   48 is the ESP32-S3's figure. A classic ESP32 and an S2 have 64, and the difference is the
   point of deriving the constant rather than writing a number - see RmtMemoryBlockSymbols. */
#ifndef _SOC_CAPS_H_
#define _SOC_CAPS_H_

#define SOC_RMT_MEM_WORDS_PER_CHANNEL   48

#endif // _SOC_CAPS_H_
