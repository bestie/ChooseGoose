#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <yaml.h>

void print_config(const Config *config) {
  printf("Configuration:\n");
  printf("---------------\n");
  printf("Title: %s\n", config->title);
  printf("Font Filepath: %s\n", config->font_filepath);
  printf("Background Image Filepath: %s\n", config->background_image_filepath);
  printf("Font Size: %d\n", config->font_size);
  printf("Top Padding: %d\n", config->top_padding);
  printf("Bottom Padding: %d\n", config->bottom_padding);
  printf("Left Padding: %d\n", config->left_padding);
  printf("Right Padding: %d\n", config->right_padding);
  printf("Start at Nth: %d\n", config->start_at_nth);
  printf("Logging Enabled: %s\n", config->logging_enabled ? "Yes" : "No");
  printf("Prefix with Number: %s\n", config->prefix_with_number ? "Yes" : "No");
  printf("Background Color: R:%d, G:%d, B:%d\n", config->background_color.r,
         config->background_color.g, config->background_color.b);
  printf("Text Color: R:%d, G:%d, B:%d\n", config->text_color.r,
         config->text_color.g, config->text_color.b);
  printf("Text Selected Color: R:%d, G:%d, B:%d\n",
         config->text_selected_color.r, config->text_selected_color.g,
         config->text_selected_color.b);
  printf("Text Selected Background Color: R:%d, G:%d, B:%d\n",
         config->text_selected_background_color.r,
         config->text_selected_background_color.g,
         config->text_selected_background_color.b);
  printf("Text Selected Border Color: R:%d, G:%d, B:%d\n",
         config->text_selected_border_color.r,
         config->text_selected_border_color.g,
         config->text_selected_border_color.b);
  printf("Text Selected Border Size: %d\n", config->text_selected_border_size);
}

int parse_boolean(const char *value) {
  if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0 ||
      strcasecmp(value, "on") == 0) {
    return 1;
  }
  return 0;
}

void config_set_defaults(Config *config) {
  config->background_color.r = 10;
  config->background_color.g = 255;
  config->background_color.b = 10;

  config->text_color.r = 200;
  config->text_color.g = 0;
  config->text_color.b = 200;

  config->text_selected_color.r = 100;
  config->text_selected_color.g = 100;
  config->text_selected_color.b = 100;

  strcpy(config->font_filepath, "assets/font.ttf");
  config->logging_enabled = 1;
}

Color parse_color_from_hex(const char *hex) {
  Color color;
  // Skip the '#' and then read two characters for each color component
  sscanf(hex + 1, "%2x%2x%2x", &color.r, &color.g, &color.b);
  return color;
}

int parse_config_yaml_file(Config *config, const char *filename) {
  FILE *fh = fopen(filename, "r");

  if (access(filename, R_OK) != 0) {
    return 0;
  }
  if (!fh) {
    return 0;
    exit(1);
  }

  int success = 1;
  yaml_parser_t parser;
  yaml_event_t event;

  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, fh);

  while (1) {
    fprintf(stdout, "Loop\n");

    if (!yaml_parser_parse(&parser, &event)) {
      fprintf(stdout, "Exiting event %d", event.type);
      return 0;
    }
    fprintf(stdout, "YAML event %d\n", event.type);

    if (event.type == YAML_STREAM_END_EVENT) {
      fprintf(stdout, "YAML stream end event %d", event.type);
      break;
    }

    if (event.type == YAML_SCALAR_EVENT) {
      char *key = (char *)event.data.scalar.value;

      if (strcmp(key, "title") == 0) {
        yaml_parser_parse(&parser, &event);
        printf("title=%s", event.data.scalar.value);
        strncpy(config->title, (char *)event.data.scalar.value,
                sizeof(config->title));
      } else if (strcmp(key, "font_filepath") == 0) {
        yaml_parser_parse(&parser, &event);
        strncpy(config->font_filepath, (char *)event.data.scalar.value,
                sizeof(config->font_filepath));
      } else if (strcmp(key, "background_image_filepath") == 0) {
        yaml_parser_parse(&parser, &event);
        strncpy(config->background_image_filepath,
                (char *)event.data.scalar.value,
                sizeof(config->background_image_filepath));
      } else if (strcmp(key, "font_size") == 0) {
        yaml_parser_parse(&parser, &event);
        config->font_size = atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "top_padding") == 0) {
        yaml_parser_parse(&parser, &event);
        config->top_padding = atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "bottom_padding") == 0) {
        yaml_parser_parse(&parser, &event);
        config->bottom_padding = atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "left_padding") == 0) {
        yaml_parser_parse(&parser, &event);
        config->left_padding = atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "right_padding") == 0) {
        yaml_parser_parse(&parser, &event);
        config->right_padding = atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "text_selected_border_size") == 0) {
        yaml_parser_parse(&parser, &event);
        config->text_selected_border_size =
            atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "start_at_nth") == 0) {
        yaml_parser_parse(&parser, &event);
        config->start_at_nth = atoi((char *)event.data.scalar.value);
      } else if (strcmp(key, "logging_enabled") == 0) {
        yaml_parser_parse(&parser, &event);
        config->logging_enabled =
            parse_boolean((char *)event.data.scalar.value);
      } else if (strcmp(key, "prefix_with_number") == 0) {
        yaml_parser_parse(&parser, &event);
        config->prefix_with_number =
            parse_boolean((char *)event.data.scalar.value);
      } else if (strcmp(key, "background_color") == 0) {
        yaml_parser_parse(&parser, &event);
        config->background_color =
            parse_color_from_hex((char *)event.data.scalar.value);
      } else if (strcmp(key, "background_color") == 0) {
        yaml_parser_parse(&parser, &event);
        config->background_color =
            parse_color_from_hex((char *)event.data.scalar.value);
      } else if (strcmp(key, "text_color") == 0) {
        yaml_parser_parse(&parser, &event);
        config->text_color =
            parse_color_from_hex((char *)event.data.scalar.value);
      } else if (strcmp(key, "text_selected_color") == 0) {
        yaml_parser_parse(&parser, &event);
        config->text_selected_color =
            parse_color_from_hex((char *)event.data.scalar.value);
      } else if (strcmp(key, "text_background_color") == 0) {
        yaml_parser_parse(&parser, &event);
        config->text_selected_background_color =
            parse_color_from_hex((char *)event.data.scalar.value);
      } else if (strcmp(key, "text_selected_border_color") == 0) {
        yaml_parser_parse(&parser, &event);
        config->text_selected_border_color =
            parse_color_from_hex((char *)event.data.scalar.value);
      }
    }
    fprintf(stdout, "bottom of  the loop\n");
  }
  fprintf(stdout, "out the loop\n");

  yaml_parser_delete(&parser);
  fclose(fh);

  return success;
}