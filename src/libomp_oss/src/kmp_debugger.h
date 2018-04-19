#if USE_DEBUGGER
/*
 * kmp_debugger.h -- debugger support.
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

#ifndef KMP_DEBUGGER_H
#define KMP_DEBUGGER_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

/* * This external variable can be set by any debugger to flag to the runtime that we
   are currently executing inside a debugger.  This will allow the debugger to override
   the number of threads spawned in a parallel region by using __kmp_omp_num_threads() (below).
   * When __kmp_debugging is TRUE, each team and each task gets a unique integer identifier
   that can be used by debugger to conveniently identify teams and tasks.
   * The debugger has access to __kmp_omp_debug_struct_info which contains information
   about the OpenMP library's important internal structures.  This access will allow the debugger
   to read detailed information from the typical OpenMP constructs (teams, threads, tasking, etc. )
   during a debugging session and offer detailed and useful information which the user can probe
   about the OpenMP portion of their code.
   */
extern int __kmp_debugging;             /* Boolean whether currently debugging OpenMP RTL */
// Return number of threads specified by the debugger for given parallel region.
/* The ident field, which represents a source file location, is used to check if the
   debugger has changed the number of threads for the parallel region at source file
   location ident.  This way, specific parallel regions' number of threads can be changed
   at the debugger's request.
 */
int __kmp_omp_num_threads( ident_t const * ident );

#ifdef __cplusplus
    } // extern "C"
#endif // __cplusplus


#endif // KMP_DEBUGGER_H

#endif // USE_DEBUGGER
