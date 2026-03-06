#ifndef GDT_H
#define GDT_H

#include "types.h"

#define GDT_ADDRESS     0x800

/* GDT Entry structure */
typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

/* GDT Pointer structure */
typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* GDT Segment Selectors */
#define GDT_NULL_SEGMENT        0x00
#define GDT_KERNEL_CODE         0x08
#define GDT_KERNEL_DATA         0x10
#define GDT_KERNEL_STACK        0x18
#define GDT_USER_CODE           0x20
#define GDT_USER_DATA           0x28
#define GDT_USER_STACK          0x30

/* External stack symbol */
extern uint32_t stack_top;

/* GDT Functions */
void setup_gdt(void);
void print_gdt(void);
void print_kernel_stack(void);

#endif
