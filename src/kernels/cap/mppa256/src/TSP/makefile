#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable.
EXEC = tsp

# Sets the target platform.
platforms := hardware

# Compilation flags flags
cflags := -O3 -I $(INCDIR) -DMT -DNO_CACHE_COHERENCE -UDEBUG
lflags := -lm -pthread
k1-lflags := -lmppaipc

# Builds master binary.
io-bin := master
master-srcs := $(wildcard master/*.c) $(LIBSRCDIR)/timer.c $(LIBSRCDIR)/mppa.c $(wildcard *.c)

# Builds slave binary
cluster-bin := slave
slave-srcs := $(wildcard slave/*.c) $(LIBSRCDIR)/timer.c $(LIBSRCDIR)/mppa.c $(wildcard *.c)

# build image (master + slave)
mppa-bin := multibin
multibin-objs := master slave
multibin-name := $(EXEC).img

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.mppaipc
