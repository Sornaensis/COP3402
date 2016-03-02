bindir=bin
bin=pl0
objs=main.o SStr.o PL_Lexer.o Lexer.o LexTree.o PL_Parser.o PL_Generator.o PL_StackMachine.o PL_Tree.o
srcdir=src
subdirs=lexer pl_zero string 
CFLAGS=-Wall -Wextra -std=c99
VPATH=$(srcdir):$(addprefix $(srcdir)/,$(subdirs)):$(bindir)
INC=-Iinclude
cc=gcc $(CFLAGS) $(INC)

all: makebindir
all: cc := $(cc) -DPL_PARAM_EXTENSION_SUPPORT -DPL_SYNTAX_EXTENSION_SUPPORT
all: pl0compiler

makebindir:
	-mkdir -p $(bindir)

params-only: makebindir
params-only: cc := $(cc) -DPL_PARAM_EXTENSION_SUPPORT
params-only: pl0compiler

pl0compiler: $(objs)
pl0compiler: objs := $(addprefix $(bindir)/, $(objs))
pl0compiler:
	  $(cc) -o $(bin) $(objs)

$(objs): %.o: %.c
	$(cc) -o $(bindir)/$(notdir $@) -c $<

clean : 
	-rm $(bin) $(addprefix $(bindir)/, $(objs))
