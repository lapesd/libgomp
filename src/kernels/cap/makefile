#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR  = $(CURDIR)/bin

# Builds all kernels for Intel x86.
all-x86:
	mkdir -p bin
	cd x86 && $(MAKE) all BINDIR=$(BINDIR)

# Builds all kernels for MPPA-256.
all-mppa256: 
	mkdir -p bin
	cd mppa256 && $(MAKE) all BINDIR=$(BINDIR)

# Cleans compilation files.
clean:
	cd x86 && $(MAKE) clean BINDIR=$(BINDIR)
	cd mppa256 && $(MAKE) clean BINDIR=$(BINDIR)
