CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread
OBJS = list.o threadpool.o main.o

.PHONY: all clean 

GIT_HOOKS := .git/hooks/pre-commit

ifeq ($(strip $(BENCH)),1)
CFLAGS += -DBENCH
endif

ifeq ($(strip $(CHECK)),1)
CFLAGS += -DCHECK
endif

all: $(GIT_HOOKS) sort

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -rdynamic

input_generator: all
	$(CC) $(CFLAGS) input_generator.c -o input_generator

bench: input_generator
	for i in `seq 100000 10000 300000`; do \
	    ./input_generator $$i; \
	    for j in 1 2 4 8 16 32 64; do \
		./sort $$j $$i; \
	    done \
	done

plot: bench
	gnuplot scripts/runtime.gp

check: input_generator
	for i in `seq 1 1 100`; do\
	    ./input_generator $$i; \
	    ./sort 4 $$i; \
	    sort -n input > sorted; \
	    diff output sorted; \
	done

clean:
	rm -f $(OBJS) sort \
	    input_generator input output runtime.png sorted
	@rm -rf $(deps)

-include $(deps)
