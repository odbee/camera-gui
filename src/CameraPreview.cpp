#include "CameraPreview.hpp"

CameraPreview::CameraPreview(int width, int height):
width(width),
height(height),
VAO(0),
VBO(0),
EBO(0),
textureID(0),
shaderProgram("shaders/shader.vert", "shaders/shader.frag"),
sharedContext(),
eglBuffers(sharedContext)
{
    console = spdlog::stdout_color_mt("camera_preview");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // // TEXTURE ESCAPADES
    // glGenTextures(1, &textureID);
    // glBindTexture(GL_TEXTURE_2D, textureID);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // int texturewidth, textureheight, nrChannels;
    // unsigned char *data = stbi_load("assets/container.jpg", &texturewidth, &textureheight, &nrChannels, 0);
    // if (data)
    // {  
    //     glBindTexture(GL_TEXTURE_2D, textureID);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texturewidth, textureheight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }
    // else
    // {
    //     console->error("Failed to load texture");
    // }
    // shaderProgram.setInt("PreviewTexture", 0);
    // stbi_image_free(data);
    // glBindTexture(GL_TEXTURE_2D, 0);

    //FINISHED TEXTURE ESCAPADES



    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
}

CameraPreview::~CameraPreview()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void CameraPreview::render()
{   
    
    eglBuffers.simpleUpdate();
    // console->info("a");
    if(eglBuffers.new_frame()){
    console->info("b");
        glClear(GL_COLOR_BUFFER_BIT);
        shaderProgram.use();
        eglBuffers.updateTexture();
        // console->info("New frame, texture updated");
        shaderProgram.setInt("PreviewTexture", 0);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, eglBuffers.textureID);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, eglBuffers.textureID);


}
