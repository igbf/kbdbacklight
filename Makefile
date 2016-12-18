default: build

build: src/kbdbacklight.c
	gcc -Wall -o kbdbacklight src/kbdbacklight.c

install: kbdbacklight
	install -m 4755 kbdbacklight $(DESTDIR)/usr/local/bin/kbdbacklight

remove:
	rm -fv $(DESTDIR)/usr/local/bin/kbdbacklight
