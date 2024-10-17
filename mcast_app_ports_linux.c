#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MCAST_APP_PORT	49151

#define min(a, b) ((a) < (b) ? (a) : (b))
#define SOCKET_ERROR -1

int main(int argc, char *argv[]) {
    struct sockaddr_in sockaddr;
    struct ip_mreq mreq;
    int sock;
    char buf[128];
    ssize_t rcvd;

    if (argc != 3) {
        fprintf(stderr, "Usage: mcast_app_ports_linux [multicast address] [interface index]\n");
        return 1;
    }

    if (inet_pton(AF_INET, argv[1], &mreq.imr_multiaddr) != 1) {
        fprintf(stderr, "Cannot convert multicast address\n");
        return 1;
    }

    if (inet_pton(AF_INET, argv[2], &mreq.imr_interface) != 1) {
        fprintf(stderr, "Cannot convert interface address\n");
        return 1;
    }

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr = mreq.imr_multiaddr;
    sockaddr.sin_port = htons(MCAST_APP_PORT);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == SOCKET_ERROR) {
        perror("socket() failure");
        return 1;
    }

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) {
        perror("bind() failure");
        return 1;
    }

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == SOCKET_ERROR) {
        perror("setsockopt() failure");
        return 1;
    }

    while (1) {
        rcvd = recv(sock, buf, sizeof(buf), 0);
        if (rcvd == SOCKET_ERROR) {
            perror("recv() failure");
            return 1;
        }
        buf[min(rcvd, sizeof(buf) - 1)] = '\0';
        printf("%s", buf);
    }

    return 0;
}