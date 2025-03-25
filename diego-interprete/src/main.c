#include <_string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EXTENSION ".diego"
#define BUFFER_SIZE 1024

char* read_file(char* filename);
int has_extension(const char* filename, const char* extension);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    char* filename = argv[1];

    if (!has_extension(filename, EXTENSION)) {
        printf("Missing extension \".diego\"\n");
        return 1;
    }
    char* buffer = malloc(BUFFER_SIZE);
    FILE *file = fopen(filename, "r");

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        char* command = strtok(buffer, " ");
        char* arg = strtok(NULL, "");

        if (arg[strlen(arg)-1] == '\n') {
            arg[strlen(arg)-1] = '\0';
        }
        if (arg[0] == '\"' && arg[strlen(arg)-1] == '\"') {
            arg++;
            arg[strlen(arg)-1] = '\0';
        }

        if (command && !strcmp(command, "dout.")) {
            if (arg) {
                printf("%s\n", arg);
            } else {
                printf("Error: Missing argument for dout.\n");
                return 1;
            }
        } else if (command && !strcmp(command, "dat.")) {
            if (arg) {
                printf("%s\n", read_file(arg));
            } else {
                printf("Error: Missing argument for dout.\n");
                return 1;
            }
        } else if (command && !strcmp(command, "din.")) {
            printf("%s", arg);
            getchar();
        } else if (command[0] == ' ') {
            continue;
        } else {
            printf("Error: Invalid command -> %s\n", buffer);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

char* read_file(char* filename) {
    char* buffer = 0;
    long length;
    FILE* f = fopen(filename, "r");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);

        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
    }
    if (buffer) {
        return buffer;
    } else {
        printf("Error: FILE \"%s\" NOT FOUND\n", filename);
        exit(0);
    }
}

int has_extension(const char* filename, const char* extension) {
    size_t filename_len = strlen(filename);
    size_t extension_len = strlen(extension);

    if (filename_len < extension_len) {
        return 0; // El archivo es más corto que la extensión
    }

    return strcmp(filename + filename_len - extension_len, extension) == 0;
}
