#include "SDLHandler.hpp"
#include "ShaderUtils.hpp"
#include "CameraPreview.hpp"
#include <GLES3/gl3.h> // <-- Add this line
#include <GLES2/gl2ext.h>

int main (int argc, char *argv[]) {

    SDLHandler guiHandler("My SDL Window",0);
    CameraPreview cameraPreview(guiHandler.getSelectedDisplayWidth(), guiHandler.getSelectedDisplayHeight());
    
    bool quit = false;
    while (!quit) {

        quit=!guiHandler.processEvents();

        glViewport(0, 0, guiHandler.getSelectedDisplayWidth(), guiHandler.getSelectedDisplayHeight());
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);
        // cameraPreview.FBOrender();
        // cameraPreview.ScreenRender();
        cameraPreview.render();
        

        guiHandler.swapWindow();
    }
    return 0;
}