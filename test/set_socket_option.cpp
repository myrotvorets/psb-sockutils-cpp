#include <gtest/gtest.h>

#include <system_error>

#include <sys/socket.h>

#include <gsl/util>

#include "sockutils.h"
#include "utils.h"

TEST(SetSocketOption, HappyPath)
{
    int sock{};
    ASSERT_NO_THROW(sock = create_socket(AF_INET, SOCK_STREAM, 0));
    auto close_socket = gsl::finally([sock]() { close(sock); });

    constexpr int expected_optval = 1;

    ASSERT_NO_THROW(psb::set_socket_option(sock, SOL_SOCKET, SO_REUSEADDR, expected_optval, "SO_REUSEADDR"));

    int optval{};
    ASSERT_NO_THROW(optval = get_socket_option(sock, SOL_SOCKET, SO_REUSEADDR));
    ASSERT_EQ(optval, expected_optval);
}

TEST(SetSocketOption, BadFD)
{
    EXPECT_THROW(psb::set_socket_option(-1, SOL_SOCKET, SO_REUSEADDR, 1, "SO_REUSEADDR"), std::system_error);
}
