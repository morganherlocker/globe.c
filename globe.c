#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GLOBE_COLS ((size_t)43200)
#define GLOBE_ROWS ((size_t)21600)
#define GLOBE_CELLS ((size_t)GLOBE_COLS * GLOBE_ROWS)
#define MAX_CHUNK_SIZE ((size_t)10800 * 6000)
#define NUM_CHUNKS ((size_t)16)
#define NO_DATA -500
#define CELL_DEG 0.008333

struct Chunk {
  char name[11];
  size_t num_cols;
  size_t num_rows;
  size_t row_offset;
  size_t col_offset;
};

enum RGBMode { TERRAIN, GREYSCALE };

void elev_to_rgb(int16_t value, uint8_t *r, uint8_t *g, uint8_t *b,
                 enum RGBMode rmode) {
  uint8_t gray;
  switch (rmode) {
  case TERRAIN:
    if (value < 0) { // water
      *r = 10;
      *g = 20;
      *b = 140;
    } else if (value <= 5) { // beach
      *r = 130;
      *g = 98;
      *b = 95;
    } else if (value <= 50) { // sand
      *r = 107;
      *g = 128;
      *b = 75;
    } else if (value <= 250) { // lowland
      *r = 100;
      *g = 200;
      *b = 100;
    } else if (value <= 900) { // midland
      *r = 40;
      *g = 140;
      *b = 50;
    } else if (value <= 1000) { // highland
      *r = 40;
      *g = 120;
      *b = 60;
    } else if (value <= 1800) { // mountain
      *r = 145;
      *g = 145;
      *b = 145;
    } else { // snow
      *r = 255;
      *g = 255;
      *b = 255;
    }
    break;
  case GREYSCALE:
    gray = (uint8_t)((value + 500) * 255 / 9000);
    *r = gray;
    *g = gray;
    *b = gray;
    break;
  }
}

void print_help() {
  printf("Usage:\n");
  printf("globe -o ./globe.bin merge;\n");
  printf("globe -i ./globe.bin -o globe.csv table;\n");
  printf("globe -i ./globe.bin -o globe.png render;\n");
}

int merge(char *out_file) {
  struct Chunk chunks[NUM_CHUNKS] = {
      {"all10/a11g", 10800, 4800, 0, 0},
      {"all10/b10g", 10800, 4800, 0, 10800},
      {"all10/c10g", 10800, 4800, 0, 10800 * 2},
      {"all10/d10g", 10800, 4800, 0, 10800 * 3},
      {"all10/e10g", 10800, 6000, 4800, 0},
      {"all10/f10g", 10800, 6000, 4800, 10800},
      {"all10/g10g", 10800, 6000, 4800, 10800 * 2},
      {"all10/h10g", 10800, 6000, 4800, 10800 * 3},
      {"all10/i10g", 10800, 6000, 4800 + 6000, 0},
      {"all10/j10g", 10800, 6000, 4800 + 6000, 10800},
      {"all10/k10g", 10800, 6000, 4800 + 6000, 10800 * 2},
      {"all10/l10g", 10800, 6000, 4800 + 6000, 10800 * 3},
      {"all10/m10g", 10800, 4800, 4800 + 6000 + 6000, 0},
      {"all10/n10g", 10800, 4800, 4800 + 6000 + 6000, 10800},
      {"all10/o10g", 10800, 4800, 4800 + 6000 + 6000, 10800 * 2},
      {"all10/p10g", 10800, 4800, 4800 + 6000 + 6000, 10800 * 3}};

  // Alocate array for chunk. Reused and freed at the end.
  int16_t *chunk_data = malloc(MAX_CHUNK_SIZE * sizeof(int16_t));
  if (chunk_data == NULL) {
    perror("chunk malloc");
    return 1;
  }
  // Alocate array for global array.
  int16_t *globe_data = malloc(GLOBE_CELLS * sizeof(int16_t));
  if (globe_data == NULL) {
    perror("globe malloc");
    free(chunk_data);
    return 1;
  }

  for (size_t c = 0; c < NUM_CHUNKS; c++) {
    struct Chunk chunk = chunks[c];

    FILE *fp;
    size_t num_vals;

    // Open chunk file.
    if ((fp = fopen(chunk.name, "rb")) == NULL) {
      perror("fopen");
      free(chunk_data);
      free(globe_data);
      return 1;
    }

    // Find end of chunk.
    fseek(fp, 0, SEEK_END);
    if (ferror(fp)) {
      perror("fseek");
      fclose(fp);
      free(chunk_data);
      free(globe_data);
      return 1;
    }
    errno = 0;
    size_t file_length = ftell(fp);
    if (errno != 0) {
      perror("ftell");
      fclose(fp);
      free(chunk_data);
      free(globe_data);
      return 1;
    }

    // Go back to start of chunk.
    errno = 0;
    rewind(fp);
    if (errno != 0) {
      perror("rewind");
      fclose(fp);
      free(chunk_data);
      free(globe_data);
      return 1;
    }

    // Read file into array.
    num_vals =
        fread(chunk_data, sizeof(int16_t), file_length / sizeof(int16_t), fp);
    if (ferror(fp)) {
      perror("fread");
      fclose(fp);
      free(chunk_data);
      free(globe_data);
      return 1;
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

    // Copy chunk data row by row into global array.
    size_t row_offset = chunk.row_offset;
    // Log debug info.
    printf("name: %s, count: %zu, mean: %.2f, min: %hd, max: %hd\n", chunk.name,
           num_vals, mean, min, max);
    for (size_t row = 0; row < chunk.num_rows; row++) {
      // Calculate the starting position in globe_data for this row.
      size_t globe_row_start =
          (row_offset + row) * GLOBE_COLS + chunk.col_offset;
      // Calculate the starting position in chunk_data for this row.
      size_t chunk_row_start = row * chunk.num_cols;
      // Copy the row from chunk_data to globe_data.
      memcpy(globe_data + globe_row_start, chunk_data + chunk_row_start,
             chunk.num_cols * sizeof(int16_t));
    }
  }

  // Write globe bin data.
  FILE *globe_bin_file;
  // char globebin_name[] = "globe.16int_t.bin";
  if ((globe_bin_file = fopen(out_file, "wb")) == NULL) {
    perror("fopen");
    exit(1);
  }
  fwrite(globe_data, sizeof(int16_t), GLOBE_CELLS, globe_bin_file);
  fclose(globe_bin_file);

  // Free data.
  free(chunk_data);
  free(globe_data);

  return 0;
}

int table(char *in_file, char *out_file) {
  int16_t *globe_data = malloc(GLOBE_CELLS * sizeof(int16_t));
  if (globe_data == NULL) {
    perror("globe malloc");
    return 1;
  }

  FILE *fp;
  // Open file.
  if ((fp = fopen(in_file, "rb")) == NULL) {
    perror("fopen");
    free(globe_data);
    return 1;
  }

  // Read file into array.
  fread(globe_data, sizeof(int16_t), GLOBE_CELLS, fp);
  if (ferror(fp)) {
    perror("fread");
    fclose(fp);
    free(globe_data);
    return 1;
  }

  // Done, close file.
  fclose(fp);

  // Open csv.
  FILE *globe_csv_file;
  // Open file.
  if ((globe_csv_file = fopen(out_file, "ab")) == NULL) {
    perror("fopen");
    free(globe_data);
    return 1;
  }

  // Traverse cells.
  float lon = -180.0;
  float lat = 90.0;
  int16_t elevation = NO_DATA;
  size_t idx = 0;
  fprintf(globe_csv_file, "lon,lat,elev\n");
  for (size_t y = 0; y < GLOBE_ROWS; y++) {
    for (size_t x = 0; x < GLOBE_COLS; x++) {
      idx = y * GLOBE_COLS + x;
      elevation = globe_data[idx];

      if (elevation != NO_DATA && elevation != 0) {
        fprintf(globe_csv_file, "%f,%f,%d\n", lon, lat, elevation);
      }
      lon += 0.008333;
      idx++;
    }
    lon = -180.0;
    lat -= 0.008333;
  }

  return 0;
}

int render(char *in_file, char *out_file) {
  int16_t *globe_data = malloc(GLOBE_CELLS * sizeof(int16_t));
  if (globe_data == NULL) {
    perror("globe malloc");
    return 1;
  }

  FILE *fp;
  printf("reading: %s\n", in_file);
  // Open file.
  if ((fp = fopen(in_file, "rb")) == NULL) {
    perror("fopen");
    free(globe_data);
    return 1;
  }

  // Read file into array.
  fread(globe_data, sizeof(int16_t), GLOBE_CELLS, fp);
  if (ferror(fp)) {
    perror("fread");
    fclose(fp);
    free(globe_data);
    return 1;
  }

  // Done, close file.
  fclose(fp);

  // Write .png image.
  printf("writing: %s\n", out_file);
  size_t width = GLOBE_COLS / 2;
  size_t height = GLOBE_ROWS;
  int channels = 3; // RGB

  // Allocate memory for the image data
  uint8_t *image = malloc(width * height * channels);

  if (image == NULL) {
    fprintf(stderr, "Failed to allocate memory for image.\n");
    free(globe_data);
    return 1;
  }

  // Convert data to rgb.
  uint8_t r, g, b;
  float lon = -180.0;
  float lat = 90.0;
  size_t idx = 0;
  size_t image_idx = 0;
  for (size_t y = 0; y < GLOBE_ROWS; y++) {
    image_idx = y * width * channels;
    for (size_t x = 0; x < GLOBE_COLS; x++) {
      if (x < width && y < height) {
        idx = y * GLOBE_COLS + x;
        if (globe_data[idx] == NO_DATA) {
          r = 30;
          g = 40;
          b = 80;
        } else {
          elev_to_rgb(globe_data[idx], &r, &g, &b, TERRAIN);
        }
        image[image_idx + 0] = r;
        image[image_idx + 1] = g;
        image[image_idx + 2] = b;
        image_idx += 3;
      }
      lon += 0.008333;
    }
    lon = -180.0;
    lat -= 0.008333;
  }

  // Write the image to a PNG file.
  if (!stbi_write_png(out_file, width, height, channels, image,
                      width * channels * sizeof(uint8_t))) {
    fprintf(stderr, "Failed to write image to file.\n");
    free(image);
    return 1;
  }

  // Free allocated memory.
  free(image);
  free(globe_data);

  return 0;
}

int main(int argc, char **argv) {
  int opt;
  char *command = NULL;
  char *in = NULL;
  char *out = NULL;

  // Parse flags.
  while ((opt = getopt(argc, argv, "-hi:o:")) != -1) {
    switch (opt) {
    case 'h':
      print_help();
      return 0;
    case 'i':
      if (optarg && *optarg) {
        in = optarg;
      }
      break;
    case 'o':
      if (optarg && *optarg) {
        out = optarg;
      }
      break;
    case '?':
      fprintf(stderr, "Unknown option: %c\n", optopt);
      return 1;
    default:
      return 1;
    }
  }

  // Parse and execute command.
  if (optind >= argc) {
    fprintf(stderr, "Error: Command argument is required.\n");
    print_help();
    return 1;
  }
  command = argv[optind];
  if (strcmp(command, "merge") == 0) {
    int merge_result = merge(out);
    if (merge_result != 0)
      return merge_result;
  } else if (strcmp(command, "table") == 0) {
    if (in && out) {
      int table_result = table(in, out);
      if (table_result != 0)
        return table_result;
    } else {
      printf("globe table requires -i, -o flags.\n");
      return 1;
    }
  } else if (strcmp(command, "render") == 0) {
    if (in && out) {
      int render_result = render(in, out);
      if (render_result != 0)
        return render_result;
    } else {
      printf("globe render requires -i, -o flags.\n");
      return 1;
    }
  } else {
    printf("Unrecognized command. Exiting.\n");
    return 1;
  }

  return 0;
}