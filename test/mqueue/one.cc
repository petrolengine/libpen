#include <vector>
#include <assert.h>
#include <string.h>
#include <sstream>
#include <chrono>

#include "../../source/log.h"
#include "../../source/option.h"
#include "../../source/pen_mqueue.h"

#include "common.h"

int
main()
{
    PenMqueue_t *pqueue = NULL;

    PEN_OPTION_NAME(log_console) = true;
    PEN_OPTION_NAME(log_level) = 0;
    char *data = NULL;

    pqueue = pen_mqueue_init("test queue", 10);

    for (int i = 0; i < LOOP; i++) {
        data = strdup("hello world");
        sleep_0003s(10);
        pen_mqueue_put(pqueue, data);
        data = (char*)pen_mqueue_get(pqueue);
        assert(strcmp(data, "hello world") == 0);
        sleep_0003s(200);
        free(data);
    }

    pen_mqueue_exit(pqueue);
    pen_mqueue_destroy(pqueue);
}
