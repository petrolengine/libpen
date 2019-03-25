#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/stat.h>

#include "log.h"


#define BUFFER_SIZE 10239

static int access_log_fd = -1;
static int error_log_fd = -1;

struct log_buffer {
    time_t t;
    size_t offset;
    char buf[BUFFER_SIZE + 1];
};

static thread_local struct log_buffer buf;

static inline const char *
__get_clean_filename(const char *filename)
{
    const char *p = strrchr(filename, '/');
    if (p != NULL )
        return p + 1;
    return filename;
}

static inline void
__clean_end_line()
{
    if (buf.buf[buf.offset - 1] == '\n') {
        buf.buf[buf.offset - 1] = '\0';
        buf.offset --;
    }
}

static inline void
__get_log_base_info(
        const enum eLogLevel lv,
        const char *f,
        const unsigned l,
        const char *func)
{
    int ret = 0, rest = BUFFER_SIZE;
    static const char *LEVEL_STR[] = {
        "[DEBUG] ",
        "[INFO] ",
        "[WARN] ",
        "[ERROR] "
    };

    time(&buf.t);
#if defined(HAVE_CTIME_S)
    if (ctime_s(buf.buf, BUFFER_SIZE, &buf.t) != 0)
#elif defined(HAVE_CTIME_R)
    if (ctime_r(&buf.t, buf.buf) == NULL)
#endif
        abort();
    buf.offset = strlen(buf.buf);
    __clean_end_line();
    rest -= buf.offset;

    ret = snprintf(buf.buf + buf.offset,
            rest,
            " %s%s<%s(%u)> ",
            LEVEL_STR[lv],
            __get_clean_filename(f),
            func,
            l);
    assert(ret < rest);
    buf.offset += ret;
}

static inline void
__do_log(const enum eLogLevel lv)
{
    if (PEN_OPTION_NAME(log_console)) {
        printf("%s", buf.buf);
    }

    if (error_log_fd == -1 || access_log_fd == -1) {
        return ;
    }
    if (lv >= eLogWarn) {
        assert((size_t)write(error_log_fd, buf.buf, buf.offset) == buf.offset);
    } else {
        assert((size_t)write(access_log_fd, buf.buf, buf.offset) == buf.offset);
    }
}

void
__log_printf(
        const enum eLogLevel lv,
        const char *f,
        const unsigned l,
        const char *func,
        const char *fmt,
        ...)
{
    va_list ap;
    int ret = 0;

    if (lv < PEN_OPTION_NAME(log_level)) {
        return ;
    }

    __get_log_base_info(lv, f, l, func);

    va_start(ap, fmt);
    ret = vsnprintf(buf.buf + buf.offset, BUFFER_SIZE - buf.offset, fmt, ap);
    va_end(ap);

    assert(ret > 0);
    buf.offset += ret;
    if (errno != 0) {
        __clean_end_line();
        snprintf(buf.buf + buf.offset, BUFFER_SIZE - buf.offset, ": %s\n"
                , strerror(errno));
    }
    __do_log(lv);
}

static inline void
__make_log_base_dir(const char *base_dir)
{
    char *dir = NULL;
    char *last = NULL;
    char *p = NULL;
    char filename[FILENAME_MAX] = { 0x00 };

    if (base_dir == NULL)
        return;

    dir = strdup(base_dir);
    last = dir + 1;

    while (last && (p = strchr(last, '/'))) {
        if (last != p) {
            *p = '\0';
            if (access(dir, F_OK) == -1) {
                if (mkdir(dir, 0755) == -1) {
                    perror("mkdir log base dir error.");
                    exit(EXIT_FAILURE);
                }
            }
            *p = '/';
        }
        last = p + 1;
    }
    free(dir);

    snprintf(
            filename,
            FILENAME_MAX - 1,
            "%s/%s.access_log",
            base_dir,
            appname);
    access_log_fd = open(filename, O_CREAT | O_RDWR | O_APPEND, 0644);
    snprintf(
            filename,
            FILENAME_MAX - 1,
            "%s/%s.error_log",
            base_dir,
            appname);
    error_log_fd = open(filename, O_CREAT | O_RDWR | O_APPEND, 0644);
}

PEN_NOTHROW
PEN_CONSTRUCTOR(PEN_CONSTRUCTOR_LOG)
static void
pen_log_init()
{
    assert(access_log_fd == -1);
    __make_log_base_dir(PEN_OPTION_NAME(log_dir));
}

PEN_NOTHROW
PEN_DESTRUCTOR(PEN_CONSTRUCTOR_LOG)
static void
pen_log_destroy()
{
    if (access_log_fd != -1) {
        close(access_log_fd);
    }
    if (error_log_fd != -1) {
        close(error_log_fd);
    }
}
