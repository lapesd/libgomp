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

# This file holds Microsoft Visual Studio dependent flags
# The flag types are:
#   1) C/C++ Compiler flags
#   2) Fortran Compiler flags

#########################################################
# Visual Studio C/C++ Compiler flags
function(append_compiler_specific_c_and_cxx_flags input_c_flags input_cxx_flags)
    set(local_c_flags)
    set(local_cxx_flags)
    append_c_flags("-TP") # Tells the compiler to process a file as a C++ source file.
    append_cxx_flags("-EHsc") # Enable C++ exception handling.
    append_c_and_cxx_flags("-W3") # Enables diagnostics for remarks, warnings, and errors.
                                  # Additional warnings are also enabled above level 2 warnings.
    append_c_and_cxx_flags("-GS") # Lets you control the threshold at which the stack checking routine is called or not called.
    if(${IA32})
        append_c_and_cxx_flags("-arch:ia32") # Tells the compiler which features it may target (ia32)
        append_c_and_cxx_flags("-Oy-") # equivalent to -fno-omit-frame-pointer
    endif()
    # CMake prefers the /MD flags when compiling Windows sources, but libomp needs to use /MT instead
    # So we replace these /MD instances with /MT within the CMAKE_*_FLAGS variables and put that out to the CACHE.
    # replace_md_with_mt() is in HelperFunctions.cmake
    replace_md_with_mt(CMAKE_C_FLAGS)
    replace_md_with_mt(CMAKE_C_FLAGS_RELEASE)
    replace_md_with_mt(CMAKE_C_FLAGS_RELWITHDEBINFO)
    replace_md_with_mt(CMAKE_C_FLAGS_DEBUG)
    replace_md_with_mt(CMAKE_CXX_FLAGS)
    replace_md_with_mt(CMAKE_CXX_FLAGS_RELEASE)
    replace_md_with_mt(CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    replace_md_with_mt(CMAKE_CXX_FLAGS_DEBUG)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS_RELEASE)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS_RELWITHDEBINFO)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS_DEBUG)
    set(${input_c_flags}   ${${input_c_flags}}   "${local_c_flags}" PARENT_SCOPE)
    set(${input_cxx_flags} ${${input_cxx_flags}} "${local_cxx_flags}" PARENT_SCOPE)
endfunction()

#########################################################
# Visual Studio Linker flags
function(append_compiler_specific_linker_flags input_ld_flags input_ld_flags_libs)
    set(local_ld_flags)
    set(local_ld_flags_libs)
    append_linker_flags("-WX:NO")
    append_linker_flags("-version:${LIBOMP_VERSION}.0")
    append_linker_flags("-NXCompat")
    append_linker_flags("-DynamicBase") # This option modifies the header of an executable to indicate
                                           # whether the application should be randomly rebased at load time.
    if(${IA32})
        append_linker_flags("-machine:i386")
        append_linker_flags("-safeseh")
    elseif(${INTEL64})
        append_linker_flags("-machine:amd64")
    endif()
    if(NOT "${def_file}" STREQUAL "")
        append_linker_flags("-def:${def_file}")
    endif()
    # Have Visual Studio use link.exe directly
    #set(CMAKE_C_CREATE_SHARED_LIBRARY "link.exe /out:<TARGET> <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES>" PARENT_SCOPE)
    #set(CMAKE_SHARED_LINKER_FLAGS "$ENV{LDLFAGS}" CACHE STRING "Linker Flags" FORCE)
    set(${input_ld_flags}      ${${input_ld_flags}}      "${local_ld_flags}"       PARENT_SCOPE)
    set(${input_ld_flags_libs} ${${input_ld_flags_libs}} "${local_ld_flags_libs}"  PARENT_SCOPE)
endfunction()

