#include <gtest/gtest.h>

#include "../../source/pen_buffer.h"
#include "../../source/log.h"
#include "../../source/option.h"

class PenBufferTest : public ::testing::Test {
protected:
    PenBufferTest() {
    }

    ~PenBufferTest() {
    }

    void SetUp() override {
        PEN_OPTION_NAME(log_level) = 0;
    }

    void TearDown() override {

    }
};


TEST_F(PenBufferTest, test) {
    char s[] = "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas";
    char buf[sizeof(s)] = { 0X00 };
    char s2[] = "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas"
               "fasdklfjklasfjklasjfklasjflasjlfjaslkfjaskljfklasdjfklas";
    PenBuffer *self = pen_buffer_get(16);
    pen_buffer_print(self);
    ASSERT_EQ(pen_buffer_read(self, buf, sizeof(buf)), 0);
    ASSERT_EQ(pen_buffer_write(self, s, strlen(s)), strlen(s));
    pen_buffer_print(self);
    ASSERT_EQ(pen_buffer_read(self, buf, sizeof(buf)), strlen(s));
    ASSERT_STREQ(buf, s);
    pen_buffer_print(self);
    ASSERT_EQ(pen_buffer_write(self, s2, strlen(s2)), strlen(s2));
    pen_buffer_print(self);
    for (int i = 0; i < 18; i++) {
        ASSERT_EQ(pen_buffer_read(self, buf, sizeof(buf) - 1), strlen(s));
        ASSERT_STREQ(buf, s);
    }
    pen_buffer_print(self);
    for (int i = 0; i < 18; i++) {
        ASSERT_EQ(pen_buffer_write(self, s, strlen(s)), strlen(s));
    }
    pen_buffer_print(self);
    for (int i = 0; i < 18; i++) {
        ASSERT_EQ(pen_buffer_read(self, buf, sizeof(buf) - 1), strlen(s));
        ASSERT_STREQ(buf, s);
    }
    ASSERT_EQ(pen_buffer_read(self, s, strlen(s)), 0);
    pen_buffer_print(self);
    pen_buffer_free(self);
}
