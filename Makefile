bindir=bin
bin=pl_lexer
objs=main.o SStr.o PL_Lexer.o Lexer.o LexTree.o PL_Parser.o PL_Generator.o
srcdir=src
subdirs=lexer pl_zero string 
CFLAGS=-Wall -Wextra -std=c99
VPATH=$(srcdir):$(addprefix $(srcdir)/,$(subdirs)):$(bindir)
INC=-Iinclude
cc=gcc $(CFLAGS) $(INC)

all: makebindir
all: pl0lexer

makebindir:
	-mkdir -p $(bindir)

debug: cc += -ggdb 
debug: pl0lexer

pl0lexer: $(objs)
pl0lexer: objs := $(addprefix $(bindir)/, $(objs))
pl0lexer:
	  $(cc) -o $(bin) $(objs)

$(objs): %.o: %.c
	$(cc) -o $(bindir)/$(notdir $@) -c $<

clean : 
	-rm $(bin) $(addprefix $(bindir)/, $(objs))
