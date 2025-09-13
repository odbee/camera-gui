#pragma once


#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <string>
#include "microui.h"



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
        void r_draw_rect(mu_Rect rect, mu_Color color);
        void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
        void r_draw_icon(int id, mu_Rect rect, mu_Color color);
        int r_get_text_width(const char *text, int len);
        int r_get_text_height(void);
        void r_set_clip_rect(mu_Rect rect);
        void r_clear(mu_Color color);
        void r_present(void);

    private:
        SDL_Window* m_window;
        SDL_GLContext m_context;
        SDL_Event m_event;
        SDL_DisplayMode *modes = NULL;
        int selected_display; // Change this to select a different display

};
