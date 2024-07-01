#ifndef CLI_OPTS_H
#define CLI_OPTS_H

#include <stdio.h>

typedef struct {
  int r, g, b;
} Color;

typedef struct {
  int screen_width;
  int screen_height;
  int bits_per_pixel;
  char title[255];
  char font_filepath[255];
  char background_image_filepath[255];
  int font_size;
  int top_padding;
  int bottom_padding;
  int left_padding;
  int right_padding;
  int text_selected_border_size;
  int start_at_nth;
  int logging_enabled;
  char log_filepath[255];
  int hide_file_extensions;
  int prefix_with_number;
  Color background_color;
  Color text_color;
  Color text_selected_color;
} Config;

void config_set_defaults(Config *config);
void print_usage();
void parse_command_line_options(int argc, char **argv, Config *config);

#endif
