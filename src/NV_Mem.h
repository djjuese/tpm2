/* qsun - This header file is the NV underlying operation function,
which can be replaced with platform-specific operation function when necessary.
This header file is added in Platform.h */

#ifndef NV_MEM_H
#define NV_MEM_H

#define SIMULATED_FILE_SIZE 32768

typedef struct {
    uint8_t data[SIMULATED_FILE_SIZE];
    size_t position;
    int is_open;
    char mode[3]; // 模式字符串
} SimulatedFile;

extern SimulatedFile simulatedFile;

// #define ROSELAND_FILE FILE
// #define ROSELAND_OPEN fopen
// #define ROSELAND_CLOSE fclose
// #define ROSELAND_SEEK fseek
// #define ROSELAND_TELL ftell
// #define ROSELAND_READ fread
// #define ROSELAND_WRITE fwrite
// #define ROSELAND_FLUSH fflush

#define ROSELAND_FILE SimulatedFile
#define ROSELAND_OPEN my_fopen
#define ROSELAND_CLOSE my_fclose
#define ROSELAND_SEEK my_fseek
#define ROSELAND_TELL my_ftell
#define ROSELAND_READ my_fread
#define ROSELAND_WRITE my_fwrite
#define ROSELAND_FLUSH my_fflush


ROSELAND_FILE* my_fopen(const char* ROSELAND_FILEname, const char* mode);
int my_fclose(ROSELAND_FILE* stream);
size_t my_fread(void* ptr, size_t size, size_t nmemb, ROSELAND_FILE* stream);
size_t my_fwrite(const void* ptr, size_t size, size_t nmemb, ROSELAND_FILE* stream);
int my_fseek(ROSELAND_FILE* stream, long offset, int whence);
long my_ftell(ROSELAND_FILE* stream);
int my_fflush(ROSELAND_FILE* stream);

#endif // NV_MEM_H