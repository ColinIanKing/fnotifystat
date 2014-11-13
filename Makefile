#
# Copyright (C) 2014 Canonical, Ltd.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

VERSION=0.01.04

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
