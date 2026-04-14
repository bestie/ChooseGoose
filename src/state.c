#include "main.h"
#include <sys/select.h>

#define MAX_MENU_ITEMS 4096
#define MAX_LINE_LENGTH 255

State* init_state() {
    State* state = malloc(sizeof(State));

    state->title_height = 0;

    state->menu_height = 0;
    state->font_pixel_height = 0;
    state->selected_index = 0;
    state->button_repeat_active = 0;

    state->screen = NULL;
    state->background_image = NULL;
    state->title = NULL;
    state->title_font = NULL;
    state->font = NULL;
    state->joystick = NULL;
    state->menu_items = NULL;
    state->cover_images_enabled = false;

    return state;
}

void cleanup_state(State *state) {
    SDL_Interface* sdl = get_sdl_interface();
    if (state->log_file) {
        fclose(state->log_file);
    }
    if (state->background_image) {
        sdl->free_surface(state->background_image);
    }
    if (state->title) {
        sdl->free_surface(state->title);
    }
    if (state->screen) {
        sdl->free_surface(state->screen);
    }
    if (state->title_font) {
        sdl->ttf_close_font(state->title_font);
    }
    if (state->font) {
        sdl->ttf_close_font(state->font);
    }
    if (state->joystick) {
        sdl->joystick_close(state->joystick);
    }
    sdl->quit();
}

int check_readable(FILE *stream, int timeout_msec)
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(fileno(stream), &set);

    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_msec * 1000;

    int readable = select(fileno(stream) + 1, &set, NULL, NULL, &timeout);
    return readable;
}

BunchOfLines* read_lines_from_stdin(int max_lines, int max_line_length) {
    size_t total_memory =
        max_lines * sizeof(char *) + max_lines * max_line_length;
    char **lines = malloc(total_memory);
    BunchOfLines *bunch = malloc(sizeof(BunchOfLines));
    bunch->count = 0;
    bunch->max_length = max_line_length;
    bunch->lines = lines;

    char *line_memory = (char *)(lines + max_lines);
    for (int i = 0; i < max_lines; i++) {
        lines[i] = line_memory + i * max_line_length;
    }

    int lines_i = 0;
    int result = check_readable(stdin, 100);
    if (result < 1) {
        fprintf(stderr, "Input not readable\n");
        exit(1);
    }

    while (lines_i < max_lines && fgets(lines[lines_i], max_line_length, stdin)) {
        lines[lines_i][strcspn(lines[lines_i], "\n")] = '\0';
        if(strlen(lines[lines_i]) > 0) {
            lines_i++;
        }
    }

    bunch->count = lines_i;
    return bunch;
}

