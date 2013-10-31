CC=$(shell which clang)
INCLUDEPATH=
CFLAGS+= -std=c11 -Weverything -Wno-cast-align $(COPT) ${INCLUDEPATH} -g -D RRB_DEBUG
LINKING = -lgc
OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, main.o rrb.o)

EXEC = bin/main

.PHONY: clear purge png

obj/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@
%.png: %.dot
	dot $*.dot -Tpng -o $@

all: $(EXEC)

$(EXEC): $(OBJS) bin
	$(CC) ${LINKING} $(OBJS) -o $(EXEC)

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
