#include <WinMain.h>
#include <emmintrin.h>
#include <time.h>
#include <immintrin.h>
#include <cmath>

#define PIXEL_COORD_COUNT    2
#define WINDOW_WIDTH         1920
#define WINDOW_HEIGHT        1080
#define PIXELS_COUNT         WINDOW_WIDTH * WINDOW_HEIGHT
#define POINTS_COUNT         PIXELS_COUNT * PIXEL_COORD_COUNT
#define COLORS_COUNT         PIXELS_COUNT 
#define MAX_DEPTH            256
#define RADIUS_MAX           100.f
#define X_STEP               1.0f / (WINDOW_WIDTH)

float  X_SCALE  = 1.25 / 4.0f;
float  Y_SCALE  = 0.2f;
float  X_OFFSET = 0.0f;
float  Y_OFFSET = 0.0f;

void glfw_key_callback(GLFWwindow* p_window, int key, int scancode, int action, int mode)
{
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(p_window, GLFW_TRUE);
        } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
                X_SCALE *= 2.0f;
                Y_SCALE *= 2.0f;

        } else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
                X_SCALE /= 2.0f;
                Y_SCALE /= 2.0f;
        } else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
                Y_OFFSET -= (0.2f * Y_SCALE);
        } else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
                Y_OFFSET += (0.2f * Y_SCALE);
        } else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
                X_OFFSET -= (0.2f * X_SCALE);
        } else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
                X_OFFSET += (0.2f * X_SCALE);
        }

}

GLFWwindow* make_fullscreen_window()
{
    /* Initialize the library */
        if (!glfwInit()) {
                ErrorPrint("GLFW library initializitation error ocurred\n");
                return nullptr;
        }

    // Required minimum 4.6 OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

   /* Create a windowed mode window and its OpenGL context */
        GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH,
                                              WINDOW_HEIGHT,
                                              "Hello World",
                                              glfwGetPrimaryMonitor(),
                                              NULL);

        if (!window) {
                ErrorPrint("Window creating error\n");
                glfwTerminate();
                return nullptr;
        }

        glfwSetKeyCallback(window, glfw_key_callback);
   /* Make the window's context current */
        glfwMakeContextCurrent(window);

        if (!gladLoadGL()) {
                ErrorPrint("Fatal error: GLAD cannot be loaded\n");
                return nullptr;
        }

        printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
        glClearColor(0, 0, 0, 1);

        return window;

}

int make_shad_prog_n_res_man(const char *const execution_path, const char *const shader_prog_name)
{
        resource_manager res_man = {};
        init_resource_manager(&res_man, execution_path);
        bind_resource_manager(res_man);

        create_shader_prog(shader_prog_name, "res/Shaders/vertex_shader.glsl", "res/Shaders/fragment_shader.glsl");
        resource_manager_shader_log();

        return 0;

}

static void generate_buffers(GLfloat   points[POINTS_COUNT], GLfloat colors[COLORS_COUNT],
                             GLuint*   p_points_vbo,         GLuint* p_colors_vbo,
                             GLuint*   p_vao)
{
        int buffer_count  = 1;
        GLuint points_vbo = 0;
        GLuint colors_vbo = 0;
        GLuint vao        = 0;

        glGenBuffers(buffer_count, &points_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glBufferData(GL_ARRAY_BUFFER, POINTS_COUNT * sizeof(GLfloat), points, GL_DYNAMIC_DRAW);

        glGenBuffers(buffer_count, &colors_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
        glBufferData(GL_ARRAY_BUFFER, COLORS_COUNT * sizeof(GLfloat), colors, GL_DYNAMIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glVertexAttribPointer(0, PIXEL_COORD_COUNT, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);


        *p_points_vbo = points_vbo;
        *p_vao       = vao;
        *p_colors_vbo = colors_vbo;
 
}

void set_position(GLfloat* points, int x, int y)
{
        GLfloat x_pos = ((float)x - WINDOW_WIDTH  / 2.0f) / (WINDOW_WIDTH / 2.0f);
        GLfloat y_pos = ((float)-y + WINDOW_HEIGHT / 2.0f) / (WINDOW_HEIGHT / 2.0f);

        points[2*x +  2 * y * WINDOW_WIDTH]     = x_pos;
        points[2*x +  2 * y * WINDOW_WIDTH + 1] = y_pos;
}

void init_points(float* points)
{
        for (int x = 0; x < WINDOW_WIDTH; ++x) {
                for (int y = 0; y < WINDOW_HEIGHT; ++y) {
                        set_position(points, x, y);
                }
        }
}

void generate_color_mandelbrot_avx2(GLfloat* colors, GLfloat* points) __attribute__((nothrow, hot));

void generate_color_mandelbrot_avx2(GLfloat* colors, GLfloat* points)
{
        const __m256 R_MAX    = _mm256_set1_ps(RADIUS_MAX);
        const __m256 _7_to_0 =  _mm256_set_ps(  8.0f, 7.0f,  6.0f, 
                                                4.0f, 3.0f,  2.0f, 
                                                1.0f, 0.0f);

        for (int x = 0; x < WINDOW_WIDTH; x += 8) {
   
                for (int y = 0; y < WINDOW_HEIGHT; ++y) {
                        int pixel_pos  = x + y * WINDOW_WIDTH;
                        int points_pos = pixel_pos * 2;

                        float x0 = X_SCALE * (points[points_pos]) + X_OFFSET;
                        float y0 = Y_SCALE * (points[points_pos + 1]) + Y_OFFSET;

                        __m256 X0 = _mm256_add_ps(_mm256_set1_ps(x0), 
                                                  _mm256_mul_ps (_7_to_0, _mm256_set1_ps(X_SCALE * X_STEP)));
                        __m256 Y0 = _mm256_set1_ps(y0);
                               
                        __m256 X = X0;
                        __m256 Y = Y0;
                        
                        __m256i N  = _mm256_setzero_si256();

                        for (int n = 0; n < MAX_DEPTH; ++n) {
                                __m256 X2   = _mm256_mul_ps(X, X);
                                __m256 Y2   = _mm256_mul_ps(Y, Y);
                                __m256 R2   = _mm256_add_ps(X2, Y2);

                                __m256 mask = _mm256_cmp_ps(R2, R_MAX, _CMP_LT_OQ);
                                int i_mask  = _mm256_movemask_ps(mask);

                                if (!i_mask) break; 

                                N           = _mm256_sub_epi32(N, _mm256_castps_si256(mask));
                                __m256 XY   = _mm256_mul_ps(X, Y);

                                X = _mm256_add_ps (_mm256_sub_ps (X2, Y2), X0);
                                Y = _mm256_add_ps (_mm256_add_ps (XY, XY), Y0);
                        }

//                        _mm256_storeu_ps(&colors[pixel_pos], 
//                                             _mm256_cvtepi32_ps(_mm256_srli_epi32(N, 8))
//                               );
                                
                        for (int i = 0; i < 8; ++i) { 
                                int* pn = (int*)&N + i;
                                colors[pixel_pos + i] = sin(*pn);
                        }

                }
        }
}

inline void count_N(int N[4], float X0[4], float Y0[4])
{
        float X2     = 0;
        float XY     = 0;
        float Y2     = 0;
        float R2     = 0;


        for (int iter_count = 0; iter_count < 4; ++iter_count) {

                float X = X0[iter_count];
                float Y = Y0[iter_count];

                for (; N[iter_count] < MAX_DEPTH; ++N[iter_count]) {

                        X2 = X*X;
                        Y2 = Y*Y;
                        XY = X*Y;

                        R2 = X2 + Y2;

                        if (R2 >= RADIUS_MAX) break;

                        X = X2 - Y2 + X0[iter_count];
                        Y = XY + XY + Y0[iter_count];
                }

        }
        


}

inline void count_color(GLfloat* colors, int N[4], int first_color_pos)
{
        for (int iter_count = 0; iter_count < 4; ++iter_count) {

                int colors_pos      = first_color_pos + iter_count;
                colors[colors_pos]  = sin(N[iter_count]);
        }

}


void generate_color_mandelbrot(GLfloat* colors, GLfloat* points)
{
        float x0     = 0;
        float y0     = 0;

        for (int x = 0; x < WINDOW_WIDTH; x += 4) {

                for (int y = 0; y < WINDOW_HEIGHT; ++y) {

                        int first_color_pos  = (x +  y * WINDOW_WIDTH);
                        int points_pos       = 2 * (x +  y * WINDOW_WIDTH);

                        x0 = X_SCALE * (points[points_pos] + X_OFFSET);
                        y0 = Y_SCALE * (points[points_pos + 1] + Y_OFFSET);

                        float X0[4] = {x0, x0 + X_STEP, x0 + 2 * X_STEP, x0 + 3 * X_STEP};
                        float Y0[4] = {y0, y0,          y0,              y0};

                        int N[4] = {};

                        count_N(N, X0, Y0);
                        count_color(colors, N, first_color_pos);
                }
        }
}

void WinMain(GLFWwindow* window, GLuint shader_program)
{
        int vertex_indx_start = 0;
        int vertex_indx_end   = PIXELS_COUNT;
        int vertex_position   = 0;
        int color_position    = 1;

        GLfloat* points = (GLfloat*)calloc(POINTS_COUNT, sizeof(GLfloat));
        GLfloat* colors = (GLfloat*)calloc(COLORS_COUNT, sizeof(GLfloat));

        init_points(points);

        GLuint points_vbo  = 0;
        GLuint colors_vbo  = 0;
        GLuint vao         = 0;

        generate_buffers(points, colors, &points_vbo, &colors_vbo, &vao);
        clock_t stop    = 0,
                start   = 0;

       while (!glfwWindowShouldClose(window)) {
                /* Render here */

                glClear(GL_COLOR_BUFFER_BIT);

                generate_color_mandelbrot_avx2(colors, points);
                // generate_color_mandelbrot(colors, points);

                

                glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
                glBufferData(GL_ARRAY_BUFFER, COLORS_COUNT * sizeof(GLfloat), colors, GL_DYNAMIC_DRAW);

                glVertexAttribPointer(color_position, 1,  GL_FLOAT,
                                      GL_FALSE,       0,  nullptr);

                glUseProgram(shader_program);  // Use shader program that was
                glBindVertexArray(vao);        // generated bu glfw_make_triangle
                                               // Bind current VAO
                glDrawArrays(GL_POINTS, vertex_indx_start,
                             vertex_indx_end);           // Draw current VAO
        /* Swap front and back buffers */
                glfwSwapBuffers(window);
                /* Poll for and process events */
                glfwPollEvents();



//                first_time = second_time;

        }

       free(points);
       free(colors);
}
