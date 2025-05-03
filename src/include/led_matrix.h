#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

void led_matrix_init();
void led_matrix_clear();
void led_matrix_draw_walk_symbol();
void led_matrix_draw_dont_walk_symbol(bool flash_state);

#endif // LED_MATRIX_H