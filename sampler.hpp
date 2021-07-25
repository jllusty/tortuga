#ifndef SAMPLER_HPP

#include "include/glad/glad.h"
#include "include/GLFW/glfw3.h"

#include <vector>
#include <memory>
#include <utility>
#include <iostream>

#include "shader.hpp"
#include "vec3.hpp"

#include "image_write.hpp"

// for debugging purposes (located in voxel.cpp)
extern GLFWwindow* window;
extern void getGLError();

namespace sampler {
    // voxel domain resolution
    int resolution = 64;
    int gridRoot = ceil(sqrt((float)resolution));
        //const int gridWidth = resolution;
        //const int gridHeight = resolution;
        //const int gridDepth = resolution
    // framebuffer
    GLuint fbo;
    //  color attachment
    GLuint tex;
    // vertex buffer (stores sampling grid)
    GLuint vbo;
    // geometry data textures
    const int tWidth = 64, tHeight = 64;
    GLubyte lineData[tWidth][tHeight][3]{};
    GLubyte lineAttr[tWidth][tHeight][3]{};
    int numLines;
    GLuint texLines;
    GLuint texLineAttrs;
    // sampling grid
    //std::vector<float> grid;//(resolution*resolution*2);
    float * grid{nullptr};
    // texture containing sampled voxel data
    GLuint voxelTex{};
    // GLSL program
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    GLuint levelLoc;
    GLuint texLoc;
    GLuint attrLoc;
    GLuint numLinesLoc;
    GLuint vPosLoc;

    // set sampling resolution
    void setResolution(int newResolution) {
        resolution = newResolution;
        gridRoot = ceil(sqrt((float)resolution));
    }
    void init() {
        // first program - generate voxel data via 3D sampling
        // FBO
        std::cout << "initializing off-screen render target ... \n";
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // color attachment texture
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution, resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        // bind texture to FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        // check if fbo is complete
        if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
            std::cout << "Error: Framebuffer is not complete!\n";
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::cout << "done!\n";
        std::cout << "creating voxel-slice sample domain...\n";
        //glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // create sampling grid
        grid = new float[resolution*resolution*3]; //std::vector<float>(resolution*resolution*3);
        int c = 0;
        for(int k = 0; k < resolution*resolution; k++) {
            int i = k % resolution;
            int j = k / resolution;
            grid[3*k] = 2.0f*(float)i/(float)resolution - 1.0f + 1.0f/(float)resolution;
            grid[3*k+1] = 2.0f*(float)j/(float)resolution - 1.0f + 1.0f/(float)resolution;
            grid[3*k+2] = 0.f;
            //std::cout << "grid[" << i << "," << j << "] = (" << grid[3*k] << "," << grid[3*k+1] << ")\n";
        }
        std::cout << "done!\n";
    }
    void setTurtlePath(std::vector<turtle::Line>& moves)
    {
        std::cout << "num lines = " << moves.size() << "\n";
        if(moves.size() > 64*64/2) {
            std::cout << "Too many lines, yo. Slow it down.\n";
            exit(-1);
        }

        numLines = moves.size();
        float fD = 0; float shiftX = 0; float shiftY = 0;
        for(auto& p : moves) {
            vec3 first = p.initial, second = p.terminal;
            float width = p.width;
            float dx = abs(second.x());
            float dy = abs(second.y());
            float dz = abs(second.z());
            fD = (fD < dx) ? dx : fD;
            fD = (fD < dy) ? dy : fD;
            fD = (fD < dz) ? dz : fD;
        }
        float d = ceil(fD);
        std::cout << "scale = " << d << "\n";

        // create geometry textures
        std::cout << "creating turtle path texture...\n";
        glGenTextures(1, &texLines);
        glBindTexture(GL_TEXTURE_2D, texLines);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        for(int k = 0; k < moves.size(); ++k) {
            vec3 first = moves[k].initial, second = moves[k].terminal;
            int i = k % 64;
            int j = k / 64;
            vec3 r0 = first, rf = second;
            r0 /= (d); rf /= (d);
            r0+=vec3(1.0f,1.0f,1.0f);
            rf+=vec3(1.0f,1.0f,1.0f);
            r0 *= 32.0f; rf *= 32.0f;
            GLubyte x0 = (GLubyte)ceil(r0.x());
            GLubyte y0 = (GLubyte)ceil(r0.y());
            GLubyte z0 = (GLubyte)ceil(r0.z());
            GLubyte xf = (GLubyte)ceil(rf.x());
            GLubyte yf = (GLubyte)ceil(rf.y());
            GLubyte zf = (GLubyte)ceil(rf.z());
            lineData[i][2*j][0] = x0;
            lineData[i][2*j][1] = y0;
            lineData[i][2*j][2] = z0;
            lineData[i][2*j+1][0] = xf;
            lineData[i][2*j+1][1] = yf;
            lineData[i][2*j+1][2] = zf;
        }
        std::cout << "done!\n";
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, &lineData);

        // texture for line attributes
        glGenTextures(1, &texLineAttrs);
        glBindTexture(GL_TEXTURE_2D, texLineAttrs);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        for(int k = 0; k < moves.size(); ++k) {
            //vec3 first = moves[k].initial, second = moves[k].terminal;
            float width = moves[k].width;
            int i = k % 64;
            int j = k / 64;
            //vec3 r0 = first, rf = second;
            //r0 /= (d); rf /= (d);
            //r0+=vec3(1.0f,1.0f,1.0f);
            //rf+=vec3(1.0f,1.0f,1.0f);
            //r0 *= 32.0f; rf *= 32.0f;
            //GLubyte x0 = (GLubyte)ceil(r0.x());
            //GLubyte y0 = (GLubyte)ceil(r0.y());
            //GLubyte z0 = (GLubyte)ceil(r0.z());
            //GLubyte xf = (GLubyte)ceil(rf.x());
            //GLubyte yf = (GLubyte)ceil(rf.y());
            //GLubyte zf = (GLubyte)ceil(rf.z());
            lineAttr[i][2*j][0] = (GLubyte)ceil(255*width);
            //lineData[i][2*j][0] = x0;
            //lineData[i][2*j][1] = y0;
            //lineData[i][2*j][2] = z0;
            //lineData[i][2*j+1][0] = xf;
            //lineData[i][2*j+1][1] = yf;
            //lineData[i][2*j+1][2] = zf;
        }
        std::cout << "done!\n";
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, &lineAttr);
    }
    void initVoxels() {
        glGenTextures(1, &voxelTex);
        glBindTexture(GL_TEXTURE_2D, voxelTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // declare this memory on the GPU, don't fill it from here you sicko
        const int voxelTextureWidth = resolution*gridRoot; 
        const int voxelTextureHeight = resolution*gridRoot;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
            voxelTextureWidth, voxelTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
    void initProgram() {
        vertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/v_vertex.glsl");
        if(vertexShader == -1) exit(-1);
        fragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/v_frag.glsl");
        if(fragmentShader == -1) exit(-1);
        program = glsl::linkShaders(vertexShader, fragmentShader);
        glUseProgram(program);

        // uniform locations
        levelLoc = glGetUniformLocation(program, "level");
        texLoc = glGetUniformLocation(program, "lineData");
        attrLoc = glGetUniformLocation(program, "lineAttr");
        numLinesLoc = glGetUniformLocation(program, "numLines");
    }
    void setProgramData() {
        // load vertices
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 3*resolution*resolution*sizeof(float), grid, GL_STATIC_DRAW);
        delete[] grid;
        // set attrib location
        vPosLoc = glGetAttribLocation(program, "vPos");
        glVertexAttribPointer(vPosLoc, 3, GL_FLOAT, false, 3*sizeof(float), nullptr);
        glEnableVertexAttribArray(vPosLoc);

        // set textures
        glUniform1i(numLinesLoc, numLines);
        //   endpoint data
        glUniform1i(texLoc, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texLines);
        //   attributes
        glUniform1i(attrLoc, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texLineAttrs);

        // for the sampler to have a texture to draw into
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, voxelTex);
    }
    void run() {
        // voxelization / 3D sampling
        std::cout << "starting voxelization...\n";

        glfwSetTime(0.0);
        std::vector<GLubyte> pixels(resolution*resolution*4,0);
        int rCount = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDisable(GL_DEPTH_TEST);
        glViewport(0,0,resolution,resolution);

        for(int i = 0; i < resolution; ++i) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            float level = 2.0f*(float)i/(float)resolution - 1.0f + 1.0f/(float)resolution;
            //std::cout << "level = " << level << "\n";
            glUniform1f(levelLoc, level);
            // rasterize using our texLines as the bound texture
            //glBindTexture(GL_TEXTURE_2D, texLines);
            glDrawArrays(GL_POINTS, 0, resolution*resolution);

            //glBindTexture(GL_TEXTURE_2D, tex);
            //GLsizei nrChannels = 4;
            //GLsizei stride = nrChannels * resolution;
            //stride += (stride % 4) ? (4 - stride % 4) : 0;
            //GLsizei bufferSize = stride*resolution;
            //std::vector<char> buffer(bufferSize);
            //std::cout << "bufferSize = " << bufferSize << "\n";
            //glReadPixels(0,0,resolution,resolution,GL_RGBA,GL_UNSIGNED_BYTE,buffer.data());
            //stbi_flip_vertically_on_write(true);
            //std::string filename = "output/" + std::to_string(rCount++) + ".png";
            //stbi_write_png(filename.c_str(), resolution, resolution, nrChannels, buffer.data(), stride);
            //glReadPixels(0, 0, resolution, resolution, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            //std::string filename = "output/" + std::to_string(i) + "_voxels.tga";
            //tga::write(filename, pixels, resolution, resolution);
            // bind our voxel master structure and copy the data
            //glBindTexture(GL_TEXTURE_2D, voxelTex);
            int xOffset = resolution*(i % gridRoot); 
            int yOffset = resolution*(i / gridRoot);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 
                xOffset, yOffset, 0, 0, resolution, resolution);
            //glBindTexture(GL_TEXTURE_2D, 0);
            
            //glBindFramebuffer(GL_FRAMEBUFFER, 0);
            //glfwSwapBuffers(window);
            //glReadPixels(0,0,gridWidth,gridHeight,GL_RGBA,GL_UNSIGNED_BYTE,&pixel[4*gridWidth*gridHeight*i]);
            //glfwSwapBuffers(window);
        }
        double elapsed = glfwGetTime();
        std::cout << "voxelization done! elapsed = " << elapsed << "\n";
        //glViewport(0,0,resolution*gridRoot,resolution*gridRoot);
        // cleanup
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glDeleteProgram(program);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glDeleteBuffers(1,&vbo);
        glDeleteFramebuffers(1,&fbo);
        glDeleteTextures(1,&tex);
        glBindTexture(GL_TEXTURE_2D, 0);
        std::cout << "cleanup done!\n";
    }
}

#define SAMPLER_HPP
#endif