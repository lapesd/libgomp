/*
 * kmp_wrapper_getpid.h -- getpid() declaration.
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

#ifndef KMP_WRAPPER_GETPID_H
#define KMP_WRAPPER_GETPID_H

#if KMP_OS_UNIX

    // On Unix-like systems (Linux* OS and OS X*) getpid() is declared in standard headers.
    #include <sys/types.h>
    #include <unistd.h>

#elif KMP_OS_WINDOWS

    // On Windows* OS _getpid() returns int (not pid_t) and is declared in "process.h".
    #include <process.h>
    // Let us simulate Unix.
    typedef int pid_t;
    #define getpid _getpid

#else

    #error Unknown or unsupported OS.

#endif

/*
    TODO: All the libomp source code uses pid_t type for storing the result of getpid(), it is good.
    But often it printed as "%d", that is not good, because it ignores pid_t definition (may pid_t
    be longer that int?). It seems all pid prints should be rewritten as

        printf( "%" KMP_UINT64_SPEC, (kmp_uint64) pid );

    or (at least) as

        printf( "%" KMP_UINT32_SPEC, (kmp_uint32) pid );

    (kmp_uint32, kmp_uint64, KMP_UINT64_SPEC, and KMP_UNIT32_SPEC are defined in "kmp_os.h".)

*/

#endif // KMP_WRAPPER_GETPID_H

// end of file //
