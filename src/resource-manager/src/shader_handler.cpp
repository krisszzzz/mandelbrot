#include <shader_handler.h>
#include <log.h>
#include <string.h>

//++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+-
// Working with files

char* file_to_buffer(FILE* source, int* buffer_size)
{
        if (source == nullptr) {
                ErrorPrint(RED "ERROR: " END " nullptr FILE pointer in file_to_buffer\n");
                return nullptr;
        }

        fseek(source, 0, SEEK_END);
	int file_size = ftell(source);
	fseek(source, 0, SEEK_SET);

	char* buffer = (char*)calloc(file_size + 1, sizeof(char));

        if (buffer == nullptr) {

                ErrorPrint(RED "ERROR:" END " memory allocation error\n");
                return nullptr;
        }

        if (buffer_size) {
                *buffer_size = file_size;
        }

        fread(buffer, sizeof(char), file_size, source);

	return buffer;
}

char* load_file_source(const char *const src_file_path)
{
        if (!src_file_path) {
                ErrorPrint("src_file_path is nullptr.\n");
                return nullptr;
        }

        FILE *source_file = fopen(src_file_path, "r");

        if (!source_file) {
                ErrorPrint(RED "ERROR: " END " cannot open file. File name : %s\n", src_file_path);
                return nullptr;
        }

        char* source_buffer = file_to_buffer(source_file, nullptr);
        fclose(source_file);

        if (!source_buffer) {
                ErrorPrint(RED "ERROR: " END " memory allocation error\n");
                return nullptr;
        }

        return source_buffer;
}

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
// Checkers, validators, dump etc.

static int validate_shader_type(GLenum shader_type)
{
        switch(shader_type) {
                case GL_VERTEX_SHADER:
                case GL_TESS_CONTROL_SHADER:
                case GL_TESS_EVALUATION_SHADER:
                case GL_GEOMETRY_SHADER:
                case GL_FRAGMENT_SHADER:
                case GL_COMPUTE_SHADER: {
                        return 1;
                }

                default:
                        return 0;
        }
}


static int check_shader_status(long long shader_status, enum SHADER_HANDLER_STATUS status)
{
        int get_bit = ( shader_status >> status) & 1; // Get status bit
                                                          //
        if (get_bit) {
                return 1;
        }

        return 0;

}

static long long save_bit(long long value, int bit_position)
{
        return value | (1 << bit_position);
}

#define set_shad_status(value)                                                                            \
        binary_shaders->shaders[iter_count].status = save_bit(binary_shaders->shaders[iter_count].status, \
                                                              value);

#define set_bin_status(value)                                  \
        binary_shaders->status = save_bit(binary_shaders->status, value);

#define check_stat(status)  \
        check_shader_status(shader_status, status)

#define check_bin_stat(status) \
        check_shader_status(bin_status, status)


int status_validator(long long shader_status)
{
        int error_count = 0;

        if(shader_status == SHADERS_DESTROYED) {
                return shader_status;
        }

// Global errors
        error_count += check_stat(SHADERS_INITIALIZATION_FAILED);
        error_count += check_stat(SHADERS_COMPILATION_FAILED);
        error_count += check_stat(SHADERS_LINKING_FAILED);
// Init error
//         error_count += check_stat(SHADERS_INITIALIZATION_UNKNOWN_SHADER_TYPE);
// // Compilation error
//         error_count += check_stat(SHADERS_COMPILATION_VALIDATION_ERROR);
//         error_count += check_stat(SHADERS_COMPILATION_NO_INITIALIZATION_ERROR);
//         error_count += check_stat(SHADERS_COMPILATION_FILE_LOAD_ERROR);
//         error_count += check_stat(SHADERS_COMPILATION_UNKNOWN_SHADER_TYPE);
//         error_count += check_stat(SHADERS_COMPILATION_SYNTAX_ERROR);
// // Linking error
//         error_count += check_stat(SHADERS_LINKING_VALIDATION_ERROR);
//         error_count += check_stat(SHADERS_LINKING_UNITIALIZED_ERROR);
//         error_count += check_stat(SHADERS_LINKING_UNCOMPILED);

        if (error_count) {
                return SHE_ERROR;
        }

        // if (!validate_shader_type(SHADER(iter_count).shader_type)) {

        //         set_shad_status(SHADERS_COMPILATION_UNKNOWN_SHADER_TYPE);
        //         set_shad_status(SHADERS_COMPILATION_FAILED);

        //         is_error = 1;

        // }



        if (!check_stat(SHADERS_INITIALIZED) && (check_stat(SHADERS_LINKED) ||
                                                check_stat(SHADERS_COMPILED))) {
                return SHE_ERROR;
        }

        if(check_stat(SHADERS_LINKED) && !(check_stat(SHADERS_COMPILED))) {
                return SHE_ERROR;
        }

        return 0;

}


const char* define_shader_type(GLenum shader_type)
{
        switch(shader_type) {
                case GL_VERTEX_SHADER:
                        return "GL_VERTEX_SHADER";
                case GL_TESS_CONTROL_SHADER:
                        return "GL_TESS_CONTROL_SHADER";
                case GL_FRAGMENT_SHADER:
                        return "GL_FRAGMENT_SHADER";
                case GL_TESS_EVALUATION_SHADER:
                        return "GL_TESS_EVALUATION_SHADER";
                case GL_GEOMETRY_SHADER:
                        return "GL_GEOMETRY_SHADER";
                case GL_COMPUTE_SHADER:
                        return "GL_COMPUTE_SHADER";
                default:
                        return nullptr;
        }
}

static void shader_write_errors(long long shader_status, int shader_stage)
{
        int is_error = 0;

        if(shader_stage == SHADER_STAGE_INITIALIZATION) {

                if(!check_stat(SHADERS_INITIALIZED)) {
                        printf(GREEN "NOT_STARTED\n" END);
                        return;
                }

                if (check_stat(SHADERS_INITIALIZATION_UNKNOWN_SHADER_TYPE)) {
                        printf(RED "UNKNOWN_SHADER_TYPE " END);
                        is_error = 1;
                }

                if(check_stat(SHADERS_INITIALIZATION_FAILED) && !is_error) {
                        printf(RED "UNEXPECT_ERROR " END);
                        is_error = 1;
                }
        }

        if (shader_stage == SHADER_STAGE_COMPILATION) {

                if (!check_stat(SHADERS_COMPILED)) {
                        printf(GREEN "NOT_STARTED\n" END);
                        return;
                }

                if (check_stat(SHADERS_COMPILATION_FILE_LOAD_ERROR)) {
                        printf(RED "FILE_LOAD_ERROR " END);
                        is_error = 1;
                }

                if (check_stat(SHADERS_COMPILATION_NO_INITIALIZATION_ERROR)) {
                        printf(RED "UNINITILIZED " END);
                        is_error = 1;
                }

                if (check_stat(SHADERS_COMPILATION_VALIDATION_ERROR)) {
                        printf(FATAL_RED "INVALID_SHADER " END);
                        is_error = 1;
                }

                if (check_stat(SHADERS_COMPILATION_UNKNOWN_SHADER_TYPE)) {
                        printf(RED "INVALID_TYPE " END);
                        is_error = 1;
                }

                if (check_stat(SHADERS_COMPILATION_SYNTAX_ERROR)) {
                        printf(RED "SYNTAX_ERROR " END);
                        is_error = 1;
                }

                if(check_stat(SHADERS_COMPILATION_FAILED) && !is_error) {
                        printf(RED "UNEXPECT_ERROR " END);
                        is_error = 1;
                }


        }

        if (shader_stage == SHADER_STAGE_LINKING) {

                if(!(check_stat(SHADERS_LINKED))) {
                        printf(GREEN "NOT_STARTED\n" END);
                        return;
                }

                if(check_stat(SHADERS_LINKING_VALIDATION_ERROR)) {
                        printf(FATAL_RED "INVALID_SHADER " END);
                        is_error = 1;
                }

                if(check_stat(SHADERS_LINKING_UNITIALIZED)) {
                        printf(RED "UNITIALIZED " END);
                        is_error = 1;
                }

                if(check_stat(SHADERS_LINKING_UNCOMPILED)) {
                        printf(RED "UNCOMPILED " END);
                        is_error = 1;
                }

                if(check_stat(SHADERS_LINKING_FAILED) && !is_error) {
                        printf(RED "UNEXPECTED_ERROR " END);
                        is_error = 1;
                }

        }

        if(!is_error) {
                printf(GREEN "NO_ERROR " END);
        }

        printf("\n");
}

static void shader_status_write(long long shader_status, int shader_struct_type)
{

         if (shader_status == SHADERS_DESTROYED) {
                printf(RED "❌❌❌" FATAL_RED " DESTROYED" RED);
                return;
         }

        if (check_stat(SHADERS_INITIALIZATION_FAILED)) {
                printf(RED "❌" END);
        } else if (!check_stat(SHADERS_INITIALIZED)) {
                printf(RED "❌" END);
        } else {
                printf(GREEN "✓ " END);
        }

        if (check_stat(SHADERS_COMPILATION_FAILED)) {
                printf(RED "❌" END);
        } else if (!check_stat(SHADERS_COMPILED)) {
                printf(RED "❌" END);
        } else {
                printf(GREEN "✓ " END);
        }

        if (check_stat(SHADERS_LINKING_FAILED)) {
                printf(RED "❌" END);
        } else if (!check_stat(SHADERS_LINKED)) {
                printf(RED "❌" END);
        } else {
                printf(GREEN "✓ " END);
        }
        printf("\n");

        if(shader_struct_type == SHADER_STRUCT_SHADER) {

                printf("      ");  // Offset
                printf(MAGENTA "INIT:        " END);
                shader_write_errors(shader_status, SHADER_STAGE_INITIALIZATION);
                printf("      ");  // Offset
                printf(CYAN "COMPILATION: " END);
                shader_write_errors(shader_status, SHADER_STAGE_COMPILATION);
                printf("      ");  // Offset

                printf(BLUE  "LINKING:     " END);
                shader_write_errors(shader_status, SHADER_STAGE_LINKING);
        }

}

static void shader_write_info(shader* shader_to_write, int logical_number, char* shader_name = nullptr)
{
        if(shader_to_write == nullptr) {
                ErrorPrint("nullptr error\n");
                return;
        }

        printf(YELLOW "__________________________________"
                      "__________________________________\n" END);
        printf(YELLOW "Shader info: \n"
                      "Shader_hash   = %d\n" END,
                      shader_to_write->shader_hash);

        if(shader_name) {
                printf("shader related path: %s\n", shader_name);
        }

        printf(YELLOW "shader_count  = " END);

        if(logical_number == -1)
                printf(YELLOW "unknown\n" END);
        else
                printf(YELLOW "%d\n" END, logical_number);

        printf(YELLOW "shader_type   = %s (%d)\n"
                      "shader_id     = %d\n"
                      "shader_status = %d\n"
                      "PRETTY_STATUS:\n" END,
                      define_shader_type(shader_to_write->shader_type),
                      shader_to_write->shader_type,
                      shader_to_write->shader_id,
                      shader_to_write->status
               );

        printf(YELLOW "status info: I - initialized, C - compiled, L - linked\n"
               "I C L\n" END);

        shader_status_write(shader_to_write->status, SHADER_STRUCT_SHADER);

        printf(YELLOW "__________________________________"
                      "__________________________________\n" END);

}


int bin_shaders_log_(bin_shaders* binary_shaders, char* bin_shaders_name, char** shaders_names)
{
        if(binary_shaders == nullptr) {
                ErrorPrint("nullptr shaders\n");
                return SHE_NULLPTR;
        }
        if(bin_shaders_name == nullptr) {
                ErrorPrint("nullptr bin_shaders name\n");
                return SHE_NULLPTR;
        }

        int is_bin_shaders_log = 1;

        printf(BLUE  "++++++++++++++++++++++++++++++++++"
                     "++++++++++++++++++++++++++++++++++\n"
                     "bin_shaders log:\n"
                     "bin_shaders name - %s\n"
                     "bin_shaders shader_count - %d\n" END,
                      bin_shaders_name,
                      binary_shaders->shader_count);

        printf(BLUE  "__________________________________"
                     "__________________________________\n"
                     "BUILD LOG\n" END);

        printf(BLUE  "status info: I - initialized, C - compiled, L - linked\n"
               "I C L\n" END);
// TODO::: Write bin_shaders own error handler
        shader_status_write(binary_shaders->status, SHADER_STRUCT_BIN_SHADER);

        printf(BLUE  "__________________________________"
                     "__________________________________\n\n" END);

        printf(YELLOW "INDIVIDUAL SHADERS LOG\n" END);


        for(int iter_count = 0; iter_count < binary_shaders->shader_count; ++iter_count)
        {
                shader_write_info(&binary_shaders->shaders[iter_count], iter_count);
        }

              printf(BLUE  "++++++++++++++++++++++++++++++++++"
                           "++++++++++++++++++++++++++++++++++\n" END);
}


int init_shaders_types_(bin_shaders* binary_shaders, int shader_type_count, ...)
{
        if (binary_shaders == nullptr) {
                ErrorPrint(RED "ERROR: " END
                           "cannot init shaders because binary_shaders* shaders is nullptr\n");
                return SHE_NULLPTR;
        }
        if (binary_shaders->status != SHADERS_NO_STATUS) {
                ErrorPrint(RED "ERROR: " END
                           "binary_shaders object status is not zeroed.\n");
                return SHE_INCCORECT_STATUS;
        }

        set_bin_status(SHADERS_INITIALIZED);

        shader* shaders = (shader*)calloc(shader_type_count, sizeof(shader));

        if(shaders == nullptr) {
                ErrorPrint(RED "FATAL ERROR: " END
                           "Memory allocation error\n");
                set_bin_status(SHADERS_INITIALIZATION_FAILED);
                return SHE_MALLOC_ERROR;
        }

        va_list shader_types;
        va_start(shader_types, shader_type_count);

        binary_shaders->shaders      = shaders;
        binary_shaders->shader_count = shader_type_count;
        GLenum shader_type           = 0;

        int error_ret = 0;

        for (int iter_count = 0; iter_count < shader_type_count; ++iter_count) {

                shader_type = va_arg(shader_types, GLenum);

                set_shad_status(SHADERS_INITIALIZED);

                if (!validate_shader_type(shader_type)) {
                        set_shad_status(SHADERS_INITIALIZATION_FAILED);
                        set_shad_status(SHADERS_INITIALIZATION_UNKNOWN_SHADER_TYPE);
                        set_bin_status(SHADERS_INITIALIZATION_FAILED);
                        continue;
                }

                shaders[iter_count].shader_type = shader_type;

        }

        return 0;
}

#define SHADER(iter_count) binary_shaders->shaders[iter_count]

// int destroy_shaders(bin_shaders* binary_shaders)
// {
//         if (binary_shaders == nullptr) {
//                 ErrorPrint("Cannot destroy shaders because the passed pointer is nullptr\n");
//                 return SHE_SHADERS_DESTR_ERROR;
//         }

//         if(binary_shaders->shaders == nullptr) {
//                 ErrorPrint(RED "Error:" END "Cannot destroy object, because they probably corrupted\n");
//                 return SHE_SHADERS_DESTR_ERROR;
//         }

//         for (int iter_count = 0; iter_count < binary_shaders->shader_count; ++iter_count) {
//                 glDeleteShader(SHADER(iter_count).shader_id);
//         }

//         free(binary_shaders->shaders);
//         binary_shaders->status = SHADERS_DESTROYED;

//         return 0;
// }

static int shaders_compile_error_handler(bin_shaders* binary_shaders, char* shader_source, int iter_count)
{
        if(binary_shaders == nullptr) {
                ErrorPrint(RED "FATAL ERROR:" END
                       "Cannot work with nullltpr bin_shaders\n");
                return SHE_ERROR;
        }

        int is_error = 0;

        if (status_validator(SHADER(iter_count).status)) {
                set_shad_status(SHADERS_COMPILATION_VALIDATION_ERROR);
                set_shad_status(SHADERS_COMPILATION_FAILED);

                is_error = 1;
        }

        if (shader_source == nullptr) {
                printf(RED "FATAL ERROR: " END
                       "Cannot load shader source. Skipping this shader compilation\n");

                set_shad_status(SHADERS_COMPILATION_FILE_LOAD_ERROR);
                set_shad_status(SHADERS_COMPILATION_FAILED);
                is_error = 1;
        }

        if (!check_shader_status(SHADER(iter_count).status, SHADERS_INITIALIZED)) {
                set_shad_status(SHADERS_COMPILATION_NO_INITIALIZATION_ERROR);
                set_shad_status(SHADERS_COMPILATION_FAILED);

                is_error = 1;
        }

        if (!validate_shader_type(SHADER(iter_count).shader_type)) {
                set_shad_status(SHADERS_COMPILATION_UNKNOWN_SHADER_TYPE);
                set_shad_status(SHADERS_COMPILATION_FAILED);

                is_error = 1;

        }

        return is_error ? SHE_ERROR : 0;


}

static int shader_compile_status(bin_shaders* binary_shaders, const char* shader_path, int iter_count)
{
        if(!binary_shaders) {
                ErrorPrint("Shader validation error. Current shader is nullptr\n");
                return SHE_ERROR;
        }

        int result = 0;

        glGetShaderiv(SHADER(iter_count).shader_id, GL_COMPILE_STATUS, &result);

        if(!result) {  // Compilation syntax error
                printf(RED "Shader compilation error\n" END);

                int log_length = 0;

                glGetShaderiv(SHADER(iter_count).shader_id, GL_INFO_LOG_LENGTH, &log_length);

                char* message = (char*) calloc(log_length, sizeof(char));

                if (message == nullptr) {
                        ErrorPrint(RED "ERROR:" END "memory allocation error, cannot make log about"
                                   "inccorect shader compilation\n");
                        return SHE_ERROR;
                }

                glGetShaderInfoLog(SHADER(iter_count).shader_id, log_length, &log_length, message);

                printf("Failed to compile shader file. Shader file path " RED " %s\n" END,
                       shader_path);
                shader_write_info(&SHADER(iter_count), iter_count);

                printf("Compilation log: \n");
                printf("%s\n\n\n", message);

                free(message);

                set_shad_status(SHADERS_COMPILATION_FAILED);
                set_shad_status(SHADERS_COMPILATION_SYNTAX_ERROR);

                return SHE_ERROR;
        }

        return 0;
}

int shaders_compile_(bin_shaders* binary_shaders, int shader_count, ...)
{
        if (binary_shaders == nullptr) {
                ErrorPrint("Cannot work with nullptr shaders\n");
                return SHE_NULLPTR;
        }

        if (check_shader_status(binary_shaders->status, SHADERS_COMPILED)) {
                printf(CYAN "Warning: " END
                       "Already compiled bin_shaders object."
                       " Leaving the function\n"
                       );
                set_bin_status(SHADERS_COMPILATION_FAILED);
                return SHE_ALREADY_COMPILED;
        }


        set_bin_status(SHADERS_COMPILED); // Compilation was started

        if (binary_shaders->shader_count <= 0) {
                printf(CYAN "Warning:" END
                       "Current bin_shaders is has shader_count <= 0"
                       " no one shader will be compiled or checked\n");
                set_bin_status(SHADERS_COMPILATION_FAILED);
                return SHE_BIN_SHADERS_INCCORECT_SHADER_COUNT;
        }

        if (shader_count != binary_shaders->shader_count) {
                printf(RED "Error: " END
                       "bin_shaders shader_count isn't equal to argument count\n"
                       );

                set_bin_status(SHADERS_COMPILATION_FAILED);
                return SHE_SHADER_COUNT_DISMATCH;
        }

        va_list shader_names;
        va_start(shader_names, binary_shaders);

        const int k_string_buffer_size = 1;


        for (int iter_count = 0; iter_count < shader_count; ++iter_count) {


                char* shader_path               = va_arg(shader_names, char*);
                SHADER(iter_count).shader_hash  = MurmurHash(shader_path, strlen(shader_path));
                char* shader_source             = load_file_source(shader_path);

                GLuint shader_id                = glCreateShader(SHADER(iter_count).shader_type);

                if (shaders_compile_error_handler(binary_shaders, shader_source, iter_count)) {

                        set_bin_status(SHADERS_COMPILATION_FAILED);
                        set_shad_status(SHADERS_COMPILED);
                        free(shader_source);

                        continue;
                }

                glShaderSource(shader_id,
                               k_string_buffer_size,
                               &shader_source,
                               nullptr);


                glCompileShader(shader_id);
                SHADER(iter_count).shader_id = shader_id;

                set_shad_status(SHADERS_COMPILED);
                free(shader_source);

                if (shader_compile_status(binary_shaders, shader_path, iter_count)) {
                        set_bin_status(SHADERS_COMPILATION_FAILED);
                }

        }

        return 0;

}

static int shader_to_link_validate(bin_shaders* binary_shaders, int iter_count)
{
       if (binary_shaders == nullptr) {
               ErrorPrint(FATAL_RED "ERROR:" END " cannot handle nullptr shader\n");
               // set_bin_(SHADER_NULLPTR);
               return SHE_ERROR;
       }


       set_shad_status(SHADERS_LINKED);

       if (!check_shader_status(binary_shaders->status, SHADERS_INITIALIZED)) {
               set_shad_status(SHADERS_LINKING_UNITIALIZED);
               set_shad_status(SHADERS_LINKING_FAILED);
               set_bin_status(SHADERS_LINKING_FAILED);

               return SHE_ERROR;
       }

       if (!check_shader_status(binary_shaders->status, SHADERS_COMPILED)) {
               set_shad_status(SHADERS_LINKING_UNCOMPILED);
               set_shad_status(SHADERS_LINKING_FAILED);
               set_bin_status(SHADERS_LINKING_FAILED);

               return SHE_ERROR;
       }

       if (status_validator(SHADER(iter_count).status)) {
               set_shad_status(SHADERS_LINKING_VALIDATION_ERROR);
               set_shad_status(SHADERS_LINKING_FAILED);
               set_bin_status(SHADERS_LINKING_FAILED);

               return SHE_ERROR;
       }

      // if (check_shader_status(shader_to_shadle->status, SHADER_DESTROYED)) {
      //          printf(BLUE "Linking error:" END " cannot link, because shader was already destroyed\n");
      //          shader_write_info(shader_to_handle, logical_number);
      //          return SHE_ERROR;
      //  }

      //  if (check_shader_status(shader_to_handle->status, SHADER_LINKED)) {
      //          printf(YELLOW "Linking warning: " END " already linked shader. It is probably error\n");
      //          shader_write_info(shader_to_handle, logical_number);
      //  }

       return 0;


}

static int bin_shader_error_count(long long bin_status)
{
        int is_error = 0;

        is_error += check_bin_stat(SHADERS_LINKING_FAILED);
        is_error += check_bin_stat(SHADERS_INITIALIZATION_FAILED);
        is_error += check_bin_stat(SHADERS_COMPILATION_FAILED);
        is_error += check_bin_stat(BIN_SHADERS_LINKING_BIN_UNITIALIZED);
        is_error += check_bin_stat(BIN_SHADERS_LINKING_BIN_UNCOMPILED);
        is_error += check_bin_stat(BIN_SHADERS_LINKING_BIN_WITH_ERRORS);

        return is_error;
}

static int validate_bin(bin_shaders* binary_shaders)
{
        int is_error = 0;
        int bin_status = binary_shaders->status;

        if((is_error = bin_shader_error_count(binary_shaders->status)) != 0) {
                set_bin_status(BIN_SHADERS_LINKING_BIN_WITH_ERRORS);
        }

        if(!check_bin_stat(SHADERS_INITIALIZED)) {
                set_bin_status(BIN_SHADERS_LINKING_BIN_UNITIALIZED);
                is_error += 1;
        }

        if(!check_bin_stat(SHADERS_COMPILED)) {
                set_bin_status(BIN_SHADERS_LINKING_BIN_UNCOMPILED);
                is_error += 1;
        }


        return is_error ? 1 : 0;
}


int shaders_link_(shader_program* prog_to_link, int binary_count, ...)
{
        if (prog_to_link == nullptr) {
                ErrorPrint(FATAL_RED "ERROR: " END
                           "pointer to program in shader_link is nullptr\n");
                return SHE_PROGRAM_LINK_ERROR;
        }

        va_list va_binaries;
        va_start(va_binaries, binary_count);

        prog_to_link->shader_prog_id = glCreateProgram();

        int ret_val   = 0;
        int error_val = 0;

        bin_shaders* binaries = (bin_shaders*)calloc(binary_count, sizeof(bin_shaders));
        bin_shaders* binary_shaders = nullptr;
        // HANDLE

        for (int bin_count = 0; bin_count < binary_count; ++bin_count) {

                binary_shaders = va_arg(va_binaries, bin_shaders*);

                if (binary_shaders == nullptr) {
                        ErrorPrint(FATAL_RED "ERROR: " END
                                   "nullptr bin_shaders pass in this function."
                                   "Skipping this file linking\n");

//                        set_shad_prog_stat(SHADER_PROG_BIN_FILE_IS_NULLPTR);
                        continue;
                }
                set_bin_status(SHADERS_LINKED);

                if (validate_bin(binary_shaders)) {
                        set_bin_status(SHADERS_LINKING_FAILED);
                }

                for(int shader_count = 0; shader_count < binary_shaders->shader_count; ++shader_count) {

                        if (shader_to_link_validate(binary_shaders, shader_count)) {
                                // SAVE shader prog status
                                continue;
                        }

                        glAttachShader(prog_to_link->shader_prog_id,
                                       SHADER(shader_count).shader_id);


                        // TODO

                }



        }

        glLinkProgram(prog_to_link->shader_prog_id);
        // TODO CHECK THIS
        glValidateProgram(prog_to_link->shader_prog_id);

        return 0;

}


#undef SHADER
