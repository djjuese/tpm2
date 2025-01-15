#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "NV_Mem.h"

// #define SIMULATED_FILE_SIZE 32768

SimulatedFile simulatedFile;

ROSELAND_FILE* my_fopen(const char* filename, const char* mode) {
    if (simulatedFile.is_open) {
        return NULL; // 文件已经打开
    }
    simulatedFile.position = 0;
    simulatedFile.is_open = 1;
    strncpy(simulatedFile.mode, mode, sizeof(simulatedFile.mode) - 1);
    simulatedFile.mode[sizeof(simulatedFile.mode) - 1] = '\0';
    
    if (strchr(mode, 'w') != NULL) {
        memset(simulatedFile.data, 0, SIMULATED_FILE_SIZE); // 清空文件内容
    }
    return &simulatedFile;
}

int my_fclose(ROSELAND_FILE* stream) {
    if (!stream->is_open) {
        return EOF; // 文件未打开
    }
    stream->is_open = 0;
    return 0;
}

size_t my_fread(void* ptr, size_t size, size_t nmemb, ROSELAND_FILE* stream) {
    if (!stream->is_open || strchr(stream->mode, 'r') == NULL) {
        return 0; // 文件未打开或未以读模式打开
    }
    size_t bytes_to_read = size * nmemb;
    if (stream->position + bytes_to_read > SIMULATED_FILE_SIZE) {
        bytes_to_read = SIMULATED_FILE_SIZE - stream->position;
    }
    memcpy(ptr, stream->data + stream->position, bytes_to_read);
    stream->position += bytes_to_read;
    return bytes_to_read / size;
}

size_t my_fwrite(const void* ptr, size_t size, size_t nmemb, ROSELAND_FILE* stream) {
    if (!stream->is_open || strchr(stream->mode, 'w') == NULL) {
        return 0; // 文件未打开或未以写模式打开
    }
    size_t bytes_to_write = size * nmemb;
    if (stream->position + bytes_to_write > SIMULATED_FILE_SIZE) {
        bytes_to_write = SIMULATED_FILE_SIZE - stream->position;
    }
    memcpy(stream->data + stream->position, ptr, bytes_to_write);
    stream->position += bytes_to_write;
    return bytes_to_write / size;
}

int my_fseek(ROSELAND_FILE* stream, long offset, int whence) {
    if (!stream->is_open) {
        return -1; // 文件未打开
    }
    size_t new_position;
    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = stream->position + offset;
            break;
        case SEEK_END:
            new_position = SIMULATED_FILE_SIZE + offset;
            break;
        default:
            return -1; // 无效的whence
    }
    if (new_position > SIMULATED_FILE_SIZE || new_position < 0) {
        return -1; // 无效的位置
    }
    stream->position = new_position;
    return 0;
}

long my_ftell(ROSELAND_FILE* stream) {
    if (!stream->is_open) {
        return -1; // 文件未打开
    }
    return stream->position;
}

int my_fflush(ROSELAND_FILE* stream) {
    if (!stream->is_open) {
        return EOF; // 文件未打开
    }
    // 对于内存文件，没有实际需要刷新的内容，但我们模拟这个行为
    // 如果将来扩展为真正的文件系统，这里可以实现实际的刷新操作
    return 0;
}
