#define LOOP 20000000

#pragma GCC push_options
#pragma GCC optimize ("O0")

static inline void
sleep_0003s(int num) {
    for (int i = 0; i < num; i++) {
        ;
    }
}

#pragma GCC pop_options
