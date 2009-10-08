# Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([wiigestures], [0.0], [josh@joshisanerd.com])
AC_CONFIG_HEADERS([config.h:config-h.in])

AM_INIT_AUTOMAKE([1.9 foreign])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_CHECK_HEADERS([math.h])
AC_CHECK_HEADERS([string.h strings.h], [break])

AC_CHECK_FUNCS([strrchr])
AC_REPLACE_FUNCS([basename])

AC_CONFIG_FILES([test/Makefile lib/Makefile src/Makefile Makefile])
AC_OUTPUT
