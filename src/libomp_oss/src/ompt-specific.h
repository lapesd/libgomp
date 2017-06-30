/*
 * ompt-specific.h -- OMPT specific header file.
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

#ifndef OMPT_SPECIFIC_H
#define OMPT_SPECIFIC_H

#include "kmp.h"

/*****************************************************************************
 * types
 ****************************************************************************/

typedef kmp_info_t ompt_thread_t;



/*****************************************************************************
 * forward declarations
 ****************************************************************************/

void __ompt_team_assign_id(kmp_team_t *team, ompt_parallel_id_t ompt_pid);
void __ompt_thread_assign_wait_id(void *variable);

void __ompt_lw_taskteam_init(ompt_lw_taskteam_t *lwt, ompt_thread_t *thr,
                             int gtid, void *microtask,
                             ompt_parallel_id_t ompt_pid);

void __ompt_lw_taskteam_link(ompt_lw_taskteam_t *lwt,  ompt_thread_t *thr);

ompt_lw_taskteam_t * __ompt_lw_taskteam_unlink(ompt_thread_t *thr);

ompt_parallel_id_t __ompt_parallel_id_new(int gtid);
ompt_task_id_t __ompt_task_id_new(int gtid);

ompt_team_info_t *__ompt_get_teaminfo(int depth, int *size);

ompt_task_info_t *__ompt_get_taskinfo(int depth);

void __ompt_thread_begin(ompt_thread_type_t thread_type, int gtid);

void __ompt_thread_end(ompt_thread_type_t thread_type, int gtid);

int __ompt_get_parallel_team_size_internal(int ancestor_level);

ompt_task_id_t __ompt_get_task_id_internal(int depth);

ompt_frame_t *__ompt_get_task_frame_internal(int depth);



/*****************************************************************************
 * macros
 ****************************************************************************/
#ifdef KMP_DEBUG
# define OMPT_DEBUG 1
#endif
#define OMPT_STR_MATCH(haystack, needle) __kmp_str_match(haystack, 0, needle)



//******************************************************************************
// inline functions
//******************************************************************************

inline ompt_thread_t *
ompt_get_thread_gtid(int gtid)
{
    return (gtid >= 0) ? __kmp_thread_from_gtid(gtid) : NULL;
}


inline ompt_thread_t *
ompt_get_thread()
{
    int gtid = __kmp_get_gtid();
    return ompt_get_thread_gtid(gtid);
}


inline void
ompt_set_thread_state(ompt_thread_t *thread, ompt_state_t state)
{
    thread->th.ompt_thread_info.state = state;
}


inline const char *
ompt_get_runtime_version()
{
    return &__kmp_version_lib_ver[KMP_VERSION_MAGIC_LEN];
}

#endif
