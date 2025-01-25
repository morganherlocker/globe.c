#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h> 

#define MAX_SIZE 2000000000 // 2B (2GB)
#define NUM_CHUNKS 16

struct Chunk {
    char name[11];
    float min_lat;
    float max_lat;
    float min_lon;
    float max_lon;
    int16_t elevation_min;
    int16_t elevation_max;
    size_t num_cols;
    size_t num_rows;
};

int main() {
    struct Chunk chunks[NUM_CHUNKS] = {
        {"all10/a11g", 10.5f, 20.5f, 30.5f, 40.5f, 100, 200, 10, 20},
        {"all10/b10g", 15.5f, 25.5f, 35.5f, 45.5f, 150, 250, 15, 25},
        {"all10/c10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/d10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/e10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/f10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/g10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/h10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/i10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/j10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/k10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/l10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/m10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/n10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/o10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30},
        {"all10/p10g", 20.5f, 30.5f, 40.5f, 50.5f, 200, 300, 20, 30}
    };

    for (int c = 0; c < NUM_CHUNKS; c++) {
        struct Chunk chunk = chunks[c];
        printf("%s\n", chunk.name);
        char buffer[10] = "";

        FILE *fp;
        size_t num_vals;
        int16_t *data = malloc(MAX_SIZE * sizeof(int16_t));
        if (data == NULL) {
            perror("malloc");
            exit(1);
        }

        if ((fp = fopen(chunk.name, "rb")) == NULL) {
            perror("fopen");
            exit(1);
        }
        if (ferror(fp)) {
            perror("fopen");
            fclose(fp);
            exit(1);
        }

        fseek(fp, 0, SEEK_END);
        if (ferror(fp)) {
            perror("fseek");
            fclose(fp);
            exit(1);
        }

        errno = 0;
        size_t file_length = ftell(fp);
        if (errno != 0) {
            perror("ftell");
            fclose(fp);
            exit(1);
        }

        errno = 0;
        rewind(fp);
        if (errno != 0) {
            perror("rewind");
            fclose(fp);
            exit(1);
        }
        
        num_vals = fread(data, sizeof(int16_t), file_length / sizeof(int16_t), fp);
        if (ferror(fp)) {
            perror("fread");
            fclose(fp);
            exit(1);
        }

        printf("DEBUG: NUM ITEMS READ: %zu\n", num_vals);
        fclose(fp);

        int16_t min = INT16_MAX;
        int16_t max = INT16_MIN;
        float sum = 0.0;
        for (size_t i = 0; i < num_vals; i++) {
            if (data[i] != -500) {
                sum += data[i];
                if (data[i] < min) min = data[i];
                if (data[i] > max) max = data[i];
            }
        }
        printf("SUM: %.2f\n", sum);

        float mean = sum / num_vals;
        printf("MEAN: %.2f\n", mean);
        printf("MIN: %hd\n", min);
        printf("MAX: %.hd\n", max);

        free(data);
    }

    return 0;
}