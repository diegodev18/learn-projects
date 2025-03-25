#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int server_fd;
	socklen_t client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	// accept -> Responses of then server
	int fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	printf("Client connected\n");

	const int BUFFER_SIZE = 1024;
	char request_buffer[BUFFER_SIZE];
    if (read(fd, request_buffer, BUFFER_SIZE) < 0) {
        printf("Read failed: %s \n", strerror(errno));
        return 1;
    } else {
        printf("Request from client: %s\n", request_buffer);
    }

	char* response_ok = "HTTP/1.1 200 OK\r\n\r\n";
	char* response_not_found = "HTTP/1.1 404 Not Found\r\n\r\n";

	char* path = strtok(request_buffer, " ");
	path = strtok(NULL, " ");

	char* header;
	char* body;
	if (strcmp(path, "/") == 0) {
	    header = response_ok;
    } else if (strncmp(path, "/echo", strlen("/echo")) == 0) {
        header = malloc(sizeof(char) * (strlen(response_ok) + strlen("Content-Type: text/plain\r\nContent-Length: 0000\r\n\r\n") + 1));
        body = path + strlen("/echo/");
        sprintf(header, "%sContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n", response_ok, strlen(body));
        printf("%s\n", header);
    } else {
        header = response_not_found;
	}

    if (header && send(fd, header, strlen(header), 0) < 0) {
        printf("Error: %s \n", strerror(errno));
    }
    if (body && send(fd, body, strlen(body), 0) < 0) {
        printf("Error: %s \n", strerror(errno));
    }

	close(server_fd);

	return 0;
}
