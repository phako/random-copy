random-copy: random-copy.c copy-task.c
	cc -g -O0 -Wall -Wextra -Werror -o random-copy random-copy.c copy-task.c

install: random-copy
	mkdir -p $(HOME)/.local/bin
	cp random-copy $(HOME)/.local/bin

clean:
	rm -f random-copy
