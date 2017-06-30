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

# This file holds Clang (clang/clang++) specific compiler dependent flags
# The flag types are:
#   1) C/C++ Compiler flags
#   2) Linker flags

#########################################################
# C/C++ Compiler flags
function(append_compiler_specific_c_and_cxx_flags input_c_flags input_cxx_flags)
    set(local_c_flags)
    set(local_cxx_flags)
    append_c_and_cxx_flags("-std=c++0x") # Enables support for many C++11 (formerly known as C++0x) features. The following are the most recently added features:
    append_c_and_cxx_flags("-fno-exceptions") # Exception handling table generation is disabled.
    append_c_and_cxx_flags("-x c++") # Compile C files as C++ files
    if(${IA32})
        append_c_and_cxx_flags("-m32") # Generate 32 bit IA-32 architecture code
        append_c_and_cxx_flags("-msse2") # Allow use of Streaming SIMD Instructions
    elseif(${ARM})
        append_c_and_cxx_flags("-marm") # Target the ARM architecture
    endif()
    append_c_and_cxx_flags("-Wno-unused-value") # Don't warn about unused values
    append_c_and_cxx_flags("-Wno-switch") # Don't warn about switch statements that don't cover entire range of values
    append_c_and_cxx_flags("-Wno-deprecated-register") # Don't warn about using register keyword
    set(${input_c_flags}   ${${input_c_flags}}   "${local_c_flags}" PARENT_SCOPE)
    set(${input_cxx_flags} ${${input_cxx_flags}} "${local_cxx_flags}" PARENT_SCOPE)
endfunction()

#########################################################
# Linker flags
function(append_compiler_specific_linker_flags input_ld_flags input_ld_flags_libs)
    set(local_ld_flags)
    set(local_ld_flags_libs)
    if(${IA32})
        append_linker_flags("-m32")
        append_linker_flags("-msse2")
    endif()
    set(${input_ld_flags}      ${${input_ld_flags}}      "${local_ld_flags}"       PARENT_SCOPE)
    set(${input_ld_flags_libs} ${${input_ld_flags_libs}} "${local_ld_flags_libs}"  PARENT_SCOPE)
endfunction()

