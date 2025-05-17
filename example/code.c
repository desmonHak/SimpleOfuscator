#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE_CHARS 32

void input(char* msj, char* line) {
    printf(msj);
    // Lee una línea de la entrada estándar
    if (fgets(line, MAX_SIZE_CHARS, stdin) == NULL) {
        // Si se recibe EOF (Ctrl+D) o error
        return;
    }
    // Si la línea no termina con un salto de línea, agregarlo
    line[strcspn(line, "\n")] = 0;
    
    // Buscar la posición del primer espacio, si existe
    char *posicion_espacio = strchr(line, ' ');
    if (posicion_espacio != NULL) {
        *posicion_espacio = '\0'; // Marcar el espacio como separador
    }
}
void transposicion(uint8_t* data, size_t size, uint8_t* pass, size_t pass_size) {
    for (size_t i = 0; i < size; i++) {
        size_t j = (i + pass[i % pass_size]) % size;
        // Intercambia data[i] y data[j]
        uint8_t tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

void destransposicion(uint8_t* data, size_t size, uint8_t* pass, size_t pass_size) {
    // Para deshacer, hay que hacerlo en orden inverso
    for (size_t i = size - 1; i >= 0; i--) {
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
    uint8_t *data_output, 
    size_t  *size_of_data_output) {

    for (
        (*size_of_data_output) = 0; 
        (*size_of_data_output) < size_of_data; 
        (*size_of_data_output)++
    ) {

        data_output[*size_of_data_output] = 
    (uint8_t)(data[*size_of_data_output] - pass[(*size_of_data_output) % size_of_pass]);

    }
    transposicion(data_output, *size_of_data_output, pass, size_of_pass);

}


void desofuc(
    uint8_t *data, 
    size_t size_of_data,
    uint8_t *pass,
    size_t size_of_pass,
    uint8_t *data_output, 
    size_t* size_of_data_output) {

        destransposicion(data, size_of_data, pass, size_of_pass);

        for (
            (*size_of_data_output) = 0; 
            (*size_of_data_output) < size_of_data; 
            (*size_of_data_output)++
        ) {
    
            data_output[*size_of_data_output] = 
    (uint8_t)(data[*size_of_data_output] + pass[(*size_of_data_output) % size_of_pass]);

    
        }
        
}

void dump_mem(uint8_t *mem, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", mem[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

char* read_file(char *filename, size_t *size_output) {
    FILE *file = fopen(filename, "rb");
    
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    buffer[file_size] = 0;

    *size_output = file_size;

    return buffer;
}

int main() {

    uint8_t data[] = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "   return 0;\n"
    "}"
    ;

    dump_mem(data, sizeof(data));

    uint8_t line[MAX_SIZE_CHARS] = {0};
    uint8_t line_out[MAX_SIZE_CHARS];

    while (1) {
        input(">>: (ofusc \"o\") or (desofuc \"d\")? ", line);

        printf("%s\n", line);

        size_t size_out = 0;
        uint8_t password[] = { 0x01, 0x0A, 0x0F, 0x1B };
        size_t password_len = sizeof(password);
        if (line[0] == 'o' ) {
            //input(">>:  ", line);
    
            ofusc(data, strlen(data), password, password_len, line_out, &size_out);

            printf("size %zu\n", size_out);
            dump_mem(line_out, size_out);
            printf("output: %s\n", line_out);

            FILE *f = fopen("output.bin", "wb");
            fwrite(line_out, size_out, 1, f);
            fclose(f);

            break;
        }
        else if (line[0] == 'd') {
            //input(">>:  ", line);
            memset(line, 0x00, MAX_SIZE_CHARS);
            line[0] = 0x01;

            size_t size_data = 0;
            char* data = read_file("output.bin", &size_data);

            desofuc(data, size_data, password, password_len, line_out, &size_out);

            printf("size %zu\n", size_out);
            dump_mem(line_out, size_out);
            printf("output: %s\n", line_out);
            break;
        }
        else puts("Opcion no valida");
    }



    return 0;
}