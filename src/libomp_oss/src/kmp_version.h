/*
 * kmp_version.h -- version number for this release
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

#ifndef KMP_VERSION_H
#define KMP_VERSION_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#ifndef KMP_VERSION_MAJOR
    #error KMP_VERSION_MAJOR macro is not defined.
#endif
#define KMP_VERSION_MINOR       0
/*
    Using "magic" prefix in all the version strings is rather convenient to get static version info
    from binaries by using standard utilities "strings" and "grep", e. g.:
        $ strings libiomp5.so | grep "@(#)"
    gives clean list of all version strings in the library. Leading zero helps to keep version
    string separate from printable characters which may occurs just before version string.
*/
#define KMP_VERSION_MAGIC_STR   "\x00@(#) "
#define KMP_VERSION_MAGIC_LEN   6                // Length of KMP_VERSION_MAGIC_STR.
#define KMP_VERSION_PREF_STR    "Intel(R) OMP "
#define KMP_VERSION_PREFIX      KMP_VERSION_MAGIC_STR KMP_VERSION_PREF_STR

/* declare all the version string constants for KMP_VERSION env. variable */
extern int  const __kmp_version_major;
extern int  const __kmp_version_minor;
extern int  const __kmp_version_build;
extern int  const __kmp_openmp_version;
extern char const __kmp_copyright[];    // Old variable, kept for compatibility with ITC and ITP.
extern char const __kmp_version_copyright[];
extern char const __kmp_version_lib_ver[];
extern char const __kmp_version_lib_type[];
extern char const __kmp_version_link_type[];
extern char const __kmp_version_build_time[];
extern char const __kmp_version_target_env[];
extern char const __kmp_version_build_compiler[];
extern char const __kmp_version_alt_comp[];
extern char const __kmp_version_omp_api[];
// ??? extern char const __kmp_version_debug[];
extern char const __kmp_version_lock[];
extern char const __kmp_version_nested_stats_reporting[];
extern char const __kmp_version_ftnstdcall[];
extern char const __kmp_version_ftncdecl[];
extern char const __kmp_version_ftnextra[];

void __kmp_print_version_1( void );
void __kmp_print_version_2( void );

#ifdef __cplusplus
    } // extern "C"
#endif // __cplusplus

#endif /* KMP_VERSION_H */
