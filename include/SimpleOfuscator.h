#ifndef SIMPLE_OFUSCATOR_H
#define SIMPLE_OFUSCATOR_H

#include "LZ77.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

double calcular_entropia(uint8_t *datos, size_t tam);
void transposicion(uint8_t* data, size_t size, uint8_t* pass, size_t pass_size);
void destransposicion(uint8_t* data, size_t size, uint8_t* pass, size_t pass_size);
void ofusc(
    uint8_t *data, 
    size_t   size_of_data,
    uint8_t *pass,
    size_t   size_of_pass,
    uint8_t **data_output,   // ¡OJO! Ahora es doble puntero
    size_t  *size_of_data_output);
void desofuc(
    uint8_t *data, 
    size_t size_of_data,
    uint8_t *pass,
    size_t size_of_pass,
    uint8_t **data_output,   // ¡OJO! Ahora es doble puntero
    size_t* size_of_data_output);
    
#endif // SIMPLE_OFUSCATOR_H