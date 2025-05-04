#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

void led_matrix_init();
void led_matrix_clear();
void led_matrix_ped_walk();
void led_matrix_ped_dont_walk(bool flash_state);

#endif // LED_MATRIX_H