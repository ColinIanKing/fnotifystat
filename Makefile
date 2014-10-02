VERSION=0.01.00

CFLAGS += -Wall -Wextra -DVERSION='"$(VERSION)"'

BINDIR=/usr/bin
MANDIR=/usr/share/man/man8

fnotifystat: fnotifystat.o
	$(CC) $(CFLAGS) $< -lm -o $@ $(LDFLAGS)

fnotifystat.1.gz: fnotifystat.1
	gzip -c $< > $@

dist:
	rm -rf fnotifystat-$(VERSION)
	mkdir fnotifystat-$(VERSION)
	cp -rp Makefile fnotifystat.c fnotifystat.1 COPYING fnotifystat-$(VERSION)
	tar -zcf fnotifystat-$(VERSION).tar.gz fnotifystat-$(VERSION)
	rm -rf fnotifystat-$(VERSION)

clean:
	rm -f fnotifystat fnotifystat.o fnotifystat.1.gz
	rm -f fnotifystat-$(VERSION).tar.gz

install: fnotifystat fnotifystat.1.gz
	mkdir -p ${DESTDIR}${BINDIR}
	cp fnotifystat ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}
	cp fnotifystat.1.gz ${DESTDIR}${MANDIR}
