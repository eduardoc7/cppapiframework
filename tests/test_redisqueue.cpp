#include "../src/queues/RedisQueue.hpp"
#include <gtest/gtest.h>

static const std::string queue_name = "test_queue_worker:queue:default";

static std::shared_ptr<RedisQueue> redisq(std::make_shared<RedisQueue>());

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, IsConnected) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, PushPops) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());

    std::string data = "0123456789 qwertyuiop";

    redisq->push(queue_name, data);
    auto popdata = redisq->pop(queue_name, 1);

    EXPECT_TRUE(popdata);

    EXPECT_EQ(popdata.value_or(std::string()), data);
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, ExpireTTLQueue) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());

    std::string data = "0123456789 qwertyuiop";

    redisq->push(queue_name, data);

    EXPECT_TRUE(redisq->expire(queue_name, 32));
    EXPECT_GT(redisq->ttl(queue_name), 10);
    EXPECT_LE(redisq->ttl(queue_name), 32);

    auto popdata = redisq->pop(queue_name, 1);
    EXPECT_TRUE(popdata);

    EXPECT_EQ(popdata.value_or(std::string()), data);
}


