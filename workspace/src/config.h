#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  int r, g, b;
} Color;

typedef struct {
  Color background_color;
  Color text_color;
  Color text_selected_color;
  char font_file[256];
  int logging;
  char option_text[3][256];
  char option_command[3][256];
} Config;

void config_set_defaults(Config *config);
int config_load(const char *filename, Config *config);

#endif
