#pragma once
#include <GLES3/gl3.h> // <-- Add this line
#include <GLES2/gl2ext.h>
#include <ShaderUtils.hpp>
#include "EglBuffers.hpp"  
#include "GetStreamData.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>



class CameraPreview {
    public:
        CameraPreview(int width, int height);
        ~CameraPreview();
        void render();
        void FBOrender();
        void ScreenRender();
        void getTextureID(GLuint &id) { id = textureID; }
        int width, height;
        GLuint VAO, VBO, EBO;
        GLuint FBO;
    private:

        
        GLuint textureID;

        Shader shaderProgram;
        Shader FBOshaderProgram;
        Shader screenShaderProgram;
        std::shared_ptr<spdlog::logger> console;
        GetStreamData sharedContext;
        EglBuffers eglBuffers;

        GLfloat vertices[20] = {
            // positions          // texture coords
            0.8f,  0.6f, 0.0f,    1.0f, 1.0f,   // top right
            0.8f, -0.6f, 0.0f,    1.0f, 0.0f,   // bottom right
            -0.8f, -0.6f, 0.0f,   0.0f, 0.0f,   // bottom left
            -0.8f,  0.6f, 0.0f,   0.0f, 1.0f    // top left
        };
        
        unsigned int indices[6] = {  // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle   
        };
};
