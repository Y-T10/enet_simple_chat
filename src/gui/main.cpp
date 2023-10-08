#include "SDL2/SDL.h"
#include "SDL2/SDL_main.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

#include "title.hpp"
#include "SceneManager.hpp"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static int setup_program(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Hello SDL", 0, 0, 640, 480, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow() failed: %s", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RendererFlags::SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer() failed: %s", SDL_GetError());
        return -1;
    }

    return 0;
}

static void shutdown_program(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

using MainSceneMgr = SceneMgr<SceneTitle>;

struct SwitcherMainScene {
    void operator()(MainSceneMgr& mgr, SceneTitle& scene){
        mgr.setFinishFlag(true);
    };
};

/* this is always a main() function, even if you're on a platform that uses
   something else. SDL_main.h takes care of figuring out the details and
   making sure the program starts here. */
int main(int argc, char **argv) {
    if (setup_program(argc, argv) != 0) {
        shutdown_program();
        return -1;
    }

    MainSceneMgr mgr;
    mgr.setCurrentScene(SceneTitle());
    while(mgr.doesContinue()) {
        mgr.executeScene(window, renderer);
        mgr.switchScene<SwitcherMainScene>();
    }
    
    shutdown_program();
    return 0;
}