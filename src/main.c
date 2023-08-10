#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MENU_ITEM_COUNT 3
#define MENU_ITEM_HEIGHT 50
#define MENU_ITEM_WIDTH 200

typedef enum {
    MENU_OPTION_1,
    MENU_OPTION_2,
    MENU_OPTION_3
} MenuItem;

SDL_Surface *screen;
SDL_Surface *menuItems[MENU_ITEM_COUNT];
MenuItem selectedOption = MENU_OPTION_1;
TTF_Font *font = NULL;

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    font = TTF_OpenFont("assets/font.ttf", 24); // Adjust the path and size as needed
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    SDL_WM_SetCaption("Simple Menu", NULL);
}

void loadMenuItems() {
    SDL_Color selectedColor = {255, 255, 255};
    SDL_Color unselectedColor = {100, 100, 100};
    SDL_Surface *tempSurface;

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        char text[50];
        snprintf(text, sizeof(text), "Option %d", i + 1);
        if (i == selectedOption) {
            tempSurface = TTF_RenderText_Solid(font, text, selectedColor);
        } else {
            tempSurface = TTF_RenderText_Solid(font, text, unselectedColor);
        }
        menuItems[i] = SDL_DisplayFormat(tempSurface);
        SDL_FreeSurface(tempSurface);
    }
}

void handleInput(SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (selectedOption > 0) {
                    selectedOption--;
                }
                break;
            case SDLK_DOWN:
                if (selectedOption < MENU_ITEM_COUNT - 1) {
                    selectedOption++;
                }
                break;
            case SDLK_RETURN:
                printf("Selected option: %d\n", selectedOption + 1);
                break;
            case SDLK_ESCAPE:
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }
    }
}

void render() {
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        SDL_Rect dest;
        dest.x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;
        dest.y = (SCREEN_HEIGHT - (MENU_ITEM_COUNT * MENU_ITEM_HEIGHT)) / 2 + (i * MENU_ITEM_HEIGHT);
        dest.w = MENU_ITEM_WIDTH;
        dest.h = MENU_ITEM_HEIGHT;

        SDL_BlitSurface(menuItems[i], NULL, screen, &dest);
    }

    SDL_Flip(screen);
}

void cleanup() {
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char **argv) {
    initSDL();

    SDL_Event event;

    while (1) {
        while (SDL_PollEvent(&event)) {
            handleInput(event);
        }

        loadMenuItems();
        render();
    }

    return 0;
}
