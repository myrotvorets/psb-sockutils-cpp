#include <gtest/gtest.h>

#include <array>
#include <gsl/util>
#include <string>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "sockutils.h"
#include "utils.h"

namespace {

void check_address_port(int domain, socklen_t min_len, const std::string& expected_address, std::uint16_t expected_port)
{
    const auto sock   = create_socket(domain, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    get_sock_name(sock, ss, len);

    ASSERT_GE(len, min_len);

    const auto actual = psb::get_socket_info(ss, len);
    EXPECT_EQ(actual.address, expected_address);
    EXPECT_EQ(actual.port, expected_port);
}

void check_bad_length(int domain, socklen_t length)
{
    const auto sock   = create_socket(domain, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    get_sock_name(sock, ss, len);

    ASSERT_GE(len, length);

    const auto actual = psb::get_socket_info(ss, sizeof(sa_family_t) + 1);
    EXPECT_EQ(actual.address, std::string{});
    EXPECT_EQ(actual.port, 0);
}

}  // namespace

TEST(GetSocketInfo, NotOpenSocketIPv4)
{
    check_address_port(AF_INET, sizeof(sockaddr_in), "0.0.0.0", 0);
}

TEST(GetSocketInfo, NotOpenSocketIPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    check_address_port(AF_INET6, sizeof(sockaddr_in6), "::", 0);
}

TEST(GetSocketInfo, NotOpenSocketLocal)
{
    check_address_port(AF_LOCAL, sizeof(sa_family_t), "", 0);
}

TEST(GetSocketInfo, ZeroLengthIPv4)
{
    check_bad_length(AF_INET, 0);
}

TEST(GetSocketInfo, BadLengthIPv4)
{
    check_bad_length(AF_INET, sizeof(sa_family_t) + 1);
}

TEST(GetSocketInfo, BadLengthIPv6)
{
    if (!ipv6_supported()) {
        GTEST_SKIP() << "IPv6 not supported";
    }

    check_bad_length(AF_INET6, sizeof(sa_family_t) + 1);
}

TEST(GetSocketInfo, BadLengthLocal)
{
    check_bad_length(AF_LOCAL, offsetof(sockaddr_un, sun_path) - 1);
}

TEST(GetSocketInfo, AbstractLocalSocket)
{
    const auto sock   = create_socket(AF_LOCAL, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    // Abstract UNIX sockets don't have to be NUL-terminated
    constexpr std::array<char, 4U> name{0, 'a', 'b', 'c'};

    sockaddr_un sun{};
    sun.sun_family = AF_LOCAL;
    std::ranges::copy(name, &sun.sun_path[0]);

    socklen_t len = offsetof(sockaddr_un, sun_path) + name.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ASSERT_NE(bind(sock, reinterpret_cast<const sockaddr*>(&sun), len), -1);

    sockaddr_storage ss{};
    len = sizeof(ss);
    get_sock_name(sock, ss, len);

    ASSERT_GE(len, sizeof(sa_family_t) + name.size());

    const auto actual = psb::get_socket_info(ss, len);
    EXPECT_EQ(actual.address, std::string(&name.at(1), name.size() - 1));
    EXPECT_EQ(actual.port, 0);
}

TEST(GetSocketInfo, RegularLocalSocket)
{
    const auto sock   = create_socket(AF_LOCAL, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    constexpr std::string_view name = "regular-local.socket";

    sockaddr_un sun{};
    sun.sun_family = AF_LOCAL;
    std::ranges::copy(name, &sun.sun_path[0]);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ASSERT_NE(bind(sock, reinterpret_cast<const sockaddr*>(&sun), sizeof(sun)), -1);

    // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
    EXPECT_EQ(std::remove(name.data()), 0);

    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    get_sock_name(sock, ss, len);

    ASSERT_GE(len, sizeof(sa_family_t) + name.size());

    const auto actual = psb::get_socket_info(ss, len);
    EXPECT_EQ(actual.address, std::string(name.data(), name.size()));
    EXPECT_EQ(actual.port, 0);
}
