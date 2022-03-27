#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <log.h>
#include <unistd.h>
#include <shader_handler.h>

enum ERRORS
{
        k_FILE_OPEN_ERROR = -155,
        k_GLAD_NOT_LOADED,
        k_SHADER_COMPILATION_ERROR,
        k_MEM_ALLOC_ERROR,
        k_SHADER_LOAD_ERROR,
};

// TODO: Made a header file. Kill the main
//


void glfw_key_callback(GLFWwindow* p_window, int key, int scancode, int action, int mode)
{
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(p_window, GLFW_TRUE);
        }
}

int make_triangle(GLuint* p_shader_prog, GLuint* p_vao)
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

        bin_shaders shaders = {};

      //  init_shaders_types(&shaders, GL_FRAGMENT_SHADER, GL_VERTEX_SHADER);

//        diff_shaders_compile(&shaders, "fragment_shader.txt", "vertex_shader.txt");

        /* shader_program prog = {}; */
        /* shader_link(&prog, &shaders); */

        GLuint shader_program = 0;

        /* destroy_shaders(&shaders); */


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
//        printf("Color = %u, Point = %u, VAO = %u, source = %s\n", colors_vbo, points_vbo, vao, vs_buffer);

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

        /* free(vs_buffer); */
        /* free(fs_buffer); */

        return 0;
}

int unit_test()
{
        bin_shaders bin = {};
        init_shaders_types(&bin, 5, GL_VERTEX_SHADER);

        shaders_compile(&bin, "fragment_shader.txt", "vertex_shader.txt");

        shader_program prog = {};
        shaders_link(&prog, &bin);

        bin_shaders_log(&bin);
        exit(0);
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

        if (!gladLoadGL()) {
                ErrorPrint("Fatal error: GLAD cannot be loaded\n");
                return k_GLAD_NOT_LOADED;
        }

        printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);

        glClearColor(0, 0, 0, 1);

        GLuint shader_program = 0;
        GLuint vao            = 0;

        unit_test();

        if (make_triangle(&shader_program, &vao)) {
                ErrorPrint("Fatal error: cannot compile file contaning shader info\n");
                return k_SHADER_COMPILATION_ERROR;
        }

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
