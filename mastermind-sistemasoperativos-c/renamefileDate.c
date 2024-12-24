#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

char *path, *fullpath, *new_fullpath;
char *logs_path = "log.txt";

void write_logs(char *text_to_write) { // Funcion para escribir logs
    FILE *f = fopen(logs_path, "a+"); // Se abre el archivo logs en 'a+' para que se reescriba el fichero
    fprintf(f, "\n%s", text_to_write); // Se escribe el log en el fichero txt
    fclose(f); // Se cierra el fichero
    printf("%s -> Log escrito!\n", text_to_write); // Se imprime el log en pantalla para mostrar el error
}

void *rename_file(void *arg) { // Funcion del hilo
    rename(fullpath, new_fullpath); // Se renombra el archivo
}

char *get_date() {
    struct statx buf;

    if (statx(AT_FDCWD, fullpath, AT_STATX_SYNC_AS_STAT, STATX_BTIME, &buf) == -1) { // Obtener información del archivo utilizando statx
        return NULL;
    }
    if (buf.stx_mask & STATX_BTIME) { // Verificar si el campo de creación está disponible
        time_t creation_time = buf.stx_btime.tv_sec; // Convertir a tiempo legible
        struct tm *tm = localtime(&creation_time);
        if (tm) {
            char *fecha = malloc(9); // Reservar memoria para la cadena de fecha -> AAAAMMDD (8 caracteres + 1 para '\0')
            strftime(fecha, 9, "%Y%m%d", tm); // Formato AAAAMMDD
            return fecha; // Devolver la cadena asignada dinámicamente
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) { // Valida si se entregan los argumentos esperados
        printf("Error: Faltan argumentos (Esperados -> [PROGRAMA, DIRECTORIO])\n"); // Si no se entregan, se imprime el error
        exit(-1); // Y se cierra el programa
    }
    path = argv[1]; // Se guarda la direccion del "directorio" en un variable global

    FILE *f = fopen(logs_path, "a+"); // Se abre el fichero de logs al inicio del programa
    write_logs("-------------------- Start logs --------------------"); // Para dividor los diferentes logs de cada ejecucion
    fclose(f); // Se cierra el fichero log

    if (path[strlen(path) - 1] == '/') { // Se valida si se entrega un '/' en el argumento
        path[strlen(path) - 1] = '\0'; // Si se entrega el '/' se elimina
    }

    printf("Dir: \033[01;33m%s\033[0m\n\n", path); // Se imprime el directorio

    struct stat data; // Se crea un struct de stat para la informacion del directorio
    struct dirent *dp; // Se crea un struct de dirent para la informacion de cada fichero o directorio dentro del directorio entregado
    DIR *dir = opendir(path); // Se abre el directorio entregado en los argumentos

    if (errno == ENOENT) { // Se valida que exista el directorio
        printf("El directorio %s no existe\n", path);
        write_logs("El directorio otorgado no existe, se cierra el programa!");
        exit(-1);
    } else if (stat(path, &data)) { // Se guarda el stat de path dentro de data y se valida que salga bien
        printf("Error: Ha fallado stat\n");
        write_logs("Ha fallado stat, se cierra el programa!");
        exit(-1);
    } else if (!S_ISDIR(data.st_mode)) { // Se valida que la direccion sea un directorio
        printf("Error: El argumento no es un directorio\n");
        write_logs("El argumento no es un directorio, se cierra el programa!");
        exit(-1);
    }

    int i;
    for (i = 0; (dp = readdir(dir)) != NULL; i++) { // Lee los archivos dentro del directorio otorgado mediante argumento
        if (!strcmp(dp -> d_name, ".") || !strcmp(dp -> d_name, "..")) { // Se valida que los ficheros iterados no sean '.' ni '..'
            i--;
            continue;
        }

        fullpath = malloc(strlen(path) + strlen(dp -> d_name) + 1); // Se crea el espacio en 'fullpath'
        sprintf(fullpath, "%s/%s", path, dp -> d_name); // Se mete la direccion del fichero iterado dentro de 'fullpath'

        char *created_date = get_date(); // Se llama a la funcion 'get_date' para obtener la fecha de creacion del fichero iterado
        if (!created_date) { // Si no se obtiene la fecha exitosamente se escribe el log, y se pasa al siguiente fichero iterado
            write_logs("Hubo un fallo al obtener la fecha");
            continue;
        }

        new_fullpath = malloc(strlen(path) + strlen(created_date) + strlen(dp -> d_name) + 2); // Se crea el espacio para el nuevo nombre del fichero
        sprintf(new_fullpath, "%s/%s_%s", path, created_date, dp -> d_name); // Se mete en 'new_fullpath' el nuevo nombre del fichero

        pthread_t thread; // Se crea el hilo

        char *log = malloc(strlen(fullpath) + strlen(new_fullpath) + strlen("Cambio nombre de a exitoso!") + 1); // Se crea el espacio para guardar el log
        sprintf(log, "Cambio nombre de %s a %s exitoso!", fullpath, new_fullpath);
        write_logs(log);

        pthread_create(&thread, NULL, rename_file, NULL); // Se inicia el hilo para renombrar el fichero
        pthread_join(thread, NULL); // Se cierra el hilo

        free(fullpath); // Se libera la direccion del fichero original
        free(new_fullpath); // Se libera la direccion del fichero renombrado
    }
    printf("Archivos modificados = %i\n", i);

    closedir(dir);

    return 0;
}
