#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLOBE_COLS ((size_t)43200)
#define GLOBE_ROWS ((size_t)21600)
#define GLOBE_CELLS ((size_t)GLOBE_COLS * GLOBE_ROWS)
#define MAX_CHUNK_SIZE ((size_t)10800 * 6000)
#define NUM_CHUNKS ((size_t)16)
#define NO_DATA -500

struct Chunk {
  char name[11];
  size_t num_cols;
  size_t num_rows;
  size_t num_pts;
};

int main() {
  struct Chunk chunks[NUM_CHUNKS] = {{"all10/a11g", 10800, 4800, 51840000},
                                     {"all10/b10g", 10800, 4800, 51840000},
                                     {"all10/c10g", 10800, 4800, 51840000},
                                     {"all10/d10g", 10800, 4800, 51840000},
                                     {"all10/e10g", 10800, 6000, 64800000},
                                     {"all10/f10g", 10800, 6000, 64800000},
                                     {"all10/g10g", 10800, 6000, 64800000},
                                     {"all10/h10g", 10800, 6000, 64800000},
                                     {"all10/i10g", 10800, 6000, 64800000},
                                     {"all10/j10g", 10800, 6000, 64800000},
                                     {"all10/k10g", 10800, 6000, 64800000},
                                     {"all10/l10g", 10800, 6000, 64800000},
                                     {"all10/m10g", 10800, 4800, 51840000},
                                     {"all10/n10g", 10800, 4800, 51840000},
                                     {"all10/o10g", 10800, 4800, 51840000},
                                     {"all10/p10g", 10800, 4800, 51840000}};

  // Alocate array for chunk. Reused and freed at the end.
  int16_t *chunk_data = malloc(MAX_CHUNK_SIZE * sizeof(int16_t));
  if (chunk_data == NULL) {
    perror("malloc");
    exit(1);
  }
  // Alocate array for global array.
  int16_t *globe_data = malloc(GLOBE_CELLS * sizeof(int16_t));
  if (globe_data == NULL) {
    perror("malloc");
    exit(1);
  }

  size_t offset = 0;
  for (size_t c = 0; c < NUM_CHUNKS; c++) {
    struct Chunk chunk = chunks[c];

    FILE *fp;
    size_t num_vals;

    // Open chunk file.
    if ((fp = fopen(chunk.name, "rb")) == NULL) {
      perror("fopen");
      exit(1);
    }
    if (ferror(fp)) {
      perror("fopen");
      fclose(fp);
      exit(1);
    }

    // Find end of chunk.
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

    // Go back to start of chunk.
    errno = 0;
    rewind(fp);
    if (errno != 0) {
      perror("rewind");
      fclose(fp);
      exit(1);
    }

    // Read file into array.
    num_vals =
        fread(chunk_data, sizeof(int16_t), file_length / sizeof(int16_t), fp);
    if (ferror(fp)) {
      perror("fread");
      fclose(fp);
      exit(1);
    }

    // Done, close file.
    fclose(fp);

    // Calculate chunk stats.
    int16_t min = INT16_MAX;
    int16_t max = INT16_MIN;
    float sum = 0.0;
    for (size_t i = 0; i < num_vals; i++) {
      if (chunk_data[i] != NO_DATA) {
        sum += chunk_data[i];
        if (chunk_data[i] < min)
          min = chunk_data[i];
        if (chunk_data[i] > max)
          max = chunk_data[i];
      }
    }
    float mean = sum / num_vals;

    // Log debug info.
    printf("name: %s, count: %zu, mean: %.2f, min: %hd, max: %hd\n", chunk.name,
           num_vals, mean, min, max);

    memcpy(globe_data + offset, chunk_data, num_vals * sizeof(int16_t));

    offset += chunk.num_pts;
  }

  // Calculate globe stats.
  int16_t min = INT16_MAX;
  int16_t max = INT16_MIN;
  float sum = 0.0;
  for (size_t i = 0; i < GLOBE_CELLS; i++) {
    if (globe_data[i] != NO_DATA) {
      sum += globe_data[i];
      if (globe_data[i] < min)
        min = globe_data[i];
      if (globe_data[i] > max)
        max = globe_data[i];
    }
  }
  float mean = sum / GLOBE_CELLS;

  // Log globe info.
  printf("name: GLOBE, count: %zu, mean: %.2f, min: %hd, max: %hd\n",
         GLOBE_CELLS, mean, min, max);

  // Free data.
  free(chunk_data);
  free(globe_data);

  return 0;
}