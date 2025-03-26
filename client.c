#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void interactive_mode(int sockfd) {
    char buffer[BUFFER_SIZE];
    int guess;

    while (1) {
        printf("Введите число: ");
        scanf("%d", &guess);
        sprintf(buffer, "%d", guess);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);

        printf("Сервер: %s\n", buffer);

        if (strcmp(buffer, "win") == 0) {
            printf("Вы угадали число!\n");
            break;
        }
    }
}

void auto_mode(int sockfd) {
    char buffer[BUFFER_SIZE];
    int low = 1, high = 100, guess;

    while (1) {
        guess = (low + high) / 2;
        printf("Авто-игрок отправляет: %d\n", guess);
        sprintf(buffer, "%d", guess);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);

        printf("Сервер ответил: %s\n", buffer);

        if (strcmp(buffer, "win") == 0) {
            printf("Авто-игрок угадал число!\n");
            break;
        } else if (strcmp(buffer, "more") == 0) {
            low = guess + 1;
        } else if (strcmp(buffer, "less") == 0) {
            high = guess - 1;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <IP> <режим>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char *mode = argv[2];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Ошибка при создании сокета");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Ошибка преобразования адреса");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка подключения");
        return 1;
    }

    printf("Подключено к серверу %s\n", argv[1]);

    if (strcmp(mode, "interactive") == 0) {
        interactive_mode(sockfd);
    } else if (strcmp(mode, "auto") == 0) {
        auto_mode(sockfd);
    } else {
        fprintf(stderr, "Ошибка: неизвестный режим %s\n", mode);
        return 1;
    }

    close(sockfd);
    printf("Отключено от сервера\n");
    return 0;
}
