#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Địa chỉ IP của server
#define SERVER_PORT 8081      // Cổng của server
#define BUFFER_SIZE 4096

void save_to_file(const char *data);

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    char *received_data = NULL;
    size_t total_data_len = 0;

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);

    // Kết nối tới server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        close(sock);
        exit(1);
    }
    printf("Connected to server\n");

    // \tSourcePort\tDestinationPort
    printf("   Time   \t\tSource_IP\t   Destination_IP   \tProtocol\t    Length\t\tInfo\n");
    // Nhận dữ liệu từ server liên tục
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        // Tăng kích thước bộ nhớ đệm để chứa dữ liệu mới nhận
        char *new_data = realloc(received_data, total_data_len + bytes_received + 1);
        if (new_data == NULL) {
            perror("Failed to reallocate memory");
            free(received_data); // Giải phóng bộ nhớ trước khi thoát
            close(sock);
            exit(1);
        }
        received_data = new_data;

        // Sao chép dữ liệu mới nhận vào bộ nhớ đệm
        memcpy(received_data + total_data_len, buffer, bytes_received);
        total_data_len += bytes_received;
        received_data[total_data_len] = '\0'; // Kết thúc chuỗi

        // Hiển thị dữ liệu nhận được
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        perror("Receive failed");
    }

    // Lưu dữ liệu vào file log.txt
    save_to_file(received_data);

    // Giải phóng bộ nhớ
    free(received_data);

    // Đóng socket
    close(sock);

    return 0;
}

// Hàm lưu dữ liệu vào tệp log.txt
void save_to_file(const char *data) {
    FILE *file = fopen("log.txt", "w");
    if (file == NULL) {
        perror("Could not open file");
        exit(1);
    }

    fprintf(file, "%s", data);
    fclose(file);

    printf("\nData saved to log.txt\n");
}
