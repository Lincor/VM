AC_INIT([vm], [0.1])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AC_PROG_CC

AC_CONFIG_HEADERS(include/config.h)

AC_CHECK_HEADERS(conio.h)

AC_ARG_VAR(SECTOR_SIZE, [ HDD sector size (in bytes) \[512\] ])
AS_IF([test -z $SECTOR_SIZE], AC_SUBST(SECTOR_SIZE, 512))

AC_ARG_ENABLE([winapi],
[  --enable-winapi         Enable WinAPI UI backend],
[ AC_DEFINE([ENABLE_WINAPI], [1], [ Define if WinAPI backend enabled. ])])

AC_ARG_ENABLE([xcb], 
[  --enable-xcb         Enable XCB UI backend],
[ AC_CHECK_LIB(xcb, xcb_connect, [], [ AC_MSG_ERROR([ XCB library not 
found. ]) ])
AC_DEFINE([ENABLE_XCB], [1], [ Define if WinAPI backend enabled. ])])

AC_OUTPUT([Makefile src/Makefile])
