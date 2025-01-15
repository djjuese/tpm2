#ifndef ROSELAND_TCM2_DEBUG_H
#define ROSELAND_TCM2_DEBUG_H

#include <stdio.h>
#ifdef ROSELAND_TCM2_DEBUG_LOG
#define ROSELAND_TCM2_LOG(format, ...) printf(format" in %s:%d\n", ##__VA_ARGS__, __func__, __LINE__)
#define ROSELAND_TCM2_BUF_LOG(str, buf, buf_len)    \
    do {                                            \
        printf("%s:\n", str);                       \
        for (uint32_t i = 1; i <= buf_len; i++) {   \
            printf("%02x", buf[i-1]);               \
            if (i%16 == 0)                          \
                printf("\n");                       \
        }                                           \
        printf("\n");                               \
    } while (0)

#else

#define ROSELAND_TCM2_LOG(format, ...)
#define ROSELAND_TCM2_BUF_LOG(buf, buf_len, str)

#endif // ROSELAND_TCM2_DEBUG_LOG

#endif // ROSELAND_TCM2_DEBUG_H