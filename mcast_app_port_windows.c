#include <winsock2.h>
#include <MSWSock.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define MCAST_APP_PORT	0x2222

int main(int argc, char *argv[]) {
    WORD wVersionRequested;
    WSADATA wsaData;
    int wsaerr;
    struct sockaddr_in sockaddr;
    struct ip_mreq mreq;
    SOCKET sock;
    DWORD reuseaddr = 1;
    char buf[128];
    int rcvd;

    if (argc != 3) {
        fprintf(stderr, "Usage: mcast_app_port_windows [multicast address] [interface address]\n");
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

    wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        fprintf(stderr, "WSAStartup() failure: %d\n", wsaerr);
        return 1;
    }

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr = mreq.imr_interface;
    sockaddr.sin_port = htons(MCAST_APP_PORT);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failure: %d\n", WSAGetLastError());
        return 1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr, sizeof(reuseaddr)) == SOCKET_ERROR) {
        fprintf(stderr, "SO_REUSEADDR failure: %d\n", WSAGetLastError());
        return 1;
    }

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failure: %d\n", WSAGetLastError());
        return 1;
    }

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
        fprintf(stderr, "IP_ADD_MEMBERSHIP failure: %d\n", WSAGetLastError());
        return 1;
    }

    while (1) {
        rcvd = recv(sock, buf, sizeof(buf), 0);
        if (rcvd == SOCKET_ERROR) {
            fprintf(stderr, "recv() failure: %d\n", WSAGetLastError());
            return 1;
        }
        buf[min(rcvd, sizeof(buf) - 1)] = '\0';
        printf("%s", buf);
    }

    return 0;
}