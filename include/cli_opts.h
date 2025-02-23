#ifndef CLI_OPTS_H
#define CLI_OPTS_H

typedef struct {
  int r, g, b;
} Color;

typedef struct {
  unsigned int screen_width;
  unsigned int screen_height;
  unsigned int bits_per_pixel;
  char title[255];
  char font_filepath[255];
  char background_image_filepath[255];
  unsigned int font_size;
  unsigned int top_padding;
  unsigned int bottom_padding;
  unsigned int left_padding;
  unsigned int right_padding;
  unsigned int start_at_nth;
  char log_filepath[255];
  unsigned int hide_file_extensions;
  unsigned int prefix_with_number;
  Color background_color;
  Color text_color;
  Color text_selected_color;
  Color text_selected_background_color;
  unsigned int title_font_size;
  unsigned int user_inactivity_timeout_ms;
  unsigned int key_repeat_delay_ms;
  unsigned int key_repeat_interval_ms;
  unsigned int menu_item_padding;
  unsigned int menu_item_margin;
  char cover_images_dir[255];
} Config;

Config* default_config(void);
void print_usage(void);
void parse_command_line_options(int argc, char **argv, Config *config);

#endif
