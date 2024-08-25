#include "cli_opts.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Color parse_color_from_hex(const char *hex) {
  Color color;
  sscanf(hex, "%2x%2x%2x", &color.r, &color.g, &color.b);
  return color;
}

static bool parsebool(char *string) {
  if (strcmp(string, "true") == 0) {
    return true;
  }
  return false;
}

void config_set_defaults(Config *config) {
  config->screen_width = 640;
  config->screen_height = 480;
  config->bits_per_pixel = 32;
  strcpy(config->title, "");
  config->title_font_size = 21;
  config->font_size = 18;
  config->top_padding = 10;
  config->bottom_padding = 10;
  config->left_padding = 20;
  config->right_padding = 0;
  config->start_at_nth = 1;
  config->hide_file_extensions = false;
  config->prefix_with_number = false;
  config->background_color = parse_color_from_hex("00FFFF");
  config->text_color = parse_color_from_hex("000000");
  config->text_selected_color = parse_color_from_hex("FF0000");
  config->user_inactivity_timeout_ms = false;
}

void print_usage() {
  fprintf(stdout,
          "ChooseGoose is a silly interactive graphical menu program originally "
          "created for the Anbernic RG35XX handheld gaming system.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Given an input list of new-line separated items on stdin, "
                  "it will render a silly interactive menu and allow the user "
                  "to make a selection.\n");
  fprintf(stdout, "The selection is then written to standard out.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Keybindings:\n");
  fprintf(stdout, "\tUp/Down arrow keys, menu up/down\n");
  fprintf(stdout, "\tLeft/Right arrow keys: \t\t  page up/down\n");
  fprintf(stdout, "\tEnter: \t\t confirm selection\n");
  fprintf(stdout, "\tEsc: \t\t exit(0) with no selection\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Joypad button bindings:\n");
  fprintf(stdout, "\tUp/Down: \t\t menu up/down\n");
  fprintf(stdout, "\tLeft/Right \t\t page up/down\n");
  fprintf(stdout, "\tStart: \t\t confirm selection\n");
  fprintf(stdout, "\tMenu: \t\t exit(0) with no selection\n");
  fprintf(stdout, "\tA: \t\t confirm selection\n");
  fprintf(stdout, "\tB: \t\t exit(0) with no selection\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\tCapture selection in a bash variable\n");
  fprintf(stdout, "\tselection=$(cat my_list.txt | choosegoose)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\tEcho selection or as argument to another program\n");
  fprintf(stdout, "\tcat my_list.txt | choosegoose | xargs echo\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Author:\tStephen Best\n");
  fprintf(stdout, "Project:\thttps://github.com/bestie/ChooseGoose\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Usage: choosegoose [options]\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  --help \t\t display this message\n");
  fprintf(stdout, "  --screen-width N px \t\tdefault 640\n");
  fprintf(stdout, "  --screen-height N px\t\tdefault 480\n");
  fprintf(stdout, "  --bits-per-pixel N \t\tdefault 32\n");
  fprintf(stdout, "  --title TITLE \t\t title text appears with extra padding and can have a different font size");
  fprintf(stdout, "  --font FILEPATH \t\t path to a custom true type font file used for all text\n");
  fprintf(stdout, "  --background-image FILEPATH \t\t path to PNG image\n");
  fprintf(stdout, "  --font-size N px\n");
  fprintf(stdout, "  --title-font-size N px\n");
  fprintf(stdout, "  --top-padding N px\n");
  fprintf(stdout, "  --bottom-padding N px\n");
  fprintf(stdout, "  --left-padding N px\n");
  fprintf(stdout, "  --right-padding N px\n");
  fprintf(stdout, "  --start-at-nth N \t\t default 1. List item to start menu from, default 1 (first), set 0 for no initial selection\n");
  fprintf(stdout, "  --user-inactivity-timeout-ms N \t\t Quits after N milliseconds if the user has not made an input. Exits with status 124 like GNU timeout\n");
  fprintf(stdout, "  --hide-file-extensions true|false \t\t when using files as input and output, extensions can be hidden from the user\n");
  fprintf(stdout, "  --prefix-with-number true|false \t\t menu items are prepended with their list order number starting at 1\n");
  fprintf(stdout, "  --background-color RRGGBB \t\t background color, visible when no background image set and through transparent PNG regions.\n");
  fprintf(stdout, "  --text-color RRGGBB \n");
  fprintf(stdout, "  --text-selected-color RRGGBB \t\t a different color for the selected item text.\n");
  fprintf(stdout, "  --text-selected-background-color RRGGBB \t\t a solid background color for the selected item text.\n");
}

void parse_command_line_options(int argc, char **argv, Config *config) {
  static struct option long_options[] = {
      {"help", required_argument, 0, 0},
      {"screen-width", required_argument, 0, 0},
      {"screen-height", required_argument, 0, 0},
      {"bits-per-pixel", required_argument, 0, 0},
      {"title", required_argument, 0, 0},
      {"font", required_argument, 0, 0},
      {"background-image", required_argument, 0, 0},
      {"font-size", required_argument, 0, 0},
      {"top-padding", required_argument, 0, 0},
      {"bottom-padding", required_argument, 0, 0},
      {"left-padding", required_argument, 0, 0},
      {"right-padding", required_argument, 0, 0},
      {"text-selected-background-color", required_argument, 0, 0},
      {"start-at-nth", required_argument, 0, 0},
      {"hide-file-extensions", required_argument, 0, 0},
      {"prefix-with-number", required_argument, 0, 0},
      {"background-color", required_argument, 0, 0},
      {"text-color", required_argument, 0, 0},
      {"text-selected-color", required_argument, 0, 0},
      {"title-font-size", required_argument, 0, 0},
      {"log-file", required_argument, 0, 0},
      {"user-inactivity-timeout-ms", required_argument, 0, 0},
      {0, 0, 0, 0}};

  bool debug = false;
  if (getenv("DEBUG") && strcmp(getenv("DEBUG"), "1") == 0) {
    debug = true;
    fprintf(stderr, "ChooseGoose DEBUG enabled, DEBUG env var is set to `1`");
  }

  int option_index = 0;
  while (1) {
    int c = getopt_long(argc, argv, "", long_options, &option_index);
    if (c == -1)
      break;

    if (debug) {
      fprintf(stderr, "ChooseGoose DEBUG option_index: `%d` name: `%s` value: `%s`\n", option_index, long_options[option_index].name, optarg);
    }

    switch (option_index) {
    case 0:
      print_usage();
      exit(EXIT_FAILURE);
    case 1:
      config->screen_width = strtol(optarg, NULL, 10);
      break;
    case 2:
      config->screen_height = strtol(optarg, NULL, 10);
      break;
    case 3:
      config->bits_per_pixel = strtol(optarg, NULL, 10);
      break;
    case 4:
      strncpy(config->title, optarg, 255);
      break;
    case 5:
      strncpy(config->font_filepath, optarg, 255);
      break;
    case 6:
      strncpy(config->background_image_filepath, optarg, 255);
      break;
    case 7:
      config->font_size = strtol(optarg, NULL, 10);
      break;
    case 8:
      config->top_padding = strtol(optarg, NULL, 10);
      break;
    case 9:
      config->bottom_padding = strtol(optarg, NULL, 10);
      break;
    case 10:
      config->left_padding = strtol(optarg, NULL, 10);
      break;
    case 11:
      config->right_padding = strtol(optarg, NULL, 10);
      break;
    case 12:
      config->text_selected_background_color = parse_color_from_hex(optarg);
      break;
    case 13:
      config->start_at_nth = strtol(optarg, NULL, 10);
      break;
    case 14:
      config->hide_file_extensions = parsebool(optarg);
      break;
    case 15:
      config->prefix_with_number = parsebool(optarg);
      break;
    case 16:
      config->background_color = parse_color_from_hex(optarg);
      break;
    case 17:
      config->text_color = parse_color_from_hex(optarg);
      break;
    case 18:
      config->text_selected_color = parse_color_from_hex(optarg);
      break;
    case 19:
      config->title_font_size = strtol(optarg, NULL, 10);
      break;
    case 20:
      strncpy(config->log_filepath, optarg, 255);
      break;
    case 21:
      config->user_inactivity_timeout_ms = strtol(optarg, NULL, 10);
      break;
    default:
      print_usage();
      exit(EXIT_FAILURE);
    }
  }
}
