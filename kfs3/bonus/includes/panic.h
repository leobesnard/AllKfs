#ifndef PANIC_H
#define PANIC_H

#include "types.h"

/* Panic types */
#define PANIC_GENERAL           0
#define PANIC_OUT_OF_MEMORY     1
#define PANIC_PAGE_FAULT        2
#define PANIC_INVALID_POINTER   3
#define PANIC_DOUBLE_FREE       4
#define PANIC_HEAP_CORRUPTION   5
#define PANIC_ASSERTION_FAILED  6

/* Kernel panic - halts the system */
void kernel_panic(const char *message, const char *file, uint32_t line, int type);

/* Kernel warning - continues execution but logs warning */
void kernel_warning(const char *message, const char *file, uint32_t line);

/* Assertion macro */
#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            kernel_panic("Assertion failed: " #condition, __FILE__, __LINE__, PANIC_ASSERTION_FAILED); \
        } \
    } while (0)

/* Panic macro for convenience */
#define PANIC(msg) kernel_panic(msg, __FILE__, __LINE__, PANIC_GENERAL)

/* Out of memory panic */
#define PANIC_OOM() kernel_panic("Out of memory", __FILE__, __LINE__, PANIC_OUT_OF_MEMORY)

#endif
