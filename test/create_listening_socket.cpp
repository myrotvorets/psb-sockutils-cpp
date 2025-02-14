#include <fcntl.h>
#include <gtest/gtest.h>

#include <gsl/util>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <opentelemetry/semconv/incubating/network_attributes.h>

#include "sockutils.h"
#include "utils.h"

TEST(CreateListeningSocket, IPv4)
{
    psb::socket_options_t opts{
        .close_on_exec        = 1,
        .reuse_addr           = 1,
        .free_bind            = 1,
        .defer_accept_timeout = 10,  // NOLINT(readability-magic-numbers)
        .listen_backlog       = SOMAXCONN,
    };

    const auto result = psb::create_listening_socket("127.0.0.1", 0, opts);
    auto close_socket = gsl::finally([sock = result.sock]() { close(sock); });

    EXPECT_GE(result.sock, 0);
    EXPECT_STREQ(result.transport, opentelemetry::semconv::network::NetworkTransportValues::kTcp);
    EXPECT_STREQ(result.type, opentelemetry::semconv::network::NetworkTypeValues::kIpv4);

    EXPECT_EQ(get_socket_option(result.sock, SOL_SOCKET, SO_REUSEADDR), opts.reuse_addr);
    EXPECT_EQ(get_socket_option(result.sock, IPPROTO_IP, IP_FREEBIND), opts.free_bind);
    // Sometimes, the returned value is greater than the set value
    EXPECT_NE(get_socket_option(result.sock, IPPROTO_TCP, TCP_DEFER_ACCEPT), 0);
    EXPECT_EQ(get_fd_flags(result.sock) & FD_CLOEXEC, FD_CLOEXEC);
}

TEST(CreateListeningSocket, IPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    psb::socket_options_t opts{
        .close_on_exec        = 0,
        .reuse_addr           = 0,
        .free_bind            = 0,
        .defer_accept_timeout = 0,
        .listen_backlog       = SOMAXCONN,
    };

    const auto result = psb::create_listening_socket("::1", 0, opts);
    auto close_socket = gsl::finally([sock = result.sock]() { close(sock); });

    EXPECT_GE(result.sock, 0);
    EXPECT_STREQ(result.transport, opentelemetry::semconv::network::NetworkTransportValues::kTcp);
    EXPECT_STREQ(result.type, opentelemetry::semconv::network::NetworkTypeValues::kIpv6);

    EXPECT_EQ(get_socket_option(result.sock, SOL_SOCKET, SO_REUSEADDR), opts.reuse_addr);
    EXPECT_EQ(get_socket_option(result.sock, IPPROTO_IP, IP_FREEBIND), opts.free_bind);
    EXPECT_EQ(get_socket_option(result.sock, IPPROTO_TCP, TCP_DEFER_ACCEPT), opts.defer_accept_timeout);
    EXPECT_EQ(get_fd_flags(result.sock) & FD_CLOEXEC, 0);
}
