#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

void interactive_mode(int sock);
void automatic_mode(int sock);

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Использование: %s <адрес сервера> <порт> [-a]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int sock;
    struct sockaddr_in server_addr;
    int port = atoi(argv[2]);
    int auto_mode = (argc == 4 && strcmp(argv[3], "-a") == 0);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Ошибка в адресе сервера");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка подключения");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Клиент подключен к серверу %s:%d\n", argv[1], port);

    if (auto_mode) {
        printf("Запущен автоматический режим.\n");
        automatic_mode(sock);
    } else {
        printf("Запущен интерактивный режим.\n");
        interactive_mode(sock);
    }

    close(sock);
    return 0;
}

// Интерактивный режим (игрок - человек)
void interactive_mode(int sock) {
    char buffer[BUFFER_SIZE];
    while (1) {
        printf("Введите число: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            printf("Ошибка ввода.\n");
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';  // убираем символ новой строки
        
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("Ошибка отправки данных");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            printf("Сервер закрыл соединение.\n");
            break;
        }
        buffer[n] = '\0';
        printf("Ответ сервера: %s\n", buffer);

        if (strcmp(buffer, "=") == 0) {
            printf("Вы угадали число!\n");
            break;
        }
    }
}

// Автоматический режим (игрок - машина)
void automatic_mode(int sock) {
    char buffer[BUFFER_SIZE];
    int low = 1, high = 100, guess;

    srand(time(NULL));

    while (1) {
        guess = (low + high) / 2; // Бинарный поиск
        printf("Клиент предполагает число: %d\n", guess);
        snprintf(buffer, BUFFER_SIZE, "%d", guess);

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("Ошибка отправки данных");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            printf("Сервер закрыл соединение.\n");
            break;
        }
        buffer[n] = '\0';
        printf("Ответ сервера: %s\n", buffer);

        if (strcmp(buffer, "=") == 0) {
            printf("Клиент угадал число: %d\n", guess);
            break;
        } else if (strcmp(buffer, "<") == 0) {
            high = guess - 1;
        } else if (strcmp(buffer, ">") == 0) {
            low = guess + 1;
        }

        sleep(1); // для наглядности. медленнее будет перебор
    }
}
