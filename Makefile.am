MKDIR = mkdir -p

check_PROGRAMS = 
AUTOMAKE_OPTIONS = foreign 1.15 subdir-objects
ACLOCAL_AMFLAGS = -I m4

warn_flags = -Wall -Werror -Wextra -Wformat -Wmissing-include-dirs -Wuninitialized
g_cppflags = $(warn_flags) -pipe -march=native -DPEN_USE_EPOLL -D_GNU_SOURCE

noinst_HEADERS =
EXTRA_DIST = tools/option/auto_gen_option.py \
			 tools/option/base_type.py \
			 tools/option/gen_data.py \
			 tools/option/gen_type.py \
			 tools/option/my_type.py \
			 tools/option/option.c.in \
			 tools/option/option.h.in \
			 source/win/config.h

if OSX
g_cppflags += -DPEN_OSX
endif
if LINUX
g_cppflags += -DPEN_LINUX
endif
if WINDOWS
g_cppflags += -DPEN_WINDOWS
endif

include include/include.am
include 3rd/include.am
include source/include.am
include test/include.am

clean-local:
	-rm -f $(top_srcdir)/source/option.c $(top_srcdir)/source/option.h
	-rm -rf $(top_srcdir)/conf
