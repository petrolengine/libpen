#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libpen],[0.0.0],[https://www.justforfun.cn/wordpress])
AC_CONFIG_HEADERS(source/config.h)
AC_CONFIG_AUX_DIR(m4)

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIRS([m4])

AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_mac=no
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_mac=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_PROG_MAKE_SET

AM_PROG_AR
LT_INIT
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h strings.h threads.h pthread.h \
    unistd.h sys/epoll.h sys/inotify.h sys/un.h sys/event.h sys/timerfd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([ctime_r ctime_s mkdir strchr strdup strerror strrchr accept4])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
