#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "pen_listener.h"
#include "pen_event.h"
#include "sock_utils.h"
#include "pen_client.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif

// TODO add close/open listener for application.

typedef struct PenListener {
    PenEventBase_t ctx_;
    bool is_unix_;
    const void *cfg_;
} PenListener_t ;

static struct {
    size_t n_listeners_;
    PenListener_t *pool_;
    PenEvent_t *ev_;
} self;


static inline void
__do_tcp_accept(PenListener_t *pl)
{
    struct sockaddr_in cli;
    socklen_t cli_len = sizeof(cli);
    SOCKET client_fd = INVALID_SOCKET;
    char buf[16] = { 0X00 };
    PEN_OPTION_STRUCT_NAME(tcp_listener) *plcfg =
        (PEN_OPTION_STRUCT_NAME(tcp_listener)*)pl->cfg_;

#if HAVE_SYS_EVENT_H
    while (pl->ctx_.data_size_ --) {
#else
    while (true) {
#endif
#if HAVE_ACCEPT4
        client_fd = accept4(pl->ctx_.fd_, (struct sockaddr*)&cli,
                &cli_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
        client_fd = accept(pl->ctx_.fd_, (struct sockaddr*)&cli, &cli_len);
        pen_set_nonblock(client_fd);
#endif
        if (client_fd == -1) {
            if (errno == EAGAIN) {
                errno = 0;
                break;
            }
            PEN_LOG_ERROR("[%s] tcp accept error!!!\n", plcfg->name);
            abort();
        }
        PEN_LOG_INFO("[%s] accept new client: %s:%u\n"
                , plcfg->name
                , inet_ntop(AF_INET, &cli.sin_addr, buf, sizeof(buf))
                , ntohs(cli.sin_port));
        pen_client_add(client_fd, plcfg->servertype);
    }
}

#if HAVE_SYS_UN_H

static inline void
__do_unix_accept(PenListener_t *pl)
{
    struct sockaddr_un cli;
    socklen_t cli_len = sizeof(cli);
    SOCKET client_fd = INVALID_SOCKET;
    PEN_OPTION_STRUCT_NAME(unix_listener) *plcfg =
        (PEN_OPTION_STRUCT_NAME(unix_listener)*)pl->cfg_;

#if HAVE_SYS_EVENT_H
    while (pl->ctx_.data_size_ --) {
#else
        while (true) {
#endif
#if HAVE_ACCEPT4
        client_fd = accept4(pl->ctx_.fd_, (struct sockaddr*)&cli,
                &cli_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
        client_fd = accept(pl->ctx_.fd_, (struct sockaddr*)&cli, &cli_len);
        pen_set_nonblock(client_fd);
#endif
        if (client_fd == -1) {
            if (errno == EAGAIN) {
                errno = 0;
                break;
            }
            PEN_LOG_ERROR("[%s] unix accept error!!!\n", plcfg->name);
            abort();
        }
        PEN_LOG_INFO("[%s] accept new unix domain client\n", plcfg->name);
        pen_client_add(client_fd, plcfg->servertype);
    }
}

#endif

static void
__event_callback(PenEventBase_t *ctx)
{
    PenListener_t *pl = (PenListener_t*)ctx;

    if (pen_event_is_closed(ctx)) {
        // TODO ??? rebind ???
        PEN_LOG_ERROR("listener [%s] exit, unexceptly.\n", pl->is_unix_
                ? ((PEN_OPTION_STRUCT_NAME(unix_listener)*)pl->cfg_)->name
                : ((PEN_OPTION_STRUCT_NAME(tcp_listener)*)pl->cfg_)->name);
        abort();
    }
    if (pen_event_is_readable(ctx)) {
        pen_event_clear_read(ctx);
        if (!pl->is_unix_)
            __do_tcp_accept(pl);
#if HAVE_SYS_UN_H
        else
            __do_unix_accept(pl);
#endif
    }
}

static inline void
__init_listener_ctx(PenListener_t *pl)
{
    int domain = 0, optval = 1, fd = INVALID_SOCKET, flags = SOCK_STREAM;

    domain = (pl->is_unix_) ? AF_UNIX : AF_INET;
#ifdef SOCK_CLOEXEC
    flags |= SOCK_CLOEXEC;
#endif
#ifdef SOCK_NONBLOCK
    flags |= SOCK_NONBLOCK;
    fd = socket(domain, flags, 0);
#else
    fd = socket(domain, flags, 0);
    pen_set_nonblock(fd);
#endif
    if (fd == INVALID_SOCKET) {
        PEN_LOG_ERROR("init listener error, create socket failed.\n");
        abort();
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) != 0) {
        PEN_LOG_ERROR("[listener] set sock opt SO_REUSEPORT error.\n");
        abort();
    }

    pl->ctx_.fd_ = fd;
    pl->ctx_.cb_ = __event_callback;
    if (!pen_event_add(self.ev_, PEN_EVENT_READ, &pl->ctx_)) {
        PEN_LOG_ERROR("init listener error, event add failed.\n");
        abort();
    }
}

static inline void
__init_tcp_listener(
        PenListener_t *pl,
        PEN_OPTION_STRUCT_NAME(tcp_listener) *cfg)
{
    struct sockaddr_in svr;

    pl->cfg_ = cfg;
    assert(cfg->host != NULL);
    __init_listener_ctx(pl);
    bzero(&svr, sizeof(svr));
    svr.sin_family = AF_INET;
    svr.sin_addr.s_addr = inet_addr(cfg->host);
    svr.sin_port = htons(cfg->port);

    if (bind(pl->ctx_.fd_, (struct sockaddr*)&svr, sizeof(svr)) != 0) {
        PEN_LOG_ERROR("listener '%s' bind '%s:%hu' error.\n",
                cfg->name, cfg->host, cfg->port);
        abort();
    }
    if (listen(pl->ctx_.fd_, cfg->backlog) != 0) {
        PEN_LOG_ERROR("listener '%s' listen '%s:%hu' error.\n",
                cfg->name, cfg->host, cfg->port);
        abort();
    }

    PEN_LOG_INFO("[%s] listen on '%s:%hu'.\n", cfg->name, cfg->host, cfg->port);
}

#if HAVE_SYS_UN_H

static inline void
__init_unix_listener(
        PenListener_t *pl,
        PEN_OPTION_STRUCT_NAME(unix_listener) *cfg)
{
    struct sockaddr_un svr;

    pl->cfg_ = cfg;
    pl->is_unix_ = true;
    assert(cfg->host);
    __init_listener_ctx(pl);
    bzero(&svr, sizeof(svr));
    svr.sun_family = AF_UNIX;
    strcpy(svr.sun_path, cfg->host);
    unlink(cfg->host);
    if (bind(pl->ctx_.fd_, (struct sockaddr*)&svr, sizeof(svr)) != 0) {
        PEN_LOG_ERROR("listener '%s' bind '%s' error.\n", cfg->name, cfg->host);
        abort();
    }
    if (listen(pl->ctx_.fd_, cfg->backlog) != 0) {
        PEN_LOG_ERROR("listener '%s' listener '%s' error.\n",
                cfg->name, cfg->host);
        abort();
    }

    PEN_LOG_INFO("[%s] listen on '%s'.\n", cfg->name, cfg->host);
}

#endif

static inline uint8_t
__get_num_of_listeners()
{
    uint8_t ret = 0;

    if (PEN_OPTION_NAME(tcp_listener).name != NULL)
        ret ++;
    ret += PEN_OPTION_NAME(tcp_listeners_size);

#if HAVE_SYS_UN_H
    if (PEN_OPTION_NAME(unix_connector).name != NULL)
        ret ++;
    ret += PEN_OPTION_NAME(unix_listeners_size);
#endif

    return ret;
}

void
pen_listener_init(PenEvent_t *ev)
{
    PenListener_t *ptr = NULL;

    self.ev_ = ev;
    self.n_listeners_ = __get_num_of_listeners();
    self.pool_ = calloc(self.n_listeners_, sizeof(PenListener_t));
    if (self.pool_ == NULL) {
        PEN_LOG_ERROR("pen listener init error, out of memory.\n");
        abort();
    }

    ptr = self.pool_;
    if (PEN_OPTION_NAME(tcp_listener).name != NULL)
        __init_tcp_listener(ptr++, &PEN_OPTION_NAME(tcp_listener));
    for (size_t i = 0; i < PEN_OPTION_NAME(tcp_listeners_size); i++)
        __init_tcp_listener(ptr++, &PEN_OPTION_NAME(tcp_listeners)[i]);

#if HAVE_SYS_UN_H
    if (PEN_OPTION_NAME(unix_listener).name != NULL)
        __init_unix_listener(ptr++, &PEN_OPTION_NAME(unix_listener));
    for (size_t i = 0; i < PEN_OPTION_NAME(unix_listeners_size); i++)
        __init_unix_listener(ptr++, &PEN_OPTION_NAME(unix_listeners)[i]);
#endif

    pen_client_init(ev);
}

static inline void
__destroy_listener(PenListener_t *pl)
{
    PEN_LOG_DEBUG("Listener %s destroy.\n"
            , pl->is_unix_
            ? ((PEN_OPTION_STRUCT_NAME(unix_listener)*)pl->cfg_)->name
            : ((PEN_OPTION_STRUCT_NAME(tcp_listener)*)pl->cfg_)->name);
    if (pl->ctx_.fd_ != INVALID_SOCKET) {
        assert(pen_event_del(self.ev_, &pl->ctx_));
        close(pl->ctx_.fd_);
        pl->ctx_.fd_ = INVALID_SOCKET;
    }
}

void
pen_listener_destroy()
{
    if (self.pool_ == NULL)
        return ;
    for (size_t i = 0 ; i < self.n_listeners_; i++)
        __destroy_listener(&self.pool_[i]);
    free(self.pool_);
    bzero(&self, sizeof(self));
    pen_client_destroy();
}
