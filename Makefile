# A stupidly written Makefile.
# Not currently portable due to excessive usage of Linux commands.
# You're on Windows? Use WSL if possible or wait for this file to be updated.
# Or you could just install a Linux distro on your machine if you want to.
#
# Author: Luth

EXE := lxt
INCLUDES := -I./src -I./include
SRCS := $(shell find ./src -name "*.c")

# I'm using LLVM Clang here as the compiler, if you're a fan of GCC, you
# might want to modify some of the lines below.
CC := clang
CFLAGS := -Wall -Wextra $(INCLUDES)

# The default build instruction if you run "make" without argument
all: obj $(EXE)

$(EXE): obj
	$(CC) *.o -o $@
	mkdir -p ./obj
	mv -f *.o ./obj

obj: $(SRCS)
	$(CC) $(CFLAGS) -O3 -c $^

# The build instruction if you run "make debug"
debug: objdebug
	$(CC) *.o -g -o $(EXE)
	mkdir -p ./obj
	mv -f *.o ./obj

objdebug: $(SRCS)
	$(CC) $(CFLAGS) -g -c $^

# The build instruction if you run "make clean"
.PHONY: clean
clean:
	$(RM) $(EXE) *.o
	$(RM) -r ./obj
