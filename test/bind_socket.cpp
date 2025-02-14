#include <gtest/gtest.h>

#include <sys/socket.h>

#include <gsl/util>
#include <stdexcept>
#include <system_error>

#include "sockutils.h"
#include "utils.h"

TEST(BindSocket, IPv4)
{
    const auto sock   = create_socket(AF_INET, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    const std::string expected_address = "127.0.0.2";

    EXPECT_NO_THROW(psb::bind_socket(sock, expected_address, 0));

    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    get_sock_name(sock, ss, len);

    const auto info = psb::get_socket_info(ss, len);
    EXPECT_EQ(info.address, expected_address);
}

TEST(BindSocket, IPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    const auto sock   = create_socket(AF_INET6, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    const std::string expected_address = "::1";

    EXPECT_NO_THROW(psb::bind_socket(sock, expected_address, 0));

    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    get_sock_name(sock, ss, len);

    const auto info = psb::get_socket_info(ss, len);
    EXPECT_EQ(info.address, expected_address);
}

TEST(BindSocket, InvalidIPv4)
{
    const auto sock   = create_socket(AF_INET, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    EXPECT_THROW(psb::bind_socket(sock, "256.0.0.1", 0), std::invalid_argument);
}

TEST(BindSocket, InvalidIPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    const auto sock   = create_socket(AF_INET6, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    EXPECT_THROW(psb::bind_socket(sock, "::G", 0), std::invalid_argument);
}

TEST(BindSocket, InvalidFDIPv4)
{
    EXPECT_THROW(psb::bind_socket(-1, "127.0.0.1", 0), std::system_error);
}

TEST(BindSocket, InvalidFDIPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    EXPECT_THROW(psb::bind_socket(-1, "::1", 0), std::system_error);
}
