#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Config global_config;

void config_set_defaults(Config *config) {
  config->background_color.r = 255;
  config->background_color.g = 0;
  config->background_color.b = 0;

  config->text_color.r = 255;
  config->text_color.g = 255;
  config->text_color.b = 255;

  fprintf(stderr, "********************************************    was ere");
  fprintf(stderr, "******************************************** %d", config->text_color.r);
  config->text_selected_color.r = 100;
  config->text_selected_color.g = 100;
  config->text_selected_color.b = 100;

  strcpy(config->font_file, "./font.ttf");
  config->logging = 1;

  strcpy(config->option_text[0], "Option 1 Text");
  strcpy(config->option_command[0], "command_for_option_1");
  strcpy(config->option_text[1], "Option 2 Text");
  strcpy(config->option_command[1], "command_for_option_2");
  strcpy(config->option_text[2], "Option 3 Text");
  strcpy(config->option_command[2], "command_for_option_3");
}

int config_load(const char *filename, Config *config) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    return -1; // Failed to open the file
  }

  char line[512];
  while (fgets(line, sizeof(line), file)) {
    // Ignore comments and empty lines
    if (line[0] == '#' || line[0] == '\n') {
      continue;
    }

    char key[256];
    char value[256];
    sscanf(line, "%[^=]=%s", key, value);

    if (strcmp(key, "background_color") == 0) {
      sscanf(value, "#%2hhx%2hhx%2hhx", &config->background_color.r,
             &config->background_color.g, &config->background_color.b);
    } else if (strcmp(key, "text_color") == 0) {
      sscanf(value, "#%2hhx%2hhx%2hhx", &config->text_color.r,
             &config->text_color.g, &config->text_color.b);
    } else if (strcmp(key, "text_selected_color") == 0) {
      sscanf(value, "#%2hhx%2hhx%2hhx", &config->text_selected_color.r,
             &config->text_selected_color.g, &config->text_selected_color.b);
    } else if (strcmp(key, "font_file") == 0) {
      strncpy(config->font_file, value, sizeof(config->font_file));
    } else if (strcmp(key, "logging") == 0) {
      config->logging = atoi(value);
    } else {
      for (int i = 1; i <= 3; i++) {
        char option_key[256];
        sprintf(option_key, "option%d_text", i);
        if (strcmp(key, option_key) == 0) {
          strncpy(config->option_text[i - 1], value,
                  sizeof(config->option_text[i - 1]));
        }

        sprintf(option_key, "option%d_command", i);
        if (strcmp(key, option_key) == 0) {
          strncpy(config->option_command[i - 1], value,
                  sizeof(config->option_command[i - 1]));
        }
      }
    }
  }

  fclose(file);
  return 0; // Successfully loaded the configuration
}
