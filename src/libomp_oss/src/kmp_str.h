/*
 * kmp_str.h -- String manipulation routines.
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

#ifndef KMP_STR_H
#define KMP_STR_H

#include <string.h>
#include <stdarg.h>

#include "kmp_os.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#if KMP_OS_WINDOWS
# define strdup    _strdup
#endif

/*  some macros to replace ctype.h functions  */
#define TOLOWER(c)	((((c) >= 'A') && ((c) <= 'Z')) ? ((c) + 'a' - 'A') : (c))

struct kmp_str_buf {
    char       * str;         // Pointer to buffer content, read only.
    unsigned int size;        // Do not change this field!
    int          used;        // Number of characters printed to buffer, read only.
    char         bulk[ 512 ]; // Do not use this field!
}; // struct kmp_str_buf
typedef struct kmp_str_buf  kmp_str_buf_t;

#define __kmp_str_buf_init( b )   { (b)->str = (b)->bulk; (b)->size = sizeof( (b)->bulk ); (b)->used = 0; (b)->bulk[ 0 ] = 0; }

void   __kmp_str_buf_clear( kmp_str_buf_t * buffer );
void   __kmp_str_buf_reserve( kmp_str_buf_t * buffer, int size );
void   __kmp_str_buf_detach( kmp_str_buf_t * buffer );
void   __kmp_str_buf_free( kmp_str_buf_t * buffer );
void   __kmp_str_buf_cat( kmp_str_buf_t * buffer, char const * str, int len );
void   __kmp_str_buf_vprint( kmp_str_buf_t * buffer, char const * format, va_list args );
void   __kmp_str_buf_print( kmp_str_buf_t * buffer, char const * format, ... );
void   __kmp_str_buf_print_size( kmp_str_buf_t * buffer, size_t size );

/*
    File name parser. Usage:

        kmp_str_fname_t fname = __kmp_str_fname_init( path );
        // Use fname.path (copy of original path ), fname.dir, fname.base.
        // Note fname.dir concatenated with fname.base gives exact copy of path.
        __kmp_str_fname_free( & fname );

*/
struct kmp_str_fname {
    char * path;
    char * dir;
    char * base;
}; // struct kmp_str_fname
typedef struct kmp_str_fname kmp_str_fname_t;
void __kmp_str_fname_init( kmp_str_fname_t * fname, char const * path );
void __kmp_str_fname_free( kmp_str_fname_t * fname );
// Compares file name with specified patern. If pattern is NULL, any fname matched.
int __kmp_str_fname_match( kmp_str_fname_t const * fname, char const * pattern );

/*
    The compiler provides source locations in string form ";file;func;line;col;;". It not not
    convenient for manupulation. These structure keeps source location in more convenient form.
    Usage:

        kmp_str_loc_t loc = __kmp_str_loc_init( ident->psource, 0 );
        // use loc.file, loc.func, loc.line, loc.col.
        // loc.fname is available if the second argument of __kmp_str_loc_init is true.
        __kmp_str_loc_free( & loc );

    If psource is NULL or does not follow format above, file and/or func may be NULL pointers.
*/
struct kmp_str_loc {
    char *          _bulk;  // Do not use thid field.
    kmp_str_fname_t fname;  // Will be initialized if init_fname is true.
    char *          file;
    char *          func;
    int             line;
    int             col;
}; // struct kmp_str_loc
typedef struct kmp_str_loc kmp_str_loc_t;
kmp_str_loc_t __kmp_str_loc_init( char const * psource, int init_fname );
void __kmp_str_loc_free( kmp_str_loc_t * loc );

int    __kmp_str_eqf( char const * lhs, char const * rhs );
char * __kmp_str_format( char const * format, ... );
void   __kmp_str_free( char const * * str );
int    __kmp_str_match( char const * target, int len, char const * data );
int    __kmp_str_match_false( char const * data );
int    __kmp_str_match_true( char const * data );
void   __kmp_str_replace( char * str, char search_for, char replace_with );
void   __kmp_str_split( char * str, char delim, char ** head, char ** tail );
char * __kmp_str_token( char * str, char const * delim, char ** buf );
int    __kmp_str_to_int( char const * str, char sentinel );

void __kmp_str_to_size( char const * str, size_t * out, size_t dfactor, char const * * error );
void __kmp_str_to_uint( char const * str, kmp_uint64 * out, char const * * error );

#ifdef __cplusplus
    } // extern "C"
#endif // __cplusplus

#endif // KMP_STR_H

// end of file //

