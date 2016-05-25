#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable file.
EXEC = mst

# Builds static version.
static:
	$(CC) *.c $(CFLAGS) -rdynamic -D_SCHEDULE_STATIC_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds dynamic version.
dynamic:
	$(CC) *.c $(CFLAGS) -rdynamic -D_SCHEDULE_DYNAMIC_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Builds smart-round-robin version.
smart-round-robin:
	$(CC) *.c $(CFLAGS) -rdynamic -D_SCHEDULE_SRR_ -o $(BINDIR)/$(EXEC).srr $(LIBS)

# Builds oracle version.
oracle:
	$(CC) *.c $(CFLAGS) -rdynamic -D_SCHEDULE_ORACLE_ -o $(BINDIR)/$(EXEC).oracle $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
