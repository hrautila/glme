
# remove all file created by tools

AUTOFILES="config.h \
 config.h.in \
 config.log \
 config.status \
 config.sub \
 config.guess \
 compile \
 depcomp \
 missing \
 Makefile \
 Makefile.in \
 ltmain.sh \
 install-sh \
 stamp-h1 \
 aclocal.m4 \
 configure \
 libtool \
 src/Makefile \
 src/Makefile.in \
 test/Makefile \
 test/Makefile.in"

AUTODIRS="autom4te.cache m4"

make distclean

for f in  ${AUTOFILES}
do 
    test -f $f && rm -f $f
done
rm -fr $AUTODIRS



