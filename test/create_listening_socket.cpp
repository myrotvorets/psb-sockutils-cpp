#include <gtest/gtest.h>

#include <gsl/util>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <opentelemetry/semconv/incubating/network_attributes.h>

#include "sockutils.h"
#include "utils.h"

TEST(CreateListeningSocket, IPv4)
{
    const psb::socket_options_t opts{
        .close_on_exec        = 1,
        .reuse_addr           = 1,
        .free_bind            = 1,
        .defer_accept_timeout = 10,  // NOLINT(readability-magic-numbers)
        .listen_backlog       = SOMAXCONN,
    };

    const auto result = psb::create_listening_socket("127.0.0.1", 0, opts);
    auto close_socket = gsl::finally([sock = result.sock]() { close(sock); });

    ASSERT_GE(result.sock, 0);
    EXPECT_STREQ(result.transport, opentelemetry::semconv::network::NetworkTransportValues::kTcp);
    EXPECT_STREQ(result.type, opentelemetry::semconv::network::NetworkTypeValues::kIpv4);

    int optval{};

    ASSERT_NO_THROW(optval = get_socket_option(result.sock, SOL_SOCKET, SO_REUSEADDR));
    EXPECT_EQ(optval, opts.reuse_addr);

    ASSERT_NO_THROW(optval = get_socket_option(result.sock, IPPROTO_IP, IP_FREEBIND));
    EXPECT_EQ(optval, opts.free_bind);

    ASSERT_NO_THROW(optval = get_socket_option(result.sock, IPPROTO_TCP, TCP_DEFER_ACCEPT));
    // Sometimes, the returned value is greater than the set value
    EXPECT_NE(optval, 0);

    unsigned int flags{};
    ASSERT_NO_THROW(flags = get_fd_flags(result.sock));
    EXPECT_EQ(flags & FD_CLOEXEC, FD_CLOEXEC);
}

TEST(CreateListeningSocket, IPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    const psb::socket_options_t opts{
        .close_on_exec        = 0,
        .reuse_addr           = 0,
        .free_bind            = 0,
        .defer_accept_timeout = 0,
        .listen_backlog       = SOMAXCONN,
    };

    const auto result = psb::create_listening_socket("::1", 0, opts);
    auto close_socket = gsl::finally([sock = result.sock]() { close(sock); });

    ASSERT_GE(result.sock, 0);
    EXPECT_STREQ(result.transport, opentelemetry::semconv::network::NetworkTransportValues::kTcp);
    EXPECT_STREQ(result.type, opentelemetry::semconv::network::NetworkTypeValues::kIpv6);

    int optval{};

    ASSERT_NO_THROW(optval = get_socket_option(result.sock, SOL_SOCKET, SO_REUSEADDR));
    EXPECT_EQ(optval, opts.reuse_addr);

    ASSERT_NO_THROW(optval = get_socket_option(result.sock, IPPROTO_IP, IP_FREEBIND));
    EXPECT_EQ(optval, opts.free_bind);

    ASSERT_NO_THROW(optval = get_socket_option(result.sock, IPPROTO_TCP, TCP_DEFER_ACCEPT));
    EXPECT_EQ(optval, opts.defer_accept_timeout);

    unsigned int flags{};
    ASSERT_NO_THROW(flags = get_fd_flags(result.sock));
    EXPECT_EQ(flags & FD_CLOEXEC, 0);
}
