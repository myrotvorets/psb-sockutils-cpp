#include "utils.h"

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <gsl/util>
#include <gtest/gtest.h>

namespace {

void syscall_succeeded(int res, const std::string& api)
{
    if (res == -1) {
        const auto err = errno;
        throw std::system_error(err, std::system_category(), api);
    }
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
    syscall_succeeded(getsockname(sock, reinterpret_cast<sockaddr*>(&ss), &len), "getsockname");
}

int get_socket_option(int sock, int level, int optname)
{
    int optval       = 0;
    socklen_t optlen = sizeof(optval);

    syscall_succeeded(getsockopt(sock, level, optname, &optval, &optlen), "getsockopt");
    EXPECT_EQ(optlen, sizeof(optval));

    return optval;
}

unsigned int get_fd_flags(int fd)
{
    const auto res = fcntl(fd, F_GETFD, 0);
    syscall_succeeded(res, "fcntl(F_GETFD)");

    return gsl::narrow_cast<unsigned int>(res);
}

unsigned int get_status_flags(int fd)
{
    const auto res = fcntl(fd, F_GETFL, 0);
    syscall_succeeded(res, "fcntl(F_GETFL)");

    return gsl::narrow_cast<unsigned int>(res);
}

int create_socket(int domain, int type, int protocol)
{
    const auto sock = socket(domain, type, protocol);
    syscall_succeeded(sock, "socket");
    return sock;
}
