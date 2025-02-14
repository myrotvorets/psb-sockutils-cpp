#include "sockutils.h"

#include <array>
#include <cerrno>
#include <cstddef>
#include <format>
#include <string_view>
#include <system_error>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <opentelemetry/semconv/incubating/network_attributes.h>

namespace {

[[noreturn]] void throw_bind_error(int err, const std::string& address, std::uint16_t port)
{
    throw std::system_error(err, std::generic_category(), std::format("bind({}:{}) failed", address, port));
}

void bind_ipv4(int sock, const std::string& address, std::uint16_t port)
{
    sockaddr_in sin{};
    auto res = inet_pton(AF_INET, address.c_str(), &sin.sin_addr);
    if (res == 1) [[likely]] {
        sin.sin_family = AF_INET;
        sin.sin_port   = htons(port);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        if (res = bind(sock, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)); res < 0) [[unlikely]] {
            throw_bind_error(errno, address, port);
        }
    }
    else if (res == 0) {
        throw std::invalid_argument("Invalid IPv4 address: " + address);
    }
    else [[unlikely]] {  // EAFNOSUPPORT case
        const auto err = errno;
        throw std::system_error(err, std::generic_category(), std::format("inet_pton(AF_INET, {})", address));
    }
}

void bind_ipv6(int sock, const std::string& address, std::uint16_t port)
{
    sockaddr_in6 sin{};
    auto res = inet_pton(AF_INET6, address.c_str(), &sin.sin6_addr);
    if (res == 1) [[likely]] {
        sin.sin6_family = AF_INET6;
        sin.sin6_port   = htons(port);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        if (res = bind(sock, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)); res < 0) [[unlikely]] {
            throw_bind_error(errno, address, port);
        }
    }
    else if (res == 0) {
        throw std::invalid_argument("Invalid IPv6 address: " + address);
    }
    else [[unlikely]] {  // EAFNOSUPPORT case
        const auto err = errno;
        throw std::system_error(err, std::generic_category(), std::format("inet_pton(AF_INET6, {})", address));
    }
}

void handle_socket_options(int sock, const psb::socket_options_t& opts)
{
    if (opts.close_on_exec != 0) {
        psb::make_close_on_exec(sock);
    }

    if (opts.reuse_addr != 0) {
        psb::set_socket_option(sock, SOL_SOCKET, SO_REUSEADDR, opts.reuse_addr, "SO_REUSEADDR");
    }

    if (opts.free_bind != 0) {
        psb::set_socket_option(sock, IPPROTO_IP, IP_FREEBIND, opts.free_bind, "IP_FREEBIND");
    }

    if (opts.defer_accept_timeout != 0) {
        psb::set_socket_option(sock, IPPROTO_TCP, TCP_DEFER_ACCEPT, opts.defer_accept_timeout, "TCP_DEFER_ACCEPT");
    }
}

psb::socket_info_t make_peer(std::string_view address, uint16_t port)
{
    return {.address = {address.data(), address.size()}, .port = port};
}

}  // namespace

namespace psb {

void make_nonblocking(int fd)
{
    if (auto flags = fcntl(fd, F_GETFL, 0); flags != -1) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        if (const auto res = fcntl(fd, F_SETFL, static_cast<unsigned int>(flags) | O_NONBLOCK); res != 0) [[unlikely]] {
            throw std::system_error(errno, std::generic_category(), "fcntl(F_SETFL) failed");
        }
    }
    else {
        throw std::system_error(errno, std::generic_category(), "fcntl(F_GETFL) failed");
    }
}

void make_close_on_exec(int fd)
{
    if (auto flags = fcntl(fd, F_GETFD, 0); flags != -1) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        if (const auto res = fcntl(fd, F_SETFD, static_cast<unsigned int>(flags) | FD_CLOEXEC); res != 0) [[unlikely]] {
            throw std::system_error(errno, std::generic_category(), "fcntl(F_SETFD) failed");
        }
    }
    else {
        throw std::system_error(errno, std::generic_category(), "fcntl(F_GETFD) failed");
    }
}

void set_socket_option(int sock, int level, int optname, int optval, std::string_view name)
{
    if (const auto res = setsockopt(sock, level, optname, &optval, sizeof(optval)); res != 0) {
        const auto err = errno;
        throw std::system_error(err, std::generic_category(), std::format("setsockopt({}) failed", name));
    }
}

void bind_socket(int sock, const std::string& address, std::uint16_t port)
{
    if (address.find(':') != std::string::npos) {
        bind_ipv6(sock, address, port);
    }
    else {
        bind_ipv4(sock, address, port);
    }
}

listening_socket_t create_listening_socket(const std::string& address, std::uint16_t port, const socket_options_t& opts)
{
    const auto is_ipv6 = address.find(':') != std::string::npos;
    const auto sock    = socket(is_ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0) [[unlikely]] {
        throw std::system_error(errno, std::generic_category(), "socket() failed");
    }

    make_nonblocking(sock);
    handle_socket_options(sock, opts);
    bind_socket(sock, address, port);

    if (const auto res = listen(sock, opts.listen_backlog); res == -1) {
        throw std::system_error(errno, std::generic_category(), "listen() failed");
    }

    using namespace opentelemetry::semconv::network::NetworkTransportValues;
    using namespace opentelemetry::semconv::network::NetworkTypeValues;

    return {.sock = sock, .transport = kTcp, .type = is_ipv6 ? kIpv6 : kIpv4};
}

socket_info_t get_socket_info(const sockaddr_storage& ss, socklen_t len)
{
    std::array<char, INET6_ADDRSTRLEN> buf{};

    if (ss.ss_family == AF_INET && len >= sizeof(sockaddr_in)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto& addr = reinterpret_cast<const sockaddr_in&>(ss);
        if (inet_ntop(ss.ss_family, &addr.sin_addr, buf.data(), buf.size()) != nullptr) [[likely]] {
            return make_peer(buf.data(), ntohs(addr.sin_port));
        }
    }
    else if (ss.ss_family == AF_INET6 && len >= sizeof(sockaddr_in6)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto& addr = reinterpret_cast<const sockaddr_in6&>(ss);
        if (inet_ntop(ss.ss_family, &addr.sin6_addr, buf.data(), buf.size()) != nullptr) [[likely]] {
            return make_peer(buf.data(), ntohs(addr.sin6_port));
        }
    }
    else if (ss.ss_family == AF_UNIX && len > sizeof(sa_family_t)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto& addr = reinterpret_cast<const sockaddr_un&>(ss);
        if (addr.sun_path[0] == '\0') {
#pragma clang unsafe_buffer_usage begin
            const std::string_view abstract_socket_name(&addr.sun_path[1], len - offsetof(sockaddr_un, sun_path) - 1);
#pragma clang unsafe_buffer_usage end
            return make_peer(abstract_socket_name, 0);
        }

        return make_peer(&addr.sun_path[0], 0);
    }

    return {};
}

}  // namespace psb
