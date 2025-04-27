#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include "cJSON.h"
#include "aes.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1700
#define BUFFER_SIZE 1024

typedef struct {
    char device_id[32];
    char payload[256];
    char appeui[32];
    char appekey[32];
    char deveui[32];
} EndDevice;

void load_devices(const char *filename, EndDevice *devices, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    rewind(file);

    char *json_data = (char *)malloc(fsize + 1);
    fread(json_data, 1, fsize, file);
    fclose(file);

    json_data[fsize] = '\0';

    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        printf("Erro ao parsear JSON\n");
        free(json_data);
        exit(EXIT_FAILURE);
    }

    int n = cJSON_GetArraySize(root);
    for (int i = 0; i < n; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        strcpy(devices[i].device_id, cJSON_GetObjectItem(item, "device_id")->valuestring);
        strcpy(devices[i].payload, cJSON_GetObjectItem(item, "payload")->valuestring);
        strcpy(devices[i].appeui, cJSON_GetObjectItem(item, "appeui")->valuestring);
        strcpy(devices[i].appekey, cJSON_GetObjectItem(item, "appekey")->valuestring);
        strcpy(devices[i].deveui, cJSON_GetObjectItem(item, "deveui")->valuestring);
    }
    *count = n;

    cJSON_Delete(root);
    free(json_data);
}

void encrypt_payload(char *payload, const char *key) {
    struct AES_ctx ctx;
    uint8_t buffer[16] = {0};
    strncpy((char*)buffer, payload, 16);

    AES_init_ctx(&ctx, (const uint8_t*)key);
    AES_ECB_encrypt(&ctx, buffer);

    for (int i = 0; i < 16; i++) {
        sprintf(&payload[i*2], "%02X", buffer[i]);
    }
}

void send_uplink(SOCKET sockfd, struct sockaddr_in *serverAddr, EndDevice *device) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "%s:%s", device->device_id, device->payload);

    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));
    printf("Uplink enviado: %s:%s\n", device->device_id, device->payload);
}

void start_device() {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in serverAddr;
    EndDevice devices[10];
    int device_count = 0;

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

    // Configurar timeout
    DWORD timeout = 3000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(SERVER_PORT);

    printf("EndDevice inicializado.\n");

    load_devices("devices.json", devices, &device_count);

    for (int i = 0; i < device_count; i++) {
        encrypt_payload(devices[i].payload, devices[i].appekey);
        send_uplink(sockfd, &serverAddr, &devices[i]);

        char buffer[BUFFER_SIZE];
        struct sockaddr_in fromAddr;
        int fromLen = sizeof(fromAddr);
        int recvLen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&fromAddr, &fromLen);

        if (recvLen > 0) {
            buffer[recvLen] = '\0';
            printf("Downlink recebido: %s\n", buffer);
        } else {
            printf("Nenhum downlink recebido.\n");
        }

        Sleep(1000);
    }

    closesocket(sockfd);
    WSACleanup();
}

int main() {
    start_device();
    return 0;
}
