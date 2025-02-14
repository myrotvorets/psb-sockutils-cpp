#include <gtest/gtest.h>

#include <system_error>

#include <gsl/util>
#include <fcntl.h>
#include <sys/socket.h>

#include "sockutils.h"
#include "utils.h"

TEST(MakeCloseOnExec, HappyPath)
{
    const auto sock   = create_socket(AF_INET, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    ASSERT_NO_THROW(psb::make_close_on_exec(sock));

    const auto flags = get_fd_flags(sock);
    EXPECT_EQ(flags & FD_CLOEXEC, FD_CLOEXEC);
}

TEST(MakeCloseOnExec, BadFD)
{
    EXPECT_THROW(psb::make_close_on_exec(-1), std::system_error);
}
