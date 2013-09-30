CC=$(shell which clang)
INCLUDEPATH= -I include
CFLAGS+= -std=c11 -Weverything $(COPT) ${INCLUDEPATH} -g
OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, main.o rrb.o)

EXEC = bin/main

.PHONY: clear purge png

obj/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c $< -o $@
%.png: %.dot
	dot $*.dot -Tpng -o $@

all: $(EXEC)

$(EXEC): $(OBJS) bin
	$(CC) $(OBJS) -o $(EXEC)

png: $(EXEC) img
	bin/main
	@$(MAKE) __images

__images: $(patsubst %.dot,%.png,$(wildcard img/*.dot))

clean:
	-rm -rf obj $(wildcard img/*.dot)
purge: clean
	-rm -rf bin img

$(OBJS): | $(OBJDIR)

bin:
	mkdir -p bin
$(OBJDIR):
	mkdir -p $(OBJDIR)
img:
	mkdir -p img
