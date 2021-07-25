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
// camera + input handling
#include "input.hpp"
#include "camera.hpp"

// opengl utility
#include "shader.hpp"
void getGLError();

// image utility
#include "image_write.hpp"

// windowing settings
unsigned int width = 800;
unsigned int height = 600;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// GLFW window
GLFWwindow* window = nullptr;
void saveImage(const char* filepath, GLFWwindow* w);


int main(int argc, char * argv[]) {
    // GLFW: create window and initialize OpenGL context
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    window = glfwCreateWindow(width, height, "GetJob", nullptr, nullptr);
    if(window == nullptr) {
        std::cout << "(GLFW) Failed to create window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, input::key_callback);
    // GLAD: Load OpenGL procedures
    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "(GLAD) Failed to initialize.\n";
        glfwTerminate();
        return -1;
    }

    // set input filename
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
        //std::cout << "(i = " << i << ") = " << grammar::wordToString(axiom) << "\n";
    }

    // turtle interpretation spec.
    std::vector<turtle::Line> moves = turtle::interpret(axiom);

    // 3D sampler (needs hella cleanup)
    sampler::setResolution(json::getResolution());
        std::cout << "sampler::resolution set to: " << sampler::resolution << "\n";
        std::cout << "sampler::gridRoot set to: " << sampler::gridRoot << "\n";
    sampler::init();
        getGLError();
    sampler::setTurtlePath(moves);
        //std::cout << "sampler::numLines = " << sampler::numLines << "\n";
        getGLError();
    sampler::initVoxels();
        getGLError();
    sampler::initProgram();
        getGLError();
    sampler::setProgramData();
        getGLError();
    sampler::run();
        getGLError();

    // voxel rasterization program
    // camera settings
    Camera camera(vec3(2.f,0.f,0.f),vec3(-1.f,0.f,0.f), vec3(0.,0.,1.));
    // raytracing GLSL
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
    // vertex buffer 
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // attribute location
    GLuint vPositionLoc = glGetAttribLocation(program, "vPosition");
    glVertexAttribPointer(vPositionLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(vPositionLoc);
    glUseProgram(program);

    // get location of voxel sampler2D + resolution uniforms
    GLuint voxelTexLoc = glGetUniformLocation(program, "voxels");
    GLuint voxelResLoc = glGetUniformLocation(program, "voxelRes");
    GLuint resLoc = glGetUniformLocation(program, "resolution");
    GLuint cameraPositionLoc = glGetUniformLocation(program, "cameraPosition");
    GLuint cameraForwardLoc = glGetUniformLocation(program, "cameraForward");
    GLuint cameraUpLoc = glGetUniformLocation(program, "cameraUp");
    GLuint cameraRightLoc = glGetUniformLocation(program, "cameraRight");
    
    // set resolution + bind voxel texture to sampler
    glUniform1i(voxelResLoc, sampler::resolution);
    glUniform1i(voxelTexLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sampler::voxelTex);

    // other uniforms
    GLuint tidenLoc = glGetUniformLocation(program, "tiden");

    // line rasterization (3D)
    glViewport(0,0,width,height);
    glEnable(GL_DEPTH_TEST);
    // Main Loop
    std::cout << "initialization done! rendering scene ...\n";
    glfwSetTime(0.0);
    float last = 0;
    int frame = 0;
    while(!glfwWindowShouldClose(window)) {
        // timing
        float tiden = glfwGetTime();
        float dt = tiden - last;
        last = tiden;
        // move / turn camera
        camera.update(dt);
        //std::cout << "cameraUp = " << camera.up << "\n";

        // get/set program GLSL
        glUniform1f(tidenLoc, tiden);
        float screenRes[2] {(float)width,(float)height};
        glUniform2fv(resLoc, 1, screenRes);
        glUniform3fv(cameraPositionLoc, 1, camera.position.getData());
        glUniform3fv(cameraForwardLoc, 1, camera.forward.getData());
        glUniform3fv(cameraUpLoc, 1, camera.up.getData());
        glUniform3fv(cameraRightLoc, 1, camera.right.getData());
        
        //processInput(window);
        // clear drawing buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw quad for raymarching
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
        // NOTE: this writes nothing, as no texture color attachment is bound
        //std::string filename = "output/" + std::to_string(frame) + ".png";
        //saveImage(filename.c_str(), window);
        frame++;
    }
    // cleanup
    glDeleteTextures(1, &sampler::voxelTex);
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
}

void getGLError() {
    GLenum err = glGetError();;
    if(err != GL_NO_ERROR) std::cout << "UNHANDLED ERROR!\n";
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
