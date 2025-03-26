#include <stdio.h> //для ввода/вывода (например, printf).
#include <stdlib.h> // для стандартных функций (например, atoi для преобразования строки в число).
#include <string.h> //для работы со строками (например, memset для обнуления массива).
#include <arpa/inet.h> // для работы с интернет-адресами (например, для преобразования IP-адреса в строку с помощью
#include <sys/socket.h> // для работы с сокетами (например, socket, bind, listen).
#include <unistd.h> //  для системных вызовов (например, close для закрытия сокета).
#include <time.h> //для работы с временем (например, генерация случайных чисел на основе времени с помощью rand).

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <порт>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Ошибка: некорректный порт\n");
        return 1; 
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    srand(time(NULL));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //AF_INET для IPV4
        perror("Ошибка при создании сокета");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка при привязке сокета");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("Ошибка при прослушивании");
        close(server_fd);
        return 1;
    }

    printf("Сервер запущен на порту %d...\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Ошибка при принятии подключения");
            continue;
        }

        printf("Подключение от %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int secret_number = rand() % 100 + 1;
        int attempts = 0;
        int guess;

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                printf("Отключение клиента %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                break;
            }

            guess = atoi(buffer);
            attempts++;

            printf("[%s:%d] Попытка %d: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), attempts, guess);

            if (guess < secret_number) {
                send(client_fd, "more", 4, 0);
            } else if (guess > secret_number) {
                send(client_fd, "less", 4, 0);
            } else {
                send(client_fd, "win", 3, 0);
                printf("[%s:%d] Угадано число %d за %d попыток\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), secret_number, attempts);
                break;
            }
        }

        close(client_fd);
        printf("Отключение клиента %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(server_fd);
    return 0;
}
