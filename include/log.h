#pragma once

#if defined __GNUC__ && defined __GNUC_MINOR__
#   define __GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#   define __GNUC_PREREQ(maj, min) 0
#endif

#if defined __cplusplus ? __GNUC_PREREQ(2, 6) : __GNUC_PREREQ(2, 4)
#   define PEN_FUNC __PRETTY_FUNCTION__
#else
#   if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#       define PEN_FUNC __func__
#   else
#       define PEN_FUNC "<unknown>"
#   endif
#endif

#define PEN_LOG(level, ...) \
    __log_printf(level, __FILE__, __LINE__, PEN_FUNC, __VA_ARGS__)

#define PEN_LOG_DEBUG(...) PEN_LOG(eLogDebug, __VA_ARGS__)
#define PEN_LOG_INFO(...) PEN_LOG(eLogInfo, __VA_ARGS__)
#define PEN_LOG_WARN(...) PEN_LOG(eLogWarn, __VA_ARGS__)
#define PEN_LOG_ERROR(...) PEN_LOG(eLogError, __VA_ARGS__)

#define PEN_LOG_RESET_LEVEL(level) log_level = level

#ifdef __cplusplus
extern "C" {
#endif

enum eLogLevel {
    eLogDebug,
    eLogInfo,
    eLogWarn,
    eLogError
};

// Don't use it directory, use PEN_LOG_* instead.
void __log_printf(
        enum eLogLevel lv,
        const char *f,
        const unsigned l,
        const char *func,
        const char *msg,
        ...)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(2,4,5)))
    __attribute__((format(printf,5,6)))
#endif
    ;

#ifdef __cplusplus
}
#endif
