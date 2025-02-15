#include <gtest/gtest.h>

#include <poll.h>
#include <sys/socket.h>

#include <gsl/util>

#include "sockutils.h"
#include "utils.h"

namespace {

template<typename T>
constexpr auto make_unsigned(T value)
{
    return gsl::narrow_cast<std::make_unsigned_t<T>>(value);
}

}  // namespace

TEST(AcceptConnection, BadFD)
{
    EXPECT_THROW(psb::accept_connection(-1), std::system_error);
}

TEST(AcceptConnection, Functional)
{
    const psb::socket_options_t opts{
        .close_on_exec = 1, .reuse_addr = 1, .free_bind = 1, .defer_accept_timeout = 0, .listen_backlog = SOMAXCONN
    };

    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    psb::listening_socket_t ls{};
    ASSERT_NO_THROW(ls = psb::create_listening_socket("127.0.0.1", 0, opts));
    auto close_listening_socket = gsl::finally([sock = ls.sock]() { close(sock); });

    ASSERT_NO_THROW(get_sock_name(ls.sock, ss, len));

    const auto connecting_socket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(connecting_socket, -1);
    auto close_connecting_socket = gsl::finally([sock = connecting_socket]() { close(sock); });

    ASSERT_NO_THROW(psb::make_nonblocking(connecting_socket));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto res = connect(connecting_socket, reinterpret_cast<sockaddr*>(&ss), len);
    EXPECT_TRUE(res == 0 || (res == -1 && errno == EINPROGRESS));

    pollfd pfd{.fd = ls.sock, .events = POLLIN, .revents = 0};
    ASSERT_EQ(poll(&pfd, 1, -1), 1);
    ASSERT_NE(make_unsigned(pfd.revents) & POLLIN, 0);

    psb::accepted_socket_t accepted;
    ASSERT_NO_THROW(accepted = psb::accept_connection(ls.sock));

    close(accepted.sock);
}
