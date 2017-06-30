/*
 * z_Windows_NT-586_util.c -- platform specific routines.
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

#include "kmp.h"

#if (KMP_ARCH_X86 || KMP_ARCH_X86_64)
/* Only 32-bit "add-exchange" instruction on IA-32 architecture causes us to
 * use compare_and_store for these routines
 */

kmp_int8
__kmp_test_then_or8( volatile kmp_int8 *p, kmp_int8 d )
{
    kmp_int8 old_value, new_value;

    old_value = TCR_1( *p );
    new_value = old_value | d;

    while ( ! __kmp_compare_and_store8 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_1( *p );
        new_value = old_value | d;
    }
    return old_value;
}

kmp_int8
__kmp_test_then_and8( volatile kmp_int8 *p, kmp_int8 d )
{
    kmp_int8 old_value, new_value;

    old_value = TCR_1( *p );
    new_value = old_value & d;

    while ( ! __kmp_compare_and_store8 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_1( *p );
        new_value = old_value & d;
    }
    return old_value;
}

kmp_int32
__kmp_test_then_or32( volatile kmp_int32 *p, kmp_int32 d )
{
    kmp_int32 old_value, new_value;

    old_value = TCR_4( *p );
    new_value = old_value | d;

    while ( ! __kmp_compare_and_store32 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_4( *p );
        new_value = old_value | d;
    }
    return old_value;
}

kmp_int32
__kmp_test_then_and32( volatile kmp_int32 *p, kmp_int32 d )
{
    kmp_int32 old_value, new_value;

    old_value = TCR_4( *p );
    new_value = old_value & d;

    while ( ! __kmp_compare_and_store32 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_4( *p );
        new_value = old_value & d;
    }
    return old_value;
}

kmp_int8
__kmp_test_then_add8( volatile kmp_int8 *p, kmp_int8 d )
{
    kmp_int64 old_value, new_value;

    old_value = TCR_1( *p );
    new_value = old_value + d;
    while ( ! __kmp_compare_and_store8 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_1( *p );
        new_value = old_value + d;
    }
    return old_value;
}

#if KMP_ARCH_X86
kmp_int64
__kmp_test_then_add64( volatile kmp_int64 *p, kmp_int64 d )
{
    kmp_int64 old_value, new_value;

    old_value = TCR_8( *p );
    new_value = old_value + d;
    while ( ! __kmp_compare_and_store64 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_8( *p );
        new_value = old_value + d;
    }
    return old_value;
}
#endif /* KMP_ARCH_X86 */

kmp_int64
__kmp_test_then_or64( volatile kmp_int64 *p, kmp_int64 d )
{
    kmp_int64 old_value, new_value;

    old_value = TCR_8( *p );
    new_value = old_value | d;
    while ( ! __kmp_compare_and_store64 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_8( *p );
        new_value = old_value | d;
    }

    return old_value;
}

kmp_int64
__kmp_test_then_and64( volatile kmp_int64 *p, kmp_int64 d )
{
    kmp_int64 old_value, new_value;

    old_value = TCR_8( *p );
    new_value = old_value & d;
    while ( ! __kmp_compare_and_store64 ( p, old_value, new_value ) )
    {
        KMP_CPU_PAUSE();
        old_value = TCR_8( *p );
        new_value = old_value & d;
    }

    return old_value;
}

#endif /* KMP_ARCH_X86 || KMP_ARCH_X86_64 */

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

