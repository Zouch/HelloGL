#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>

bool InitializeRenderer();
void Update(float dt);
void Render(float dt);

int g_Width, g_Height;

int main()
{
    GLFWwindow* window;
    
    if (!glfwInit())
    {
        fprintf(stderr, "glfwInit() failed.\n");
        return -1;
    }
    
    window = glfwCreateWindow(640, 480, "Hello, World", nullptr, nullptr);
    if (!window)
    {
        fprintf(stderr, "glfwCreateWindow() failed.\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &g_Width, &g_Height);
    
    if (gl3wInit())
    {
        fprintf(stderr, "gl3wInit() failed.\n");
        return -1;
    }
    
    if (!gl3wIsSupported(4, 5))
    {
        fprintf(stderr, "Opengl 4.5 not supported.\n");
        return -1;
    }
    
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    if (!InitializeRenderer())
    {
        fprintf(stderr, "InitRenderer() failed.\n");
        return -1;
    }
    
    double t0 = glfwGetTime();
    double t1;
    float dt;
    while (!glfwWindowShouldClose(window))
    {
        t1 = glfwGetTime();
        dt = (float)(t1 - t0);
        t0 = t1;
        
        glfwPollEvents();
        Update(dt);
        Render(dt);
        
        glfwSwapBuffers(window);
    }
    
    return 0;
}

static float g_CurrentAngle = 0.0f;
static float g_RotationMatrix[9] = { 0.f };

static GLuint g_TriangleVBO;
static GLuint g_TriangleVAO;

static GLuint g_ShaderProgram;
static GLuint g_RotationMatrixLoc;

static const char* g_VertexShader =
"#version 450 core\n"
"layout (location = 0) in vec3 in_Position;\n"
"uniform mat3 u_RotationMatrix;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(u_RotationMatrix * in_Position, 1.0);\n"
"}\n";

static const char* g_FragmentShader =
"#version 450 core\n"
"layout (location = 0) out vec4 out_Color;\n"
"void main()\n"
"{\n"
"    out_Color = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n";

bool CheckShader(GLuint Shader)
{
    GLint CompileStatus;
    
    GLchar InfoLog[512];
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);
    
    if (!CompileStatus)
    {
        glGetShaderInfoLog(Shader, 512, nullptr, InfoLog);
        fprintf(stderr, "Shader compilation error : %s\n", InfoLog);
    }
    
    return (bool)CompileStatus;
}

bool CheckProgram(GLuint Program)
{
    GLint LinkStatus;
    
    GLchar InfoLog[512];
    glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);
    
    if (!LinkStatus)
    {
        glGetProgramInfoLog(Program, 512, nullptr, InfoLog);
        fprintf(stderr, "Program link error : %s\n", InfoLog);
    }
    
    return (bool)LinkStatus;
}

bool InitializeRenderer()
{
    bool Result = true;
    
    // Setup buffers
    glGenVertexArrays(1, &g_TriangleVAO);
    glBindVertexArray(g_TriangleVAO);
    
    GLfloat vertices[] =
    {
        -0.5f, -0.5f, 0.0f,
        0.5, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f,
    };
    
    glGenBuffers(1, &g_TriangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_TriangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)nullptr);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    // Compile shaders
    GLuint VertexShader;
    VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &g_VertexShader, nullptr);
    glCompileShader(VertexShader);
    if (!CheckShader(VertexShader))
    {
        Result = false;
        return Result;
    }
    
    GLuint FragmentShader;
    FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &g_FragmentShader, nullptr);
    glCompileShader(FragmentShader);
    if (!CheckShader(FragmentShader))
    {
        Result = false;
        return Result;
    }
    
    // Link shaders
    g_ShaderProgram = glCreateProgram();
    glAttachShader(g_ShaderProgram, VertexShader);
    glAttachShader(g_ShaderProgram, FragmentShader);
    glLinkProgram(g_ShaderProgram);
    if (!CheckProgram(g_ShaderProgram))
    {
        Result = false;
        return Result;
    }
    
    glDetachShader(g_ShaderProgram, VertexShader);
    glDeleteShader(VertexShader);
    
    glDetachShader(g_ShaderProgram, FragmentShader);
    glDeleteShader(FragmentShader);
    
    g_RotationMatrixLoc = glGetUniformLocation(g_ShaderProgram, "u_RotationMatrix");
    
    return Result;
}

void Update(float dt)
{
    g_RotationMatrix[0] = cosf(g_CurrentAngle);
    g_RotationMatrix[1] = -sinf(g_CurrentAngle);
    g_RotationMatrix[3] = sinf(g_CurrentAngle);
    g_RotationMatrix[4] = cosf(g_CurrentAngle);
    g_RotationMatrix[8] = 1.0f;
    
    g_CurrentAngle = fmodf(g_CurrentAngle + 1.f * dt, 2.0 * M_PI);
}

void Render(float dt)
{
    glClear(GL_COLOR_BUFFER_BIT);
    // Set viewport
    glViewport(0, 0, g_Width, g_Height);
    
    // Use program
    glUseProgram(g_ShaderProgram);
    
    // Setup uniforms
    glUniformMatrix3fv(g_RotationMatrixLoc, 1, GL_FALSE, g_RotationMatrix);
    
    // Render triangle
    glBindVertexArray(g_TriangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
