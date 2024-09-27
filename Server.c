#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int client_sock;

void analyze_and_send_packets() {
    char data[2048];
    FILE *fp;

    // Gọi tshark 
    // -e tcp.srcport -e tcp.dstport
    fp = popen("tshark -i wlp0s20f3 -T fields -e frame.time_delta -e ip.src -e ip.dst -e _ws.col.Protocol  -e frame.len -e _ws.col.Info", "r");
    if (fp == NULL) {
        perror("Failed to run tshark");
        exit(1);
    }

    char formatted_data[4096];
    char frame_time_delta[64], ip_src[64], ip_dst[64], protocol[32], frame_len[32], info[2048];


    // Đọc từng dòng từ `tshark` và gửi tới Client
    while (fgets(data, sizeof(data) - 1, fp) != NULL) {
        // Sử dụng sscanf để tách từng phần dữ liệu
        sscanf(data, "%s %s %s %s %s %[^\n]", frame_time_delta, ip_src, ip_dst, protocol, frame_len, info);

        // Định dạng dữ liệu với các khoảng trắng tùy chỉnh
        sprintf(formatted_data, "%-15s       %-15s        %-15s        %-10s        %-10s        %s\n",
                frame_time_delta, ip_src, ip_dst, protocol, frame_len, info);

        // Gửi dữ liệu đã được định dạng tới client
        if (send(client_sock, formatted_data, strlen(formatted_data), 0) < 0) {
            puts("Send failed");
            break;
        }
    }

    pclose(fp);
}

int main() {
    struct sockaddr_in server, client;
    int socket_desc, c;

    // Tạo socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    // Chuẩn bị sockaddr_in cho server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8081);

    // Bind socket
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed. Error");
        return 1;
    }

    // Lắng nghe kết nối
    listen(socket_desc, 3);

    // Chấp nhận kết nối từ client
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    }
    printf("CONNECTION ACCEPT !!! \n");

    // Thu thập và phân tích gói tin mạng
    analyze_and_send_packets();

    // Đóng kết nối
    close(client_sock);
    close(socket_desc);

    return 0;
}
