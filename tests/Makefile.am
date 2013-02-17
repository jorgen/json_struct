TESTS =						\
	json-tokenizer-test			\
	json-tokenizer-fail-test		\
	json-tokenizer-partial-test		\
	json-tree-test				\
	json-tree-printer-test

check_PROGRAMS =				\
	$(TESTS)

json_tokenizer_test_SOURCES = json-tokenizer-test.cpp

json_tokenizer_fail_test_SOURCES = json-tokenizer-fail-test.cpp

json_tokenizer_partial_test_SOURCES = json-tokenizer-partial-test.cpp

json_tree_test_SOURCES = json-tree-test.cpp

json_tree_printer_test_SOURCES = json-tree-printer-test.cpp

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libjson_tools.la

