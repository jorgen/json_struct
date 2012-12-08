TESTS =						\
	json-tokenizer-test

check_PROGRAMS =				\
	$(TESTS)

json_tokenizer_test_SOURCES = json-tokenizer-test.cpp

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libjson_tokenizer.la

