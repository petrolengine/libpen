#include <config.h>

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

#if HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>
#endif

#include "../../source/pen_event.h"
#include "../../source/pen_file_monitor.h"

static PenEvent_t *self = NULL;

void
event_callback(const char *filename, uint32_t mask)
{
    if (mask == PEN_FILE_MONITOR_EVENT_MODIFY) {
        printf("modify.\n");
    } else if (mask == PEN_FILE_MONITOR_EVENT_DELETE) {
        pen_event_stop(self);
        printf("delete.\n");
    } else{
        assert(false);
    }
}

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        perror("usage: libpen_event [test file].\n");
        exit(EXIT_FAILURE);
    }

    PEN_OPTION_NAME(log_level) = 0;

    self = pen_event_init(1);
    pen_file_monitor_init(self);

    assert(pen_file_monitor_add(argv[1], event_callback));
    pen_event_start(self);

    pen_file_monitor_destroy(self);
    pen_event_destroy(self);

    printf("exit.\n");

    return 0;
}
