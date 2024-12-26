#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

char *new_put(char *input);

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
  char *used_input = strndup(input, strlen(input));
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

char **separate(char *input) {
    int length = strlen(input);
    char **separated = malloc(length * sizeof(char *));
    if (!separated) {
        perror("Fallo en la asignación de memoria");
        return NULL;
    }

    int quote_passed = 0;           // Indica si estamos dentro de comillas
    char current_quote = '\0';      // Almacena la comilla actual (' o ")
    int j = 0;                      // Índice para las subcadenas
    separated[j] = malloc(length + 1);  // Asignar memoria para la primera subcadena
    if (!separated[j]) {
        perror("Fallo en la asignación de memoria");
        free(separated);
        return NULL;
    }

    int k = 0;  // Índice dentro de la subcadena actual

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\'' || input[i] == '\"') {
            if (quote_passed == 0) {
                // Comenzamos una nueva subcadena con una comilla
                if (k > 0) {  // Finalizar subcadena sin comillas antes de empezar una nueva con comillas
                    separated[j][k] = '\0';
                    j++;
                    separated[j] = malloc(length + 1);
                    if (!separated[j]) {
                        perror("Fallo en la asignación de memoria");
                        for (int m = 0; m < j; m++) {
                            free(separated[m]);
                        }
                        free(separated);
                        return NULL;
                    }
                    k = 0;
                }
                quote_passed = 1;
                current_quote = input[i];
                separated[j][k++] = input[i];
            } else if (quote_passed == 1 && input[i] == current_quote) {
                // Cerramos la subcadena actual
                separated[j][k++] = input[i];
                separated[j][k] = '\0';  // Terminar la subcadena actual
                j++;
                separated[j] = malloc(length + 1);  // Preparar para la próxima subcadena
                if (!separated[j]) {
                    perror("Fallo en la asignación de memoria");
                    for (int m = 0; m < j; m++) {
                        free(separated[m]);
                    }
                    free(separated);
                    return NULL;
                }
                k = 0;
                quote_passed = 0;
                current_quote = '\0';
            } else {
                // Añadir comillas dentro de una subcadena existente
                separated[j][k++] = input[i];
            }
        } else if (isspace(input[i]) && quote_passed == 0) {
            if (k > 0) {  // Finalizar una subcadena sin comillas
                separated[j][k] = '\0';
                j++;
                separated[j] = malloc(length + 1);
                if (!separated[j]) {
                    perror("Fallo en la asignación de memoria");
                    for (int m = 0; m < j; m++) {
                        free(separated[m]);
                    }
                    free(separated);
                    return NULL;
                }
                k = 0;
            }
        } else {
            separated[j][k++] = input[i];
        }
    }

    if (k > 0) {
        separated[j][k] = '\0';  // Terminar la última subcadena
        j++;
    }

    separated[j] = NULL;  // Terminar el array de punteros
    return separated;
}

bool execute_program(char *input, char *pathenv) {
  char *inputdup = strndup(input, strlen(input));
  int j = 1;
  char *only_put;
  if (input[0] == '\"') {
    only_put = strtok(inputdup, "\"");
    j++;
  } else if (input[0] == '\'') {
    only_put = strtok(inputdup, "'");
    j++;
  } else {
    only_put = strtok(inputdup, " ");
  }
  char **my_args = separate(input + strlen(only_put) + j);
  char *new_args[10];

  new_args[0] = only_put;
  j = 1;
  for (int i = 0; my_args[i] != NULL; i++) {
    new_args[j++] = new_put(strndup(my_args[i], strlen(my_args[i])));
  }
  new_args[j] = NULL;
  free(my_args);

  char *program;
  if ((program = get_external_command(only_put, pathenv))) {
    pid_t pid = fork();
    if (pid == 0) {
      execv(program, new_args);
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

int count_characters(char *input, char characther) {
  int counter = 0;
  for (int i = 0; input[i]; i++) {
    if (input[i] == characther) {
      counter++;
    }
  }
  return counter;
}

char *new_put(char *input) {
  char *new_input = malloc(strlen(input) + 1);
  
  bool quote = false;
  bool double_quote = false;
  int j = 0;
  for (int i = 0; input[i] != '\0'; i++) {

    if (input[i] == '\'') {
      quote = true;
      
      while (true) {
        i++;

        if (input[i] == '\'' || input[i] == '\0') {
          break;
        }

        new_input[j++] = input[i];
        // printf("Quote: %c\n", input[i]);
      }
    }

    else if (input[i] == '\"') {
      double_quote = true;

      while (true) {
        i++;

        if (input[i] == '\\' && input[i+1] == '\\') {
          new_input[j++] = '\\';
          i++;
          continue;
        }

        else if (input[i] == '\\' && input[i+1] == '\"') { // Estoy trabajando aqui
          new_input[j++] = '\"';
          i++;
          continue;
        }

        else if (input[i] == '\"' || input[i] == '\0') {
          break;
        }

        new_input[j++] = input[i];
        // printf("Double Quote: %c\n", input[i]);
      }
    }

    else if (input[i] == ' ' && input[i-1] == ' ' && double_quote) {
      continue;
    }

    else {
      if (input[i] == '\\' && input[i+1] == ' ') {
        continue;
      }

      else if (input[i] == '\\' && input[i+1] == '\\') {
        new_input[j++] = '\\';
        i++;
        continue;
      }

      else if (input[i] == '\\' && input[i+1] == '\"') {
        new_input[j++] = '\"';
        i++;
        continue;
      }

      else if (input[i] == '\\') {
        continue;
      }

      else if (input[i] == ' ' && input[i-1] == ' ') {
        continue;
      }

      new_input[j++] = input[i];
    }

  }
  new_input[j] = '\0';
  
  return new_input;
}

void get_pwd() {
  char pwd[200];
  getcwd(pwd, sizeof(pwd));
  printf("%s\n", pwd);
}

void cd(char *directory) {
  if (chdir(directory) == 0) {
    return;
  } else if (directory[0] == '~') {
    char *home = getenv("HOME");
    char *full_dir = malloc(strlen(home) + strlen(directory) + 1);
    sprintf(full_dir, "%s%s", home, directory + 1);
    if (chdir(full_dir) == 0) {
      free(full_dir);
      return;
    }
    free(full_dir);
  } else if (directory[0] == '\0') {
    char *home = getenv("HOME");
    if (chdir(home) == 0) {
      return;
    }
  }
  printf("cd: %s: No such file or directory\n", directory);
}

int main(int argc, char **argv) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Wait for user input
  char input[100];
  char *commands[] = {"exit", "echo", "type", "pwd", "cd", NULL};
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
      char *new_input = new_put(input + 5);
      printf("%s\n", new_input);
      free(new_input);
      continue;
    }
    else if (!strncmp(input, commands[2], strlen(commands[2]))) { // type
      char *new_input = new_put(input + 5);
      type_builtin(new_input, commands, getenv("PATH"));
      free(new_input);
      continue;
    }
    else if (!strncmp(input, commands[3], strlen(commands[3]))) { // pwd
      get_pwd();
      continue;
    }
    else if (!strncmp(input, commands[4], strlen(commands[4]))) { // cd
      char *new_input = new_put(input + 3);
      cd(new_input);
      free(new_input);
      continue;
    }
    else if (execute_program(input, getenv("PATH"))) {
      continue;
    }

    printf("%s: command not found\n", input);
  }

  return 0;
}
