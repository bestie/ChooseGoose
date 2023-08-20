#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

typedef struct {
  int r, g, b;
} Color;

typedef struct {
  char title[256];
  char font_filepath[256];
  char background_image_filepath[256];
  int font_size;
  int top_padding;
  int bottom_padding;
  int left_padding;
  int right_padding;
  int text_selected_border_size;
  int start_at_nth;
  int logging_enabled;
  int prefix_with_number;
  Color background_color;
  Color text_color;
  Color text_selected_color;
  Color text_selected_background_color;
  Color text_selected_border_color;
} Config;

void config_set_defaults(Config *config);
int parse_config_yaml_file(Config *config, const char *filename);
void print_config(FILE *output, const Config *config);

#endif
