/*
 * =============================================================================
 *                              KFS2 BONUS - KEYBOARD HEADER
 * =============================================================================
 * PS/2 keyboard input driver header file
 * Provides keyboard interrupt handling and input processing
 * =============================================================================
 */

#ifndef KEYBOARD_H
# define KEYBOARD_H

/*
 * keyboard_handler - Main keyboard input processing loop
 * Handles key presses, modifier keys, and screen switching
 */
void keyboard_handler(void);

#endif
