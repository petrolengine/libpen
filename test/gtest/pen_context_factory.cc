#include <gtest/gtest.h>

#include "../../source/pen_context_factory.h"
#include "../../source/log.h"
#include "../../source/option.h"

class PenContextFactoryTest : public ::testing::Test {
protected:
    PenContextFactoryTest() {
    }

    ~PenContextFactoryTest() {
    }

    void SetUp() override {
        PEN_OPTION_NAME(log_level) = 0;
    }

    void TearDown() override {

    }
};

typedef struct pen_test {
    int c;
    double b;
} pen_test;

TEST_F(PenContextFactoryTest, one) {
    PenContextFactory_t *self = pen_context_factory_init(
            5, sizeof(pen_test), alignof(pen_test));
    void *ctx = pen_context_factory_get(self);

    for (int i = 0; i < 66; i++) {
        bzero(ctx, sizeof(pen_test));
        pen_context_factory_put(self, ctx);
        ASSERT_EQ(pen_context_factory_get(self), ctx);
    }

    ASSERT_EQ(pen_context_factory_size(self), 1);
    pen_context_factory_destroy(self);
}

TEST_F(PenContextFactoryTest, two) {
    PenContextFactory_t *self = pen_context_factory_init(
            5, sizeof(pen_test), alignof(pen_test));
    void *ctx[6] = { nullptr };

    for (int j = 0; j < 6; j++) {
        ctx[j] = pen_context_factory_get(self);
        bzero(ctx[j], sizeof(pen_test));
    }

    for (int i = 0; i < 66; i++) {
        for (int j = 5; j >= 0; j--) {
            pen_context_factory_put(self, ctx[j]);
        }
        for (int j = 0; j < 6; j++) {
            ASSERT_EQ(pen_context_factory_get(self), ctx[j]);
            bzero(ctx[j], sizeof(pen_test));
        }
    }

    ASSERT_EQ(pen_context_factory_size(self), 6);
    pen_context_factory_destroy(self);
}
