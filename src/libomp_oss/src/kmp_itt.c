#if USE_ITT_BUILD
/*
 * kmp_itt.c -- ITT Notify interface.
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

#include "kmp_itt.h"

#if KMP_DEBUG
    #include "kmp_itt.inl"
#endif


#if USE_ITT_NOTIFY

    kmp_int32 __kmp_barrier_domain_count;
    kmp_int32 __kmp_region_domain_count;
    __itt_domain* __kmp_itt_barrier_domains[KMP_MAX_FRAME_DOMAINS];
    __itt_domain* __kmp_itt_region_domains[KMP_MAX_FRAME_DOMAINS];
    __itt_domain* __kmp_itt_imbalance_domains[KMP_MAX_FRAME_DOMAINS];
    kmp_int32 __kmp_itt_region_team_size[KMP_MAX_FRAME_DOMAINS];
    __itt_domain * metadata_domain = NULL;
    __itt_string_handle * string_handle_imbl = NULL;
    __itt_string_handle * string_handle_loop = NULL;
    __itt_string_handle * string_handle_sngl = NULL;

    #include "kmp_version.h"
    #include "kmp_i18n.h"
    #include "kmp_str.h"

    KMP_BUILD_ASSERT( sizeof( kmp_itt_mark_t ) == sizeof( __itt_mark_type ) );

    /*
        Previously used warnings:

        KMP_WARNING( IttAllNotifDisabled );
        KMP_WARNING( IttObjNotifDisabled );
        KMP_WARNING( IttMarkNotifDisabled );
        KMP_WARNING( IttUnloadLibFailed, libittnotify );
    */


    kmp_int32 __kmp_itt_prepare_delay = 0;
    kmp_bootstrap_lock_t __kmp_itt_debug_lock = KMP_BOOTSTRAP_LOCK_INITIALIZER( __kmp_itt_debug_lock );

#endif // USE_ITT_NOTIFY

void __kmp_itt_initialize() {

    // ITTNotify library is loaded and initialized at first call to any ittnotify function,
    // so we do not need to explicitly load it any more.
    // Jusr report OMP RTL version to ITTNotify.

    #if USE_ITT_NOTIFY
        // Report OpenMP RTL version.
        kmp_str_buf_t       buf;
        __itt_mark_type     version;
        __kmp_str_buf_init( & buf );
        __kmp_str_buf_print(
            & buf,
            "OMP RTL Version %d.%d.%d",
            __kmp_version_major,
            __kmp_version_minor,
            __kmp_version_build
        );
        if ( __itt_api_version_ptr != NULL ) {
            __kmp_str_buf_print( & buf, ":%s", __itt_api_version() );
        }; // if
        version = __itt_mark_create( buf.str );
        __itt_mark( version, NULL );
        __kmp_str_buf_free( & buf );
    #endif

} // __kmp_itt_initialize


void __kmp_itt_destroy() {
    #if USE_ITT_NOTIFY
        __kmp_itt_fini_ittlib();
    #endif
} // __kmp_itt_destroy


extern "C"
void
__itt_error_handler(
    __itt_error_code err,
    va_list args
) {

    switch ( err ) {
        case __itt_error_no_module : {
            char const * library = va_arg( args, char const * );
            #if KMP_OS_WINDOWS
                int sys_err = va_arg( args, int );
                __kmp_msg( kmp_ms_warning, KMP_MSG( IttLoadLibFailed, library ), KMP_SYSERRCODE( sys_err ), __kmp_msg_null );
            #else
                char const * sys_err = va_arg( args, char const * );
                __kmp_msg( kmp_ms_warning, KMP_MSG( IttLoadLibFailed, library ), KMP_SYSERRMESG( sys_err ), __kmp_msg_null );
            #endif
        } break;
        case __itt_error_no_symbol : {
            char const * library = va_arg( args, char const * );
            char const * symbol  = va_arg( args, char const * );
            KMP_WARNING( IttLookupFailed, symbol, library );
        } break;
        case __itt_error_unknown_group : {
            char const * var   = va_arg( args, char const * );
            char const * group = va_arg( args, char const * );
            KMP_WARNING( IttUnknownGroup, var, group );
        } break;
        case __itt_error_env_too_long : {
            char const * var     = va_arg( args, char const * );
            size_t       act_len = va_arg( args, size_t );
            size_t       max_len = va_arg( args, size_t );
            KMP_WARNING( IttEnvVarTooLong, var, (unsigned long) act_len, (unsigned long) max_len );
        } break;
        case __itt_error_cant_read_env : {
            char const * var     = va_arg( args, char const * );
            int          sys_err = va_arg( args, int );
            __kmp_msg( kmp_ms_warning, KMP_MSG( CantGetEnvVar, var ), KMP_ERR( sys_err ), __kmp_msg_null );
        } break;
        case __itt_error_system : {
            char const * func    = va_arg( args, char const * );
            int          sys_err = va_arg( args, int );
            __kmp_msg( kmp_ms_warning, KMP_MSG( IttFunctionError, func ), KMP_SYSERRCODE( sys_err ), __kmp_msg_null );
        } break;
        default : {
            KMP_WARNING( IttUnknownError, err );
        };
    }; // switch

} // __itt_error_handler

#endif /* USE_ITT_BUILD */
