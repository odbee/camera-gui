#pragma once

#include <GLES3/gl3.h> // <-- Add this line
#include <GLES2/gl2ext.h>
#include <iostream>
#include <fstream>
#include <sstream>



class Shader 
{
    public:
        GLuint ID;
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();
        void use();
        void setBool(const std::string &name, bool value) const;  
        void setInt(const std::string &name, int value) const;   
        void setFloat(const std::string &name, float value) const;
    private:
        GLchar* checkStatus(GLuint objectID, PFNGLGETSHADERIVPROC objectPropertyGetterFunc, PFNGLGETSHADERINFOLOGPROC getInfoLogFunc, GLenum statusType);
        bool checkShaderStatus(GLuint shaderID,const char* type = nullptr) ;
        bool checkProgramStatus(GLuint programID, const char* type = nullptr);
        std::string readShaderCode(const char* filename);
};