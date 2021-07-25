#include "include/glad/glad.h"
#include "include/GLFW/glfw3.h"

#include "include/glm/glm.hpp"
#include "include/glm/gtx/transform.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// tortuga engine
#include "json.hpp"
#include "grammar.hpp"
#include "vec3.hpp"
#include "turtle.hpp"
#include "sampler.hpp"

// opengl utility
#include "shader.hpp"

unsigned int width = 800;
unsigned int height = 600;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

float cameraHeight = 12.0;
float cameraDistance = 64.0;
float cameraTargetHeight = 0.0;
void processInput(GLFWwindow*);

void getGLError();

int main(int argc, char * argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "GetJob", nullptr, nullptr);
    if(window == nullptr) {
        std::cout << "(GLFW) Failed to create window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "(GLAD) Failed to initialize.\n";
        glfwTerminate();
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // set parsing filename
    json::setInputFilename("input.json");
    // L-system spec.
    //  set modules
    json::setModules(grammar::modules);
    //  get axiom
    grammar::word axiom = json::getAxiom();
    //  get iterations
    int iter = json::getIterations();
    //  get rewriting rules
    grammar::rewrites rules = json::getRules();

    std::cout << "tortuga will do " << iter << " applications\n";

    // get 'num'-th production
    std::cout << "axiom:    " << grammar::wordToString(axiom) << "\n";
    for(int i = 1; i <= iter; ++i) {
        grammar::apply(axiom,rules);
        std::cout << "(i = " << i << ") = " << grammar::wordToString(axiom) << "\n";
    }

    // turtle interpretation spec.
    std::vector<std::pair<vec3,vec3>> moves = turtle::interpret(axiom);
    float fD = 0; float shiftX = 0; float shiftY = 0;
    for(auto& p : moves) {
        //std::cout << p.first << " --> " << p.second << "\n";
        float dx = abs(p.second.x());
        float dy = abs(p.second.y());
        float dz = abs(p.second.z());
        fD = max(fD,dx);
        fD = max(fD,dy);
        fD = max(fD,dz);
    }
    float d = ceil(fD);
    std::cout << "scale = " << d << "\n";

    // first program - generate voxel data
    // stupid idea
    const int resolution = 64;
    const int gridRoot = ceil(sqrt((float)resolution));
    const int gridWidth = resolution;
    const int gridHeight = resolution;
    const int gridDepth = resolution;
    // FBO
    std::cout << "initializing off-screen render target ... \n";
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0,0,gridWidth,gridHeight);
    // color attachment texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gridWidth, gridHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    // bind texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    // check if fbo is complete
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
        std::cout << "Error: Framebuffer is not complete!\n";
    std::cout << "done!\n";
    std::cout << "creating voxel-slice sample domain...\n";
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // create sampling grid
    float grid[gridWidth*gridHeight*2];
    int c = 0;
    for(int i = 0; i < gridWidth; ++i) {
    for(int j = 0; j < gridHeight; ++j) {
        grid[c] = 2.0f*(float)i/(float)gridWidth - 1.0f + 1.0f/(float)gridWidth;
        grid[c+1] = 2.0f*(float)j/(float)gridHeight - 1.0f + 1.0f/(float)gridHeight;
        c+=2;
    }}
    std::cout << "done!\n";
    // create geometry textures
    std::cout << "creating turtle path texture...\n";
    GLuint texLines;
    glGenTextures(1, &texLines);
    glBindTexture(GL_TEXTURE_2D, texLines);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //unsigned int numLines = 1;
    //vec3 r0 = moves[0].first, rf = moves[0].second;

    const int tWidth = 64, tHeight = 64;
    GLubyte lineData[tWidth][tHeight][3];
    std::cout << "num lines = " << moves.size() << "\n";
    if(moves.size() > 64*64/2) {
        std::cout << "Too many lines, yo. Slow it down.\n";
        return -1;
    }
    for(int k = 0; k < moves.size(); ++k) {
        int i = k % 64;
        int j = k / 64;
        vec3 r0 = moves[k].first, rf = moves[k].second;
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

    // master voxelization texture for raymarching
    GLuint voxelTex;
    glGenTextures(1, &voxelTex);
    glBindTexture(GL_TEXTURE_2D, voxelTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // declare this memory on the GPU, don't fill it from here you sicko
    const int voxelTextureWidth = gridWidth*gridRoot; 
    const int voxelTextureHeight = gridHeight * gridRoot;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
        voxelTextureWidth, voxelTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    GLuint samplerVertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/v_vertex.glsl");
    if(samplerVertexShader == 0) return -1;
    GLuint samplerFragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/v_frag.glsl");
    if(samplerFragmentShader == 0) return -1;
    GLuint samplerProgram = glsl::linkShaders(samplerVertexShader, samplerFragmentShader);
    if(samplerProgram == 0) return -1;

    // uniform location
    GLuint levelLoc = glGetUniformLocation(samplerProgram, "level");
    GLuint texLoc = glGetUniformLocation(samplerProgram, "lineData");
    GLuint numLinesLoc = glGetUniformLocation(samplerProgram, "numLines");

    // load grid vertices
    GLuint samplerVBO;
    glGenBuffers(1, &samplerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, samplerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grid), &grid, GL_STATIC_DRAW);
    // set attrib location
    GLuint vPosLoc = glGetAttribLocation(samplerProgram, "vPos");
    glVertexAttribPointer(vPosLoc, 2, GL_FLOAT, GL_FALSE, 2.0*sizeof(float), (void*)0);
    glEnableVertexAttribArray(vPosLoc);
    glUseProgram(samplerProgram);

    // set texture
    glUniform1i(numLinesLoc, moves.size());
    glUniform1i(texLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texLines);

    // voxel rasterization program
    GLuint vertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/rt_vertex.glsl");
    GLuint fragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/rt_frag.glsl");
    GLuint program = glsl::linkShaders(vertexShader, fragmentShader);

    float vertices[] = {
        // T BL
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        // T TR
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    // vertex buffer GLuint vbo;
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // index buffer
    // attribute location
    GLuint vPositionLoc = glGetAttribLocation(program, "vPosition");
    glVertexAttribPointer(vPositionLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(vPositionLoc);
    glUseProgram(program);

    // get location of voxel sampler2D + resolution uniforms
    GLuint voxelTexLoc = glGetUniformLocation(program, "voxels");
    GLuint voxelResLoc = glGetUniformLocation(program, "voxelRes");
    GLuint resLoc = glGetUniformLocation(program, "resolution");
    // set resolution + bind voxel texture to sampler
    glUniform1i(voxelResLoc, resolution);
    glUniform1i(voxelTexLoc, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, voxelTex);

    // other uniforms
    GLuint tidenLoc = glGetUniformLocation(program, "tiden");

    // real-time voxelization / 3D sampling
    while(!glfwWindowShouldClose(window)) {
        glUseProgram(samplerProgram);
        glViewport(0,0,gridWidth,gridHeight);
        glBindBuffer(GL_ARRAY_BUFFER, samplerVBO);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // offscreen rendering (samplerProgram)
        for(int i = 0; i < gridDepth; ++i) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            float level = 2.0f*(float)i/(float)gridDepth - 1.0f + 1.0f/(float)gridDepth;
            glUniform1f(levelLoc, level);
            // rasterize using our texLines as the bound texture
            glBindTexture(GL_TEXTURE_2D, texLines);
            glDrawArrays(GL_POINTS, 0, gridWidth*gridDepth);
            // bind our voxel master structure and copy the data
            glBindTexture(GL_TEXTURE_2D, voxelTex);
            int xOffset = resolution*(i % gridRoot); 
            int yOffset = resolution*(i / gridRoot);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 
                xOffset, yOffset, 0, 0, gridWidth, gridHeight);
        }
        glBindFramebuffer(GL_FRAMEBUFFER,0);

        // rendering program (renderProgram)
        glViewport(0,0,800,600);
        glUseProgram(program);
        // bind VBO
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        float tiden = glfwGetTime();
        glUniform1f(tidenLoc, tiden);
        float screenRes[2] {(float)width,(float)height};
        glUniform2fv(resLoc, 1, screenRes);
        // clear drawing buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // cleanup
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glDeleteFramebuffers(1,&fbo);
    glDeleteProgram(samplerProgram);
    glDeleteProgram(program);
    glEnableVertexAttribArray(0);
    glDeleteBuffers(1,&vbo);
    glDeleteBuffers(1,&samplerVBO);
    glDeleteTextures(1,&tex);
    glDeleteTextures(1,&voxelTex);
    std::cout << "cleanup done!\n";

    glfwTerminate();
}


void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraHeight += 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraHeight -= 1;
    }
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        cameraDistance += 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        cameraDistance -= 1;
    }
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        cameraTargetHeight += 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        cameraTargetHeight -= 1;
    }
}

void getGLError() {
    GLenum err = glGetError();
    if(err == GL_INVALID_ENUM) std::cout << "INVALID ENUM\n";
    if(err == GL_INVALID_VALUE) std::cout << "INVALID VALUE\n";
    if(err == GL_INVALID_OPERATION) std::cout << "INVALID_OPERATION\n";
    if(err == GL_INVALID_FRAMEBUFFER_OPERATION) std::cout << "INVALID FRAMEBUFFER_OPERATION\n";
    if(err == GL_OUT_OF_MEMORY) std::cout << "OUT OF MEMORY\n";
}

void framebuffer_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, newWidth, newHeight);
    width = newWidth; height = newHeight;
}