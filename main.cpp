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

const unsigned int width = 800;
const unsigned int height = 600;

float cameraHeight = 6.0;
float cameraDistance = 6.0;
void processInput(GLFWwindow*);

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

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "(GLAD) Failed to initialize.\n";
        glfwTerminate();
        return -1;
    }
    glViewport(0,0,width,height);
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
    float fD = 0;
    vec3 fV = vec3(0,0,0);
    for(auto& p : moves) {
        //std::cout << p.first << " --> " << p.second << "\n";
        float d1 = sqrt(dot(p.first,p.first));
        if(d1>fD) { fV = p.first; fD = d1; }
        float d2 = sqrt(dot(p.second,p.second));
        if(d2>fD) { fV = p.second; fD = d2; }
    }
    int d = ceil(fD);
    std::cout << "scale = " << d << "\n";

    // line rasterization (3D)
    int voxelResX = 32;
    int voxelResY = 32;
    int voxelResZ = 32;
    sampler::setContext(voxelResX,voxelResY,voxelResZ,-d,d,-d,d,0,2*d);
    //for(auto& p : moves) {
    //    sampler::sampleLine(p.first,p.second);
    //}
    //std::vector<std::pair<vec3,vec3>> myLines;
    //myLines.push_back(std::make_pair(vec3(0,0,0),vec3(0,0,4)));
    sampler::sampleLines(moves);
    //sampler::sampleLines(myLines);

    // source, compile, & link shaders into program
    GLuint vertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/vertex.glsl");
    GLuint fragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/frag.glsl");
    //GLuint vertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/rt_vertex.glsl");
    //GLuint fragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/rt_frag.glsl");
    GLuint program = glsl::linkShaders(vertexShader, fragmentShader);

    // setup vertex data (simple cube)
    float cube_vertices[] = {
        // front
        -0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        // back
        -0.5, -0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5
    };
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
    unsigned short cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
    // vertex buffer 
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // index buffer
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
    // attribute location
    GLuint vPositionLoc = glGetAttribLocation(program, "vPosition");
    glVertexAttribPointer(vPositionLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(vPositionLoc);
    glUseProgram(program);

    // get uniform locations
    GLuint modelLoc = glGetUniformLocation(program, "model");
    GLuint viewLoc = glGetUniformLocation(program, "view");
    GLuint projectionLoc = glGetUniformLocation(program, "projection");

    // initialize our matrices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f/d));
        glm::vec3 viewPos = glm::vec3(6.0f,0.0f,0.0f);
        glm::vec3 target = glm::vec3(0,0,0);
        glm::vec3 up = glm::vec3(0,0,1);
    glm::mat4 view = glm::lookAt(viewPos, target, up);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // Main Loop
    glfwSetTime(0.0);
    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        // clear drawing buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // rotate camera
        double tiden = glfwGetTime();
        float vX = cameraDistance*cos(tiden);
        float vY = cameraDistance*sin(tiden);
        float vZ = cameraHeight;
        view = glm::lookAt(glm::vec3(vX,vY,vZ), target, up);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

        //// worse possible way - iterate over EVERY voxel, filled or not
        for(int i = 0; i < sampler::img.size(); ++i) {
        for(int j = 0; j < sampler::img[0].size(); ++j) {
        for(int k = 0; k < sampler::img[0][0].size(); ++k) {
            if(sampler::img[i][j][k] > 0.5f) {
                float x = (float)i-(float)sampler::width/2.0f;//sampler::width*(sampler::wXF-sampler::wX0)+sampler::wX0;
                float y = (float)j-(float)sampler::height/2.0f;///sampler::height*(sampler::wYF-sampler::wY0)+sampler::wY0;
                float z = (float)k;///sampler::depth*(sampler::wZF-sampler::wZ0)+sampler::wZ0;
                //std::cout << "(" << x << "," << y << "," << z << ")\n";
                glm::mat4 tModel = glm::translate(model, glm::vec3(x,y,z));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &tModel[0][0]);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
            }
        }
        }
        }
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraHeight += 0.1;
    }
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraHeight -= 0.1;
    }
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        cameraDistance += 0.1;
    }
    else if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        cameraDistance -= 0.1;
    }
}