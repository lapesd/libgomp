/*
 * kmp_error.h -- PTS functions for error checking at runtime.
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

#ifndef KMP_ERROR_H
#define KMP_ERROR_H

#include "kmp_i18n.h"

/* ------------------------------------------------------------------------ */
#ifdef __cplusplus
    extern "C" {
#endif

void __kmp_error_construct(  kmp_i18n_id_t id, enum cons_type ct, ident_t const * ident );
void __kmp_error_construct2( kmp_i18n_id_t id, enum cons_type ct, ident_t const * ident, struct cons_data const * cons );

struct cons_header * __kmp_allocate_cons_stack( int gtid );
void                 __kmp_free_cons_stack( void * ptr );

void __kmp_push_parallel( int gtid, ident_t const * ident );
void __kmp_push_workshare( int gtid, enum cons_type ct, ident_t const * ident );
#if KMP_USE_DYNAMIC_LOCK
void __kmp_push_sync( int gtid, enum cons_type ct, ident_t const * ident, kmp_user_lock_p name, kmp_uint32 );
#else
void __kmp_push_sync( int gtid, enum cons_type ct, ident_t const * ident, kmp_user_lock_p name );
#endif

void __kmp_check_workshare( int gtid, enum cons_type ct, ident_t const * ident );
#if KMP_USE_DYNAMIC_LOCK
void __kmp_check_sync( int gtid, enum cons_type ct, ident_t const * ident, kmp_user_lock_p name, kmp_uint32 );
#else
void __kmp_check_sync( int gtid, enum cons_type ct, ident_t const * ident, kmp_user_lock_p name );
#endif

void __kmp_pop_parallel( int gtid, ident_t const * ident );
enum cons_type __kmp_pop_workshare( int gtid, enum cons_type ct, ident_t const * ident );
void __kmp_pop_sync( int gtid, enum cons_type ct, ident_t const * ident );
void __kmp_check_barrier( int gtid, enum cons_type ct, ident_t const * ident );

#ifdef __cplusplus
    } // extern "C"
#endif

#endif // KMP_ERROR_H

