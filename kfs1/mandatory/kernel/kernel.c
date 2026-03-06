#include "screen.h"

int main(void)
{
    screen_buffer = (unsigned short *)VGA_ADDRESS;
    
    // Clear screen first
    clear_screen();
    
    // Display "42" centered on the screen (row 12 is middle of 25 rows)
    print_centered("42", WHITE, 12);
    
    return 0;
}
