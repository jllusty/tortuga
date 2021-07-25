#ifndef IMAGE_WRITE

#include "include/glad/glad.h"
#include "include/GLFW/glfw3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb-master/stb_image_write.h"

#include <cstdio>
#include <string>
#include <vector>

namespace png {
    void write(const std::string& filename, std::vector<GLubyte>& data, int width, int height) {
        GLsizei nrChannels = 3;
        GLsizei stride = nrChannels * width;
        stride += (stride % 4) ? (4 - stride % 4) : 0;
        GLsizei bufferSize = stride*height;
    }
}

namespace tga {
    void write(const std::string& filename, std::vector<GLubyte>& data, int width, int height) {
        FILE *fp;
        fp = fopen(filename.c_str(), "w");
        // make header
        putc(0,fp);
        putc(0,fp);
        putc(2,fp);
        putc(0,fp); putc(0,fp);
        putc(0,fp); putc(0,fp);
        putc(0,fp);
        putc(0,fp); putc(0,fp);
        putc(0,fp); putc(0,fp);
        putc((width & 0x00FF), fp);
        putc((width & 0xFF00) / 256, fp);
        putc((height & 0x00FF), fp);
        putc((height & 0xFF00) / 256, fp);
        putc(24,fp);
        putc(0,fp);
        // add data
        for(int i = 0; i < width*height*4; i+=4) {
            int r = (int)data[i+0];
            int g = (int)data[i+1];
            int b = (int)data[i+2];
            //int a = (int)data[i+3];
            putc(b,fp);
            putc(g,fp);
            putc(r,fp);
            //putc(a,fp);
        }
        /*
        for(int j = 0; j < height; ++j) {
        for(int i = 0; i < width; ++i) {
            int r = (int)data[j*width*4+i*4+0];
            int g = (int)data[j*width*4+i*4+1];
            int b = (int)data[j*width*4+i*4+2];
            int a = (int)data[j*width*4+i*4+3];
            putc(b,fp);
            putc(g,fp);
            putc(r,fp);
            putc(a,fp);
        }}*/
        fclose(fp);
    }
}
#define IMAGE_WRITE
#endif