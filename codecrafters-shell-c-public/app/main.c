#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>

void type_builtin(char *input, char **commands) {
  char *used_input = input + 5;

  for (int i = 0; commands[i]; i++) {
    if (!strcmp(used_input, commands[i])) {
      printf("%s is a shell builtin\n", used_input);
      return;
    }
  }
  char *pathenv = getenv("PATH");
  char *delimit = ":";
  char *token;
  DIR *dir;
  struct dirent *dp;
  token = strtok(pathenv, delimit);

  do {
      char *temp_path = malloc(strlen(token) + strlen(used_input) + 2);
      sprintf(temp_path, "%s/%s", token, used_input);
      if (!access(temp_path, X_OK)) {
        printf("%s is %s\n", used_input, temp_path);
        free(temp_path);
        return;
      }
      free(temp_path);
  } while ((token = strtok(NULL, delimit)) != NULL);

  printf("%s: not found\n", used_input);
}

int main(int argc, char **argv) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Wait for user input
  char input[100];
  char *commands[] = {"exit", "echo", "type", NULL};
  while (1) {
    printf("$ ");
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0';

    // Salidas
    if (!strcmp(input, "")) {
      return 0;
    }
    else if (!strncmp(input, "exit 0", strlen("exit 0"))) {
      return 0;
    }
    
    // Comandos
    if (!strncmp(input, commands[1], strlen(commands[1]))) { // echo
      printf("%s\n", input + 5);
      continue;
    }
    else if (!strncmp(input, commands[2], strlen(commands[2]))) { // type
      type_builtin(input, commands);
      continue;
    }

    printf("%s: command not found\n", input);
  }

  return 0;
}
