/*
 * kmp_import.c
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

/*
    ------------------------------------------------------------------------------------------------
    Object generated from this source file is linked to Windows* OS DLL import library (libiomp5md.lib)
    only! It is not a part of regular static or dynamic OpenMP RTL. Any code that just needs to go
    in the libiomp5md.lib (but not in libiomp5mt.lib and libiomp5md.dll) should be placed in this
    file.
    ------------------------------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
    These symbols are required for mutual exclusion with Microsoft OpenMP RTL (and compatibility
    with MS Compiler).
*/

int _You_must_link_with_exactly_one_OpenMP_library = 1;
int _You_must_link_with_Intel_OpenMP_library       = 1;
int _You_must_link_with_Microsoft_OpenMP_library = 1;

#ifdef __cplusplus
}
#endif

// end of file //
