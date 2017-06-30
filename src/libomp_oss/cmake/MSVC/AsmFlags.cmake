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
#   1) Assembly flags

#########################################################
# Assembly flags
function(append_assembler_specific_asm_flags input_asm_flags)
    set(local_asm_flags)
    append_asm_flags("-nologo") # Turn off tool banner.
    if(${IA32})
        append_asm_flags("-safeseh") # Registers exception handlers for safe exception handling.
        append_asm_flags("-coff") # Generates common object file format (COFF) type of object module.
                                  # Generally required for Win32 assembly language development.
        append_asm_flags("-D _M_IA32")
    elseif(${INTEL64})
        append_asm_flags("-D _M_AMD64")
    endif()
    # CMake prefers the /MD flags when compiling Windows sources, but libomp needs to use /MT instead
    # So we replace these /MD instances with /MT within the CMAKE_*_FLAGS variables and put that out to the CACHE.
    # replace_md_with_mt() is in HelperFunctions.cmake
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS_RELEASE)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS_RELWITHDEBINFO)
    replace_md_with_mt(CMAKE_ASM_MASM_FLAGS_DEBUG)
    set(${input_asm_flags} ${${input_asm_flags}} "${local_asm_flags}" PARENT_SCOPE)
endfunction()
