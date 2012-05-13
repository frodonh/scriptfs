CC=gcc
CINCFLAGS=
ifdef TRACE	# Trace FUSE operations
	CTRACEFLAGS=-DTRACE
else
	CTRACEFLAGS=
endif
ifdef PROFILE
	CPROFFLAGS=-p -pg
else
	CPROFFLAGS=
endif
ifdef RELEASE
	COPTFLAGS=-O3 -s 
else
	COPTFLAGS=-O0 -ggdb3 -Werror -Wall
endif
CFLAGS=$(CINCFLAGS) $(COPTFLAGS) $(CPROFFLAGS) $(CTRACEFLAGS) `pkg-config fuse --cflags` 
LFLAGS=`pkg-config fuse --libs` 

BIN=.
PROJECT=scriptfs

all:$(BIN)/$(PROJECT)

$(BIN)/$(PROJECT):$(PROJECT).c $(BIN)/operations.o
	@echo --------------- Linking of executable ---------------
	@$(CC) $(CFLAGS) -o $(BIN)/$(PROJECT) $^ $(LFLAGS)

$(BIN)/%.o:%.c %.h
	@echo --------------- Compilation of $< ---------------
	@$(CC) $(CFLAGS) -c -o $(BIN)/$@ $<

clean:
	@rm *.o
	@rm sandbox

doc:
	@doxygen doxyconf

archive:
	@tar -cvjf $(PROJECT).tar.bz2 *.c *.h Makefile
