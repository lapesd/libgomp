/*
 * kmp_environment.h -- Handle environment varoiables OS-independently.
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

#ifndef KMP_ENVIRONMENT_H
#define KMP_ENVIRONMENT_H

#ifdef __cplusplus
extern "C" {
#endif

// Return a copy of the value of environment variable or NULL if the variable does not exist.
// *Note*: Returned pointed *must* be freed after use with __kmp_env_free().
char * __kmp_env_get( char const * name );
void   __kmp_env_free( char const * * value );

// Return 1 if the environment variable exists or 0 if does not exist.
int __kmp_env_exists( char const * name );

// Set the environment variable.
void __kmp_env_set( char const * name, char const * value, int overwrite );

// Unset (remove) environment variable.
void __kmp_env_unset( char const * name );


// -------------------------------------------------------------------------------------------------
//  Working with environment blocks.
// -------------------------------------------------------------------------------------------------

/*
    kmp_env_blk_t is read-only collection of environment variables (or environment-like). Usage:

        kmp_env_blk_t block;
        __kmp_env_blk_init( & block, NULL ); // Initialize block from process environment.
        // or
        __kmp_env_blk_init( & block, "KMP_WARNING=1|KMP_AFFINITY=none" ); // from string.
        __kmp_env_blk_sort( & block ); // Optionally, sort list.
        for ( i = 0; i < block.count; ++ i ) {
            // Process block.vars[ i ].name and block.vars[ i ].value...
        }; // for i
        __kmp_env_block_free( & block );
*/

struct __kmp_env_var {
    char const * name;
    char const * value;
};
typedef struct __kmp_env_var kmp_env_var_t;

struct __kmp_env_blk {
    char const *          bulk;
    kmp_env_var_t const * vars;
    int                   count;
};
typedef struct __kmp_env_blk kmp_env_blk_t;

void         __kmp_env_blk_init( kmp_env_blk_t * block, char const * bulk );
void         __kmp_env_blk_free( kmp_env_blk_t * block );
void         __kmp_env_blk_sort( kmp_env_blk_t * block );
char const * __kmp_env_blk_var(  kmp_env_blk_t * block, char const * name );

#ifdef __cplusplus
}
#endif

#endif // KMP_ENVIRONMENT_H

// end of file //

