#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable.
EXEC = gf

# Sets the target platform.
platforms := hardware

# Compilation flags flags
cflags := -O3 -I $(INCDIR)
lflags := -lm
k1-lflags := -lmppaipc

# Builds master binary.
io-bin := master
master-cflags := -D_MASTER_
master-srcs := $(wildcard master/*.c) $(wildcard $(LIBSRCDIR)/*.c)

# Builds slave binary
cluster-bin := slave
slave-lflags := -fopenmp
slave-srcs := $(wildcard slave/*.c) $(wildcard $(LIBSRCDIR)/*.c)

# build image (master + slave)
mppa-bin := multibin
multibin-objs := master slave
multibin-name := $(EXEC).img

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.mppaipc
