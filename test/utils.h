#ifndef D29F38ED_C6ED_40D1_8D66_70D5BD215292
#define D29F38ED_C6ED_40D1_8D66_70D5BD215292

#include <sys/socket.h>

bool ipv6_supported();
void get_sock_name(int sock, sockaddr_storage& ss, socklen_t& len);
int get_socket_option(int sock, int level, int optname);
unsigned int get_fd_flags(int fd);
unsigned int get_status_flags(int fd);
int create_socket(int domain, int type, int protocol);

#endif /* D29F38ED_C6ED_40D1_8D66_70D5BD215292 */
