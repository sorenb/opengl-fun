// compile with the c++ compiler:   g++ -g -lGL -lGLU -lglut triangles.cpp -o triangles `pkg-config --libs --cflags glu glew gl`
// or the c compiler:               gcc -g -lstdc++ -lGL -lGLU -lglut triangles.cpp -o triangles `pkg-config --libs --cflags glu glew gl`
// if these give the error about undefined reference to glutInit, try setting the -lglut and maybe all l's at the end instead

#include <iostream>
#include <cstdlib>
#include <GL/glew.h>
#include <stdio.h>
#include <glm/glm.hpp>   // this can be downloaded from glm.g-truc.net and put in your compilers includepath in e.g. ~/.profile with C_INCLUDE_PATH for gcc and CPLUS_INCLUDE_PATH for g++
#include <vector>
#include <algorithm> 
#include <GL/freeglut.h>

#define GLEW_STATIC
#define BUFFER_OFFSET(x)  ((const void*) (x))

using namespace std;

GLuint vao;
GLuint positionBufferObject;
GLuint vPosition = 0;

const GLuint NumVertices = 6;

//////////////////////////////////////////////////////////////////////////////
//
// Create program
//  in openGL 4 one or more shader objects is linked into a program object.
//  this function from arcsynthesis takes shaders stuffed in an array (shaderList)
//  and attaches them to a program
//  
//  arcsynthesis on not using programs:
//  OpenGL does have, in its compatibility profile, default rendering state
//  that takes over when a program is not being used. We will not be using this,
//  and you are encouraged to avoid its use as well.
//
//////////////////////////////////////////////////////////////////////////////

GLuint CreateProgram(const std::vector<GLuint> &shaderList)
{
    GLuint program = glCreateProgram();     // create empty program object
    
    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)   // loop thru shaderList
    	glAttachShader(program, shaderList[iLoop]);             // and attach each shader from it
    
    glLinkProgram(program);
    
    GLint status;
    glGetProgramiv (program, GL_LINK_STATUS, &status);          // link the program
    if (status == GL_FALSE)                                     // all this shit simply logs stuff on failure of the linking
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }
    
    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        glDetachShader(program, shaderList[iLoop]);             // detach the shaders (they are encoded in the program I guess)

    return program;                                             // return the new program to the caller
}

//////////////////////////////////////////////////////////////////////////////
//
// Shader definitions. Not a function
//  Typically you would put each of these in it's own file and e.g. suffix it 
//  with .vert og .frag
//
//////////////////////////////////////////////////////////////////////////////

const std::string strVertexShader(
	"#version 430\n"
	"layout(location = 0) in vec4 position;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = position;\n"
	"}\n"
);

const std::string strFragmentShader(
	"#version 430\n"
	"out vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"   outputColor = vec4(0.9f, 0.1453f, 0.3234f, 1.0f);\n"
	"}\n"
);


//////////////////////////////////////////////////////////////////////////////
//
// CreateShader
//
//////////////////////////////////////////////////////////////////////////////

GLuint CreateShader(GLenum eShaderType, const std::string &strShaderFile)
{
    GLuint shader = glCreateShader(eShaderType);
    const char *strFileData = strShaderFile.c_str();
    glShaderSource(shader, 1, &strFileData, NULL);
    
    glCompileShader(shader);
    
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        
        const char *strShaderType = NULL;
        switch(eShaderType)
        {
        case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
        case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
        case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }
        
        fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
        delete[] strInfoLog;
    }

	return shader;
}

//////////////////////////////////////////////////////////
//
// Init
//
//////////////////////////////////////////////////////////

void
init(void)
{
    std::vector<GLuint> shaderList;
    
    shaderList.push_back(CreateShader(GL_VERTEX_SHADER, strVertexShader));
    shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, strFragmentShader));

    GLuint program = CreateProgram(shaderList);
    
    GLfloat vertices[NumVertices][2] = {
        { -0.90, -0.90 }, // Triangle 1
        {  0.85, -0.90 },
        { -0.90,  0.85 },
        {  0.90, -0.85 }, // Triangle 2
        {  0.90,  0.90 },
        { -0.85,  0.90 }
    };

    glGenBuffers(1, &positionBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(vPosition, 2, GL_FLOAT,
                          GL_FALSE, 0, BUFFER_OFFSET(0));

    glEnableVertexAttribArray(vPosition);

    glUseProgram(program);

    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);   // delete the shaders (they are encoded in the program I guess)
}

/////////////////////////////////////////////////////////////
//
// display
//
/////////////////////////////////////////////////////////////

void
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    
    glFlush();
}

/////////////////////////////////////////////////////////////
//
// main
//
/////////////////////////////////////////////////////////////

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(512, 512);
    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow(argv[0]);

    glewExperimental = GL_TRUE;     // without this we'll have a segfault on glGenVertexArrays in init() 
                                    // I guess it means glew is only up to date with opengl 4 functions
                                    // when it is set to experimental.
                                    // see: http://stackoverflow.com/questions/8302625/segmentation-fault-at-glgenvertexarrays-1-vao

    if (glewInit()) {
        cerr << "Unable to initalize GLEW ... exiting" << endl;

        exit(EXIT_FAILURE); 
    }
    
    init();

    glutDisplayFunc(display);

    glutMainLoop();
}

