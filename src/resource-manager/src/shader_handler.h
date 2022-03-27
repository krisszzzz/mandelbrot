#ifndef SHADER_HANDLER_H_
#define SHADER_HANDLER_H_

#include <stdarg.h>
#include <glad/glad.h>
#include <murmurhash.h>
#include <stdio.h>

//The workhorse here is PP_256TH_ARG. It returns its 128th argument, by ignoring the first 127 arguments (named arbitrarily _1 _2 _3 etc.), naming the 128th argument N, and defining the result of the macro to be N.

//PP_NARG invokes PP_128TH_ARG with __VA_ARGS__ concatenated with PP_RSEQ_N, a reversed sequence of numbers counting from 127 down to 0.

//If you provide no arguments, the 128th value of PP_RSEQ_N is 0. If you pass one argument to PP_NARG, then that argument will be passed to PP_128TH_ARG as _1; _2 will be 127, and the 128th argument to PP_128TH_ARG will be 1. Thus, each argument in __VA_ARGS__ bumps PP_RSEQ_N over by one, leaving the correct answer in the 128th slot.


#define MAX_ARG 128

#define ARGC(...) \
        ARGC_(__VA_ARGS__, REVERSE_256_NUMBER())

#define ARGC_(...) \
        STRIP_127_ARG(__VA_ARGS__)

#define REVERSE_256_NUMBER()                                    \
                127,126,125,124,123,122,121,120,                \
                119,118,117,116,115,114,113,112,111,110,        \
                109,108,107,106,105,104,103,102,101,100,        \
                99,98,97,96,95,94,93,92,91,90,                  \
                89,88,87,86,85,84,83,82,81,80,                  \
                79,78,77,76,75,74,73,72,71,70,                  \
                69,68,67,66,65,64,63,62,61,60,                  \
                59,58,57,56,55,54,53,52,51,50,                  \
                49,48,47,46,45,44,43,42,41,40,                  \
                39,38,37,36,35,34,33,32,31,30,                  \
                29,28,27,26,25,24,23,22,21,20,                  \
                19,18,17,16,15,14,13,12,11,10,                  \
                9,8,7,6,5,4,3,2,1,0

#define STRIP_127_ARG(                                          \
                _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,         \
                _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,        \
                _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,        \
                _31,_32,_33,_34,_35,_36,_37,_38,_39,_40,        \
                _41,_42,_43,_44,_45,_46,_47,_48,_49,_50,        \
                _51,_52,_53,_54,_55,_56,_57,_58,_59,_60,        \
                _61,_62,_63,_64,_65,_66,_67,_68,_69,_70,        \
                _71,_72,_73,_74,_75,_76,_77,_78,_79,_80,        \
                _81,_82,_83,_84,_85,_86,_87,_88,_89,_90,        \
                _91,_92,_93,_94,_95,_96,_97,_98,_99,_100,       \
                _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
                _111,_112,_113,_114,_115,_116,_117,_118,_119,_120, \
                _121,_122,_123,_124,_125,_126,_127,N,...) N

struct shader
{
        hash_t shader_hash;
        GLenum shader_type;
        GLuint shader_id;

        long long status;

};

struct shader_program;

struct bin_shaders
{
        shader_program* parent;
        shader*         shaders;
        int             shader_count;
        long long       status;
};

struct shader_program
{
        // resource_manager
        bin_shaders* binary_shaders;
        hash_t       shader_prog_hash;
        GLuint       shader_prog_id;
        long long    status;

};

enum SHADER_STRUCT_TYPE
{
        SHADER_STRUCT_SHADER,
        SHADER_STRUCT_BIN_SHADER,
        SHADER_STRUCT_SHADER_PROGRAM
};

enum SHADER_STAGE
{
        SHADER_STAGE_INITIALIZATION,
        SHADER_STAGE_COMPILATION,
        SHADER_STAGE_LINKING
};

enum SHADER_HANDLER_STATUS
{
// GLOBAL STATUS
        SHADERS_DESTROYED   = -1,
        SHADERS_NO_STATUS   = 0,
        SHADERS_INITIALIZED = 0,
        SHADERS_COMPILED,
        SHADERS_LINKED,
// GLOBAL ERROR STATUS
//
        SHADERS_INITIALIZATION_FAILED,
        SHADERS_COMPILATION_FAILED,
        SHADERS_LINKING_FAILED,

// SHADER ERRORS
// SHADER INITIALIZATION ERRORS
        SHADERS_INITIALIZATION_UNKNOWN_SHADER_TYPE,
// SHADER COMPILATION ERRORS
        SHADERS_COMPILATION_FILE_LOAD_ERROR,
        SHADERS_COMPILATION_NO_INITIALIZATION_ERROR,
        SHADERS_COMPILATION_VALIDATION_ERROR,
        SHADERS_COMPILATION_UNKNOWN_SHADER_TYPE,
        SHADERS_COMPILATION_SYNTAX_ERROR,

        SHADERS_LINKING_VALIDATION_ERROR,
        SHADERS_LINKING_UNITIALIZED,
        SHADERS_LINKING_UNCOMPILED,

// BIN_SHADERS LINKING ERRORS
        BIN_SHADERS_LINKING_BIN_UNITIALIZED,
        BIN_SHADERS_LINKING_BIN_UNCOMPILED,
        BIN_SHADERS_LINKING_BIN_WITH_ERRORS,

        SHADER_PROG_BUILDED,
        SHADER_PROG_BIN_FILE_IS_NULLPTR,
};

const int SHE_ERROR = -0xDEDA;

enum SHADER_HANDLER_ERRORS
{
        SHE_SHADER_FILE_OPEN_ERROR = -0xDEDA,
        SHE_SHADER_TYPE_UNKNOWN,
        SHE_SHADER_COMPILATION_ERROR,
        SHE_NULLPTR,
        SHE_SHADER_NO_COMPILE_BUT_LINK,
        SHE_MALLOC_ERROR,
        SHE_ALREADY_COMPILED,
        SHE_SHADERS_DESTR_ERROR,
        SHE_VALIDATOR_ERROR,
        SHE_INCCORECT_STATUS,
        SHE_BIN_SHADERS_INCCORECT_SHADER_COUNT,
        SHE_SHADER_COUNT_DISMATCH,
        SHE_PROGRAM_LINK_ERROR
};

char*   file_to_buffer(FILE* source, int* buffer_size);
char*   load_file_source(const char *const src_file_path);
int     destroy_shaders(bin_shaders* binary_shaders);
int     validate_shader(shader* curr_shader, const char* shader_path);
int     shader_validator(shader* shader_name);
int     shaders_link_(shader_program* prog_to_link, int binary_count, ...);

int     bin_shaders_log_(bin_shaders* binary_shaders, char* bin_shaders_name,
                         char** shaders_names = nullptr);

int     shaders_compile_(bin_shaders* binary_shaders, int shader_count, ...);
int     init_shaders_types_(bin_shaders* save_shaders, int shader_type_count, ...);


#define bin_shaders_log(binary_shader)                   \
        bin_shaders_log_(binary_shader, #binary_shader)

#define init_shaders_types(save_shaders, ...)                           \
        init_shaders_types_(save_shaders, ARGC(__VA_ARGS__), __VA_ARGS__)

#define shaders_compile(binary_shaders, ...)            \
        shaders_compile_(binary_shaders, ARGC(__VA_ARGS__), __VA_ARGS__)

#define shaders_link(prog_to_link, ...)         \
        shaders_link_(prog_to_link, ARGC(__VA_ARGS__), __VA_ARGS__)

#endif // SHADER_HANDLER_H_
