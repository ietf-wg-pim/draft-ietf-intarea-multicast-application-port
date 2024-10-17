#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MCAST_APP_PORT	49151

int main(int argc, char *argv[]) {
    struct sockaddr_in sockaddr;
    struct ip_mreqn mreq;
    int sock;
    char buf[128];

    if (argc != 3) {
        fprintf(stderr, "Usage: mcast_app_ports_linux [multicast address] [interface index]\n");
        return 1;
    }

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(MCAST_APP_PORT);

    if (inet_pton(AF_INET, argv[1], &sockaddr.sin_addr) != 1) {
        fprintf(stderr, "Cannot convert address\n");
        return 1;
    }

    mreq.imr_multiaddr = sockaddr.sin_addr;
    mreq.imr_address.s_addr = INADDR_ANY;

    if (sscanf(argv[2], "%d", &mreq.imr_ifindex) != 1) {
        fprintf(stderr, "Cannot convert address\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket() failure");
        return 1;
    }

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("bind() failure");
        return 1;
    }

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
        perror("setsockopt() failure");
        return 1;
    }

    while (1) {
        if (recv(sock, buf, sizeof(buf), 0) == -1) {
            perror("recv() failure");
            return 1;
        }
        printf("%s", buf);
    }

    return 0;
}