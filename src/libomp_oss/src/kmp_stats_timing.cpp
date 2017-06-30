/** @file kmp_stats_timing.cpp
 * Timing functions
 */

/* <copyright>
    Copyright (c) 1997-2016 Intel Corporation.  All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

</copyright> */

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "kmp.h"
#include "kmp_stats_timing.h"

using namespace std;

#if KMP_HAVE_TICK_TIME
# if KMP_MIC
double tsc_tick_count::tick_time()
{
    // pretty bad assumption of 1GHz clock for MIC
    return 1/((double)1000*1.e6);
}
# elif KMP_ARCH_X86 || KMP_ARCH_X86_64
#  include <string.h>
// Extract the value from the CPUID information
double tsc_tick_count::tick_time()
{
    static double result = 0.0;

    if (result == 0.0)
    {
        kmp_cpuid_t cpuinfo;
        char brand[256];

        __kmp_x86_cpuid(0x80000000, 0, &cpuinfo);
        memset(brand, 0, sizeof(brand));
        int ids = cpuinfo.eax;

        for (unsigned int i=2; i<(ids^0x80000000)+2; i++)
            __kmp_x86_cpuid(i | 0x80000000, 0, (kmp_cpuid_t*)(brand+(i-2)*sizeof(kmp_cpuid_t)));

        char * start = &brand[0];
        for (;*start == ' '; start++)
            ;

        char * end = brand + KMP_STRLEN(brand) - 3;
        uint64_t multiplier;

        if (*end == 'M') multiplier = 1000LL*1000LL;
        else if (*end == 'G') multiplier = 1000LL*1000LL*1000LL;
        else if (*end == 'T') multiplier = 1000LL*1000LL*1000LL*1000LL;
        else
        {
            cout << "Error determining multiplier '" << *end << "'\n";
            exit (-1);
        }
        *end = 0;
        while (*end != ' ') end--;
        end++;

        double freq = strtod(end, &start);
        if (freq == 0.0)
        {
            cout << "Error calculating frequency " <<  end << "\n";
            exit (-1);
        }

        result = ((double)1.0)/(freq * multiplier);
    }
    return result;
}
# endif
#endif

static bool useSI = true;

// Return a formatted string after normalising the value into
// engineering style and using a suitable unit prefix (e.g. ms, us, ns).
std::string formatSI(double interval, int width, char unit)
{
    std::stringstream os;

    if (useSI)
    {
        // Preserve accuracy for small numbers, since we only multiply and the positive powers
        // of ten are precisely representable.
        static struct { double scale; char prefix; } ranges[] = {
            {1.e12,'f'},
            {1.e9, 'p'},
            {1.e6, 'n'},
            {1.e3, 'u'},
            {1.0,  'm'},
            {1.e-3,' '},
            {1.e-6,'k'},
            {1.e-9,'M'},
            {1.e-12,'G'},
            {1.e-15,'T'},
            {1.e-18,'P'},
            {1.e-21,'E'},
            {1.e-24,'Z'},
            {1.e-27,'Y'}
        };

        if (interval == 0.0)
        {
            os << std::setw(width-3) << std::right << "0.00" << std::setw(3) << unit;
            return os.str();
        }

        bool negative = false;
        if (interval < 0.0)
        {
            negative = true;
            interval = -interval;
        }

        for (int i=0; i<(int)(sizeof(ranges)/sizeof(ranges[0])); i++)
        {
            if (interval*ranges[i].scale < 1.e0)
            {
                interval = interval * 1000.e0 * ranges[i].scale;
                os << std::fixed << std::setprecision(2) << std::setw(width-3) << std::right <<
                    (negative ? -interval : interval) << std::setw(2) << ranges[i].prefix << std::setw(1) << unit;

                return os.str();
            }
        }
    }
    os << std::setprecision(2) << std::fixed << std::right << std::setw(width-3) << interval << std::setw(3) << unit;

    return os.str();
}
