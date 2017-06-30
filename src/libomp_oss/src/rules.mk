# rules.mk #

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

# --- Copy files to out directories ---

$(out_cmn_dir)include/% : % $(out_cmn_dir)include/.dir .rebuild
	$(target)
	$(cp) $< $@

$(out_cmn_dir)include_compat/% : % $(out_cmn_dir)include_compat/.dir .rebuild
	$(target)
	$(cp) $< $@

# Fat: touch .touch file on every update in $(out_lib_dir), so we will know should we update fat
# goal or not.
$(out_lib_dir)% : % $(out_lib_dir).dir .rebuild
	$(target)
	$(cp) $< $@
        ifneq "$(out_lib_fat_dir)" ""
	    $(touch) $(dir $@).touch
        endif

$(out_ptf_dir)include/% : % $(out_ptf_dir)include/.dir .rebuild
	$(target)
	$(cp) $< $@

$(out_ptf_dir)include_compat/% : % $(out_ptf_dir)include_compat/.dir .rebuild
	$(target)
	$(cp) $< $@

$(out_l10n_dir)%/$(cat_file) : l10n/%/$(cat_file) $(out_l10n_dir)%/.dir .rebuild
	$(target)
	$(cp) $< $@

ifeq "$(os)" "mac"
    $(out_l10n_fat_dir)%/$(cat_file) : l10n/%/$(cat_file) $(out_l10n_fat_dir)%/.dir .rebuild
	    $(target)
	    $(cp) $< $@
endif

# --- Include really common rules ---

include $(LIBOMP_WORK)tools/src/common-rules.mk

# --- Building helper tools from sources ---

.PRECIOUS: %$(exe)                     # Do not delete automatically created files.

%$(exe) : $(tools_dir)%.cpp .rebuild
	$(target)
	$(cxx) $(cxx-out)$@ $<

# --- Fat libraries ---

# Every time new file is copied to $(out_lib_dir) directory we update $(out_lib_dir).rebuild file,
# so we know should we rebuild fat libraries or not.

# Note: Original implementation built fat libraries in mac_32 directory, then copied all the
# libraries from mac_32 to mac_32e directory. However, this may work wrong if exports/mac_*/lib/
# contains other libraries. So now we build fat libraries twice: in both mac_32
# and mac_32e directories.

ifeq "$(platform)" "mac_32e"

    .PHONY : fat
    fat    : $(call _out_lib_fat_dir,mac_32).done $(call _out_lib_fat_dir,mac_32e).done

    $(call _out_lib_fat_dir,mac_32).done \
    $(call _out_lib_fat_dir,mac_32e).done : \
        $(call _out_lib_dir,mac_32).touch \
        $(call _out_lib_dir,mac_32e).touch \
        $(tools_dir)make-fat-binaries.pl \
        $(call _out_lib_fat_dir,mac_32).dir $(call _out_lib_fat_dir,mac_32e).dir .rebuild
	    $(target)
	    $(perl) $(tools_dir)make-fat-binaries.pl \
	        --output=$(dir $@) $(call _out_lib_dir,mac_32) $(call _out_lib_dir,mac_32e)
	    $(touch) $@

endif

# end of file #
