/* set makeprg=cd\ build;make\ game_code */

#if __has_include("SDL2/SDL.h")
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef __linux__
const char* game_code_lib_path = "./libgame_code.so";
#elif defined(__APPLE__)
const char* game_code_lib_path = "./libgame_code.dylib";
#endif

#include "common.h"
#include "game.h"
#include "sgl.h"
#include "util.h"

game_main_f* reload_game_code(bool* success) {
    static void* libHandle = NULL;

    if (libHandle) {
        dlclose(libHandle);
        libHandle = NULL;
    }

    libHandle = dlopen(game_code_lib_path, RTLD_NOW);
    game_main_f* dy_game_main = NULL;

    if (libHandle) {
        dy_game_main = (game_main_f*)dlsym(libHandle, "game_main");
        char* result = dlerror();
        if (result) {
            printf("Cannot find game_main() in %s: %s\n", game_code_lib_path,
                result);
        }
    } else {
        printf("Cannot load %s: %s\n", game_code_lib_path, dlerror());
    }

    if (success) {
        *success = dy_game_main;
    }

    // if the shared lib is not used (or not available), try to use the one that
    // is statically linked
    if (!dy_game_main) {
        dy_game_main = game_main;
    }

    // char cwd[PATH_MAX];
    // if (getcwd(cwd, sizeof(cwd)) != NULL) {
    // 	printf("Current working dir: %s\n", cwd);
    // }

    return dy_game_main;
}

World world;

void get_user_input(void) {
    world.keyboard = SDL_GetKeyboardState(NULL);

    uint32_t buttons = SDL_GetMouseState(&world.m.x, &world.m.y);
    world.m.x /= WINDOW_WIDTH / (float)CANVAS_WIDTH;
    world.m.y /= WINDOW_HEIGHT / (float)CANVAS_HEIGHT;
    world.m.left = (buttons & SDL_BUTTON_LMASK) != 0;
    world.m.right = (buttons & SDL_BUTTON_RMASK) != 0;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("sasma 0.0", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED /*  | SDL_RENDERER_PRESENTVSYNC */);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32,
        SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    uint32_t* pixels =
        (uint32_t*)malloc(CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(pixels));
    memset(pixels, 0, CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint32_t));

    world.buffer = sglCreateBuffer(
        pixels, CANVAS_WIDTH, CANVAS_HEIGHT, SGL_PIXELFORMAT_ABGR32);
    sglEnableAlphaBlending(world.buffer);

    SDL_Texture* debug_texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32,
            SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_SetTextureBlendMode(debug_texture, SDL_BLENDMODE_BLEND);

    uint32_t* debug_pixels =
        (uint32_t*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(pixels));
    memset(debug_pixels, 0, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));

    world.debug_buffer = sglCreateBuffer(
        debug_pixels, WINDOW_WIDTH, WINDOW_HEIGHT, SGL_PIXELFORMAT_ABGR32);
    sglEnableAlphaBlending(world.buffer);

    SDL_Event event;
    bool alive = true;

    game_main_f* dy_game_main = reload_game_code(NULL);

    dy_game_main(&world, OP_LOAD);
    dy_game_main(&world, OP_INIT);

    time_t last_game_code_update = getFileTimestamp(game_code_lib_path);

    // https://gamedev.stackexchange.com/questions/110825/how-to-calculate-delta-time-with-sdl
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t last;
    float deltaTime = 0;
    float accumulator = 0;

    while (alive) {
        last = now;
        now = SDL_GetPerformanceCounter();
        deltaTime = (now - last) / (float)SDL_GetPerformanceFrequency();
        accumulator += deltaTime;

        world.time_since_start = now / (float)SDL_GetPerformanceFrequency();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                alive = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_b) {
                    world.debug_mode = !world.debug_mode;
                    printf("toggle debug mode %s\n" RESET,
                        world.debug_mode ? GRN "ON" : RED "OFF");
                }
                if (world.debug_mode && event.key.keysym.sym == SDLK_i) {
                    printf("reseting game ...\n");
                    dy_game_main(&world, OP_INIT);
                }
                if (event.key.keysym.sym == SDLK_h) {
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                        "sasma 0.0",
                        "Player 1 moves with W, S, A, D and shoots with F\n"
                        "Player 2 moves with arrow keys and shoots with .",
                        window);
                }
            }
        }

        while (accumulator >= WORLD_DT) {
            get_user_input();

            dy_game_main(&world, OP_STEP);

            accumulator -= WORLD_DT;
        }

        SDL_UpdateTexture(texture, NULL, world.buffer->pixels,
            CANVAS_WIDTH * sizeof(uint32_t));
        if (world.debug_mode) {
            SDL_UpdateTexture(debug_texture, NULL, world.debug_buffer->pixels,
                WINDOW_WIDTH * sizeof(uint32_t));
        }
        SDL_RenderClear(renderer);
        SDL_Rect srcRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_Rect dstRect = {0, 0, CANVAS_WIDTH, CANVAS_HEIGHT};
        SDL_RenderCopy(renderer, texture, &dstRect, &srcRect);
        if (world.debug_mode) {
            SDL_RenderCopy(renderer, debug_texture, NULL, NULL);
        }
        SDL_RenderPresent(renderer);

        // NOTE: This is just a workaround because of race conditions:
        // https://stackoverflow.com/questions/56334288/how-to-hot-reload-shared-library-on-linux
        static bool loaded = false;
        static int attempts = 0;
        time_t game_code_timestamp = getFileTimestamp(game_code_lib_path);
        if (game_code_timestamp > last_game_code_update) {
            printf(GRN "Reloading %s...\n" RESET, game_code_lib_path);
            SDL_Delay(50);
            dy_game_main = reload_game_code(&loaded);
            last_game_code_update = getFileTimestamp(game_code_lib_path);
            if (!loaded && attempts++ <= 10) {
                last_game_code_update = game_code_timestamp;
                attempts = 0;
            } else if (loaded) {
                dy_game_main(&world, OP_INJECT);
            }
        }
    }

    sglFreeBuffer(world.buffer);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

// nnoremap <leader>b <cmd>wa<cr><cmd>!pushd build/ && ./build.sh; popd<cr>
// nnoremap <leader>b <cmd>wa<cr><cmd>!pushd build/ && make demo; popd<cr>
// nnoremap <leader>b <cmd>wa<cr><cmd>make -C build<CR>
