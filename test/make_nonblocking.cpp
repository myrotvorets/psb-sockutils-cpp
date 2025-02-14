#include <gtest/gtest.h>

#include <cerrno>
#include <system_error>

#include <gsl/util>
#include <fcntl.h>
#include <sys/socket.h>

#include "sockutils.h"
#include "utils.h"

TEST(MakeNonblockingTest, HappyPath)
{
    const auto sock   = create_socket(AF_INET, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    ASSERT_NO_THROW(psb::make_nonblocking(sock));

    const auto flags = get_status_flags(sock);
    EXPECT_EQ(flags & O_NONBLOCK, O_NONBLOCK);
}

TEST(MakeNonblockingTest, BadFD)
{
    EXPECT_THROW(psb::make_nonblocking(-1), std::system_error);
}

TEST(MakeNonblockingTest, Functional)
{
    const auto sock   = create_socket(AF_INET, SOCK_STREAM, 0);
    auto close_socket = gsl::finally([sock]() { close(sock); });

    ASSERT_NO_THROW(psb::make_nonblocking(sock));
    ASSERT_NO_THROW(psb::bind_socket(sock, "127.0.0.1", 0));
    ASSERT_NE(listen(sock, 5), -1);

    const auto actual_res   = accept(sock, nullptr, nullptr);
    const auto actual_errno = errno;
    EXPECT_EQ(actual_res, -1);
    EXPECT_TRUE(actual_errno == EAGAIN || actual_errno == EWOULDBLOCK);
}
