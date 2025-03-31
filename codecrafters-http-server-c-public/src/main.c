#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#define BUFFER_SIZE 1024

struct response_structure {
    char* header;
    char* body;
};

struct response_file {
    bool found;
    int length;
    char* content;
};

struct args {
    int argc;
    char** argv;
};

struct args global_args = { 0, NULL };

char* response_ok = "HTTP/1.1 200 OK\r\n";
char* response_not_found = "HTTP/1.1 404 Not Found\r\n";

struct response_file get_file_content(char* path) {
    struct response_file response = { false, 0, NULL };
    long length;
    FILE* f = fopen(path, "rb");
    if (f) {
        response.found = true;
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        response.content = malloc(length + 1);
        response.length = length;
        if (response.content) {
            fread(response.content, 1, length, f);
            return response;
        }
        fclose(f);
    }
    return response;
}

struct response_structure get_response(char* path) {
    struct response_structure response = { response_not_found, NULL };

	if (strcmp(path, "/") == 0) {
	    response.header = response_ok;
    } else if (strncmp(path, "/echo", strlen("/echo")) == 0) {
        char* message = path + strlen("/echo/");
        response.body = malloc(strlen(message) + 1);
        if (response.body == NULL) {
            perror("Malloc failed");
            exit(EXIT_FAILURE);
        }
        strcpy(response.body, message);

        response.header = malloc((strlen(response_ok) + strlen("Content-Type: text/plain\r\nContent-Length: 0000\r\n\r\n") + 20));
        if (response.header == NULL) {
            perror("Malloc failed");
            free(response.body);
            exit(EXIT_FAILURE);
        }
        sprintf(response.header, "%sContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n", response_ok, strlen(response.body));
    } else if (strncmp(path, "/file", strlen("/file")) == 0) {
        if (global_args.argc < 3) { // Cuando no se pasa el path ni el flag
            // printf("No found PATH in Program Arguments\n");
            return response;
        }
        if (strcmp(global_args.argv[1], "--directory")) { // Cuando no se usa el flag
            // printf("No \"--directory\" flag used\n");
            return response;
        }
        const char* folder_path = global_args.argv[2];
        char* file_path = path + strlen("/file/");
        char* file_path_full = malloc(strlen(folder_path) + strlen(file_path) + 1);
        if (file_path_full == NULL) {
            return response;
        }
        sprintf(file_path_full ,"%s%s", folder_path, file_path);
        struct response_file response_fil_this = get_file_content(file_path_full);
        free(file_path_full);
        if (!response_fil_this.found) { // Si no hay archivo
            response.header = response_not_found;
            return response;
        }
        // Si hay archivo...
        response.header = malloc((strlen(response_ok) + strlen("Content-Type: application/octet-stream\r\nContent-Length: 0000\r\n\r\n") + 20));
        sprintf(response.header, "%sContent-Type: application/octet-stream\r\nContent-Length: %i\r\n\r\n", response_ok, response_fil_this.length);
        response.body = response_fil_this.content;
    }
	return response;
}

void* thread_function(void* arg) {
    int fd = *((int*) arg);

   	char request_buffer[BUFFER_SIZE];
    if (read(fd, request_buffer, BUFFER_SIZE) < 0) {
        printf("Read failed: %s \n", strerror(errno));
        pthread_exit(NULL);
    } else {
        printf("Request from client: %s\n", request_buffer);
    }

	char* path = strtok(request_buffer, " ");
	path = strtok(NULL, " ");

	struct response_structure response = get_response(path);

    if (response.header && send(fd, response.header, strlen(response.header), 0) < 0) {
        printf("Error: %s \n", strerror(errno));
    }
    if (response.body && send(fd, response.body, strlen(response.body), 0) < 0) {
        printf("Error: %s \n", strerror(errno));
    }

    close(fd);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    global_args.argc = argc;
    global_args.argv = argv;

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

	printf("Waiting for a client to connect...\n\n");
	client_addr_len = sizeof(client_addr);

	// accept -> Responses of then server
	while (1) {
    	int fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
    	printf("Client connected\n");

    	pthread_t thread;
    	pthread_create(&thread, NULL, thread_function, &fd);
    	pthread_detach(thread);
	}

	close(server_fd);

	return 0;
}
