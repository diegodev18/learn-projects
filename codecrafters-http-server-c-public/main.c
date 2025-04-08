#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#define BUFFER_SIZE 1024

struct request_target {
    char* header;
    char* body;
};

struct header {
    char* status;
    char* content_type;
    char* content_length;
};

struct response_file {
    bool found;
    int length;
    char* content;
};

char* response_ok = "HTTP/1.1 200 OK\r\n";
char* response_not_found = "HTTP/1.1 404 Not Found\r\n";
char* response_create = "HTTP/1.1 201 Created\r\n";

int global_argc = 0;
char** global_argv = NULL;

char* get_path(char* request_buffer) {
    char* buf = strdup(request_buffer);
    if (!buf) return NULL;

    char* method = strtok(buf, " ");
    char* path = strtok(NULL, " ");

    if (!path) {
        free(buf);
        return NULL;
    }

    char* result = strdup(path);  // Hacer una copia segura
    free(buf);
    return result;
}

char* get_method_of_request(char* request_buffer) {
    char* buf = strdup(request_buffer);
    return strtok(buf, " ");
}

char* get_user_flag(const char* request_buffer, char* flag) {
    char* buf = strdup(request_buffer); // Clonar el buffer original
    if (!buf) return NULL; // Verificar que la asignación fue exitosa

    char* line = strtok(buf, "\n");
    while (line != NULL) {
        if (strncmp(line, flag, strlen(flag)) == 0) {
            char* user_agent = strdup(line + strlen(flag) + 2);
            user_agent[strlen(user_agent)-1] = '\0';
            free(buf);
            // printf("%s\n", user_agent);
            return user_agent;
        }
        line = strtok(NULL, "\n");
    }

    free(buf);
    return NULL;
}

char* get_terminal_flag(char* flag) {
    for (int i = 0; i < global_argc; i++) {
        if (!strcmp(global_argv[i], flag)) {
            return global_argv[i+1];
        }
    }
    return NULL;
}

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

int create_file(char* path_buffer, char* file_content) {
    char* path = strdup(path_buffer);
    FILE* fptr = fopen(path_buffer, "a");

    free(path);

    if (fptr == NULL) {
        printf("Unable to create file\n");
        fclose(fptr);
        return -1;
    }

    fputs(file_content, fptr);
    fclose(fptr);

    return 0;
}

const char* get_valid_encoder(const char* encoder) {
    if (!encoder) return NULL;

    char* buf = strdup(encoder);  // Copiar para evitar modificar el original
    if (!buf) return NULL;

    char* buf_split = strtok(buf, ", ");
    while (buf_split) {
        if (strcmp(buf_split, "gzip") == 0) {
            free(buf);
            return "gzip";
        }
        buf_split = strtok(NULL, ", ");
    }
    return NULL;
}

char* get_send_header(char* header_response, char* content_type, char* content_encoding, int body_len) {
    char* header;
    if (content_encoding) {
        const char* valid_encoder = get_valid_encoder(content_encoding);
        if (valid_encoder) {
            header = malloc(
                sizeof(char) * (strlen(header_response) + strlen("Content-Encoding: \r\n") + strlen(valid_encoder) + strlen("Content-Type: \r\n") + strlen(content_type) + strlen("Content-Length: \r\n") + 6)
            );
            sprintf(header, "%sContent-Encoding: %s\r\nContent-Type: %s\r\nContent-Length: %i\r\n", header_response, valid_encoder, content_type, body_len);
        } else {
            header = malloc(
                sizeof(char) * (strlen(header_response) + strlen("Content-Type: \r\n") + strlen(content_type) + 6)
            );
            sprintf(header, "%sContent-Type: %s\r\n", header_response, content_type);
        }
    } else {
        header = malloc(
            sizeof(char) * (strlen(header_response) + strlen("Content-Type: \r\n") + strlen(content_type) + strlen("Content-Length: \r\n") + 5)
        );
        sprintf(header, "%sContent-Type: %s\r\nContent-Length: %i\r\n", header_response, content_type, body_len);
    }
    return header;
}

char* get_request_line(char* request_buffer, int index) {
    char* buf_splitted = strtok(strdup(request_buffer), "\r\n");

    for (int i = 0; i < index; i++) {
        buf_splitted = strtok(NULL, "\r\n");
    }

    return buf_splitted;
}

int receive_only_response(char** target, const char* response) {
    if (!response) return -1;

    *target = strdup(response); // Duplicamos el string
    if (!(*target)) return -1;  // Si falla strdup, devolvemos error

    return 0;
}

struct request_target get_request_target(char* request_buffer) {
    char* path = get_path(request_buffer);
    char* method_of_request = get_method_of_request(request_buffer);
    struct request_target this_request_target = { NULL, NULL };

    if (!strcmp(path, "/")) {
        if (receive_only_response(&this_request_target.header, response_ok) != 0) printf("Error al copiar la response");

    } else if (!strncmp(path, "/echo/", strlen("/echo/"))) {
        char* encoder = get_user_flag(request_buffer, "Accept-Encoding");
        this_request_target.body = encoder ? "..." : strdup(path) + strlen("/echo/");
        this_request_target.header = get_send_header(response_ok, "text/plain", encoder, (int)strlen(this_request_target.body));

    } else if (!strcmp(path, "/user-agent")) {
        this_request_target.body = get_user_flag(request_buffer, "User-Agent");
        this_request_target.header = get_send_header(response_ok, "text/plain", NULL, strlen(this_request_target.body));

    } else if (!strncmp(path, "/files/", strlen("/files/"))) {
        char* folderpath = get_terminal_flag("--directory");
        if (!folderpath) printf("No hay directorio!\n");
        char* filename = strdup(path) + strlen("/files/");
        char* full_path_buf = malloc(sizeof(char) * (strlen(filename) + strlen(folderpath)));
        if (!full_path_buf) printf("Error al allocar memoria\n");
        sprintf(full_path_buf, "%s%s", folderpath, filename);

        if (!strcmp(method_of_request, "GET")) { // Method 'GET'
            struct response_file file = get_file_content(full_path_buf);
            free(full_path_buf);
            if (file.found) {
                this_request_target.body = file.content;
                this_request_target.header = get_send_header(response_ok, "application/octet-stream", NULL, file.length);
            } else {
                if (receive_only_response(&this_request_target.header, response_not_found) != 0) printf("Error al copiar la response");
            }

        } else { // Method 'POST'
            if (receive_only_response(&this_request_target.header, response_create) != 0) printf("Error al copiar la response");
            char* file_content = get_request_line(request_buffer, 4);
            if (create_file(full_path_buf, file_content) != 0) {
                printf("Error al crear el archivo\n");
            }
            free(full_path_buf);

        }

    } else {
        if (receive_only_response(&this_request_target.header, response_not_found) != 0) printf("Error al copiar la response");

    }

    char* f_header = malloc(sizeof(char) * (strlen(this_request_target.header) + strlen("\r\n") + 1));
    if (!f_header) printf("Error al allocar memoria en f_header\n");
    sprintf(f_header, "%s\r\n", this_request_target.header);
    free(this_request_target.header);
    this_request_target.header = strdup(f_header);

    free(f_header);
    free(path);
    free(method_of_request);
    // printf("%s%s", this_request_target.header, this_request_target.body);
    return this_request_target;
}

void* server_conexion(void* arg) {
    int socket_fd = *(int*)arg;

   	char request_buffer[BUFFER_SIZE];
	if (read(socket_fd, request_buffer, BUFFER_SIZE) < 0) {
	    printf("Error: Couldn't read\n");
		close(socket_fd);
		pthread_exit(NULL);
	}

	struct request_target this_request_target = get_request_target(request_buffer);

	if (this_request_target.header && send(socket_fd, this_request_target.header, strlen(this_request_target.header), 0) < 0) {
        printf("Error: %s \n", strerror(errno));
        close(socket_fd);
        pthread_exit(NULL);
	}
	free(this_request_target.header);
	if (this_request_target.body && send(socket_fd, this_request_target.body, strlen(this_request_target.body), 0) < 0) {
	    printf("Error: %s \n", strerror(errno));
		close(socket_fd);
		pthread_exit(NULL);
	}
	close(socket_fd);
	pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

    global_argc = argc;
    global_argv = argv;

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage

	int server_fd, client_addr_len;
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

	while (true) {
    	int fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    	printf("Client connected\n");

    	pthread_t thread;
    	pthread_create(&thread, NULL, server_conexion, &fd);
    	pthread_detach(thread);
	}
    close(server_fd);

	return 0;
}
