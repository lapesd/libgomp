/*
 * kmp_settings.h -- Initialize environment variables
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

#ifndef KMP_SETTINGS_H
#define KMP_SETTINGS_H

void __kmp_reset_global_vars( void );
void __kmp_env_initialize( char const * );
void __kmp_env_print();
#if OMP_40_ENABLED
void __kmp_env_print_2();
#endif // OMP_40_ENABLED

int __kmp_initial_threads_capacity( int req_nproc );
void __kmp_init_dflt_team_nth();
int __kmp_convert_to_milliseconds( char const * );
int __kmp_default_tp_capacity( int, int, int);

#if KMP_MIC
#define KMP_STR_BUF_PRINT_NAME          __kmp_str_buf_print( buffer, "  %s %s", KMP_I18N_STR(Device), name )
#define KMP_STR_BUF_PRINT_NAME_EX(x)    __kmp_str_buf_print( buffer, "  %s %s='", KMP_I18N_STR(Device), x )
#define KMP_STR_BUF_PRINT_BOOL          __kmp_str_buf_print( buffer, "  %s %s='%s'\n", KMP_I18N_STR(Device), name, value ? "TRUE" : "FALSE" );
#define KMP_STR_BUF_PRINT_INT           __kmp_str_buf_print( buffer, "  %s %s='%d'\n", KMP_I18N_STR(Device), name, value )
#define KMP_STR_BUF_PRINT_UINT64        __kmp_str_buf_print( buffer, "  %s %s='%" KMP_UINT64_SPEC "'\n", KMP_I18N_STR(Device), name, value );
#define KMP_STR_BUF_PRINT_STR           __kmp_str_buf_print( buffer, "  %s %s='%s'\n", KMP_I18N_STR(Device), name, value )
#else
#define KMP_STR_BUF_PRINT_NAME          __kmp_str_buf_print( buffer, "  %s %s", KMP_I18N_STR(Host), name )
#define KMP_STR_BUF_PRINT_NAME_EX(x)    __kmp_str_buf_print( buffer, "  %s %s='", KMP_I18N_STR(Host), x )
#define KMP_STR_BUF_PRINT_BOOL          __kmp_str_buf_print( buffer, "  %s %s='%s'\n", KMP_I18N_STR(Host), name, value ? "TRUE" : "FALSE" );
#define KMP_STR_BUF_PRINT_INT           __kmp_str_buf_print( buffer, "  %s %s='%d'\n", KMP_I18N_STR(Host), name, value )
#define KMP_STR_BUF_PRINT_UINT64        __kmp_str_buf_print( buffer, "  %s %s='%" KMP_UINT64_SPEC "'\n", KMP_I18N_STR(Host), name, value );
#define KMP_STR_BUF_PRINT_STR           __kmp_str_buf_print( buffer, "  %s %s='%s'\n", KMP_I18N_STR(Host), name, value )
#endif

#endif // KMP_SETTINGS_H

// end of file //

