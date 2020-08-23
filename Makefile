include config.mk

SRC = pptxt.c util.c
OBJ = ${SRC:.c=.o}

all: options clean pptxt

options:
	@echo pptxt build options:
	@echo "CFLAGS	= ${CFLAGS}"
	@echo "LDFLAGS	= ${LDFLAGS}"
	@echo "CC		= ${CC}"

${OBJ}: config.mk

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

pptxt: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f pptxt ${OBJ} pptxt-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p pptxt-${VERSION}
	@cp -R LICENSE Makefile config.mk ${SRC} pptxt-${VERSION}
	@tar -cf pptxt-${VERSION}.tar pptxt-${VERSION}
	@gzip pptxt-${VERSION}.tar
	@rm -rf pptxt-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f pptxt ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/pptxt

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/pptxt


.PHONY: all options clean install uninstall
