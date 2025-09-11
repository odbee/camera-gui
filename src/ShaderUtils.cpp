#include "ShaderUtils.hpp"


GLchar* Shader::checkStatus(GLuint objectID, PFNGLGETSHADERIVPROC objectPropertyGetterFunc, PFNGLGETSHADERINFOLOGPROC getInfoLogFunc, GLenum statusType)
{
    GLint status;
    GLchar* infoLog= nullptr;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE)
    {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);

        infoLog = new GLchar[infoLogLength + 1];
        getInfoLogFunc(objectID, infoLogLength, NULL, infoLog);
    }
    return infoLog;
}

bool Shader::checkShaderStatus(GLuint shaderID,const char* type) 
{   
    GLchar* infoLog= checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
    if (infoLog != nullptr) {
        std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    delete[] infoLog;
    return true;
    
}
bool Shader::checkProgramStatus(GLuint programID, const char* type)
{
    GLchar* infoLog=  checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
    if (infoLog != nullptr) {
        if(type!=nullptr){
            std::cerr << "ERROR::SHADER::PROGRAM::" << type << "::LINKING_FAILED\n" << infoLog << std::endl;
        }else{
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        return false;
    }
    delete[] infoLog;
    return true;
}  

std::string Shader::readShaderCode(const char* filename) {
    std::ifstream file(filename);
    std::stringstream fileStream;
    if (!file.is_open()) {
        std::cerr << "Could not open the file - '" << filename << "'" << std::endl;
        return "";
    }
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return fileContent;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // Load and compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexCode = readShaderCode(vertexPath);
    std::string fragmentCode = readShaderCode(fragmentPath);

    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();

    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    checkShaderStatus(vertexShader, "VERTEX");

    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    checkShaderStatus(fragmentShader, "FRAGMENT");

    // Link shaders to create program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    checkProgramStatus(ID, "PROGRAM");

    // Clean up shaders (they are no longer needed once linked)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    glDeleteProgram(ID);

}

void Shader::use() 
{ 
    glUseProgram(ID); 
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    
}