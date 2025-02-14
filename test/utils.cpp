#include "utils.h"

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <gsl/util>
#include <gtest/gtest.h>

namespace {

testing::AssertionResult syscall_succeeded(int res)
{
    if (res == -1) {
        int err = errno;
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        return testing::AssertionFailure() << std::strerror(err);
    }

    return testing::AssertionSuccess();
}

}  // namespace

bool ipv6_supported()
{
    static int is_supported = -1;

    if (is_supported == -1) {
        if (const auto sock = socket(AF_INET6, SOCK_STREAM, 0); sock != -1) {
            close(sock);
            is_supported = 1;
        }
        else {
            is_supported = 0;
        }
    }

    return is_supported == 1;
}

void get_sock_name(int sock, sockaddr_storage& ss, socklen_t& len)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ASSERT_TRUE(syscall_succeeded(getsockname(sock, reinterpret_cast<sockaddr*>(&ss), &len)));
}

int get_socket_option(int sock, int level, int optname)
{
    int optval       = 0;
    socklen_t optlen = sizeof(optval);

    EXPECT_TRUE(syscall_succeeded(getsockopt(sock, level, optname, &optval, &optlen)));
    EXPECT_EQ(optlen, sizeof(optval));

    return optval;
}

unsigned int get_fd_flags(int fd)
{
    const auto res = fcntl(fd, F_GETFD, 0);
    EXPECT_TRUE(syscall_succeeded(res));

    return gsl::narrow_cast<unsigned int>(res);
}

unsigned int get_status_flags(int fd)
{
    const auto res = fcntl(fd, F_GETFL, 0);
    EXPECT_TRUE(syscall_succeeded(res));

    return gsl::narrow_cast<unsigned int>(res);
}

int create_socket(int domain, int type, int protocol)
{
    const auto sock = socket(domain, type, protocol);
    EXPECT_TRUE(syscall_succeeded(sock));

    return sock;
}
