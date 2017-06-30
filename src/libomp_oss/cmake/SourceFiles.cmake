# <copyright>
#    Copyright (c) 2013-2016 Intel Corporation.  All Rights Reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions
#    are met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of Intel Corporation nor the names of its
#        contributors may be used to endorse or promote products derived
#        from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# </copyright>

macro(append_c_source_file new_c_file)
    list(APPEND local_c_source_files "${new_c_file}")
endmacro()

macro(append_cpp_source_file new_cpp_file)
    list(APPEND local_cpp_source_files "${new_cpp_file}")
endmacro()

macro(append_asm_source_file new_asm_file)
    list(APPEND local_asm_source_files "${new_asm_file}")
endmacro()

macro(append_imp_c_source_file new_import_c_file)
    list(APPEND local_imp_c_files "${new_import_c_file}")
endmacro()

# files are relative to the src directory

function(set_c_files input_c_source_files)
    set(local_c_source_files "")
    append_c_source_file("kmp_ftn_cdecl.c")
    append_c_source_file("kmp_ftn_extra.c")
    append_c_source_file("kmp_version.c")
    if(${STUBS_LIBRARY})
        append_c_source_file("kmp_stub.c")
    else()
        append_c_source_file("kmp_alloc.c")
        append_c_source_file("kmp_atomic.c")
        append_c_source_file("kmp_csupport.c")
        append_c_source_file("kmp_debug.c")
        append_c_source_file("kmp_debugger.c")
        append_c_source_file("kmp_itt.c")
        append_c_source_file("kmp_environment.c")
        append_c_source_file("kmp_error.c")
        append_c_source_file("kmp_global.c")
        append_c_source_file("kmp_i18n.c")
        append_c_source_file("kmp_io.c")
        append_c_source_file("kmp_runtime.c")
        append_c_source_file("kmp_settings.c")
        append_c_source_file("kmp_str.c")
        append_c_source_file("kmp_tasking.c")
        append_c_source_file("kmp_taskq.c")
        append_c_source_file("kmp_threadprivate.c")
        append_c_source_file("kmp_utility.c")
        if(${LIBOMP_OMPT_SUPPORT})
            append_c_source_file("ompt-general.c")
        endif()
        if(${LIBOMP_USE_ITT_NOTIFY})
            append_c_source_file("thirdparty/ittnotify/ittnotify_static.c")
        endif()
        if(${WINDOWS})
            append_c_source_file("z_Windows_NT_util.c")
            append_c_source_file("z_Windows_NT-586_util.c")
        else()
            append_c_source_file("z_Linux_util.c")
            append_c_source_file("kmp_gsupport.c")
            append_c_source_file("thirdparty/safeclib/ignore_handler_s.c")
            append_c_source_file("thirdparty/safeclib/mem_primitives_lib.c")
            append_c_source_file("thirdparty/safeclib/memcpy_s.c")
            append_c_source_file("thirdparty/safeclib/safe_mem_constraint.c")
            append_c_source_file("thirdparty/safeclib/safe_str_constraint.c")
            append_c_source_file("thirdparty/safeclib/strcpy_s.c")
            append_c_source_file("thirdparty/safeclib/strncpy_s.c")
            append_c_source_file("thirdparty/safeclib/strnlen_s.c")
            append_c_source_file("thirdparty/safeclib/snprintf_support.c")
        endif()
    endif()
    set(${input_c_source_files} "${local_c_source_files}" PARENT_SCOPE)
endfunction()

function(set_cpp_files input_cpp_source_files)
    set(local_cpp_source_files "")
    if(NOT ${STUBS_LIBRARY})
        append_cpp_source_file("kmp_barrier.cpp")
        append_cpp_source_file("kmp_wait_release.cpp")
        append_cpp_source_file("kmp_affinity.cpp")
        append_cpp_source_file("kmp_dispatch.cpp")
        append_cpp_source_file("kmp_lock.cpp")
        append_cpp_source_file("kmp_sched.cpp")
        if(${LIBOMP_OMP_VERSION} GREATER 40 OR ${LIBOMP_OMP_VERSION} EQUAL 40)
            append_cpp_source_file("kmp_taskdeps.cpp")
            append_cpp_source_file("kmp_cancel.cpp")
        endif()
        if(${LIBOMP_STATS})
            append_cpp_source_file("kmp_stats.cpp")
            append_cpp_source_file("kmp_stats_timing.cpp")
        endif()
    endif()

    set(${input_cpp_source_files} "${local_cpp_source_files}" PARENT_SCOPE)
endfunction()


function(set_asm_files input_asm_source_files)
    set(local_asm_source_files "")
    if(NOT ${STUBS_LIBRARY})
        if(${WINDOWS})
            append_asm_source_file("z_Windows_NT-586_asm.asm")
        else()
            append_asm_source_file("z_Linux_asm.s")
        endif()
    endif()
    set(${input_asm_source_files} "${local_asm_source_files}" PARENT_SCOPE)
endfunction()


function(set_imp_c_files input_imp_c_files)
    set(local_imp_c_files "")
    if(${WINDOWS})
        append_imp_c_source_file("kmp_import.c")
    endif()
    set(${input_imp_c_files} "${local_imp_c_files}" PARENT_SCOPE)
endfunction()
