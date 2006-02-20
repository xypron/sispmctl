#ifndef NETHELP_H
#define NETHELP_H 1

int sock_write_bytes(int sockfd, const unsigned char *buff, int len);
int sock_read_bytes(int sockfd, const unsigned char *buff, int len);

#endif /* ! NETHELP_H */
