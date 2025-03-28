#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUFFER_SIZE 1024

// Функция для обработки одного клиента
void handle_client(int client_sock, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    int n;
    int target = rand() % 100 + 1; // Случайное число от 1 до 100
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    printf("%s: Клиент подключился.\n", client_ip);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            break;
        }
        buffer[n] = '\0';
        printf("%s: Получено сообщение: %s\n", client_ip, buffer);
        
        int guess = atoi(buffer);
        char response[BUFFER_SIZE];
        if (guess < target) {
            strcpy(response, ">");
        } else if (guess > target) {
            strcpy(response, "<");
        } else {
            strcpy(response, "=");
        }
        
        send(client_sock, response, strlen(response), 0);
        printf("%s: Отправлен ответ: %s\n", client_ip, response);
        
        if (guess == target) {
            break;
        }
    }
    
    printf("%s: Клиент отключился.\n", client_ip);
    close(client_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <порт>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int port = atoi(argv[1]);
    
    srand(time(NULL));
    
    // Создание сокета
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }
    
    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Привязка сокета
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    // Перевод сокета в режим прослушивания
    if (listen(server_sock, 5) < 0) {
        perror("Ошибка listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер запущен на порту %d\n", port);
    
    // Основной цикл для приема соединений
    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            perror("Ошибка accept");
            continue;
        }
        
        // Обработка клиента (в данном примере в одном процессе; для масштабирования можно создавать потоки)
        handle_client(client_sock, client_addr);
    }
    
    close(server_sock);
    return 0;
}
