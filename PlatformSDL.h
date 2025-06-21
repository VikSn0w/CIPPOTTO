//
// Created by vitto on 05/06/2025.
//

#ifndef PLATFORMSDL_H
#define PLATFORMSDL_H

#include <cstdint>
#include <SDL3/SDL.h>

class PlatformSDL {
public:
    PlatformSDL(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
    ~PlatformSDL();

    void Update(void const* buffer, int pitch);
    bool ProcessInput(uint8_t* keys);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    int textureWidth;
    int textureHeight;

    void cleanup();
};


#endif //PLATFORMSDL_H
