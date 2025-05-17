#include "SimpleOfuscator.h"

uint8_t* read_file(const char *filename, size_t *size_output) {
    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *buffer = (uint8_t *)malloc(file_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);
    *size_output = file_size;
    return buffer;
}

int write_file(const char *filename, uint8_t *data, size_t size) {
    FILE *file = fopen(filename, "wb");
    if (!file) return 0;
    fwrite(data, 1, size, file);
    fclose(file);
    return 1;
}

void print_usage(const char *progname) {
    printf("Uso:\n");
    printf("  %s o <archivo_entrada> <archivo_salida> <password>\n", progname);
    printf("  %s d <archivo_entrada> <archivo_salida> <password>\n", progname);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
        return 1;
    }

    char modo = argv[1][0];
    const char *archivo_entrada = argv[2];
    const char *archivo_salida  = argv[3];
    const char *password_str    = argv[4];

    uint8_t *password = (uint8_t*)password_str;
    size_t password_len = strlen(password_str);

    if (password_len == 0) {
        printf("La clave no puede ser vacía.\n");
        return 2;
    }

    size_t size_data = 0;
    uint8_t *data = read_file(archivo_entrada, &size_data);
    if (!data) {
        printf("No se pudo abrir el archivo de entrada: %s\n", archivo_entrada);
        return 3;
    }
    printf("Entropia de los datos de entrada: %f\n", calcular_entropia(data, size_data));

    uint8_t *data_out = NULL;
    size_t size_out = 0;

    if (modo == 'o') {
        ofusc(data, size_data, password, password_len, &data_out, &size_out);
        printf("Entropia de los datos de salida: %f\n", calcular_entropia(data_out, size_out));
        if (!data_out) { printf("Error en ofuscación\n"); free(data); return 5; }
        if (write_file(archivo_salida, data_out, size_out))
            printf("Archivo ofuscado y comprimido guardado en: %s\n", archivo_salida);
        else
            printf("No se pudo escribir el archivo de salida: %s\n", archivo_salida);
    } else if (modo == 'd') {
        desofuc(data, size_data, password, password_len, &data_out, &size_out);
        if (!data_out) { printf("Error en desofuscación\n"); free(data); return 6; }
        if (write_file(archivo_salida, data_out, size_out))
            printf("Archivo descomprimido y desofuscado guardado en: %s\n", archivo_salida);
        else
            printf("No se pudo escribir el archivo de salida: %s\n", archivo_salida);
    } else {
        print_usage(argv[0]);
    }

    free(data);
    if (data_out) free(data_out);
    return 0;
}
