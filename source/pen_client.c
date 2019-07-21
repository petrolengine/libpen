#include <assert.h>

#include "pen_context_factory.h"
#include "pen_client.h"
#include "pen_callback.h"
#include "pen_event.h"
#include "pen_read_internal.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

static void __reader_finish_callback(PenEventBase_t *ctx);
static void __reader_closed_callback(PenEventBase_t *ctx);
static void __reader_tcp_message_callback(
        PenEventBase_t *ctx, PenTcpHeader_t *hdr, void *data);

struct PenClient {
    PenEventBase_t ctx_;
    PenReaderBase_t reader_;
    bool is_writeable_;
    bool is_readable_;
    uint8_t server_type_;
    void *user_;
};

typedef struct PenClientMgr {
    PenContextFactory_t *cf_;
} PenClientMgr_t;

static struct {
    PenClientMgr_t **gm_;
    PenEvent_t *ev_;
} self;

static PenReaderCallback_t g_reader_cbs = {
    __reader_finish_callback,
    __reader_closed_callback,
    __reader_tcp_message_callback
};

static inline void
__client_mgr_init(size_t st)
{
    PenClientMgr_t *cm = NULL;
    size_t clients_sz = PEN_OPTION_NAME(server_types)[st].maxclients;

    if (clients_sz == 0)
        return ;

    cm = (PenClientMgr_t*)calloc(1, sizeof(PenClientMgr_t));
    if (cm == NULL) {
        PEN_LOG_ERROR("pen client mgr init error. outof memory.\n");
        abort();
    }
    cm->cf_ = PEN_CONTEXT_FACTORY_INIT(clients_sz, PenClient_t);
    self.gm_[st] = cm;
}

void
pen_client_init(PenEvent_t *ev)
{
    self.ev_ = ev;
    self.gm_ = calloc(PEN_OPTION_NAME(server_types_size), sizeof(void*));
    if (self.gm_ == NULL) {
        PEN_LOG_ERROR("pen client init error. out of memory.\n");
        abort();
    }
    PEN_FOREACH_SERVER_TYPES() {
        __client_mgr_init(i);
    }
}

static inline void
__client_mgr_destroy(PenClientMgr_t *cm)
{
    PenClient_t *client = NULL;

    while ((client = pen_context_factory_pop(cm->cf_)) != NULL) {
        assert(pen_event_del(self.ev_, &client->ctx_));
        close(client->ctx_.fd_);
        pen_context_factory_put(cm->cf_, client);
    }

    pen_context_factory_destroy(cm->cf_);
    free(cm);
}

void
pen_client_destroy()
{
    PEN_FOREACH_SERVER_TYPES() {
        if (self.gm_[i] != NULL) {
            __client_mgr_destroy(self.gm_[i]);
        }
    }
    free(self.gm_);
    bzero(&self, sizeof(self));
}

static void
__event_callback(PenEventBase_t *ctx)
{
    PenClient_t *client = PEN_STRUCT_ENTRY(ctx, PenClient_t, ctx_);

    if (pen_event_is_closed(ctx)) {
        // TODO check working clients
        pen_client_del(client);
        return ;
    }

    if (pen_event_is_readable(ctx)) {
        pen_event_clear_read(ctx);
        // TODO put to read queue
        pen_read_tcp(&client->reader_, ctx);
    }

    // TODO thread safe ???
    if (pen_event_is_writeable(ctx)) {
        pen_event_clear_write(ctx);
        client->is_writeable_ = true;
        // TODO send rest data
    }
}

void
pen_client_add(int fd, uint8_t servertype)
{
    PenClient_t *client = NULL;
    PenClientMgr_t *pm = self.gm_[servertype - 1];

    assert(pm != NULL);
    client = pen_context_factory_get(pm->cf_);
    client->ctx_.fd_ = fd;
    client->ctx_.cb_ = __event_callback;
    client->reader_.cbs_ = &g_reader_cbs;
    client->server_type_ = servertype;
    client->user_ = pen_callback_new_client(client, servertype);

    if (!pen_event_add(self.ev_, PEN_EVENT_RDWR, &client->ctx_)) {
        PEN_LOG_ERROR("pen client add event error.\n");
        abort();
    }
}

void
pen_client_del(PenClient_t *client)
{
    if (!pen_event_del(self.ev_, &client->ctx_)) {
        PEN_LOG_ERROR("pen client del event error.\n");
        abort();
    }
    close(client->ctx_.fd_);
    pen_context_factory_put(self.gm_[client->server_type_ -1]->cf_, client);
}

static void
__reader_finish_callback(PenEventBase_t *ctx)
{
    PenClient_t *client = PEN_STRUCT_ENTRY(ctx, PenClient_t, ctx_);

    if (client->reader_.is_readable_) {
        // TODO put to read queue
        pen_read_tcp(&client->reader_, ctx);
    }
}

static void
__reader_closed_callback(PenEventBase_t *ctx)
{
    (void) ctx;
}

static void
__reader_tcp_message_callback(
        PenEventBase_t *ctx, PenTcpHeader_t *hdr, void *data)
{
    PenClient_t *client = PEN_STRUCT_ENTRY(ctx, PenClient_t, ctx_);
    pen_callback_new_message(client->server_type_, hdr, data, client->user_);
}

void
pen_client_send(PenClient_t *client, const void *data, size_t len)
{
    pen_write_internal(client->ctx_.fd_,
                       pen_get_sock_type(client->server_type_), data, len);
}
