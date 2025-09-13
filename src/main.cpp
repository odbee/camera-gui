#include "SDLHandler.hpp"
#include "ShaderUtils.hpp"
#include "CameraBuffers.hpp"
#include <GLES3/gl3.h> // <-- Add this line
#include <GLES2/gl2ext.h>

#include "renderer.h"
#include "microui.h"



int main (int argc, char *argv[]) {

    SDLHandler guiHandler("My SDL Window",0);
    CameraBuffers cameraBuffer;

    bool quit = false;
    while (!quit) {

        quit=!guiHandler.processEvents();

        glViewport(0, 0, guiHandler.getSelectedDisplayWidth(), guiHandler.getSelectedDisplayHeight());
        glViewport(20, 20, guiHandler.getSelectedDisplayWidth()-80, guiHandler.getSelectedDisplayHeight()-80);
        cameraBuffer.Show();
        

        guiHandler.swapWindow();
    }
    return 0;
}