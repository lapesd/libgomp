# common-rules.mk #

# <copyright>
#    Copyright (c) 2008-2016 Intel Corporation.  All Rights Reserved.
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

# --------------------------------------------------------------------------------------------------
# This file contains really common definitions used by multiple makefiles. Modify it carefully!
# --------------------------------------------------------------------------------------------------

# --- Creating a directory ---
# A directory cannot be a target, because in Linux* OS directory's timestamp is updated each time a
# file is created or deleted in the directory. We use ".dir" file in place of directory. If such
# file exists, it means directory exists also.

.PRECIOUS : %/.dir                     # Do not delete automatically created files.

%/.dir :
	$(target)
	$(mkdir) $(dir $@)
	$(touch) $@

# --- Rebuilding ---
# Removing or touching .rebuild file causes rebuild.
# To let it work, .rebuild should be added as prerequisite to every rule (dependency with commands)
# except clean* and force*, in this and other makefiles.
.rebuild :
	$(target)
	$(touch) $@

# -- Creating dependency file for C/C++ ---

%.d : %.c .rebuild
	$(target)
	$(c) $(cpp-flags) $(c-flags) $(c-flags-m) $< > $@

%.d : %.cpp .rebuild
	$(target)
	$(cxx) $(cpp-flags) $(cxx-flags) $(cxx-flags-m) $< > $@

# -- Creating preprocessed file for C/C++ ---

%.i : %.c .rebuild
	$(target)
	$(c) $(cpp-flags) $(c-flags) -P $(c-out)$@ $<

%.i : %.cpp .rebuild
	$(target)
	$(cxx) $(cpp-flags) $(cxx-flags) -P $(cxx-out)$@ $<

# -- Compiling C/C++ files ---

%$(obj) : %.c .rebuild
	$(target)
	$(c) $(cpp-flags) $(c-flags) $(c-out)$@ $<

%$(obj) : %.cpp .rebuild
	$(target)
	$(cxx) $(cpp-flags) $(cxx-flags) $(cxx-out)$@ $<

# -- Generate assembly files ---

%$(asm) : %.c .rebuild
	$(target)
	$(c) $(cpp-flags) $(c-flags) -S $(c-out)$@ $<

%$(asm) : %.cpp .rebuild
	$(target)
	$(cxx) $(cpp-flags) $(cxx-flags) -S $(cxx-out)$@ $<

# -- Compiling asm files ---

%$(obj) : %$(asm) .rebuild
	$(target)
        # There is a bug on mic: icc does not work with "-x assembler-with-cpp" option, so we have
        # to preprocess file manually and then assembly it.
        ifeq "$(arch)" "mic"
	    $(c) -E $(cpp-flags) $< > $@.tmp
	    $(as) $(as-flags) -x assembler $(as-out)$@ $@.tmp
        else
	    $(as) $(as-flags) $(as-out)$@ $<
        endif

# -- Expanding variables in template files ---

# General rule "% : %.var" does not work good, so we have to write more specific rules:
# "%.h : %.h.var", etc.

.PRECIOUS : %.h %.f %.rc               # Do not delete automatically created files.

expand-vars = $(perl) $(tools_dir)expand-vars.pl --strict $(ev-flags) $< $@

# Any generated file depends on kmp_version.c, because we extract build number from that file.

%.h  : %.h.var  \
    kmp_version.c $(tools_dir)expand-vars.pl .rebuild
	$(target)
	$(expand-vars)

%.f  : %.f.var  \
    kmp_version.c $(tools_dir)expand-vars.pl .rebuild
	$(target)
	$(expand-vars)

%.f90  : %.f90.var  \
    kmp_version.c $(tools_dir)expand-vars.pl .rebuild
	$(target)
	$(expand-vars)

%.rc : %.rc.var \
   kmp_version.c $(tools_dir)expand-vars.pl .rebuild
	$(target)
	$(expand-vars)

# -- Making static library ---

.PRECIOUS : %$(lib)                    # Do not delete automatically created files.

%$(lib) : %$(lib).lst .rebuild
	$(target)
	$(rm) $@
	$(ar) $(ar-flags) $(ar-out)$@ $$(cat $<)
        # strip debug info in case it is requested (works for Linux* OS only)
        ifneq "$(dbg_strip)" ""
            ifeq "$(DEBUG_INFO)" "off"
                ifeq "$(arch)" "mic"
	            x86_64-k1om-linux-objcopy --strip-debug $@
                else
	            objcopy --strip-debug $@
                endif
            endif
        endif

# -- Making dynamic library ---

.PRECIOUS : %$(dll)                    # Do not delete automatically created files.

# makefile.mk should properly define imp_file, def_file, res_file, and pdb_file:
#     lin and mac: def_file and res_file should be empty, imp_file and pdb_file do not matter.
#     win: all the variabe may be empty; if a variable specified, it affects ld-flags.
# Note: imp_file and pdb_file are side effect of building this target.
# Note: to workaround CQ215229 $ld-flags-extra introduced to keep options be placed after objects
%$(dll) : %$(dll).lst $(def_file) $(res_file) .rebuild
	$(target)
	$(ld) $(ld-flags-dll) $(ld-flags) $(ld-out)$@ $$(cat $<) $(ld-flags-extra) $(res_file)
        # If stripped pdb exist, rename it to normal pdb name. See devtools.mk for explanation.
        ifneq "$(pdb_file)" ""
            ifeq "$(DEBUG_INFO)" "off"
	        mv $(pdb_file) $(pdb_file).nonstripped
	        mv $(pdb_file).stripped $(pdb_file)
            endif
        endif

ifneq "$(pdb_file)" ""
$(pdb_file) : $(lib_file)
endif

%.dbg : %$(dll) .rebuild
	$(target)
        ifeq "$(arch)" "mic"
	    x86_64-k1om-linux-objcopy  --only-keep-debug $< $@
        else
	    objcopy --only-keep-debug $< $@
        endif


.PRECIOUS: %.res                       # Do not delete automatically created files.

%.res : %.rc .rebuild
	$(target)
	rc -fo$@ $<

# --- Building helper tools from sources ---

.PRECIOUS: %$(exe)                     # Do not delete automatically created files.

%$(exe) : $(tools_dir)%.cpp .rebuild
	$(target)
	$(cxx) $(cxx-out)$@ $<

# --- Forcing a test ---

test-%/.force : test-%/.dir
	$(target)
	$(rm) $(dir $@).{test,force}

# --- Removing a file in build directory ---

rm-% :
	$(target)
	$(rm) $(patsubst rm-%,%,$@)

# end of file #
