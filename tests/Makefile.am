TESTS =						\
	json-tokenizer-test			\
	json-tokenizer-fail-test		\
	json-tokenizer-partial-test

check_PROGRAMS =				\
	$(TESTS)

json_tokenizer_test_SOURCES = json-tokenizer-test.cpp

json_tokenizer_fail_test_SOURCES = json-tokenizer-fail-test.cpp

json_tokenizer_partial_test_SOURCES = json-tokenizer-partial-test.cpp

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libjson_tokenizer.la

