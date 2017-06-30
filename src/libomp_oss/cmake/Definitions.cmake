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

# void append_definitions(string new_flag);
# - appends new_flag to cpp_flags list
macro(append_definitions new_flag)
    list(APPEND local_cpp_flags "${new_flag}")
endmacro()

function(append_cpp_flags input_cpp_flags)
    set(local_cpp_flags)

    append_definitions("-D USE_ITT_BUILD")
    append_definitions("-D KMP_ARCH_STR=\"\\\\\"${legal_arch}\\\\\"\"")
    if((NOT ${ARM}) AND (NOT ${IA32}))
        append_definitions("-D BUILD_I8")
    endif()
    append_definitions("-D KMP_LIBRARY_FILE=\\\\\"${lib_file}\\\\\"") # yes... you need 5 backslashes...
    append_definitions("-D KMP_VERSION_MAJOR=${LIBOMP_VERSION}")
    append_definitions("-D KMP_NESTED_HOT_TEAMS")

    # customize to 128 bytes for ppc64
    if(${PPC64})
    	append_definitions("-D CACHE_LINE=128")
    else()
    	append_definitions("-D CACHE_LINE=64")
    endif()

    append_definitions("-D KMP_ADJUST_BLOCKTIME=1")
    append_definitions("-D BUILD_PARALLEL_ORDERED")
    append_definitions("-D KMP_ASM_INTRINS")
    if(${LIBOMP_USE_ITT_NOTIFY})
        append_definitions("-D USE_ITT_NOTIFY=1")
    else()
        append_definitions("-D USE_ITT_NOTIFY=0")
        append_definitions("-D INTEL_NO_ITTNOTIFY_API")
    endif()
    append_definitions("-D INTEL_ITTNOTIFY_PREFIX=__kmp_itt_")

    if(${LIBOMP_USE_VERSION_SYMBOLS})
        append_definitions("-D KMP_USE_VERSION_SYMBOLS")
    endif()

    #####################
    # Windows definitions
    if(${WINDOWS})
        append_definitions("-D _CRT_SECURE_NO_WARNINGS")
        append_definitions("-D _CRT_SECURE_NO_DEPRECATE")
        append_definitions("-D _WINDOWS")
        append_definitions("-D _WINNT")
        append_definitions("-D _WIN32_WINNT=0x0501")
        append_definitions("-D KMP_WIN_CDECL")
        append_definitions("-D _USRDLL")
        if(${DEBUG_BUILD})
            append_definitions("-D _ITERATOR_DEBUG_LEVEL=0")
        endif()
    else() # Other than windows... (Unix based systems, Intel(R) Many Integrated Core Architecture (Intel(R) MIC Architecture), and Mac)
        append_definitions("-D _GNU_SOURCE")
        append_definitions("-D _REENTRANT")
        append_definitions("-D BUILD_TV")
        append_definitions("-D USE_CBLKDATA")
        if(NOT "${LIBOMP_VERSION}" STREQUAL "4")
            append_definitions("-D KMP_GOMP_COMPAT")
        endif()
    endif()

    # Any architecture other than Intel(R) MIC Architecture
    if(NOT ${MIC})
        append_definitions("-D USE_LOAD_BALANCE")
    endif()

    ##################
    # Unix definitions
    if(${LINUX})
        append_definitions("-D KMP_TDATA_GTID")
    endif()

    ##################################
    # Other conditional definitions
    if(${LIBOMP_ENABLE_ASSERTIONS})
        append_definitions("-D KMP_USE_ASSERT")
    endif()
    append_definitions("-D KMP_DYNAMIC_LIB")
    if(${STUBS_LIBRARY})
        append_definitions("-D KMP_STUB")
    endif()
    if(${DEBUG_BUILD} OR ${RELWITHDEBINFO_BUILD})
        append_definitions("-D KMP_DEBUG")
    endif()
    if(${DEBUG_BUILD})
        append_definitions("-D _DEBUG")
        append_definitions("-D BUILD_DEBUG")
    endif()
    if(${LIBOMP_STATS})
        append_definitions("-D KMP_STATS_ENABLED=1")
    else()
        append_definitions("-D KMP_STATS_ENABLED=0")
    endif()
    if(${LIBOMP_USE_DEBUGGER})
        append_definitions("-D USE_DEBUGGER=1")
    else()
        append_definitions("-D USE_DEBUGGER=0")
    endif()
    if(${LIBOMP_OMPT_SUPPORT})
        append_definitions("-D OMPT_SUPPORT=1")
    else()
        append_definitions("-D OMPT_SUPPORT=0")
    endif()
    if(${LIBOMP_OMPT_BLAME})
        append_definitions("-D OMPT_BLAME=1")
    else()
        append_definitions("-D OMPT_BLAME=0")
    endif()
    if(${LIBOMP_OMPT_TRACE})
        append_definitions("-D OMPT_TRACE=1")
    else()
        append_definitions("-D OMPT_TRACE=0")
    endif()
    if(${LIBOMP_USE_HWLOC})
        append_definitions("-D KMP_USE_HWLOC=1")
    else()
        append_definitions("-D KMP_USE_HWLOC=0")
    endif()

    # OpenMP version flags
    set(have_omp_50 0)
    set(have_omp_45 0)
    set(have_omp_40 0)
    set(have_omp_30 0)
    if(${LIBOMP_OMP_VERSION} EQUAL 50 OR ${LIBOMP_OMP_VERSION} GREATER 50)
        set(have_omp_50 1)
    endif()
    if(${LIBOMP_OMP_VERSION} EQUAL 45 OR ${LIBOMP_OMP_VERSION} GREATER 45)
        set(have_omp_45 1)
    endif()
    if(${LIBOMP_OMP_VERSION} EQUAL 40 OR ${LIBOMP_OMP_VERSION} GREATER 40)
        set(have_omp_40 1)
    endif()
    if(${LIBOMP_OMP_VERSION} EQUAL 30 OR ${LIBOMP_OMP_VERSION} GREATER 30)
        set(have_omp_30 1)
    endif()
    append_definitions("-D OMP_50_ENABLED=${have_omp_50}")
    append_definitions("-D OMP_45_ENABLED=${have_omp_45}")
    append_definitions("-D OMP_40_ENABLED=${have_omp_40}")
    append_definitions("-D OMP_30_ENABLED=${have_omp_30}")

    # Architectural definitions
    if(${INTEL64} OR ${IA32})
        if(${LIBOMP_USE_ADAPTIVE_LOCKS})
            append_definitions("-D KMP_USE_ADAPTIVE_LOCKS=1")
        else()
            append_definitions("-D KMP_USE_ADAPTIVE_LOCKS=0")
        endif()
        append_definitions("-D KMP_DEBUG_ADAPTIVE_LOCKS=0")
    else()
        append_definitions("-D KMP_USE_ADAPTIVE_LOCKS=0")
        append_definitions("-D KMP_DEBUG_ADAPTIVE_LOCKS=0")
    endif()
    if(${LIBOMP_USE_INTERNODE_ALIGNMENT})
        append_definitions("-D KMP_USE_INTERNODE_ALIGNMENT=1")
    else()
        append_definitions("-D KMP_USE_INTERNODE_ALIGNMENT=0")
    endif()
    set(${input_cpp_flags} "${${input_cpp_flags}}" "${local_cpp_flags}" "${LIBOMP_CPPFLAGS}" "$ENV{CPPFLAGS}" PARENT_SCOPE)
endfunction()

