#include "SDLHandler.hpp"
#include <iostream>



SDLHandler::SDLHandler(
    const std::string& title,int display)
    :
    m_window(nullptr),
    m_context(nullptr),
    selected_display(display)
{   
    // 1. Initialize SDL with the Video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("Failed to initialize SDL");
    }

    int n = SDL_GetNumVideoDrivers();
    printf("Available video drivers:\n");
    for (int i = 0; i < n; i++) {
        printf("  %s\n", SDL_GetVideoDriver(i));
    }


    const char* driver = SDL_GetCurrentVideoDriver();
    if (driver) {
        printf("SDL video driver in use: %s\n", driver);
    } else {
        printf("No SDL video driver initialized!\n");
    }


    // 2. Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int num_displays = SDL_GetNumVideoDisplays();
    std::cout << "Number of displays: " << num_displays << std::endl;
    if (num_displays > 0) {
         modes = (SDL_DisplayMode *)malloc(sizeof(SDL_DisplayMode) * num_displays);
             for (int i = 0; i < num_displays; ++i) {
        if (SDL_GetCurrentDisplayMode(i, &modes[i]) == 0) {
            std::cout << "Display " << i << ": " << modes[i].w << "x" << modes[i].h << " @ " << modes[i].refresh_rate << "Hz" << std::endl;
        } else {
            std::cerr << "Could not get display mode for display " << i << ": " << SDL_GetError() << std::endl;
        }
    }
    std::cout << "Using Resolution for Display " << selected_display << ": " << modes[selected_display].w << "x" << modes[selected_display].h << std::endl;
    } else {
        std::cerr << "No displays found!" << std::endl;
        SDL_Quit();
        throw std::runtime_error("No displays found");
    }
    


    // 3. Create a window (SDL will use the kmsdrm backend if it's the only option)
    m_window = SDL_CreateWindow("My SDL Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              modes[selected_display].w, modes[selected_display].h,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if (!m_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL window");
    }
    // 4. Create an OpenGL context and make it current
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) {
        std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error("Failed to create OpenGL context");
    }
    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::runtime_error("Failed to initialize GLAD");
    }
    SDL_GL_MakeCurrent(m_window, m_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

}


SDLHandler::~SDLHandler() {
    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool SDLHandler::processEvents()
{
        while (SDL_PollEvent(&m_event) != 0) {
            if (m_event.type == SDL_QUIT) {
                return false; // Signal to stop the loop
            }
             // You can add more event handling here for input, etc.
        }
        return true; // Continue the loop
}

void SDLHandler::swapWindow()
{
    SDL_GL_SwapWindow(m_window);

}
