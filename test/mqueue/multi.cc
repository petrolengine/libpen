#include <thread>
#include <vector>
#include <assert.h>
#include <string.h>
#include <sstream>
#include <chrono>

#include "../../source/log.h"
#include "../../source/option.h"
#include "../../source/pen_mqueue.h"

#include "common.h"

std::vector<PenMqueue_t*> qs;


static inline PenMqueue_t *
get_queue(int rnd)
{
    return qs[rnd % qs.size()];
}

void
setter_func()
{
    char *data = NULL;
    std::stringstream ss;
    PenMqueue_t *pqueue = NULL;

    ss << std::this_thread::get_id();
    PEN_LOG_DEBUG("set thread %s start.\n", ss.str().c_str());
    for (int i = 0; i < LOOP; i++) {
        data = strdup("hello world");
        pqueue = get_queue(rand());
        pen_mqueue_put(pqueue, data);
        sleep_0003s(100);
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
    std::vector<std::thread> setter;
    std::vector<std::thread> getter;
    char buf[32];

    PEN_OPTION_NAME(log_console) = true;
    PEN_OPTION_NAME(log_level) = 0;

    for (int i = 0; i < 2; i++) {
        sprintf(buf, "test queue[%d]", i);
        qs.push_back(pen_mqueue_init(buf, 10));
        getter.push_back(std::thread(getter_func, qs[i]));
    }
    for (int i = 0; i < 1; i++) {
        setter.push_back(std::thread(setter_func));
    }

    for (auto &th : setter) th.join();
    for (auto th : qs) pen_mqueue_exit(th);
    for (auto &th : getter) th.join();
    for (auto th : qs) pen_mqueue_destroy(th);
}
