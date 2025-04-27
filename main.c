#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_PORT 1700
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];
    int clientAddrLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        perror("socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Gateway iniciado e escutando na porta %d...\n", SERVER_PORT);

    while (1) {
        int recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (recvLen > 0) {
            buffer[recvLen] = '\0';
            printf("Pacote recebido: %s\n", buffer);

            char downlink[] = "Tudo ook!";
            sendto(sockfd, downlink, strlen(downlink), 0, (struct sockaddr*)&clientAddr, clientAddrLen);
            printf("Downlink enviado: %s\n", downlink);
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
