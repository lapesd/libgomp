/*
 * kmp_version.c
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
#include "kmp_io.h"
#include "kmp_version.h"

// Replace with snapshot date YYYYMMDD for promotion build.
#define KMP_VERSION_BUILD    20160808

// Helper macros to convert value of macro to string literal.
#define _stringer( x ) #x
#define stringer( x )  _stringer( x )

// Detect compiler.
#if KMP_COMPILER_ICC
    #if   __INTEL_COMPILER == 1010
        #define KMP_COMPILER "Intel C++ Compiler 10.1"
    #elif __INTEL_COMPILER == 1100
        #define KMP_COMPILER "Intel C++ Compiler 11.0"
    #elif __INTEL_COMPILER == 1110
        #define KMP_COMPILER "Intel C++ Compiler 11.1"
    #elif __INTEL_COMPILER == 1200
        #define KMP_COMPILER "Intel C++ Compiler 12.0"
    #elif __INTEL_COMPILER == 1210
        #define KMP_COMPILER "Intel C++ Compiler 12.1"
    #elif __INTEL_COMPILER == 1300
        #define KMP_COMPILER "Intel C++ Compiler 13.0"
    #elif __INTEL_COMPILER == 1310
        #define KMP_COMPILER "Intel C++ Compiler 13.1"
    #elif __INTEL_COMPILER == 1400
        #define KMP_COMPILER "Intel C++ Compiler 14.0"
    #elif __INTEL_COMPILER == 1410
        #define KMP_COMPILER "Intel C++ Compiler 14.1"
    #elif __INTEL_COMPILER == 1500
        #define KMP_COMPILER "Intel C++ Compiler 15.0"
    #elif __INTEL_COMPILER == 1600
        #define KMP_COMPILER "Intel C++ Compiler 16.0"
    #elif __INTEL_COMPILER == 9999
        #define KMP_COMPILER "Intel C++ Compiler mainline"
    #endif
#elif KMP_COMPILER_CLANG
    #define KMP_COMPILER "Clang " stringer( __clang_major__ ) "." stringer( __clang_minor__ )
#elif KMP_COMPILER_GCC
    #define KMP_COMPILER "GCC " stringer( __GNUC__ ) "." stringer( __GNUC_MINOR__ )
#elif KMP_COMPILER_MSVC
    #define KMP_COMPILER "MSVC " stringer( _MSC_FULL_VER )
#endif
#ifndef KMP_COMPILER
    #warning "Unknown compiler"
    #define KMP_COMPILER "unknown compiler"
#endif

// Detect librray type (perf, stub).
#ifdef KMP_STUB
    #define KMP_LIB_TYPE "stub"
#else
    #define KMP_LIB_TYPE "performance"
#endif // KMP_LIB_TYPE

// Detect link type (static, dynamic).
#ifdef KMP_DYNAMIC_LIB
    #define KMP_LINK_TYPE "dynamic"
#else
    #define KMP_LINK_TYPE "static"
#endif // KMP_LINK_TYPE

// Finally, define strings.
#define KMP_LIBRARY   KMP_LIB_TYPE " library (" KMP_LINK_TYPE ")"
#define KMP_COPYRIGHT ""

int const __kmp_version_major = KMP_VERSION_MAJOR;
int const __kmp_version_minor = KMP_VERSION_MINOR;
int const __kmp_version_build = KMP_VERSION_BUILD;
int const __kmp_openmp_version =
    #if OMP_40_ENABLED
        201307;
    #else
        201107;
    #endif

/* Do NOT change the format of this string!  Intel(R) Thread Profiler checks for a
   specific format some changes in the recognition routine there need to
   be made before this is changed.
*/
char const __kmp_copyright[] =
    KMP_VERSION_PREFIX KMP_LIBRARY
    " ver. " stringer( KMP_VERSION_MAJOR ) "." stringer( KMP_VERSION_MINOR )
    "." stringer( KMP_VERSION_BUILD ) " "
    KMP_COPYRIGHT;

char const __kmp_version_copyright[]      = KMP_VERSION_PREFIX KMP_COPYRIGHT;
char const __kmp_version_lib_ver[]        = KMP_VERSION_PREFIX "version: " stringer( KMP_VERSION_MAJOR ) "." stringer( KMP_VERSION_MINOR ) "." stringer( KMP_VERSION_BUILD );
char const __kmp_version_lib_type[]       = KMP_VERSION_PREFIX "library type: " KMP_LIB_TYPE;
char const __kmp_version_link_type[]      = KMP_VERSION_PREFIX "link type: " KMP_LINK_TYPE;
char const __kmp_version_build_time[]     = KMP_VERSION_PREFIX "build time: " _KMP_BUILD_TIME;
#if KMP_MIC2
    char const __kmp_version_target_env[] = KMP_VERSION_PREFIX "target environment: MIC2";
#endif
char const __kmp_version_build_compiler[] = KMP_VERSION_PREFIX "build compiler: " KMP_COMPILER;

//
// Called at serial initialization time.
//
static int __kmp_version_1_printed = FALSE;

void
__kmp_print_version_1( void )
{
    if ( __kmp_version_1_printed ) {
        return;
    }; // if
    __kmp_version_1_printed = TRUE;

    #ifndef KMP_STUB
        kmp_str_buf_t buffer;
        __kmp_str_buf_init( & buffer );
        // Print version strings skipping initial magic.
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_lib_ver[ KMP_VERSION_MAGIC_LEN ] );
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_lib_type[ KMP_VERSION_MAGIC_LEN ] );
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_link_type[ KMP_VERSION_MAGIC_LEN ] );
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_build_time[ KMP_VERSION_MAGIC_LEN ] );
      #if KMP_MIC
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_target_env[ KMP_VERSION_MAGIC_LEN ] );
      #endif
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_build_compiler[ KMP_VERSION_MAGIC_LEN ] );
        #if defined(KMP_GOMP_COMPAT)
            __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_alt_comp[ KMP_VERSION_MAGIC_LEN ] );
        #endif /* defined(KMP_GOMP_COMPAT) */
        __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_omp_api[ KMP_VERSION_MAGIC_LEN ] );
        __kmp_str_buf_print( & buffer, "%sdynamic error checking: %s\n", KMP_VERSION_PREF_STR, ( __kmp_env_consistency_check ? "yes" : "no" )  );
        #ifdef KMP_DEBUG
            for ( int i = bs_plain_barrier; i < bs_last_barrier; ++ i ) {
                __kmp_str_buf_print(
                    & buffer,
                    "%s%s barrier branch bits: gather=%u, release=%u\n",
                    KMP_VERSION_PREF_STR,
                    __kmp_barrier_type_name[ i ],
                    __kmp_barrier_gather_branch_bits[ i ],
                    __kmp_barrier_release_branch_bits[ i ]
                ); // __kmp_str_buf_print
            }; // for i
            for ( int i = bs_plain_barrier; i < bs_last_barrier; ++ i ) {
                __kmp_str_buf_print(
                    & buffer,
                    "%s%s barrier pattern: gather=%s, release=%s\n",
                    KMP_VERSION_PREF_STR,
                    __kmp_barrier_type_name[ i ],
                    __kmp_barrier_pattern_name[ __kmp_barrier_gather_pattern[ i ] ],
                    __kmp_barrier_pattern_name[ __kmp_barrier_release_pattern[ i ] ]
                ); // __kmp_str_buf_print
            }; // for i
            __kmp_str_buf_print( & buffer, "%s\n", & __kmp_version_lock[ KMP_VERSION_MAGIC_LEN ] );
        #endif
        __kmp_str_buf_print(
            & buffer,
            "%sthread affinity support: %s\n",
            KMP_VERSION_PREF_STR,
            #if KMP_AFFINITY_SUPPORTED
                (
                    KMP_AFFINITY_CAPABLE()
                    ?
                    (
                        __kmp_affinity_type == affinity_none
                        ?
                        "not used"
                        :
                        "yes"
                    )
                    :
                    "no"
                )
            #else
                "no"
            #endif
        );
        __kmp_printf( "%s", buffer.str );
        __kmp_str_buf_free( & buffer );
        K_DIAG( 1, ( "KMP_VERSION is true\n" ) );
    #endif // KMP_STUB
} // __kmp_print_version_1

//
// Called at parallel initialization time.
//
static int __kmp_version_2_printed = FALSE;

void
__kmp_print_version_2( void ) {
    if ( __kmp_version_2_printed ) {
        return;
    }; // if
    __kmp_version_2_printed = TRUE;
} // __kmp_print_version_2

// end of file //
