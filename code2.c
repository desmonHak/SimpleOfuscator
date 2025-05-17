#include "LZ77_c/LZ77.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include <math.h>

double calcular_entropia(uint8_t *datos, size_t tam) {
    // fórmula de la entropía de Shannon
    size_t conteo[256] = {0};
    for (size_t i = 0; i < tam; i++) conteo[datos[i]]++;
    double entropia = 0.0;
    for (int i = 0; i < 256; i++) {
        if (conteo[i] == 0) continue;
        double p = (double)conteo[i] / tam;
        entropia -= p * log2(p);
    }
    return entropia;
}


void transposicion(uint8_t* data, size_t size, uint8_t* pass, size_t pass_size) {
    for (size_t i = 0; i < size; i++) {
        size_t j = (i + pass[i % pass_size]) % size;
        uint8_t tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

void destransposicion(uint8_t* data, size_t size, uint8_t* pass, size_t pass_size) {
    for (ssize_t i = size - 1; i >= 0; i--) {
        size_t j = (i + pass[i % pass_size]) % size;
        uint8_t tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

void ofusc(
    uint8_t *data, 
    size_t   size_of_data,
    uint8_t *pass,
    size_t   size_of_pass,
    uint8_t **data_output,   // ¡OJO! Ahora es doble puntero
    size_t  *size_of_data_output) 
{
    // 1. Ofuscar y transponer
    uint8_t *tmp = malloc(size_of_data);
    if (!tmp) { *data_output = NULL; *size_of_data_output = 0; return; }
    for (size_t i = 0; i < size_of_data; i++)
        tmp[i] = (uint8_t)(data[i] - pass[i % size_of_pass]);
    transposicion(tmp, size_of_data, pass, size_of_pass);

    // 2. Comprimir con LZ77
    LZ77Params params = default_params;
    DerivedParams derived = calculate_derived_params(&params);
    diccionario = (unsigned char *)malloc(derived.tam_diccionario + derived.max_coincidencia);
    hash = (unsigned int *)malloc(derived.tam_hash * sizeof(unsigned int));
    siguiente_enlace = (unsigned int *)malloc(derived.tam_diccionario * sizeof(unsigned int));
    if (!diccionario || !hash || !siguiente_enlace) {
        free(tmp); *data_output = NULL; *size_of_data_output = 0; return;
    }

    size_t max_comprimido = size_of_data * 2 + 32;
    uint8_t *comprimido = malloc(max_comprimido);
    if (!comprimido) {
        free(tmp); free(diccionario); free(hash); free(siguiente_enlace);
        *data_output = NULL; *size_of_data_output = 0; return;
    }

    int tam_comprimido = CodificarBuffer(&params, &derived, tmp, size_of_data, comprimido, max_comprimido);
    if (tam_comprimido < 0) {
        fprintf(stderr, "Error al comprimir con LZ77\n");
        free(tmp); free(diccionario); free(hash); free(siguiente_enlace); free(comprimido);
        *data_output = NULL; *size_of_data_output = 0; return;
    }

    *data_output = comprimido;
    *size_of_data_output = tam_comprimido;

    free(tmp);
    free(diccionario); free(hash); free(siguiente_enlace);
}


void desofuc(
    uint8_t *data, 
    size_t size_of_data,
    uint8_t *pass,
    size_t size_of_pass,
    uint8_t **data_output,   // ¡OJO! Ahora es doble puntero
    size_t* size_of_data_output) 
{
    // 1. Descomprimir con LZ77
    LZ77Params params = default_params;
    DerivedParams derived = calculate_derived_params(&params);
    diccionario = (unsigned char *)malloc(derived.tam_diccionario + derived.max_coincidencia);
    hash = (unsigned int *)malloc(derived.tam_hash * sizeof(unsigned int));
    siguiente_enlace = (unsigned int *)malloc(derived.tam_diccionario * sizeof(unsigned int));
    if (!diccionario || !hash || !siguiente_enlace) {
        *data_output = NULL; *size_of_data_output = 0; return;
    }

    // El tamaño real descomprimido no lo sabemos, pero puedes pedirlo como argumento o estimarlo
    size_t max_descomprimido = size_of_data * 4 + 32;
    uint8_t *descomprimido = malloc(max_descomprimido);
    if (!descomprimido) {
        free(diccionario); free(hash); free(siguiente_enlace);
        *data_output = NULL; *size_of_data_output = 0; return;
    }

    int tam_descomprimido = DecodificarBuffer(&params, &derived, data, size_of_data, descomprimido, max_descomprimido);
    if (tam_descomprimido < 0) {
        fprintf(stderr, "Error al descomprimir con LZ77\n");
        free(diccionario); free(hash); free(siguiente_enlace); free(descomprimido);
        *data_output = NULL; *size_of_data_output = 0; return;
    }

    // 2. Destransponer y desofuscar
    destransposicion(descomprimido, tam_descomprimido, pass, size_of_pass);
    for (int i = 0; i < tam_descomprimido; i++)
        descomprimido[i] = (uint8_t)(descomprimido[i] + pass[i % size_of_pass]);

    *data_output = descomprimido;
    *size_of_data_output = tam_descomprimido;

    free(diccionario); free(hash); free(siguiente_enlace);
}


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
