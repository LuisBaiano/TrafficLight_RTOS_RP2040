#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init();
void buzzer_play_success(); // Optional startup sound
// void buzzer_joystick_beep(); // Removed if joystick not used
void buzzer_play_tone(uint freq, uint duration_ms);

#endif // BUZZER_H