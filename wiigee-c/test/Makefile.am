INCLUDES   = -I$(top_srcdir)/include

CFLAGS = -std=c99

bin_PROGRAMS = gesturemodel_test hmm_test quantizer_test

gesturemodel_test_SOURCES   = gesturemodel_test.c
gesturemodel_test_LDADD     = $(top_builddir)/lib/libwiigestures.la
hmm_test_SOURCES   = hmm_test.c
hmm_test_LDADD     = $(top_builddir)/lib/libwiigestures.la
quantizer_test_SOURCES   = quantizer_test.c
quantizer_test_LDADD     = $(top_builddir)/lib/libwiigestures.la
