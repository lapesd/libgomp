#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# CLASS
CLASS=CLASS_D

NUM_BUCKETS_LOG_2=3

# Executable file.
EXEC = is

# Class.
CFLAGS += -DCLASS=$(CLASS) -DNUM_BUCKETS_LOG_2=$(NUM_BUCKETS_LOG_2)

# Builds static version.
static:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_STATIC_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds dynamic version.
dynamic:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_DYNAMIC_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Builds guided version.
guided:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_GUIDED_ -o $(BINDIR)/$(EXEC).guided $(LIBS)

# Builds smart-round-robin version.
smart-round-robin:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_SRR_ -o $(BINDIR)/$(EXEC).srr $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
