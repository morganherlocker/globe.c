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

struct Chunk {
  char name[11];
  size_t num_cols;
  size_t num_rows;
  size_t num_pts;
};

void elev_to_rgb(int16_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
  if (value <= 0) { // water
    *r = 250;
    *g = 250;
    *b = 121;
  } else if (value <= 5) { // beach
    *r = 230;
    *g = 230;
    *b = 200;
  } else if (value <= 18) { // sand
    *r = 237;
    *g = 222;
    *b = 122;
  } else if (value <= 600) { // lowland
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
}

void print_help() {
  printf("Usage:\n");
  printf("globe -o ./globe.bin merge;\n");
  printf("globe -i ./globe.bin -o globe.ppm render;\n");
}

int merge(char *out_file) {
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

  size_t offset = 0;
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
    if (ferror(fp)) {
      perror("fopen");
      fclose(fp);
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

    // Log debug info.
    printf("name: %s, count: %zu, mean: %.2f, min: %hd, max: %hd\n", chunk.name,
           num_vals, mean, min, max);

    memcpy(globe_data + offset, chunk_data, num_vals * sizeof(int16_t));

    offset += chunk.num_pts;
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
  if (ferror(fp)) {
    perror("fopen");
    fclose(fp);
    return 1;
  }

  // Read file into array.
  fread(globe_data, sizeof(int16_t), GLOBE_CELLS / sizeof(int16_t), fp);
  if (ferror(fp)) {
    perror("fread");
    fclose(fp);
    free(globe_data);
    return 1;
  }

  // Done, close file.
  fclose(fp);

  // Write .ppm image.
  // FILE *globe_img_file;
  printf("writing: %s\n", out_file);
  int width = 800;
  int height = 800;
  int channels = 3; // RGB

  // Allocate memory for the image data
  unsigned char *image = (unsigned char *)malloc(GLOBE_CELLS * channels);

  if (image == NULL) {
    fprintf(stderr, "Failed to allocate memory for image.\n");
    free(globe_data);
    return 1;
  }

  /*
  // stb_image_write demo. Creates a tiled gradient image.
  for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
          int index = (y * width + x) * channels;
          image[index + 0] = x;       // Red
          image[index + 1] = y;       // Green
          image[index + 2] = 128;     // Blue
      }
  }*/

  // Convert data to rgb.
  uint8_t r, g, b;
  for (size_t i = 0; i < (size_t)(width * height); i++) {
    if (globe_data[i] == NO_DATA) {
      r = 10;
      g = 20;
      b = 140;
    } else {
      elev_to_rgb(globe_data[i], &r, &g, &b);

      image[i] = r;
      image[i + 1] = g;
      image[i + 2] = b;
    }
  }

  // Write the image to a PNG file
  if (!stbi_write_png(out_file, width, height, channels, image,
                      width * channels)) {
    fprintf(stderr, "Failed to write image to file.\n");
    free(image);
    return 1;
  }

  // Free the allocated memory
  free(image);

  /*
  if ((globe_img_file = fopen(out_file, "wb")) == NULL) {
    perror("fopen");
    exit(1);
  }
  fprintf(globe_img_file, "P6\n%zu %zu\n255\n", GLOBE_COLS, GLOBE_ROWS);
  uint8_t r, g, b;
  for (size_t i = 0; i < num_vals; i++) {
    if (globe_data[i] == NO_DATA) {
      r = 10;
      g = 20;
      b = 140;
    } else {
      elev_to_rgb(globe_data[i], &r, &g, &b);
    }
    r = 250;
    g = 20;
    b = 250;
    fwrite(&r, 1, 1, globe_img_file);
    fwrite(&g, 1, 1, globe_img_file);
    fwrite(&b, 1, 1, globe_img_file);
  }
  fclose(globe_img_file);
  */

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