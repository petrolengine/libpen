#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <3rd/list.h>

#include "pen_callback.h"
#include "pen_connector.h"
#include "pen_event.h"
#include "pen_read_internal.h"
#include "sock_utils.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif

// TODO add locks for registed list.
// TODO read write for connector

static void __reader_finish_callback(PenEventBase_t *ctx);
static void __reader_closed_callback(PenEventBase_t *ctx);
static void __reader_tcp_message_callback(
        PenEventBase_t *ctx, PenTcpHeader_t *hdr, void *data);

typedef struct PenConnector {
    PenEventBase_t ctx_;
    PenReaderBase_t reader_;
    bool is_unix_;
    bool is_connected_;
    bool is_writeable_;
    const void *cfg_;
    void *user_;
} PenConnector_t;

typedef struct PenConnectorGroup {
    PenConnector_t *conns_;
    size_t size_;
} PenConnectorGroup_t;

static struct {
    PenConnectorGroup_t **gc_;
    PenEvent_t *ev_;
} self;

static PenReaderCallback_t g_reader_cbs = {
    __reader_finish_callback,
    __reader_closed_callback,
    __reader_tcp_message_callback
};


static inline void
__print_connector(PenConnector_t *conn, const char *msg)
{
#if HAVE_SYS_UN_H
    if (conn->is_unix_)
        PEN_LOG_INFO("[%s] %s to %s.\n"
                , ((PEN_OPTION_STRUCT_NAME(unix_connector)*)conn->cfg_)->name
                , msg
                , ((PEN_OPTION_STRUCT_NAME(unix_connector)*)conn->cfg_)->host);
    else
#endif
        PEN_LOG_INFO("[%s] %s to %s:%u.\n"
                , ((PEN_OPTION_STRUCT_NAME(tcp_connector)*)conn->cfg_)->name
                , msg
                , ((PEN_OPTION_STRUCT_NAME(tcp_connector)*)conn->cfg_)->ip
                , ((PEN_OPTION_STRUCT_NAME(tcp_connector)*)conn->cfg_)->port);
}

static inline bool
__reset_connector(PenConnector_t *conn)
{
    int domain = 0, flags = SOCK_STREAM;

    if (conn->is_connected_)
        return false;
    if (conn->ctx_.fd_ != INVALID_SOCKET) {
        assert(pen_event_del(self.ev_, &conn->ctx_));
        close(conn->ctx_.fd_);
        conn->ctx_.fd_ = INVALID_SOCKET;
        conn->ctx_.state_ = 0;
    }
    domain = (conn->is_unix_) ? AF_UNIX : AF_INET;
#ifdef SOCK_CLOEXEC
    flags |= SOCK_CLOEXEC;
#endif
#ifdef SOCK_NONBLOCK
    flags |= SOCK_NONBLOCK;
    conn->ctx_.fd_ = socket(domain, flags, 0);
#else
    conn->ctx_.fd_ = socket(domain, flags, 0);
    pen_set_nonblock(conn->ctx_.fd_);
#endif
    if (conn->ctx_.fd_ == INVALID_SOCKET) {
        PEN_LOG_ERROR("Do tcp reconnect error, create socket failed.\n");
        abort();
    }
    if (!pen_event_add(self.ev_, PEN_EVENT_RDWR, &conn->ctx_)) {
        PEN_LOG_ERROR("Do tcp reconnect error, event add failed.\n");
        abort();
    }
    return true;
}

#if HAVE_SYS_UN_H

static inline void
__do_unix_reconnect(PenConnector_t *conn)
{
    struct sockaddr_un svr;
    const PEN_OPTION_STRUCT_NAME(unix_connector) *cfg = conn->cfg_;

    if (!__reset_connector(conn))
        return ;
    bzero(&svr, sizeof(svr));
    svr.sun_family = AF_UNIX;
    strcpy(svr.sun_path, cfg->host);

    __print_connector(conn, "connecting");
    if (connect(conn->ctx_.fd_, (struct sockaddr*)&svr, sizeof(svr)) == 0)
        conn->is_connected_ = true;
    else
        errno = 0;
}

#endif

static inline void
__do_tcp_reconnect(PenConnector_t *conn)
{
    struct sockaddr_in svr;
    const PEN_OPTION_STRUCT_NAME(tcp_connector) *cfg = conn->cfg_;

    if (!__reset_connector(conn))
        return ;
    bzero(&svr, sizeof(svr));
    svr.sin_family = AF_INET;
    svr.sin_addr.s_addr = inet_addr(cfg->ip);
    svr.sin_port = htons(cfg->port);

    __print_connector(conn, "connecting");
    if (connect(conn->ctx_.fd_, (struct sockaddr*)&svr, sizeof(svr)) == 0)
        conn->is_connected_ = true;
    else
        errno = 0;
}

static inline uint8_t
__get_connector_server_type(PenConnector_t *conn)
{
    return conn->is_unix_
        ? ((PEN_OPTION_STRUCT_NAME(unix_connector)*)conn->cfg_)->servertype
        : ((PEN_OPTION_STRUCT_NAME(tcp_connector)*)conn->cfg_)->servertype;
}

static inline const char *
__get_connector_name(PenConnector_t *conn)
{
    return conn->is_unix_
        ? ((PEN_OPTION_STRUCT_NAME(unix_connector)*)conn->cfg_)->name
        : ((PEN_OPTION_STRUCT_NAME(tcp_connector)*)conn->cfg_)->name;
}

static inline void
__do_reconnect(PenConnector_t *conn)
{
    pen_event_clear_close(&conn->ctx_);
    // TODO 1. reconnect auto???
    // TODO 2. clear temp buffer
    if (conn->is_connected_) {
        conn->is_connected_ = false;
        __print_connector(conn, "disconnected");
        pen_callback_disconnected(__get_connector_server_type(conn),
                                  __get_connector_name(conn), conn->user_);
    } else {
        __print_connector(conn, "connect failed");
    }
    if (!conn->is_unix_)
        __do_tcp_reconnect(conn);
#if HAVE_SYS_UN_H
    else
        __do_unix_reconnect(conn);
#endif
}

static void
__event_callback(PenEventBase_t *ctx)
{
    PenConnector_t *conn = PEN_STRUCT_ENTRY(ctx, PenConnector_t, ctx_);

    if (pen_event_is_closed(ctx)) {
        __do_reconnect(conn);
        return ;
    }

    if (pen_event_is_readable(ctx)) {
        pen_event_clear_read(ctx);
        // TODO put to read queue
        pen_read_tcp(&conn->reader_, ctx);
    }

    if (pen_event_is_writeable(ctx)) {
        pen_event_clear_write(ctx);
        conn->is_writeable_ = true;
        if (!conn->is_connected_) {
            conn->is_connected_ = true;
            __print_connector(conn, "connected");
        }
        conn->user_ = pen_callback_connected(__get_connector_server_type(conn),
                __get_connector_name(conn));
        // TODO send rest data
    }
}

#if HAVE_SYS_UN_H

static inline void
__do_init_unix_connector(
        PenConnector_t *conn,
        PEN_OPTION_STRUCT_NAME(unix_connector) *cfg)
{
    conn->is_unix_ = true;
    conn->cfg_ = cfg;
    conn->ctx_.fd_ = INVALID_SOCKET;
    conn->ctx_.cb_ = __event_callback;
    conn->reader_.cbs_ = &g_reader_cbs;
    __do_unix_reconnect(conn);
}

#endif

static inline void
__do_init_tcp_connector(
        PenConnector_t *conn,
        PEN_OPTION_STRUCT_NAME(tcp_connector) *cfg)
{
    conn->cfg_ = cfg;
    conn->ctx_.fd_ = INVALID_SOCKET;
    conn->ctx_.cb_ = __event_callback;
    conn->reader_.cbs_ = &g_reader_cbs;
    __do_tcp_reconnect(conn);
}

static inline void
__init_connector_for_type(uint8_t tp, size_t num_of_type)
{
    size_t total_sz = 0;
    PenConnectorGroup_t *pg = NULL;
    PenConnector_t *ptr = NULL;

    total_sz = sizeof(PenConnectorGroup_t)
        + sizeof(PenConnector_t) * num_of_type
        + alignof(PenConnector_t);
    pg = calloc(1, total_sz);
    if (pg == NULL) {
        PEN_LOG_ERROR("pen_connector_init error, out of memory.\n");
        abort();
    }
    pg->conns_ = (PenConnector_t*)pen_align_ptr(
            (char*)pg + sizeof(PenConnectorGroup_t),
            alignof(PenConnector_t));
    pg->size_ = num_of_type;

    ptr = pg->conns_;
    if (PEN_OPTION_NAME(tcp_connector).servertype == tp)
        __do_init_tcp_connector(ptr++, &PEN_OPTION_NAME(tcp_connector));

    for (size_t i = 0; i < PEN_OPTION_NAME(tcp_connectors_size); i++) {
        if (PEN_OPTION_NAME(tcp_connectors)[i].servertype == tp)
            __do_init_tcp_connector(ptr++, &PEN_OPTION_NAME(tcp_connectors)[i]);
    }

#if HAVE_SYS_UN_H

    if (PEN_OPTION_NAME(unix_connector).servertype == tp)
        __do_init_unix_connector(ptr++, &PEN_OPTION_NAME(unix_connector));

    for (size_t i = 0; i < PEN_OPTION_NAME(unix_connectors_size); i++) {
        if (PEN_OPTION_NAME(unix_connectors)[i].servertype == tp)
            __do_init_unix_connector(ptr++,
                    &PEN_OPTION_NAME(unix_connectors)[i]);
    }

#endif

    self.gc_[tp - 1] = pg;
}

static inline uint8_t
__get_num_of_type(uint8_t tp)
{
    uint8_t ret = 0;

    if (PEN_OPTION_NAME(tcp_connector).servertype == tp)
        ret ++;

    for (size_t i = 0; i < PEN_OPTION_NAME(tcp_connectors_size); i++) {
        if (PEN_OPTION_NAME(tcp_connectors)[i].servertype == tp)
            ret ++;
    }

#if HAVE_SYS_UN_H

    if (PEN_OPTION_NAME(unix_connector).servertype == tp)
        ret ++;

    for (size_t i = 0; i < PEN_OPTION_NAME(unix_connectors_size); i++) {
        if (PEN_OPTION_NAME(unix_connectors)[i].servertype == tp)
            ret ++;
    }

#endif

    return ret;
}

void
pen_connector_init(PenEvent_t *ev)
{
    size_t num_of_type = 0;

    pen_connector_destroy();

    self.gc_ = calloc(PEN_OPTION_NAME(server_types_size), sizeof(void*));
    if (self.gc_ == NULL) {
        PEN_LOG_ERROR("pen_connector_init error, out of memory.\n");
        abort();
    }
    self.ev_ = ev;
    
    PEN_FOREACH_SERVER_TYPES() {
        num_of_type = __get_num_of_type(i + 1);
        if (num_of_type == 0)
            continue;
        __init_connector_for_type(i + 1, num_of_type);
    }
}

static inline void
__do_connector_destroy(PenConnector_t *conn)
{
    if (conn->ctx_.fd_ != INVALID_SOCKET) {
        assert(pen_event_del(self.ev_, &conn->ctx_));
        close(conn->ctx_.fd_);
        conn->ctx_.fd_ = INVALID_SOCKET;
    }
    PEN_LOG_DEBUG("Connector %s destroy.\n"
            , conn->is_unix_
            ? ((PEN_OPTION_STRUCT_NAME(unix_connector)*)conn->cfg_)->name
            : ((PEN_OPTION_STRUCT_NAME(tcp_connector)*)conn->cfg_)->name);
}

static inline void
__do_connector_group_destroy(PenConnectorGroup_t *gp)
{
    if (gp == NULL)
        return ;

    for (size_t i = 0; i < gp->size_; i++)
        __do_connector_destroy(&gp->conns_[i]);

    free(gp);
}

void
pen_connector_destroy()
{
    if (self.gc_ == NULL)
        return ;

    PEN_FOREACH_SERVER_TYPES()
        __do_connector_group_destroy(self.gc_[i]);

    free(self.gc_);
    bzero(&self, sizeof(self));
}

static void
__reader_finish_callback(PenEventBase_t *ctx)
{
    PenConnector_t *conn = PEN_STRUCT_ENTRY(ctx, PenConnector_t, ctx_);

    if (conn->reader_.is_readable_) {
        // TODO put to read queue
        pen_read_tcp(&conn->reader_, ctx);
    }
}

static void
__reader_closed_callback(PenEventBase_t *ctx)
{
    __do_reconnect(PEN_STRUCT_ENTRY(ctx, PenConnector_t, ctx_));
}

static void
__reader_tcp_message_callback(
        PenEventBase_t *ctx, PenTcpHeader_t *hdr, void *data)
{
    PenConnector_t *conn = PEN_STRUCT_ENTRY(ctx, PenConnector_t, ctx_);
    (void) hdr;
    (void) data;
    __print_connector(conn, "new tcp message");
}
