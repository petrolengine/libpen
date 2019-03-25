#include <gtest/gtest.h>

#include "../../source/log.h"
#include "../../source/option.h"

class OptionTest : public ::testing::Test {
protected:
    OptionTest() {
    }

    ~OptionTest() {
    }

    void SetUp() override {
        PEN_OPTION_NAME(log_level) = 0;
        PEN_OPTION_NAME(log_console) = true;
    }

    void TearDown() override {
        PEN_OPTION_NAME(log_level) = 0;
        PEN_OPTION_NAME(log_console) = true;
    }
};

TEST_F(OptionTest, test) {
    ASSERT_EQ(0, PEN_OPTION_NAME(log_level));
    ASSERT_EQ(0, PEN_OPTION_NAME(dummy)[0]);
    ASSERT_EQ(1, PEN_OPTION_NAME(dummy)[1]);
    ASSERT_EQ(2, PEN_OPTION_NAME(dummy)[2]);
    ASSERT_TRUE(PEN_OPTION_NAME(log_console));
    ASSERT_STREQ("test1", PEN_OPTION_NAME(tcp_connector).name);
    ASSERT_STREQ("127.0.0.1", PEN_OPTION_NAME(tcp_connector).ip);
    ASSERT_EQ(80, PEN_OPTION_NAME(tcp_connector).port);
    ASSERT_EQ(0, PEN_OPTION_NAME(tcp_connector).servertype);

    ASSERT_STREQ("test2", PEN_OPTION_NAME(tcp_connectors)[0].name);
    ASSERT_STREQ("127.0.0.1", PEN_OPTION_NAME(tcp_connectors)[0].ip);
    ASSERT_EQ(80, PEN_OPTION_NAME(tcp_connectors)[0].port);
    ASSERT_EQ(0, PEN_OPTION_NAME(tcp_connectors)[0].servertype);
}
