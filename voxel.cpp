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

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "(GLAD) Failed to initialize.\n";
        glfwTerminate();
        return -1;
    }
    //glEnable(GL_DEPTH_TEST);

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
    const int resolution = 256;
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
        lineData[i][2*j+1][0] = xf;//xf;
        lineData[i][2*j+1][1] = yf;//yf;
        lineData[i][2*j+1][2] = zf;//zf;
        //std::cout << "line " << i << " is " << 
        //    r0/*-vec3(32.,32.,0.)*/ << " --> " << 
        //    rf/*-vec3(32.,32.,0.)*/ << "\n";
    }
    std::cout << "done!\n";

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, &lineData);

    std::cout << "compiling sampling shaders...\n";
    GLuint vertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/v_vertex.glsl");
    getGLError();
    if(vertexShader == 0) return -1;
    GLuint fragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/v_frag.glsl");
    getGLError();
    if(fragmentShader == 0) return -1;
    GLuint program = glsl::linkShaders(vertexShader, fragmentShader);
    getGLError();
    if(program == 0) return -1;
    std::cout << "done!\n";

    // uniform location
    GLuint levelLoc = glGetUniformLocation(program, "level");
    GLuint texLoc = glGetUniformLocation(program, "lineData");
    GLuint numLinesLoc = glGetUniformLocation(program, "numLines");

    // load vertices
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grid), &grid, GL_STATIC_DRAW);
    // set attrib location
    GLuint vPosLoc = glGetAttribLocation(program, "vPos");
    glVertexAttribPointer(vPosLoc, 2, GL_FLOAT, GL_FALSE, 2.0*sizeof(float), (void*)0);
    glEnableVertexAttribArray(vPosLoc);
    glUseProgram(program);

    // set texture
    glUniform1i(numLinesLoc, moves.size());
    glUniform1i(texLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texLines);

    // get max size of array
    std::cout << "max size = " << sizeof(std::size_t) << "\n";

    GLubyte *pixel = new GLubyte[gridWidth*gridHeight*gridDepth*4];
    std::cout << "starting voxelization...\n";
    glfwSetTime(0.0);
    for(int i = 0; i < gridDepth; ++i) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        float level = 2.0f*(float)i/(float)gridDepth - 1.0f + 1.0f/(float)gridDepth;
        glUniform1f(levelLoc, level);
        glDrawArrays(GL_POINTS, 0, gridWidth*gridDepth);
        glReadPixels(0,0,gridWidth,gridHeight,GL_RGBA,GL_UNSIGNED_BYTE,&pixel[4*gridWidth*gridHeight*i]);
    }
    double elapsed = glfwGetTime();
    std::cout << "voxelization done! elapsed = " << elapsed << "\n";
    glViewport(0,0,800,600);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glDeleteProgram(program);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glDeleteBuffers(1,&vbo);
    glDeleteFramebuffers(1,&fbo);
    glDeleteTextures(1,&tex);

    glEnable(GL_DEPTH_TEST);
    // voxel rasterization program
    vertexShader = glsl::compileShader(GL_VERTEX_SHADER, "shaders/vertex.glsl");
    fragmentShader = glsl::compileShader(GL_FRAGMENT_SHADER, "shaders/frag.glsl");
    program = glsl::linkShaders(vertexShader, fragmentShader);

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
    //GLuint vbo;
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
    GLuint colorLoc = glGetUniformLocation(program, "color");
    GLuint modelLoc = glGetUniformLocation(program, "model");
    GLuint viewLoc = glGetUniformLocation(program, "view");
    GLuint projectionLoc = glGetUniformLocation(program, "projection");

    // initialize our matrices
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::scale(model, glm::vec3(2.0f/d));
        glm::vec3 viewPos = glm::vec3(6.0f,0.0f,0.0f);
        glm::vec3 target = glm::vec3(0,0,0);
        glm::vec3 up = glm::vec3(0,0,1);
    glm::mat4 view = glm::lookAt(viewPos, target, up);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)width/(float)height, 0.1f, 1000.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // line rasterization (3D)
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
        view = glm::lookAt(glm::vec3(vX,vY,vZ), glm::vec3(0.,0.,cameraTargetHeight), up);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

        //// worse possible way - iterate over EVERY voxel, filled or not
        // worst possible way to draw
        for(int i = 0; i < gridDepth; ++i) {
        for(int j = 0; j < gridHeight; ++j) {
        for(int k = 0; k < gridWidth; ++k) {
            GLubyte* p = &pixel[i*gridWidth*gridHeight*4+gridHeight*j*4+k*4];
            if(*(p+3) > 0.0) {
                glm::vec4 f = glm::vec4(*p,*(p+1),*(p+2),*(p+3));
                f = f/255.0f;
                glUniform4fv(colorLoc, 1, &f[0]);
                float x = ((float)k-(float)gridWidth/2.0f);//sampler::width*(sampler::wXF-sampler::wX0)+sampler::wX0;
                //x = x/(float)gridWidth*2.0;
                float y = ((float)j-(float)gridHeight/2.0f);///((float)gridHeight/2.0);///sampler::height*(sampler::wYF-sampler::wY0)+sampler::wY0;
                //y = y/(float)gridHeight*2.0;
                float z = (float)i-(float)gridDepth/2.0f;///(float)gridDepth;//-(float)gridDepth/2.0f;///sampler::depth*(sampler::wZF-sampler::wZ0)+sampler::wZ0;
                //z = z/(float)gridDepth;
                glm::mat4 tModel = glm::translate(model, glm::vec3(x,y,z));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &tModel[0][0]);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
            }
        }}}
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    delete[] pixel;
    std::cout << "pixels deleted\n";

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