/*
 * ompt-event-specific.h -- OMPT events header file.
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

#ifndef  __OMPT_EVENT_SPECIFIC_H__
#define  __OMPT_EVENT_SPECIFIC_H__

/******************************************************************************
 * File: ompt-event-specific.h
 *
 * Description:
 *
 *   specify which of the OMPT events are implemented by this runtime system
 *   and the level of their implementation by a runtime system.
 *****************************************************************************/

#define _ompt_tokenpaste_helper(x,y)        x ## y
#define _ompt_tokenpaste(x,y)               _ompt_tokenpaste_helper(x,y)
#define ompt_event_implementation_status(e) _ompt_tokenpaste(e,_implemented)


/*----------------------------------------------------------------------------
 | Specify whether an event may occur or not, and whether event callbacks
 | never, sometimes, or always occur.
 |
 | The values for these constants are defined in section 6.1.2 of
 | the OMPT TR. They are exposed to tools through ompt_set_callback.
 +--------------------------------------------------------------------------*/

#define ompt_event_NEVER             ompt_set_result_event_never_occurs
#define ompt_event_UNIMPLEMENTED     ompt_set_result_event_may_occur_no_callback
#define ompt_event_MAY_CONVENIENT    ompt_set_result_event_may_occur_callback_some
#define ompt_event_MAY_ALWAYS        ompt_set_result_event_may_occur_callback_always

#if OMPT_TRACE
#define ompt_event_MAY_ALWAYS_TRACE   ompt_event_MAY_ALWAYS
#else
#define ompt_event_MAY_ALWAYS_TRACE   ompt_event_UNIMPLEMENTED
#endif

#if OMPT_BLAME
#define ompt_event_MAY_ALWAYS_BLAME   ompt_event_MAY_ALWAYS
#else
#define ompt_event_MAY_ALWAYS_BLAME   ompt_event_UNIMPLEMENTED
#endif

/*----------------------------------------------------------------------------
 | Mandatory Events
 +--------------------------------------------------------------------------*/

#define ompt_event_parallel_begin_implemented           ompt_event_MAY_ALWAYS
#define ompt_event_parallel_end_implemented             ompt_event_MAY_ALWAYS

#define ompt_event_task_begin_implemented               ompt_event_MAY_ALWAYS
#define ompt_event_task_end_implemented                 ompt_event_MAY_ALWAYS

#define ompt_event_thread_begin_implemented             ompt_event_MAY_ALWAYS
#define ompt_event_thread_end_implemented               ompt_event_MAY_ALWAYS

#define ompt_event_control_implemented                  ompt_event_MAY_ALWAYS

#define ompt_event_runtime_shutdown_implemented         ompt_event_MAY_ALWAYS


/*----------------------------------------------------------------------------
 | Optional Events (blame shifting)
 +--------------------------------------------------------------------------*/

#define ompt_event_idle_begin_implemented               ompt_event_MAY_ALWAYS_BLAME
#define ompt_event_idle_end_implemented                 ompt_event_MAY_ALWAYS_BLAME

#define ompt_event_wait_barrier_begin_implemented       ompt_event_MAY_ALWAYS_BLAME
#define ompt_event_wait_barrier_end_implemented         ompt_event_MAY_ALWAYS_BLAME

#define ompt_event_wait_taskwait_begin_implemented      ompt_event_UNIMPLEMENTED
#define ompt_event_wait_taskwait_end_implemented        ompt_event_UNIMPLEMENTED

#define ompt_event_wait_taskgroup_begin_implemented     ompt_event_UNIMPLEMENTED
#define ompt_event_wait_taskgroup_end_implemented       ompt_event_UNIMPLEMENTED

#define ompt_event_release_lock_implemented             ompt_event_MAY_ALWAYS_BLAME
#define ompt_event_release_nest_lock_last_implemented   ompt_event_MAY_ALWAYS_BLAME
#define ompt_event_release_critical_implemented         ompt_event_MAY_ALWAYS_BLAME
#define ompt_event_release_atomic_implemented           ompt_event_MAY_ALWAYS_BLAME
#define ompt_event_release_ordered_implemented          ompt_event_MAY_ALWAYS_BLAME


/*----------------------------------------------------------------------------
 | Optional Events (synchronous events)
 +--------------------------------------------------------------------------*/

#define ompt_event_implicit_task_begin_implemented      ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_implicit_task_end_implemented        ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_initial_task_begin_implemented       ompt_event_UNIMPLEMENTED
#define ompt_event_initial_task_end_implemented         ompt_event_UNIMPLEMENTED

#define ompt_event_task_switch_implemented              ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_loop_begin_implemented               ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_loop_end_implemented                 ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_sections_begin_implemented           ompt_event_UNIMPLEMENTED
#define ompt_event_sections_end_implemented             ompt_event_UNIMPLEMENTED

#define ompt_event_single_in_block_begin_implemented    ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_single_in_block_end_implemented      ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_single_others_begin_implemented      ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_single_others_end_implemented        ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_workshare_begin_implemented          ompt_event_UNIMPLEMENTED
#define ompt_event_workshare_end_implemented            ompt_event_UNIMPLEMENTED

#define ompt_event_master_begin_implemented             ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_master_end_implemented               ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_barrier_begin_implemented            ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_barrier_end_implemented              ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_taskwait_begin_implemented           ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_taskwait_end_implemented             ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_taskgroup_begin_implemented          ompt_event_UNIMPLEMENTED
#define ompt_event_taskgroup_end_implemented            ompt_event_UNIMPLEMENTED

#define ompt_event_release_nest_lock_prev_implemented   ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_wait_lock_implemented                ompt_event_UNIMPLEMENTED
#define ompt_event_wait_nest_lock_implemented           ompt_event_UNIMPLEMENTED
#define ompt_event_wait_critical_implemented            ompt_event_UNIMPLEMENTED
#define ompt_event_wait_atomic_implemented              ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_wait_ordered_implemented             ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_acquired_lock_implemented            ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_acquired_nest_lock_first_implemented ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_acquired_nest_lock_next_implemented  ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_acquired_critical_implemented        ompt_event_UNIMPLEMENTED
#define ompt_event_acquired_atomic_implemented          ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_acquired_ordered_implemented         ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_init_lock_implemented                ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_init_nest_lock_implemented           ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_destroy_lock_implemented             ompt_event_MAY_ALWAYS_TRACE
#define ompt_event_destroy_nest_lock_implemented        ompt_event_MAY_ALWAYS_TRACE

#define ompt_event_flush_implemented                    ompt_event_UNIMPLEMENTED

#if OMP_40_ENABLED
# define ompt_event_task_dependences_implemented         ompt_event_MAY_ALWAYS_TRACE
# define ompt_event_task_dependence_pair_implemented     ompt_event_MAY_ALWAYS_TRACE
#else
# define ompt_event_task_dependences_implemented         ompt_event_UNIMPLEMENTED
# define ompt_event_task_dependence_pair_implemented     ompt_event_UNIMPLEMENTED
#endif /* OMP_40_ENABLED */

#endif
