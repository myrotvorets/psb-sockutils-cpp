#ifndef C4E7C8D4_DF90_421A_BAC2_E1BE5862ABBE
#define C4E7C8D4_DF90_421A_BAC2_E1BE5862ABBE

#include <cstdint>
#include <string>
#include <string_view>

#include <netinet/in.h>
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
    int sock{};
    const char* transport{};  // opentelemetry::semconv::network::NetworkTransportValues; e.g., kTcp
    const char* type{};       // opentelemetry::semconv::network::NetworkTypeValues; e.g., kIpv4
};

struct socket_info_t {
    std::string address;
    std::uint16_t port{};
};

struct accepted_socket_t {
    int sock{};
    std::string address;
    std::uint16_t port{};
};

/**
 * @brief Makes the file descriptor @a fd non-blocking.
 *
 * @param fd File descriptor.
 * @throw std::system_error Call to `fcntl()` failed.
 */
PSB_SOCKUTILS_EXPORT void make_nonblocking(int fd);

/**
 * @brief Makes the file descriptor @a fd close-on-exec.
 *
 * @param fd File descriptor.
 * @throw std::system_error Call to `fcntl()` failed.
 */
PSB_SOCKUTILS_EXPORT void make_close_on_exec(int fd);

/**
 * @brief Sets the socket option @a optname to @a optval.
 *
 * @param sock Socket descriptor.
 * @param level The protocol level.
 * @param optname The option name.
 * @param optval The option value.
 * @param name The option name as a string.
 * @throw std::system_error Call to `setsockopt()` failed.
 */
PSB_SOCKUTILS_EXPORT void set_socket_option(int sock, int level, int optname, int optval, std::string_view name);

/**
 * @brief Binds the socket @a sock to the address @a address and port @a port.
 *
 * @param sock Socket descriptor.
 * @param address IP address.
 * @param port Port number.
 * @throw std::system_error Call to `bind()` failed.
 * @throw std::system_error The address family is not supported.
 * @throw std::invalid_argument The address is not valid IPv4 or IPv6 address.
 */
PSB_SOCKUTILS_EXPORT void bind_socket(int sock, const std::string& address, std::uint16_t port);

/**
 * @brief Creates a listening socket bound to the address @a address and port @a port.
 *
 * @param address IP address.
 * @param port Port number.
 * @param opts Socket options.
 * @return The listening socket.
 * @throw std::system_error Call to a system API failed.
 * @throw std::invalid_argument The address is not valid IPv4 or IPv6 address.
 */
PSB_SOCKUTILS_EXPORT listening_socket_t
create_listening_socket(const std::string& address, std::uint16_t port, const socket_options_t& opts);

/**
 * @brief Gets the socket information from the network address structure @a ss.
 *
 * @param ss Network address structure.
 * @param len Length of the network address structure.
 * @return The socket information.
 */
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

/**
 * @brief Accepts a connection on the socket @a fd and makes the accepted socket non-blocking and close-on-exec.
 *
 * @param fd Socket descriptor.
 * @return Accepted socket and peer information, if available.
 * @throw std::system_error Call to a system API failed.
 */
PSB_SOCKUTILS_EXPORT accepted_socket_t accept_connection(int fd);

}  // namespace psb

#endif /* C4E7C8D4_DF90_421A_BAC2_E1BE5862ABBE */
