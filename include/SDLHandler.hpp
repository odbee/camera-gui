#pragma once


#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <string>



class SDLHandler {
    public:
        SDLHandler(const std::string& title, int display=0);
        ~SDLHandler();
        bool processEvents(); // Processes events and returns true if stream should continue
        SDL_Event getEvent() const { return m_event; }
        void swapWindow();
        SDL_Window* getWindow() const { return m_window; }
        SDL_GLContext getContext() const { return m_context; }
        int getSelectedDisplayWidth() const { return modes[selected_display].w; }
        int getSelectedDisplayHeight() const { return modes[selected_display].h; }

    private:
        SDL_Window* m_window;
        SDL_GLContext m_context;
        SDL_Event m_event;
        SDL_DisplayMode *modes = NULL;
        int selected_display; // Change this to select a different display

};
