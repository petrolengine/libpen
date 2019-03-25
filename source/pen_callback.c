#include <stdio.h>

#include "pen_callback.h"


static struct {
    PenCallbackEvent_t *g_cbs_[PEN_CALLBACK_EVENT_MAX];
} self;

void
pen_callback_init()
{
    PenCallbackEvent_t *mem = NULL;

    if (self.g_cbs_[0] != NULL)
        return ;

    mem = calloc(
            PEN_OPTION_NAME(server_types_size) * PEN_CALLBACK_EVENT_MAX
            , sizeof(PenCallbackEvent_t));
    for(int i = 0; i < PEN_CALLBACK_EVENT_MAX; i++) {
        self.g_cbs_[i] = mem + (i * PEN_OPTION_NAME(server_types_size));
    }
}

void
pen_callback_destroy()
{
    if (self.g_cbs_[0] == NULL)
        return ;

    free(self.g_cbs_[0]);
    bzero(&self, sizeof(self));
}

bool
__pen_callback_set_internal(
        uint8_t server_type,
        int event,
        void *cb,
        void *data)
{
    if (!PEN_IS_VALIDE_SERVER_TYPES(server_type))
        return false;
    if (event < 0 || event >= PEN_CALLBACK_EVENT_MAX)
        return false;
    server_type --;
    self.g_cbs_[event][server_type].cb_ = cb;
    self.g_cbs_[event][server_type].data_ = data;
    return true;
}

PenCallbackEvent_t *
__pen_callback_get_internal(int servertype, int event)
{
    servertype --;
    return &self.g_cbs_[event][servertype];
}
