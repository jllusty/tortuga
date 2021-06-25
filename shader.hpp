#include "include/glad/glad.h"
#include "include/GLFW/glfw3.h"

#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <vector>

namespace glsl {
    
    // read filename's GLSL, compile into a returned glShader
    GLuint compileShader(GLenum type, const std::string& filename)
    {
        // read file contents into string
        std::ifstream f(filename);
        //std::cout << "fetching data...\n";
        std::string source((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
        const char * data = source.c_str();
        //std::cout << "got data!\n";
        // create shader 
        GLuint shader = glCreateShader(type);
        // source shader GLSL source
        glShaderSource(shader, 1, &data, nullptr);
        // compile GLSL source
        glCompileShader(shader);
        // check if something went wrong
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE) {
            std::cout << "Error: Shader Compilation of '" << filename << "' failed:\n";
            // something went wrong, ask what happened
            GLint logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> infoLogData(logLength);
            std::cout << "Log length = " << logLength << "\n";
            glGetShaderInfoLog(shader, logLength, nullptr, infoLogData.data());
            std::string infoLog(infoLogData.begin(), infoLogData.end());
            std::cout << infoLog << "\n";
            return 0;
        }
        else return shader;
    }

    // link shaders, return glProgram
    GLuint linkShaders(GLuint vertexShader, GLuint fragmentShader) {
        // create glProgram
        GLuint program = glCreateProgram();
        // attach shaders
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        // link progra
        glLinkProgram(program);
        // check if something went wrong
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status == GL_FALSE) {
            // something went wrong, ask what happened
            GLint logLength;
            glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> infoLogData(logLength);
            glGetShaderInfoLog(program, logLength, nullptr, infoLogData.data());
            std::string infoLog(infoLogData.begin(), infoLogData.end());
            std::cout << "Error: Shader Linking Failed:\n";
            std::cout << infoLog << "\n";
            // maybe still detach & delete shaders before stopping?
            return 0;
        }
        // else
        // detach shaders
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        // then delete
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }
}