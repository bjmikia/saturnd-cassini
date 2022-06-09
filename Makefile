
.PHONY: all clean distclean

all: cassini saturnd

cassini: src/cassini.c
	gcc -g -Wall -std=c17 -I ./include -o cassini src/cassini.c src/common.c src/timing-text-io.c

saturnd : src/saturnd.c
	gcc -g -Wall -std=c17 -I ./include -o saturnd src/saturnd.c src/create.c src/list.c src/remove.c src/stdout_stderr.c src/terminate.c src/time_exitcode.c src/common.c src/timing-text-io.c src/jobs.c

clean:

distclean:
	rm -f cassini saturnd