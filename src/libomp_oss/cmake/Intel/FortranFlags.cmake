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

# This file holds Intel(R) C Compiler / Intel(R) C++ Compiler / Intel(R) Fortran Compiler (icc/icpc/icl.exe/ifort) dependent flags
# The flag types are:
#   1) Fortran Compiler flags

#########################################################
# icc Fortran Compiler flags (for creating .mod files)
function(append_fortran_compiler_specific_fort_flags input_fort_flags)
    set(local_fort_flags)
    #set(CMAKE_Fortran_FLAGS      "$ENV{FFLAGS}"   CACHE STRING "Fortran flags"  FORCE)
    #set(CMAKE_Fortran_FLAGS_RELEASE          ""   CACHE STRING "Fortran flags"  FORCE)
    #set(CMAKE_Fortran_FLAGS_DEBUG            ""   CACHE STRING "Fortran flags"  FORCE)
    #set(CMAKE_Fortran_FLAGS_RELWITHDEBINFO   ""   CACHE STRING "Fortran flags"  FORCE)
    if(${WINDOWS})
        append_fort_flags("-Qdiag-disable:177,5082")
        append_fort_flags("-Qsox")
        append_fort_flags("-nologo")
        append_fort_flags("-GS")
        append_fort_flags("-DynamicBase")
        append_fort_flags("-Zi")
        # On Linux and Windows Intel(R) 64 architecture we need offload attribute
        # for all Fortran entries in order to support OpenMP function calls inside device contructs
        if(${INTEL64})
            append_fort_flags("/Qoffload-attribute-target:mic")
        endif()
    else()
        if(${MIC})
            append_fort_flags("-mmic")
        endif()
        if(NOT ${MAC})
            append_fort_flags("-sox")
            if(${INTEL64} AND ${LINUX})
                append_fort_flags("-offload-attribute-target=mic")
            endif()
        endif()
    endif()
    set(${input_fort_flags} ${${input_fort_flags}} "${local_fort_flags}" PARENT_SCOPE)
endfunction()
