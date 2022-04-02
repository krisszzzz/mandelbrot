#include <bits/types/FILE.h>
#include <stdio.h>
#include <resource_manager.h>

void make_some_patch(const char* path_to_patch)
{
        int patch_position_start = 0x11c7;
        int patch_position_end   = patch_position_start + 6;
        int patch_next_start     = 0x11d3;

        FILE* patch = fopen(path_to_patch, "rb");
        int buffer_size = 0;

        if (patch == nullptr) {
                printf("Error opening patching file\n");
                return;
        }
        char* source = file_to_buffer(patch, &buffer_size);
        for (int iter_count = patch_position_start; iter_count < patch_position_end; ++iter_count) {
                source[iter_count] = (char)0x90; // nop
        }

        source[patch_next_start] = (char)0x90;
        source[patch_next_start + 1] = source[patch_position_start];

        FILE* patched_version = fopen("Hack_patched.exe", "wb");

        if (patched_version == nullptr) {
                printf("Error opening patching file\n");
                return;
        }

        fwrite(source, sizeof(char), buffer_size, patched_version);
        fclose(patch);
        free(source);
}
