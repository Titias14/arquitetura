#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/resource.h>
#define _GNU_SOURCE

// Define o tamanho da Lookup Table e o intervalo de valores
#define LOOKUP_TABLE_SIZE 10000
#define MAX_VALUE 10000.0f

// Tabela de consulta para a raiz quadrada inversa
float inv_sqrt_lookup_table[LOOKUP_TABLE_SIZE];

// Função para preencher a tabela de consulta
void initialize_inv_sqrt_lookup_table() {
    for (int i = 0; i < LOOKUP_TABLE_SIZE; i++) {
        float value = (float)i / LOOKUP_TABLE_SIZE * MAX_VALUE;
        inv_sqrt_lookup_table[i] = 1.0f / sqrt(value);
    }
}

// Função para buscar o valor na Lookup Table
float fast_inv_sqrt(float value) {
    if (value <= 0) return 0.0f;  // Evita divisão por zero
    int index = (int)(value / MAX_VALUE * (LOOKUP_TABLE_SIZE - 1));
    return inv_sqrt_lookup_table[index];
}

// Função otimizada para normalizar um vetor de características
void normalize_feature_vector(float* features, int length) {
    float sum = 0.0f;
    for (int i = 0; i < length; i++) {
        sum += features[i] * features[i];
    }
    
    float inv_sqrt = fast_inv_sqrt(sum);

    for (int i = 0; i < length; i++) {
        features[i] *= inv_sqrt;
    }
}

// Função para ler dados de um arquivo CSV
float** read_csv(const char* filename, int* num_elements, int* num_dimensions) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    *num_elements = 0;
    *num_dimensions = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (*num_elements == 0) {
            char* token = strtok(line, ",");
            while (token) {
                (*num_dimensions)++;
                token = strtok(NULL, ",");
            }
        }
        (*num_elements)++;
    }
    rewind(file);

    float** features = (float**)malloc(*num_elements * sizeof(float*));
    for (int i = 0; i < *num_elements; i++) {
        features[i] = (float*)malloc(*num_dimensions * sizeof(float));
    }

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        int j = 0;
        char* token = strtok(line, ",");
        while (token) {
            features[i][j++] = atof(token);
            token = strtok(NULL, ",");
        }
        i++;
    }

    fclose(file);
    return features;
}

// Função para medir o tempo de execução usando a biblioteca 'resources'
void get_resource_usage(struct rusage* usage) {
    getrusage(RUSAGE_SELF, usage);
}

void print_resource_usage(const char* label, struct rusage* usage) {
    printf("%s\n", label);
    printf("User time: %ld.%06ld seconds\n", usage->ru_utime.tv_sec, usage->ru_utime.tv_usec);
    printf("System time: %ld.%06ld seconds\n", usage->ru_stime.tv_sec, usage->ru_stime.tv_usec);
    printf("Maximum resident set size: %ld kilobytes\n", usage->ru_maxrss);
}

int main() {
    // Inicializa a tabela de consulta
    initialize_inv_sqrt_lookup_table();

    int num_elements, num_dimensions;
    float** features = read_csv("data.csv", &num_elements, &num_dimensions);

    struct rusage start_usage, end_usage;

    get_resource_usage(&start_usage);
    for (int i = 0; i < num_elements; i++) {
        normalize_feature_vector(features[i], num_dimensions);
    }
    get_resource_usage(&end_usage);

    printf("Normalized features:\n");
    for (int i = 0; i < num_elements; i++) {
        for (int j = 0; j < num_dimensions; j++) {
            printf("%f ", features[i][j]);
        }
        printf("\n");
    }

    printf("Execution time and resource usage:\n");
    print_resource_usage("Start Usage", &start_usage);
    print_resource_usage("End Usage", &end_usage);

    // Libera a memória alocada
    for (int i = 0; i < num_elements; i++) {
        free(features[i]);
    }
    free(features);

    return 0;
}
