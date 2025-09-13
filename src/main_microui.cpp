#include "ShaderUtils.hpp"
#include "CameraBuffers.hpp"
#include <GLES3/gl3.h> // <-- Add this line
#include <GLES2/gl2ext.h>

#include "renderer.h"
#include "microui.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <array>

static float bg[3] = { 90, 95, 100 };


static void title_bar(mu_Context *ctx) {

     if (mu_begin_window(ctx, "Title Bar", mu_rect(100, 40, 300, 450))) {


    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      mu_Container *win = mu_get_current_container(ctx);
      char buf[64];
      mu_layout_row(ctx, 2, std::array<int, 2>{ 54, -1 }.data(), 0);
      mu_label(ctx,"Position:");
      sprintf(buf, "%d, %d", win->rect.x, win->rect.y); mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sprintf(buf, "%d, %d", win->rect.w, win->rect.h); mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 3, std::array<int, 3>{ 86, -110, -1 }.data(), 0);
      mu_label(ctx, "Test buttons 1:");
      if (mu_button(ctx, "Button 1")) {  }
      if (mu_button(ctx, "Button 2")) {  }
      mu_label(ctx, "Test buttons 2:");
      if (mu_button(ctx, "Button 3")) {  }
      if (mu_button(ctx, "Popup")) { mu_open_popup(ctx, "Test Popup"); }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, std::array<int, 2>{ 140, -1 }.data(), 0);
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          if (mu_button(ctx, "Button 1")) {  }
          if (mu_button(ctx, "Button 2")) {  }
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        mu_layout_row(ctx, 2, std::array<int, 2>{ 54, 54 }.data(), 0);
        if (mu_button(ctx, "Button 3")) {  }
        if (mu_button(ctx, "Button 4")) {  }
        if (mu_button(ctx, "Button 5")) {  }
        if (mu_button(ctx, "Button 6")) {  }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = { 1, 0, 1 };
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 1, std::array<int, 1>{ -1 }.data(), 0);

      mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
        "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
        "ipsum, eu varius magna felis a nulla.");
      mu_layout_end_column(ctx);
    }

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, std::array<int, 2>{-78, -1}.data(), 74);
      /* sliders */
      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 2, std::array<int, 2>{46, -1}.data(), 0);
      mu_label(ctx, "Red:");   mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:"); mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");  mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
      char buf[32];
      sprintf(buf, "#%02X%02X%02X", (int) bg[0], (int) bg[1], (int) bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

         mu_end_window(ctx);

     }
}




static void process_frame(mu_Context *ctx) {
  mu_begin(ctx);
  title_bar(ctx);

  mu_end(ctx);
}



constexpr std::array<char, 256> create_button_map() {
    std::array<char, 256> map{};
    map[SDL_BUTTON_LEFT & 0xff]   = MU_MOUSE_LEFT;
    map[SDL_BUTTON_RIGHT & 0xff]  = MU_MOUSE_RIGHT;
    map[SDL_BUTTON_MIDDLE & 0xff] = MU_MOUSE_MIDDLE;
    return map;
}
static constexpr auto button_map = create_button_map();

constexpr std::array<char, 256> create_key_map() {
    std::array<char, 256> map{};
    map[SDLK_LSHIFT & 0xff]    = MU_KEY_SHIFT;
    map[SDLK_RSHIFT & 0xff]    = MU_KEY_SHIFT;
    map[SDLK_LCTRL & 0xff]     = MU_KEY_CTRL;
    map[SDLK_RCTRL & 0xff]     = MU_KEY_CTRL;
    map[SDLK_LALT & 0xff]      = MU_KEY_ALT;
    map[SDLK_RALT & 0xff]      = MU_KEY_ALT;
    map[SDLK_RETURN & 0xff]    = MU_KEY_RETURN;
    map[SDLK_BACKSPACE & 0xff] = MU_KEY_BACKSPACE;
    return map;
}
static constexpr auto key_map = create_key_map();

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
}



int main (int argc, char *argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);
    r_init();

    // Test 2: Save/restore GL state around camera init
// Save current GL state
GLint current_program, current_vao, current_texture_unit;
glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
glGetIntegerv(GL_ACTIVE_TEXTURE, &current_texture_unit);

    CameraBuffers cameraBuffer;

// Restore GL state
glUseProgram(current_program);
glBindVertexArray(current_vao);
glActiveTexture(current_texture_unit);

      /* init microui */
    mu_Context *ctx =(mu_Context*) malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;


    bool quit = false;
    while (!quit) {
  /* main loop */

        /* handle SDL events */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: quit=true;
            case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
            case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
            case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
            int b = button_map[e.button.button & 0xff];
            if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
            if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
            break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
            int c = key_map[e.key.keysym.sym & 0xff];
            if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
            if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
            break;
            }
        }
        }

        /* process frame */
        // process_frame(ctx);

        /* render */
        // r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        cameraBuffer.Show();


        // mu_Command *cmd = NULL;
        // while (mu_next_command(ctx, &cmd)) {
        // switch (cmd->type) {
        //     case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
        //     case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
        //     case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
        //     case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
        // }
        // }
        r_present();    
    }
    return 0;
}