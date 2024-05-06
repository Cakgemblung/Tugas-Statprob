#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <curl/curl.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define MAX_LOG_SIZE 1024

// Function to log changes
void log_change(const char *type, const char *message) {
    FILE *log_file = fopen("change.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, 20, "%d/%m/%y", tm_info);
    fprintf(log_file, "[%s] [%s] %s\n", timestamp, type, message);
    fclose(log_file);
}

// Function to download file from URL
void download_file(const char *url, const char *output_filename) {
    CURL *curl;
    FILE *file;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        file = fopen(output_filename, "wb");
        if (file) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            fclose(file);
        } else {
            perror("Failed to open file for writing");
            exit(EXIT_FAILURE);
        }
        curl_easy_cleanup(curl);
    } else {
        perror("Failed to initialize libcurl");
        exit(EXIT_FAILURE);
    }
}

// Function to handle client requests
void handle_client(int client_socket) {
    char buffer[MAX_BUFFER_SIZE] = {0};

    // Receive command from client
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    printf("Received command from client: %s", buffer);

    // Process command and send response
    // Placeholder implementation, modify as needed
    if (strcmp(buffer, "GET_ALL\n") == 0) {
        strcpy(buffer, "List of all anime titles...");
        send(client_socket, buffer, strlen(buffer), 0);
    } else if (strcmp(buffer, "exit\n") == 0) {
        strcpy(buffer, "exit\n");
        send(client_socket, buffer, strlen(buffer), 0);
    } else {
        strcpy(buffer, "Invalid Command\n");
        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Log command if it is ADD, EDIT, or DEL
    if (strncmp(buffer, "[ADD]", 5) == 0 || strncmp(buffer, "[EDIT]", 6) == 0 || strncmp(buffer, "[DEL]", 5) == 0) {
        log_change(buffer, "anime changed");
    }

    // Close client socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(server_addr);

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind server socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Download file from URL
    download_file("https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50", "myanimelist.csv");

    // Accept incoming connections and handle them
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Handle client request
        handle_client(client_socket);
    }

    return 0;
}
