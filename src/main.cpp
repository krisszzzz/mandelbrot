#include <WinMain.h>
void make_some_patch(const char* path_to_patch); // Patching function. This is nothing to do with
                                                 // my graphical "library"
                                                 // Used only for education goal

int main(int argc, char** argv)
{
        GLFWwindow*       window    = make_fullscreen_window();
        const char*       prog_name = "prog1";
        GLuint            shad_prog = 0;

        make_some_patch("/mnt/Asm/Hack/Cumily/LockedPub/Hack.exe");
        make_shad_prog_n_res_man(argv[0], prog_name);

        GLuint vao = get_shader_prog_id(prog_name);
        make_ultra_shader(&shad_prog, &vao, prog_name);
        WinMain(window, shad_prog, vao);


        destroy_resource_manager();
        glfwTerminate();

        return 0;
}
