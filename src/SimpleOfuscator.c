#ifndef SIMPLE_OFUSCATOR_C
#define SIMPLE_OFUSCATOR_C

#include "SimpleOfuscator.h"


static inline int log2_int(size_t n) {
    if (n == 0) return -1; // indefinido para log2(0)

#if defined(__GNUC__) || defined(__clang__)
#if SIZE_MAX == UINT64_MAX
    return 63 - __builtin_clzll(n);
#else
    return 31 - __builtin_clz((unsigned int)n);
#endif

#elif defined(_MSC_VER)
#include <intrin.h>
    unsigned long index;
#if defined(_M_X64) || defined(_M_ARM64)
    _BitScanReverse64(&index, n);
#else
    _BitScanReverse(&index, (unsigned long)n);
#endif
    return (int)index;

#else
    // Fallback portable version
    int res = 0;
    while (n >>= 1) res++;
    return res;
#endif
}

double calcular_entropia(uint8_t *datos, size_t tam) {
    // fórmula de la entropía de Shannon
    size_t conteo[256] = {0};
    for (size_t i = 0; i < tam; i++) conteo[datos[i]]++;
    double entropia = 0.0;
    for (int i = 0; i < 256; i++) {
        if (conteo[i] == 0) continue;
        double p = (double)conteo[i] / tam;
        entropia -= p * log2_int(p);
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
    uint8_t *tmp = calloc(size_of_data, sizeof(uint8_t));
    if (!tmp) { *data_output = NULL; *size_of_data_output = 0; return; }
    for (size_t i = 0; i < size_of_data; i++)
        tmp[i] = (uint8_t)(data[i] - pass[i % size_of_pass]);
    transposicion(tmp, size_of_data, pass, size_of_pass);

    // 2. Comprimir con LZ77
    LZ77Params params = default_params;
    DerivedParams derived = calculate_derived_params(&params);
    diccionario = (unsigned char *)calloc(derived.tam_diccionario + derived.max_coincidencia,  sizeof(unsigned char));
    hash = (unsigned int *)calloc(derived.tam_hash, sizeof(unsigned int));
    siguiente_enlace = (unsigned int *)calloc(derived.tam_diccionario, sizeof(unsigned int));
    if (!diccionario || !hash || !siguiente_enlace) {
        free(tmp); *data_output = NULL; *size_of_data_output = 0; return;
    }

    size_t max_comprimido = size_of_data * 2 + 32;
    uint8_t *comprimido = calloc(max_comprimido, sizeof(uint8_t));
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
    diccionario = (unsigned char *)calloc(derived.tam_diccionario + derived.max_coincidencia, sizeof(unsigned char));
    hash = (unsigned int *)calloc(derived.tam_hash * sizeof(unsigned int), sizeof(unsigned char));
    siguiente_enlace = (unsigned int *)calloc(derived.tam_diccionario * sizeof(unsigned int), sizeof(unsigned char));
    if (!diccionario || !hash || !siguiente_enlace) {
        *data_output = NULL; *size_of_data_output = 0; return;
    }

    // El tamaño real descomprimido no lo sabemos, pero puedes pedirlo como argumento o estimarlo
    size_t max_descomprimido = size_of_data * 4 + 32;
    uint8_t *descomprimido = calloc(max_descomprimido, sizeof(uint8_t));
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





#endif // SIMPLE_OFUSCATOR_C