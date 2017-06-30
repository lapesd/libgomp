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

# void append_ev_flags(string new_flag);
# - appends new_flag to ev_flags list
macro(append_ev_flags new_flag)
    list(APPEND local_ev_flags "${new_flag}")
endmacro()

# void append_gd_flags(string new_flag);
# - appends new_flag to gd_flags list
macro(append_gd_flags new_flag)
    list(APPEND local_gd_flags "${new_flag}")
endmacro()

include(HelperFunctions) # for set_legal_type(), set_legal_arch()

# Perl expand-vars.pl flags
function(set_ev_flags input_ev_flags)
    set(local_ev_flags)
    set_legal_type("${LIBOMP_LIB_TYPE}" legal_type)
    set_legal_arch("${LIBOMP_ARCH}" legal_arch)
    # need -D Revision="\$Revision" to show up
    append_ev_flags("-D Revision=\"\\\\$$Revision\"")
    append_ev_flags("-D Date=\"\\\\$$Date\"")
    append_ev_flags("-D KMP_TYPE=\"${legal_type}\"")
    append_ev_flags("-D KMP_ARCH=\"${legal_arch}\"")
    append_ev_flags("-D KMP_VERSION_MAJOR=${LIBOMP_VERSION}")
    append_ev_flags("-D KMP_VERSION_MINOR=0")
    append_ev_flags("-D KMP_VERSION_BUILD=${build_number}")
    append_ev_flags("-D KMP_BUILD_DATE=\"${date}\"")
    append_ev_flags("-D KMP_VERSION_BUILD_YEAR=${LIBOMP_VERSION_BUILD_YEAR}")
    append_ev_flags("-D KMP_VERSION_BUILD_MONTH_DAY=${LIBOMP_VERSION_BUILD_MONTH_DAY}")
    append_ev_flags("-D KMP_TARGET_COMPILER=12")
    if(${DEBUG_BUILD} OR ${RELWITHDEBINFO_BUILD})
        append_ev_flags("-D KMP_DIAG=1")
        append_ev_flags("-D KMP_DEBUG_INFO=1")
    else()
        append_ev_flags("-D KMP_DIAG=0")
        append_ev_flags("-D KMP_DEBUG_INFO=0")
    endif()
    if(${LIBOMP_VERSION} EQUAL 40)
        append_ev_flags("-D OMP_VERSION=201307")
    elseif(${LIBOMP_VERSION} EQUAL 30)
        append_ev_flags("-D OMP_VERSION=201107")
    else()
        append_ev_flags("-D OMP_VERSION=200505")
    endif()
    set(${input_ev_flags} "${local_ev_flags}" PARENT_SCOPE)
endfunction()

function(set_gd_flags input_gd_flags)
    set(local_gd_flags)
    if(${IA32})
        append_gd_flags("-D arch_32")
    elseif(${INTEL64})
        append_gd_flags("-D arch_32e")
    else()
        append_gd_flags("-D arch_${LIBOMP_ARCH}")
    endif()
    if(${NORMAL_LIBRARY})
        append_gd_flags("-D norm")
    elseif(${PROFILE_LIBRARY})
        append_gd_flags("-D prof")
    elseif(${STUBS_LIBRARY})
        append_gd_flags("-D stub")
    endif()
    if(${LIBOMP_OMP_VERSION} GREATER 45 OR ${LIBOMP_OMP_VERSION} EQUAL 45)
        append_gd_flags("-D OMP_45")
    endif()
    if(${LIBOMP_OMP_VERSION} GREATER 40 OR ${LIBOMP_OMP_VERSION} EQUAL 40)
        append_gd_flags("-D OMP_40")
    endif()
    if(${LIBOMP_OMP_VERSION} GREATER 30 OR ${LIBOMP_OMP_VERSION} EQUAL 30)
        append_gd_flags("-D OMP_30")
    endif()
    if(NOT "${LIBOMP_VERSION}" STREQUAL "4")
        append_gd_flags("-D msvc_compat")
    endif()
    if(${DEBUG_BUILD} OR ${RELWITHDEBINFO_BUILD})
        append_gd_flags("-D KMP_DEBUG")
    endif()
    if(${LIBOMP_COMPILER_SUPPORTS_QUAD_PRECISION})
        append_gd_flags("-D HAVE_QUAD")
    endif()
    if(${LIBOMP_USE_DEBUGGER})
        append_gd_flags("-D USE_DEBUGGER")
    endif()
    set(${input_gd_flags} "${local_gd_flags}" PARENT_SCOPE)
endfunction()
