/*
 * kmp_stub.h
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

#ifndef KMP_STUB_H
#define KMP_STUB_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

void __kmps_set_blocktime( int arg );
int  __kmps_get_blocktime( void );
void __kmps_set_dynamic( int arg );
int  __kmps_get_dynamic( void );
void __kmps_set_library( int arg );
int  __kmps_get_library( void );
void __kmps_set_nested( int arg );
int  __kmps_get_nested( void );
void __kmps_set_stacksize( int arg );
int  __kmps_get_stacksize();

#ifndef KMP_SCHED_TYPE_DEFINED
#define KMP_SCHED_TYPE_DEFINED
typedef enum kmp_sched {
    kmp_sched_static            = 1, // mapped to kmp_sch_static_chunked           (33)
    kmp_sched_dynamic           = 2, // mapped to kmp_sch_dynamic_chunked          (35)
    kmp_sched_guided            = 3, // mapped to kmp_sch_guided_chunked           (36)
    kmp_sched_auto              = 4, // mapped to kmp_sch_auto                     (38)
    kmp_sched_default = kmp_sched_static   // default scheduling
} kmp_sched_t;
#endif
void __kmps_set_schedule( kmp_sched_t kind, int modifier );
void __kmps_get_schedule( kmp_sched_t *kind, int *modifier );

#if OMP_40_ENABLED
void __kmps_set_proc_bind( kmp_proc_bind_t arg );
kmp_proc_bind_t __kmps_get_proc_bind( void );
#endif /* OMP_40_ENABLED */

double __kmps_get_wtime();
double __kmps_get_wtick();

#ifdef __cplusplus
    } // extern "C"
#endif // __cplusplus

#endif // KMP_STUB_H

// end of file //
