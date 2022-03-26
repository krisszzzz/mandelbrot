#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <log.h>
#include <unistd.h>

enum ERRORS
{
        k_FILE_OPEN_ERROR = -155,
        k_GLAD_NOT_LOADED,
        k_SHADER_COMPILATION_ERROR,
        k_MEM_ALLOC_ERROR
};

// TODO: Made a header file. Kill the main
//
char* file_to_buffer(FILE* source, int* buffer_size)
{
        ERROR_IF(source == nullptr,
                 nullptr,
                 "Error: nullptr FILE pointer in alloc_buffer\n");

        fseek(source, 0, SEEK_END);
	int file_size = ftell(source);
	fseek(source, 0, SEEK_SET);

	char* buffer = (char*)calloc(file_size + 1, sizeof(char));

        ERROR_IF(buffer == nullptr, nullptr,
                 "Memory allocation error\n");

        if (buffer_size) {
                *buffer_size = file_size;
        }

        fread(buffer, sizeof(char), file_size, source);

	return buffer;
}



void glfw_key_callback(GLFWwindow* p_window, int key, int scancode, int action, int mode)
{
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(p_window, GLFW_TRUE);
        }
}

int glfw_compile_shaders(GLuint shader_type, const char *const source, GLuint* shader_to_save)
{
        const int k_string_buffer_size = 1;

        GLuint shader_id = glCreateShader(shader_type);

        glShaderSource(shader_id, k_string_buffer_size, &source, nullptr);
        glCompileShader(shader_id);

        int result = 0;

        // Get compilation result
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

        if (!result) {
                ErrorPrint("Shader compilation error\n");

                int log_length = 0;

                glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

                char *message = (char*)calloc(log_length, sizeof(char));

                glGetShaderInfoLog(shader_id, log_length, &log_length, message);

                ERROR_IF(message == nullptr, k_MEM_ALLOC_ERROR,
                       "Memory allocation error, cannot make log about "
                       "inccorect shader compilation\n");

                const char* shader_type_message = nullptr;

                if (shader_type == GL_VERTEX_SHADER)
                        shader_type_message = "vertex";
                else
                        shader_type_message = "fragment";

                printf("Failed to compile shader file. Shader file type - %s\n", shader_type_message);
                printf("Compilation Log:\n%s", message);

                glDeleteShader(shader_id);        // delete incorrect shader

                free(message);
                return -1;
        }

       *shader_to_save = shader_id;
       return 0;

}

int glfw_load_shaders(char* fs_source, char* vs_source, GLuint* p_program)
{
        GLuint program = glCreateProgram();

        GLuint vert_shader = 0, frag_shader = 0;

        ERROR_IF(glfw_compile_shaders(GL_VERTEX_SHADER, vs_source, &vert_shader),
                 k_SHADER_COMPILATION_ERROR,
                 "Exiting from all function");

        ERROR_IF(glfw_compile_shaders(GL_FRAGMENT_SHADER, fs_source, &frag_shader),
                 k_SHADER_COMPILATION_ERROR,
                 "Exiting from all function");

        glAttachShader(program, vert_shader);
        glAttachShader(program, frag_shader);

        glLinkProgram(program);

        glValidateProgram(program);

        *p_program = program;

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        free(vs_source);
        free(fs_source);

        return 0;

}

int glfw_make_triangle(GLuint* p_shader_prog, GLuint* p_vao)
{
        GLfloat point[] = {          // Vertex of triangle
                0.0,  0.5, 0.0,      // See OpenGL info abount normalized coordinates
                0.5, -0.5, 0.0,
                -0.5, -0.5, 0.0
        };

        GLfloat color[] = {
                1.0, 0.0, 0.0,      // Set vertex colors.
                0.0, 1.0, 0.0,  // These numbers will be interpolated using vertex_shader
                0.0, 0.0, 1.0
        };


        FILE* v_shader_source = fopen("vertex_shader.txt", "r");  // Open vertex_shader
                                                                  // To compile this code
        FILE* f_shader_source = fopen("fragment_shader.txt", "r");
        // Open fragment shader source code

        ERROR_IF(f_shader_source == nullptr || v_shader_source == nullptr,
                 k_FILE_OPEN_ERROR,
                "Error while opening one of the shader source file\n"
                 );

        char*     vs_buffer             = nullptr;
        char*     fs_buffer             = nullptr;
        const int k_string_buffer_size  = 1;

        vs_buffer = file_to_buffer(v_shader_source, nullptr);
        fs_buffer = file_to_buffer(f_shader_source, nullptr);
        // Save source to appropriate buffers
        //

        GLuint shader_program = 0;

        glfw_load_shaders(fs_buffer, vs_buffer, &shader_program);

        int    buffer_count = 1;

        // Vertex buffer object
        GLuint points_vbo   = 0;

        glGenBuffers(buffer_count, &points_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
        // Make buffer to contain points in videomemory

        GLuint colors_vbo   = 0;

        glGenBuffers(buffer_count, &colors_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
        // Contain colors in videomemory

        // Vertex array object
        GLuint vao = 0;

        glGenVertexArrays(buffer_count, &vao); // Create VAO
        glBindVertexArray(vao);    // Bind VAO
        printf("Color = %u, Point = %u, VAO = %u, source = %s\n", colors_vbo, points_vbo, vao, vs_buffer);

        int vertex_position = 0;
        int vertex_color    = 1;
        int vertex_count    = 3;
        int stride          = 0;


        glEnableVertexAttribArray(vertex_position); // layout(location = 0); See vertex_shader.txt
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo); // Vertex proccessing

        glVertexAttribPointer(vertex_position, vertex_count, GL_FLOAT,
                              GL_FALSE,        stride,       nullptr);


        glEnableVertexAttribArray(vertex_color); // layout(location = 1); See fragment_shader.txt
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo); // Vertex proccessing

        glVertexAttribPointer(vertex_color,    vertex_count, GL_FLOAT,
                              GL_FALSE,        stride,       nullptr);

        *p_shader_prog = shader_program;
        *p_vao         = vao;

        fclose(v_shader_source);
        fclose(f_shader_source);

        return 0;
}

int main()
{
        int window_width  = 1920;
        int window_height = 1080;
    /* Initialize the library */
        if (!glfwInit()) {
                ErrorPrint("GLFW library initializitation error ocurred\n");
                return -1;
        }

    // Required minimum 4.6 OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

   /* Create a windowed mode window and its OpenGL context */
        GLFWwindow* window = glfwCreateWindow(window_width,
                                              window_height,
                                              "Hello World",
                                              glfwGetPrimaryMonitor(),
                                              NULL);

        if (!window) {
                ErrorPrint("Window creating error\n");
                glfwTerminate();
                return -1;
        }


        glfwSetKeyCallback(window, glfw_key_callback);
   /* Make the window's context current */
        glfwMakeContextCurrent(window);

        ERROR_IF(!gladLoadGL(),
                 k_GLAD_NOT_LOADED,
                 "Fatal error: GLAD cannot be loaded\n"
                )

        printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);

        glClearColor(0, 0, 0, 1);

        GLuint shader_program = 0;
        GLuint vao            = 0;

        ERROR_IF(glfw_make_triangle(&shader_program, &vao),
                 k_SHADER_COMPILATION_ERROR,
                 "Fatal error: cannot compile file contaning shader info\n"
                 )

        int vertex_indx_start = 0;
        int vertex_indx_count = 3;
        printf("VAO = %u, shader program = %u\n", vao, shader_program);
    /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)) {
        /* Render here */
                glClear(GL_COLOR_BUFFER_BIT);

                glUseProgram(shader_program);  // Use shader program that was
                glBindVertexArray(vao);        // generated bu glfw_make_triangle
                                               // Bind current VAO

                glDrawArrays(GL_TRIANGLES, vertex_indx_start,
                             vertex_indx_count);           // Draw current VAO
        /* Swap front and back buffers */
                glfwSwapBuffers(window);

        /* Poll for and process events */
                glfwPollEvents();

        }

        glfwTerminate();
        return 0;
}
