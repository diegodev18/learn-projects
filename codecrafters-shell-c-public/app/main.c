#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

char *get_external_command(char *input, char *pathenv) {
  char *pathdup = strndup(pathenv, strlen(pathenv));
  char *delimit = ":";
  char *token;
  DIR *dir;
  struct dirent *dp;
  token = strtok(pathdup, delimit);

  if (strlen(input) < 2) {
    return NULL;
  }

  do {
      char *temp_path = malloc(strlen(token) + strlen(input) + 2);
      sprintf(temp_path, "%s/%s", token, input);
      if (!access(temp_path, X_OK)) {
        return temp_path;
      }
      free(temp_path);
  } while ((token = strtok(NULL, delimit)));
}

void type_builtin(char *input, char **commands, char *pathenv) {
  char *used_input = input + 5;
  char *external_command;

  for (int i = 0; commands[i]; i++) {
    if (!strcmp(used_input, commands[i])) {
      printf("%s is a shell builtin\n", used_input);
      return;
    }
  }

  if ((external_command = get_external_command(used_input, pathenv))) {
    printf("%s is %s\n", used_input, external_command);
    free(external_command);
    return;
  }

  printf("%s: not found\n", used_input);
}

bool execute_program(char *input, char *pathenv) {
  char *inputdup = strndup(input, strlen(input));
  char *only_put = strtok(inputdup, " ");
  char *my_args[10];
  my_args[0] = only_put;

  int i = 1;
  char *the_arg;
  while ((the_arg = strtok(NULL, " ")) != NULL) {
    my_args[i++] = the_arg;
  }
  my_args[i] = NULL;

  char *program;
  if ((program = get_external_command(only_put, pathenv))) {
    pid_t pid = fork();
    if (pid == 0) {
      execv(program, my_args);
      perror("execv failed");
      exit(-1);
    } else if (pid > 0) {
      wait(NULL);
    } else {
      perror("fork failed");
    }
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Wait for user input
  char input[100];
  char *commands[] = {"exit", "echo", "type", NULL};
  while (true) {
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
      type_builtin(input, commands, getenv("PATH"));
      continue;
    }
    else if (execute_program(input, getenv("PATH"))) {
      continue;
    }

    printf("%s: command not found\n", input);
  }

  return 0;
}
