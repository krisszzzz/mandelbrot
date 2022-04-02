#ifndef SHADER_HANDLER_H_
#define SHADER_HANDLER_H_

#include <stdarg.h>
#include <glad/glad.h>
#include <murmurhash.h>
#include <stdio.h>

//The workhorse here is PP_256TH_ARG. It returns its 128th argument, by ignoring the first 127 arguments (named arbitrarily _1 _2 _3 etc.), naming the 128th argument N, and defining the result of the macro to be N.

//PP_NARG invokes PP_128TH_ARG with __VA_ARGS__ concatenated with PP_RSEQ_N, a reversed sequence of numbers counting from 127 down to 0.

//If you provide no arguments, the 128th value of PP_RSEQ_N is 0. If you pass one argument to PP_NARG, then that argument will be passed to PP_128TH_ARG as _1; _2 will be 127, and the 128th argument to PP_128TH_ARG will be 1. Thus, each argument in __VA_ARGS__ bumps PP_RSEQ_N over by one, leaving the correct answer in the 128th slot.


const int MAX_SHADER_TYPES = 6;

struct shader
{
        char*     shader_path;
        hash_t    shader_hash;
        GLenum    shader_type;
        GLuint    shader_id;

        long long status;

};

struct shader_program
{
        shader      shaders[MAX_SHADER_TYPES];
        hash_t      shader_prog_hash;
        GLuint      shader_prog_id;

        long long   status;

};

struct resource_manager
{
        char*             execute_path;
        char*             shader_names_chunk;
        int               shader_names_offset;
        int               shader_names_size;

        shader_program*   programs;

        int               shader_program_size;
        int               shader_program_count;

        long long         status;

        int               is_binded;
        int               is_initialized;
};

enum RESOURCE_MANAGER_ERRORS
{
        RME_NULLPTR = -0xDED,
        RME_NO_BIND,
        RME_INCCORECT_MANAGER,
        RME_MALLOC_ERROR,
};

enum SHADER_STATUS
{
// GLOBAL STATUS
        SHADERS_DESTROYED   = -1,
        SHADERS_NO_STATUS   = 0,
//        SHADERS_INITIALIZED = 0,
        SHADERS_COMPILED,
        SHADERS_LINKED,

        SHADERS_COMPILATION_FILE_LOAD_ERROR,
        SHADERS_COMPILATION_SYNTAX_ERROR,

        SHADERS_LINKING_COMPILE_ERROR,
        SHADERS_LINKING_GL_ATTACH_ERROR,

// BIN_SHADERS LINKING ERRORS
};

enum SHADER_PROGRAM_STATUS
{
        SHADER_PROG_DESTROYED = -1,
        SHADER_PROG_NO_STATUS,
        SHADER_PROG_BUILDED,
        SHADER_PROG_BUILDING_ERROR,
        SHADER_PROG_GL_VALIDATE_ERROR,
        SHADER_PROG_LOG_MALLOC_ERROR
};

enum RESOURCE_MANAGER_STATUS
{
        RES_MAN_NO_STATUS,
        RES_MAN_INITIALIZED,
        RES_MAN_BINDED,
        RES_MAN_SHADER_PROG_MALLOC_ERROR,
};

extern shader_program* NOT_FOUNDED;


enum RET_VAL_ERRORS
{
        NULLPTR = -0xEBAFF,
        ERROR_RET
};

char*   file_to_buffer(FILE* source, int* buffer_size);
char*   load_file_source(const char *const src_file_path);
//int     validate_shader(shader* curr_shader, const char* shader_path);

int     init_resource_manager(resource_manager* res_manager, const char* exec_path);
int     bind_resource_manager(resource_manager res_manager);

int     make_shader_prog_(const char* prog_name, int binary_count, ...);
int     shader_prog_log(shader_program* prog);
int     destroy_programs();

int     destroy_resource_manager();
int     resource_manager_log();

int     create_shader_prog(const char* const shader_prog_name,     const char* const vert_s = nullptr,
                           const char* const frag_s     = nullptr, const char* const geom_s = nullptr,
                           const char* const tess_ctl_s = nullptr, const char* const tess_eval_s = nullptr,
                           const char* const comp_s     = nullptr);
int    resource_manager_shader_log();

GLuint          get_shader_prog_id(const char* prog_name);

shader_program* find_shader_prog(const char* prog_name);


#endif // SHADER_HANDLER_H_
