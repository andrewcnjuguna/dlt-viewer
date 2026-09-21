#include <gtest/gtest.h>

#include <qdltctrlmsg.h>

#include <QByteArray>

TEST(CtrlPayload, parse) {
    // this is real payload of a dlt control message which collect from dlt-daemon
    const auto data = QByteArray::fromHex("030000000701005359530001004d475200ffff2200436f6e74657874206f66206d61696e20646c742073797374656d206d616e616765721200444c542053797374656d204d616e6167657272656d6f");

    auto payload = qdlt::msg::payload::parse(data, false);

    ASSERT_TRUE(std::holds_alternative<qdlt::msg::payload::GetLogInfo>(payload));
    auto getLogInfo = std::get<qdlt::msg::payload::GetLogInfo>(payload);
    EXPECT_EQ(getLogInfo.status, 7);
    ASSERT_EQ(getLogInfo.apps.size(), 1);

    auto app = getLogInfo.apps[0];
    EXPECT_EQ(app.id, "SYS");
    EXPECT_EQ(app.description, "DLT System Manager");
    ASSERT_EQ(app.ctxs.size(), 1);

    auto ctx = app.ctxs[0];
    EXPECT_EQ(ctx.id, "MGR");
    EXPECT_EQ(ctx.logLevel, '\xff');
    EXPECT_EQ(ctx.traceStatus, '\xff');
    EXPECT_EQ(ctx.description, "Context of main dlt system manager");
}

/* A get log info response only carries the fields that were requested: option 4 adds
 * the log level, option 5 the trace status, option 6 both of them and option 7 also
 * the textual descriptions. Everything below option 7 must parse as well, because a
 * response containing descriptions can exceed the maximum DLT message size. */
TEST(CtrlPayload, parseGetLogInfoStatus6) {
    // status 6: application id, context id, log level and trace status, no descriptions
    const auto data = QByteArray::fromHex("030000000601005359530001004d475200ffff72656d6f");

    auto payload = qdlt::msg::payload::parse(data, false);

    ASSERT_TRUE(std::holds_alternative<qdlt::msg::payload::GetLogInfo>(payload));
    auto getLogInfo = std::get<qdlt::msg::payload::GetLogInfo>(payload);
    EXPECT_EQ(getLogInfo.status, 6);
    ASSERT_EQ(getLogInfo.apps.size(), 1);

    auto app = getLogInfo.apps[0];
    EXPECT_EQ(app.id, "SYS");
    EXPECT_EQ(app.description, "");
    ASSERT_EQ(app.ctxs.size(), 1);

    auto ctx = app.ctxs[0];
    EXPECT_EQ(ctx.id, "MGR");
    EXPECT_EQ(ctx.logLevel, -1);
    EXPECT_EQ(ctx.traceStatus, -1);
    EXPECT_EQ(ctx.description, "");
}

TEST(CtrlPayload, parseGetLogInfoStatus4) {
    // status 4: application id, context id and log level only
    const auto data = QByteArray::fromHex("030000000401005359530001004d4752000472656d6f");

    auto payload = qdlt::msg::payload::parse(data, false);

    ASSERT_TRUE(std::holds_alternative<qdlt::msg::payload::GetLogInfo>(payload));
    auto getLogInfo = std::get<qdlt::msg::payload::GetLogInfo>(payload);
    EXPECT_EQ(getLogInfo.status, 4);
    ASSERT_EQ(getLogInfo.apps.size(), 1);
    ASSERT_EQ(getLogInfo.apps[0].ctxs.size(), 1);

    auto ctx = getLogInfo.apps[0].ctxs[0];
    EXPECT_EQ(ctx.id, "MGR");
    EXPECT_EQ(ctx.logLevel, 4);
    // not part of the response, so it stays at the default
    EXPECT_EQ(ctx.traceStatus, -1);
}

TEST(CtrlPayload, parseGetLogInfoStatus3) {
    // status 3: application id and context id only
    const auto data = QByteArray::fromHex("030000000301005359530001004d47520072656d6f");

    auto payload = qdlt::msg::payload::parse(data, false);

    ASSERT_TRUE(std::holds_alternative<qdlt::msg::payload::GetLogInfo>(payload));
    auto getLogInfo = std::get<qdlt::msg::payload::GetLogInfo>(payload);
    EXPECT_EQ(getLogInfo.status, 3);
    ASSERT_EQ(getLogInfo.apps.size(), 1);
    ASSERT_EQ(getLogInfo.apps[0].ctxs.size(), 1);
    EXPECT_EQ(getLogInfo.apps[0].ctxs[0].id, "MGR");
    EXPECT_EQ(getLogInfo.apps[0].ctxs[0].logLevel, -1);
    EXPECT_EQ(getLogInfo.apps[0].ctxs[0].traceStatus, -1);
}

TEST(CtrlPayload, parseGetLogInfoOverflow) {
    // status 9: the ECU could not fit the log info into one message, no data follows
    const auto data = QByteArray::fromHex("0300000009");

    auto payload = qdlt::msg::payload::parse(data, false);

    ASSERT_TRUE(std::holds_alternative<qdlt::msg::payload::GetLogInfo>(payload));
    auto getLogInfo = std::get<qdlt::msg::payload::GetLogInfo>(payload);
    EXPECT_EQ(getLogInfo.status, 9);
    EXPECT_TRUE(getLogInfo.apps.empty());
}
