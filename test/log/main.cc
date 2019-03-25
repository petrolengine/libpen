#include <thread>
#include <vector>

#include "../../source/log.h"
#include "../../source/option.h"
#include "../../include/petrol_engine.h"

void
func(int n)
{
    for (int i = 0; i < n; i++) {
        PEN_LOG_DEBUG("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
    }
}

int
main(int argc, char *argv[])
{
    std::vector<std::thread> threads;

    PEN_OPTION_NAME(log_console) = false;
    PEN_OPTION_NAME(log_level) = 0;
    petrol_engine_init(argc, argv);

    for (int i = 0; i < 8; i++) {
        threads.push_back(std::thread(func, 100000));
    }

    for (auto &th : threads) th.join();

    petrol_engine_destroy();
}
