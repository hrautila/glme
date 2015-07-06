
AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = -I../ -I ../src  -I../src/inc
LDADD = ../src/libglme.la

PROGS = \
	test_s0 \
	test_s1 \
	test_da0 \
	test_da1


test_s0_SOURCES = test_s0.c
test_s1_SOURCES = test_s1.c
test_da0_SOURCES = test_da0.c
test_da1_SOURCES = test_da1.c

check_PROGRAMS = $(PROGS)

TESTS = $(check_PROGRAMS)



