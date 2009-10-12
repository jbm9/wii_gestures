INCLUDES   = -I$(top_srcdir)/include

CFLAGS = -std=c99 -lcwiid -Wall -g

bin_PROGRAMS = quantizer_grab hmm_grab

quantizer_grab_SOURCES   = quantizer_grab.c
quantizer_grab_LDADD     = $(top_builddir)/lib/libwiigestures.la

hmm_grab_SOURCES   = hmm_grab.c
hmm_grab_LDADD     = $(top_builddir)/lib/libwiigestures.la
