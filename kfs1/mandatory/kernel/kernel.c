/* ************************************************************************** */
/*                                                                            */
/*   kernel.c - Kernel entry point                                            */
/*                                                                            */
/* ************************************************************************** */

#include "screen.h"

/*
** Kernel main function - called from bootloader
** Initializes VGA display and shows centered "42" message
*/
int
main(void)
{
    /* Initialize VGA buffer pointer to video memory address */
    vga_buffer = (unsigned short *)VGA_MEMORY_BASE;

    /* Clear display before outputting content */
    vga_clear();

    /* Display "42" centered on screen (row 12 is vertical center) */
    vga_puts_centered("42", WHITE, 12);

    return 0;
}
