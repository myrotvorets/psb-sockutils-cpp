#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>

#include "sockutils.h"
#include "utils.h"

TEST(InetPton, IPv4)
{
    in_addr addr{};
    psb::inet_pton("127.0.0.1", addr);
    EXPECT_EQ(addr.s_addr, htonl(INADDR_LOOPBACK));
}

TEST(InetPton, IPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    in6_addr addr{};
    psb::inet_pton("::1", addr);
    static_assert(sizeof(in6addr_loopback) == sizeof(in6_addr));
    EXPECT_TRUE(
        std::equal(std::begin(in6addr_loopback.s6_addr), std::end(in6addr_loopback.s6_addr), std::begin(addr.s6_addr))
    );
}

TEST(InetPton, BadIPv4)
{
    in_addr addr{};
    EXPECT_THROW(psb::inet_pton("256.0.0.1", addr), std::invalid_argument);
}

TEST(InetPton, BadIPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    in6_addr addr{};
    EXPECT_THROW(psb::inet_pton("::G", addr), std::invalid_argument);
}
