! include/45/iomp_lib.h.var

! <copyright>
!    Copyright (c) 1985-2016 Intel Corporation.  All Rights Reserved.
!
!    Redistribution and use in source and binary forms, with or without
!    modification, are permitted provided that the following conditions
!    are met:
!
!      * Redistributions of source code must retain the above copyright
!        notice, this list of conditions and the following disclaimer.
!      * Redistributions in binary form must reproduce the above copyright
!        notice, this list of conditions and the following disclaimer in the
!        documentation and/or other materials provided with the distribution.
!      * Neither the name of Intel Corporation nor the names of its
!        contributors may be used to endorse or promote products derived
!        from this software without specific prior written permission.
!
!    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
!    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
!    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
!    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
!    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
!    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
!    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
!    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
!    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
!    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
!    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
!
! </copyright>

!***
!*** omp_integer_kind and omp_logical_kind appear to be predefined by gcc and
!*** gfortran (definitions do not appear in the omp.h / omp_lib.h /omp_lib.f).
!*** omp_real_kind is not predefined, however.
!***

        integer, parameter :: kmp_version_major = 5
        integer, parameter :: kmp_version_minor = 0
        integer, parameter :: kmp_version_build = 20160808
        character(*)          kmp_build_date
        parameter( kmp_build_date = '2017-05-18 12:33:58 UTC' )

        integer, parameter :: omp_real_kind = 4

!***
!*** kmp_* type extensions
!***

        integer, parameter :: kmp_pointer_kind       = 8
        integer, parameter :: kmp_size_t_kind        = 8
        integer, parameter :: kmp_affinity_mask_kind = 8

!***
!*** kmp_* entry points
!***

        external kmp_set_stacksize
        external kmp_set_stacksize_s
        external kmp_set_blocktime
        external kmp_set_library_serial
        external kmp_set_library_turnaround
        external kmp_set_library_throughput
        external kmp_set_library
        external kmp_set_defaults
        external kmp_get_stacksize
        integer kmp_get_stacksize
        external kmp_get_stacksize_s
        integer (kind = kmp_size_t_kind) kmp_get_stacksize_s
        external kmp_get_blocktime
        integer kmp_get_blocktime
        external kmp_get_library
        integer kmp_get_library
        external kmp_set_affinity
        integer kmp_set_affinity
        external kmp_get_affinity
        integer kmp_get_affinity
        external kmp_get_affinity_max_proc
        integer kmp_get_affinity_max_proc
        external kmp_create_affinity_mask
        external kmp_destroy_affinity_mask
        external kmp_set_affinity_mask_proc
        integer kmp_set_affinity_mask_proc
        external kmp_unset_affinity_mask_proc
        integer kmp_unset_affinity_mask_proc
        external kmp_get_affinity_mask_proc
        integer kmp_get_affinity_mask_proc
        external kmp_malloc
        integer (kind = kmp_pointer_kind) kmp_malloc
        external kmp_aligned_malloc
        integer (kind = kmp_pointer_kind) kmp_aligned_malloc
        external kmp_calloc
        integer (kind = kmp_pointer_kind) kmp_calloc
        external kmp_realloc
        integer (kind = kmp_pointer_kind) kmp_realloc
        external kmp_free

        external kmp_set_warnings_on
        external kmp_set_warnings_off


