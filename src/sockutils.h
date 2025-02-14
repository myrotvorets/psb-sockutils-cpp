#ifndef C4E7C8D4_DF90_421A_BAC2_E1BE5862ABBE
#define C4E7C8D4_DF90_421A_BAC2_E1BE5862ABBE

#include <cstdint>
#include <string>
#include <string_view>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "export.h"

namespace psb {

struct socket_options_t {
    int close_on_exec;
    int reuse_addr;
    int free_bind;
    int defer_accept_timeout;
    int listen_backlog;
};

struct listening_socket_t {
    int sock;
    const char* transport;  // opentelemetry::semconv::network::NetworkTransportValues; e.g., kTcp
    const char* type;       // opentelemetry::semconv::network::NetworkTypeValues; e.g., kIpv4
};

struct socket_info_t {
    std::string address;
    std::uint16_t port;
};

PSB_SOCKUTILS_EXPORT void make_nonblocking(int fd);
PSB_SOCKUTILS_EXPORT void make_close_on_exec(int fd);
PSB_SOCKUTILS_EXPORT void set_socket_option(int sock, int level, int optname, int optval, std::string_view name);
PSB_SOCKUTILS_EXPORT void bind_socket(int sock, const std::string& address, std::uint16_t port);

PSB_SOCKUTILS_EXPORT listening_socket_t
create_listening_socket(const std::string& address, std::uint16_t port, const socket_options_t& opts);

PSB_SOCKUTILS_EXPORT socket_info_t get_socket_info(const sockaddr_storage& ss, socklen_t len);

/**
 * @brief Converts the IPv4 address @a address src into a network address structure @a dst.
 *
 * @param address IPv4 address.
 * @param dst Network address structure to store the result.
 * @throw std::invalid_argument The address is not valid IPv4 address.
 * @throw std::system_error The address family is not supported.
 */
PSB_SOCKUTILS_EXPORT void inet_pton(const std::string& address, in_addr& dst);

/**
 * @brief Converts the IPv6 address @a address src into a network address structure @a dst.
 *
 * @param address IPv6 address.
 * @param dst Network address structure to store the result.
 * @throw std::invalid_argument The address is not valid IPv4 address.
 * @throw std::system_error The address family is not supported.
 */
PSB_SOCKUTILS_EXPORT void inet_pton(const std::string& address, in6_addr& dst);

}  // namespace psb

#endif /* C4E7C8D4_DF90_421A_BAC2_E1BE5862ABBE */
