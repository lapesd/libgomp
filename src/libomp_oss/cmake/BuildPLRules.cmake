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

###############################################################################
# This file contains additional build rules that correspond to build.pl's rules.
# Building libomp.dbg is linux only, Windows will build libompmd.dll.pdb
# This file is only active if ${LIBOMP_USE_BUILDPL_RULES} is true.
#
#                        ######### BUILD DEPENDENCIES ##########
#
#        exports/.../libomp.so                        exports/.../libomp.dbg
#        [copy]  |                                                 | [copy]
#                |                                                 |
#           ./libomp.so                                     ./libomp.dbg
#    [copy]    /  OR  \____________ [copy]                         | [copy]
#             /                    \                               |
#    ./unstripped/libomp.so   ./stripped/libomp.so   ./unstripped/libomp.dbg
#           /                                \                /
#          / [linking]                        \[strip]       /[strip and store]
#         /                                    \            /
#     ${objs} (maybe compiled with -g)     ./unstripped/libomp.so (library with debug info in it)
#                                                    |
#                                                    | [linking]
#                                                    |
#                                                 ${objs} (always compiled with -g)
#
# For icc Linux builds, we always include debugging information via -g and create libomp.dbg
# so that Intel(R) Parallel Amplifier can use the .dbg file.
# For icc Windows builds, we always include debugging information via -Zi and create libomp.pdb
# in a fashion similar to libomp.dbg
# For icc Mac builds, we don't bother with the debug info.

# We build library in unstripped directory
file(MAKE_DIRECTORY ${build_dir}/unstripped)

# Only build the .dbg file for Release builds
# Debug and RelWithDebInfo builds should not create a .dbg file.
# The debug info should remain in the library file.
if(${LINUX} AND ${RELEASE_BUILD})
    set(dbg_file ${lib_item}.dbg)
endif()

################################
# --- Create $(lib_file).dbg ---
if(NOT "${dbg_file}" STREQUAL "")
    # if a ${dbg_file} file is going to be created, then
    file(MAKE_DIRECTORY ${build_dir}/stripped)

    # ./${lib_file} : stripped/${lib_file}
    #     copy stripped/${lib_file} ./${lib_file}
    simple_copy_recipe("${lib_file}"   "${build_dir}/stripped"   "${build_dir}")

    # stripped/${lib_file} : unstripped/${lib_file} ./${dbg_file}
    #     objcopy --strip-debug unstripped/${lib_file} stripped/${lib_file}.tmp
    #     objcopy --add-gnu-debuglink=${dbg_file} stripped/${lib_file}.tmp stripped/${lib_file}
    add_custom_command(
        OUTPUT  ${build_dir}/stripped/${lib_file}
        COMMAND ${CMAKE_OBJCOPY} --strip-debug ${build_dir}/unstripped/${lib_file} ${build_dir}/stripped/${lib_file}.tmp
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${dbg_file} ${build_dir}/stripped/${lib_file}.tmp ${build_dir}/stripped/${lib_file}
        DEPENDS "${build_dir}/${dbg_file}"
    )

    # ./${dbg_file} : unstripped/${dbg_file}
    #     copy unstripped/${dbg_file} ./${dbg_file}
    simple_copy_recipe("${dbg_file}"   "${build_dir}/unstripped" "${build_dir}")

    # unstripped/${dbg_file} : unstripped/${lib_file}
    #     objcopy --only-keep-debug unstripped/${lib_file} unstripped/${dbg_file}
    add_custom_command(
        OUTPUT  ${build_dir}/unstripped/${dbg_file}
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${build_dir}/unstripped/${lib_file} ${build_dir}/unstripped/${dbg_file}
        DEPENDS omp
    )

else()

    # ./${lib_file} : unstripped/${lib_file}
    #      copy unstripped/${lib_file} ./${lib_file}
    simple_copy_recipe("${lib_file}"   "${build_dir}/unstripped"  "${build_dir}")
endif()

# Windows specific command to move around debug info files post-build
if(NOT "${pdb_file}" STREQUAL "" AND ${RELEASE_BUILD})
    add_custom_command(TARGET omp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename ${pdb_file} ${pdb_file}.nonstripped
        COMMAND ${CMAKE_COMMAND} -E rename ${pdb_file}.stripped ${pdb_file}
    )
endif()

# Have icc build libomp in unstripped directory
set_target_properties(omp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${build_dir}/unstripped"
    RUNTIME_OUTPUT_DIRECTORY "${build_dir}/unstripped"
    ARCHIVE_OUTPUT_DIRECTORY "${build_dir}"
)

# Always use RelWithDebInfo flags for Release builds when using the build.pl's build rules (use -g -O2 instead of just -O3)
# The debug info is then stripped out at the end of the build and put into libomp.dbg for Linux
if(${RELEASE_BUILD} AND NOT ${MAC})
    set(CMAKE_C_FLAGS_RELEASE   ${CMAKE_C_FLAGS_RELWITHDEBINFO}  )
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    set(CMAKE_ASM_FLAGS_RELEASE ${CMAKE_ASM_FLAGS_RELWITHDEBINFO})
endif()

