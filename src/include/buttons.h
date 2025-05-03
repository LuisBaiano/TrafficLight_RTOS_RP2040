#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h> // For bool type

void buttons_init();
bool button_a_pressed();
bool button_b_pressed();
// bool joystick_button_pressed(); // Removed if not used

#endif // BUTTONS_H