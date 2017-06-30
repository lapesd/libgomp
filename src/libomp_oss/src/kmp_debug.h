/*
 * kmp_debug.h -- debug / assertion code for Assure library
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

#ifndef KMP_DEBUG_H
#define KMP_DEBUG_H

#include <stdarg.h>

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

// -------------------------------------------------------------------------------------------------
// Build-time assertion.
// -------------------------------------------------------------------------------------------------

/*
    Build-time assertion can do compile-time checking of data structure sizes, etc. This works by
    declaring a negative-length array if the conditional expression evaluates to false.  In that
    case, the compiler issues a syntax error and stops the compilation. If the expression is
    true, we get an extraneous static single character array in the scope of the macro.

    Usage:

        KMP_BUILD_ASSERT( sizeof( some_t ) <= 32 );
        KMP_BUILD_ASSERT( offsetof( some_t, field ) % 8 == 0 );

    Do not use _KMP_BUILD_ASSERT and __KMP_BUILD_ASSERT directly, it is working guts.
*/

#define __KMP_BUILD_ASSERT( expr, suffix )  typedef char __kmp_build_check_##suffix[ (expr) ? 1 : -1 ]
#define _KMP_BUILD_ASSERT( expr, suffix )   __KMP_BUILD_ASSERT( (expr), suffix )
#define KMP_BUILD_ASSERT( expr )            _KMP_BUILD_ASSERT( (expr), __LINE__ )

// -------------------------------------------------------------------------------------------------
// Run-time assertions.
// -------------------------------------------------------------------------------------------------

extern void __kmp_dump_debug_buffer( void );

#ifdef KMP_USE_ASSERT
    extern int __kmp_debug_assert( char const * expr, char const * file, int line );
    #ifdef KMP_DEBUG
        #define KMP_ASSERT( cond )             ( (cond) ? 0 : __kmp_debug_assert( #cond, __FILE__, __LINE__ ) )
        #define KMP_ASSERT2( cond, msg )       ( (cond) ? 0 : __kmp_debug_assert( (msg), __FILE__, __LINE__ ) )
        #define KMP_DEBUG_ASSERT( cond )       KMP_ASSERT( cond )
        #define KMP_DEBUG_ASSERT2( cond, msg ) KMP_ASSERT2( cond, msg )
    #else
        // Do not expose condition in release build. Use "assertion failure".
        #define KMP_ASSERT( cond )             ( (cond) ? 0 : __kmp_debug_assert( "assertion failure", __FILE__, __LINE__ ) )
        #define KMP_ASSERT2( cond, msg )       KMP_ASSERT( cond )
        #define KMP_DEBUG_ASSERT( cond )       0
        #define KMP_DEBUG_ASSERT2( cond, msg ) 0
    #endif // KMP_DEBUG
#else
    #define KMP_ASSERT( cond )             0
    #define KMP_ASSERT2( cond, msg )       0
    #define KMP_DEBUG_ASSERT( cond )       0
    #define KMP_DEBUG_ASSERT2( cond, msg ) 0
#endif // KMP_USE_ASSERT

#ifdef KMP_DEBUG
    extern void __kmp_debug_printf_stdout( char const * format, ... );
#endif
extern void __kmp_debug_printf( char const * format, ... );

#ifdef KMP_DEBUG

    extern int kmp_a_debug;
    extern int kmp_b_debug;
    extern int kmp_c_debug;
    extern int kmp_d_debug;
    extern int kmp_e_debug;
    extern int kmp_f_debug;
    extern int kmp_diag;

    #define KA_TRACE(d,x)     if (kmp_a_debug >= d) { __kmp_debug_printf x ; }
    #define KB_TRACE(d,x)     if (kmp_b_debug >= d) { __kmp_debug_printf x ; }
    #define KC_TRACE(d,x)     if (kmp_c_debug >= d) { __kmp_debug_printf x ; }
    #define KD_TRACE(d,x)     if (kmp_d_debug >= d) { __kmp_debug_printf x ; }
    #define KE_TRACE(d,x)     if (kmp_e_debug >= d) { __kmp_debug_printf x ; }
    #define KF_TRACE(d,x)     if (kmp_f_debug >= d) { __kmp_debug_printf x ; }
    #define K_DIAG(d,x)       {if (kmp_diag == d) { __kmp_debug_printf_stdout x ; } }

    #define KA_DUMP(d,x)     if (kmp_a_debug >= d) { int ks; __kmp_disable(&ks); (x) ; __kmp_enable(ks); }
    #define KB_DUMP(d,x)     if (kmp_b_debug >= d) { int ks; __kmp_disable(&ks); (x) ; __kmp_enable(ks); }
    #define KC_DUMP(d,x)     if (kmp_c_debug >= d) { int ks; __kmp_disable(&ks); (x) ; __kmp_enable(ks); }
    #define KD_DUMP(d,x)     if (kmp_d_debug >= d) { int ks; __kmp_disable(&ks); (x) ; __kmp_enable(ks); }
    #define KE_DUMP(d,x)     if (kmp_e_debug >= d) { int ks; __kmp_disable(&ks); (x) ; __kmp_enable(ks); }
    #define KF_DUMP(d,x)     if (kmp_f_debug >= d) { int ks; __kmp_disable(&ks); (x) ; __kmp_enable(ks); }

#else

    #define KA_TRACE(d,x)     /* nothing to do */
    #define KB_TRACE(d,x)     /* nothing to do */
    #define KC_TRACE(d,x)     /* nothing to do */
    #define KD_TRACE(d,x)     /* nothing to do */
    #define KE_TRACE(d,x)     /* nothing to do */
    #define KF_TRACE(d,x)     /* nothing to do */
    #define K_DIAG(d,x)       {}/* nothing to do */

    #define KA_DUMP(d,x)     /* nothing to do */
    #define KB_DUMP(d,x)     /* nothing to do */
    #define KC_DUMP(d,x)     /* nothing to do */
    #define KD_DUMP(d,x)     /* nothing to do */
    #define KE_DUMP(d,x)     /* nothing to do */
    #define KF_DUMP(d,x)     /* nothing to do */

#endif // KMP_DEBUG

#ifdef __cplusplus
    } // extern "C"
#endif // __cplusplus

#endif /* KMP_DEBUG_H */
