#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libpng/png.h>

typedef uint32_t u32;
typedef uint8_t u8;

u8 *load_png_rgb(u32 size_x, u32 size_y, const char *filename) {
  u8 *buf;
  png_image p;
  p.version = PNG_IMAGE_VERSION;
  p.opaque = NULL;

  png_image_begin_read_from_file(&p, filename);
  if (PNG_IMAGE_FAILED(p) || p.format != PNG_FORMAT_RGBA) {
    fprintf(stderr, "Could not load tileset from file: %s\n", filename);
    abort();
  }

  if (p.width != size_x || p.height != size_y) {
    fprintf(
      stderr, "%s dimensions %ux%u do not match expected (%ux%u).\n",
      filename,
      (unsigned)p.width, (unsigned)p.height, (unsigned)size_x, (unsigned)size_y
    );
    abort();
  }

  buf = malloc(size_x * size_y * 4);

  if (!buf) {
    fprintf(stderr, "Could not allcoate memory for image buffer.\n");
    abort();
  }

  png_image_finish_read(&p, 0, (void*)buf, 0, 0);

  if (PNG_IMAGE_FAILED(p)) {
    fprintf(stderr, "Load of \"%s\" failed: %s\n", filename, p.message);
    abort();
  }

  return buf;
}

int is_green(u8 *buf, int x, int y) {
  unsigned idx = (y*128 + x)*4;
  return (buf[idx] == 0) && (buf[idx + 1] == 255) && (buf[idx + 2] == 0);
}

int is_black(u8 *buf, int x, int y) {
  unsigned idx = (y*128 + x)*4;
  return (buf[idx] == 0) && (buf[idx + 1] == 0) && (buf[idx + 2] == 0);
}

void load_png(u8 *data, u32 size_x, u32 size_y, const char *filename)
{
  u8 *buf = load_png_rgb(size_x, size_y, filename);

  u32 i, j;

  for (i = 0; i < size_x*size_y >> 3; ++i)
    data[i] = 0;
  
  for (i = 0; i < size_y; i++) {
    for (j = 0; j < size_x; j++) {
      u8 pixel = buf[(i*size_x + j) * 4] > 0x80;
      if (pixel) data[((i&0xfff8) << 4) | j] |= 1<<(i & 0x07);
    }
  }

  free(buf);
}

void sprite_set(u8 *p, int i, int j) {
  int idx = 0;
  if (j >= 8) {
    idx += 16;
    j -= 8;
  }

  idx += i;

  p[idx] |= (1<<j);
}

void load_png_sprite(u8 *data, u8 *mask, const char *filename) {
  u8 *buf = load_png_rgb(16, 16, filename);
  int i, j;

  for (i = 0; i < 32; i++) {
    data[i] = 0;
    mask[i] = 0;
  }

  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j) {
      if (buf[(i*16 + j)*4 + 3] == 0xff)
        sprite_set(mask, i, j);
      if (buf[(i*16 + j)*4 + 2] == 0xff)
        sprite_set(data, i, j);
    }
  }
}

#define PER_LINE 6
#define PER_LINE_U8 12

void write_hex(FILE *out, u32 *p, u32 size, int cont) {
  for (unsigned i = 0; i < size; i++) {
    if ((i % PER_LINE) == 0) {
      fputc(' ', out);
    }

    fprintf(out, " 0x%08x", p[i]);

    if (i == size - 1) {
      if (cont) fputc(',', out);
      fputc('\n', out);
    } else {
      fputc(',', out);
      if ((i % PER_LINE) == (PER_LINE - 1))
        fputc('\n', out);
    }
  }
}

void write_hex_u8(FILE *out, u8 *p, u32 size) {
  for (unsigned i = 0; i < size; i++) {
    if ((i % PER_LINE_U8) == 0) {
      fputc(' ', out);
    }

    fprintf(out, " 0x%02x", p[i]);

    if (i == size - 1) {
      fputc('\n', out);
    } else {
      fputc(',', out);
      if ((i % PER_LINE_U8) == (PER_LINE_U8 - 1))
        fputc('\n', out);
    }
  }
}

void write_triple_array(FILE *out, const char *name,
                        u32 *p1, u32 *p2, u32 *p3, u32 size)
{
  fprintf(out, "unsigned %s[] = {\n", name);
  write_hex(out, p1, size, 1);
  write_hex(out, p2, size, 1);
  write_hex(out, p3, size, 0);
  fprintf(out, "};\n\n");
}

void write_array(FILE *out, const char *name, const char *sub_name,
                 u32 *p, u32 size)
{
  fprintf(out, "unsigned %s_%s[] = {\n", name, sub_name);
  write_hex(out, p, size, 0);
  fprintf(out, "};\n\n");
}

void write_array_u8(FILE *out, const char *name, u8 *p, u32 size) {
  fprintf(out, "unsigned char %s[] = {\n", name);
  write_hex_u8(out, p, size);
  fprintf(out, "};\n\n");
}

void proc_img_file(FILE *out, const char *filename, const char *name,
                   u32 size_y)
{
  const u32 size_x = 128;

  u8 *data;
  u32 size = ((size_x * size_y) >> 3);

  data = malloc(size);

  load_png(data, size_x, size_y, filename);
  write_array_u8(out, name, data, size);

  free(data);
}

void proc_sprite_file(FILE *out, const char *filename, const char *name)
{
  const u32 size = 32;
  u8 *data, *mask;
  char *data_name, *mask_name;

  data_name = malloc(strlen(name) + 10);
  strcpy(data_name, name);
  strcat(data_name, "_data");

  mask_name = malloc(strlen(name) + 10);
  strcpy(mask_name, name);
  strcat(mask_name, "_mask");

  data = malloc(size);
  mask = malloc(size);

  load_png_sprite(data, mask, filename);
  fprintf(out, "__attribute__ ((section(\".text\"), used)) ");
  write_array_u8(out, data_name, data, size);
  fprintf(out, "__attribute__ ((section(\".text\"), used)) ");
  write_array_u8(out, mask_name, mask, size);

  free(data);
  free(mask);
  free(data_name);
  free(mask_name);
}

int main() {
  FILE *out = fopen("imgdata.c", "w");

  fprintf(out, "__attribute__ ((section(\".text\"), used)) ");
  proc_img_file(out, "imgdata.png", "imgdata", 64 * 31);

  fclose(out);

  FILE *out_sprite = fopen("spritedata.h", "w");
  proc_sprite_file(out, "sprite.png", "sprite");

  fclose(out);
  
  return 0;
}
