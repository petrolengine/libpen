#include <thread>
#include <vector>
#include <assert.h>
#include <string.h>
#include <sstream>
#include <atomic>
#include <chrono>

#include "../../source/log.h"
#include "../../source/option.h"
#include "../../source/pen_mqueue.h"

#include "common.h"

void
setter_func(PenMqueue_t *pqueue)
{
    char *data = NULL;
    std::stringstream ss;

    ss << std::this_thread::get_id();
    PEN_LOG_DEBUG("set thread %s start.\n", ss.str().c_str());
    for (int i = 0; i < LOOP; i++) {
        data = strdup("hello world");
        pen_mqueue_put(pqueue, data);
        sleep_0003s(100);
        //std::this_thread::sleep_for (std::chrono::nanoseconds(1));
    }
    PEN_LOG_DEBUG("set thread %s exit.\n", ss.str().c_str());
}

void
getter_func(PenMqueue_t *pqueue)
{
    void *data = NULL;
    std::stringstream ss;

    ss << std::this_thread::get_id();
    PEN_LOG_DEBUG("get thread %s start.\n", ss.str().c_str());
    while ((data = pen_mqueue_get(pqueue)) != NULL) {
        assert(strcmp((char*)data, "hello world") == 0);
        memset(data, 0, strlen((char*)data));
        //sleep_0003s(200);
        free(data);
    }
    PEN_LOG_DEBUG("get thread %s exit.\n", ss.str().c_str());
}

int
main()
{
    PenMqueue_t *pqueue = NULL;
    std::vector<std::thread> setter;
    std::vector<std::thread> getter;

    PEN_OPTION_NAME(log_console) = true;
    PEN_OPTION_NAME(log_level) = 0;

    pqueue = pen_mqueue_init("test queue", 10);
    for (int i = 0; i < 2; i++) {
        getter.push_back(std::thread(getter_func, pqueue));
    }
    for (int i = 0; i < 1; i++) {
        setter.push_back(std::thread(setter_func, pqueue));
    }
    for (auto &th : setter) th.join();
    pen_mqueue_exit(pqueue);
    for (auto &th : getter) th.join();
    pen_mqueue_destroy(pqueue);
}
