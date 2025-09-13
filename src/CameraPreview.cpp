#include "CameraPreview.hpp"

CameraPreview::CameraPreview(int width, int height):
width(width),
height(height),
VAO(0),
VBO(0),
EBO(0),
textureID(0),
shaderProgram("shaders/gles2shader.vert", "shaders/gles2shader.frag"),
FBOshaderProgram("shaders/gles2shader.vert", "shaders/gles2shaderoes.frag"),
screenShaderProgram("shaders/gles2shader.vert", "shaders/gles2shaders2d.frag"),
sharedContext(),
eglBuffers()
{
    console = spdlog::stdout_color_mt("camera_preview");
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);
    // glBindVertexArray(VAO);
    
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);



    // glBindVertexArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
}

CameraPreview::~CameraPreview()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void CameraPreview::render()
{   

    
    eglBuffers.Show(sharedContext.get_shared_memory()->procid,
                    sharedContext.get_shared_memory()->fd,
                    sharedContext.get_shared_memory()->span_size,
                    sharedContext.get_shared_memory()->stream_info);


    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0);



}


