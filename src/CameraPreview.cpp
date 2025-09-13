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


    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Unbind FBO

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
    console->info("Rendering to FBO with size {}x{}", width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind to default framebuffer

	glClearColor(1, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    shaderProgram.use();
    eglBuffers.simpleUpdate();
    shaderProgram.setInt("PreviewTexture", 0);
    glUniform1i(glGetUniformLocation(shaderProgram.ID, "s"), 0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    eglBuffers.draw();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);



}

void CameraPreview::FBOrender(){
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glViewport(0, 0, width, height);
        console->info("Rendering to FBO with size {}x{}", width, height);
        glClearColor(1, 0, 0, 0);

        glClear(GL_COLOR_BUFFER_BIT);


 

        FBOshaderProgram.use();
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, eglBuffers.simpleBuffer.texture);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "screenTexture"), 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind FBO to render to default framebuffer


}

void CameraPreview::ScreenRender(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind to default framebuffer
    console->info("Rendering to screen with size {}x{}", width, height);

    glViewport(0, 0, width, height);
    glClearColor(0, 1, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    screenShaderProgram.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    screenShaderProgram.setInt("screenTexture", 0);
    glBindVertexArray(VAO);
}