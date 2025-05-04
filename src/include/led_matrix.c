#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "led_matrix.h"
#include "config.h"
#include "pico/stdlib.h"
#include "led_matrix.pio.h"
#include <math.h>
#include <string.h>

static PIO pio_instance = pio0;
static uint pio_sm = 0;
static uint32_t pixel_buffer[MATRIX_SIZE]; 

typedef struct { 
    float r; 
    float g; 
    float b; 
} ws2812b_color_t;

//define os leds apagados com cor devido o controle de brilho através do pio.
static const ws2812b_color_t COLOR_BLACK = { 0.0f, 0.0f, 0.0f };
static const ws2812b_color_t COLOR_RED   = { 0.25f, 0.0f, 0.0f };
static const ws2812b_color_t COLOR_GREEN = { 0.0f, 0.25f, 0.0f };

//posição real dos leds da matrix na BitDogLab
static const uint8_t Leds_Matrix_postion[MATRIX_DIM][MATRIX_DIM] = {
    {   24,    23,    22,    21,    20 }, 
    {   15,    16,    17,    18,    19 }, 
    {   14,    13,    12,    11,    10 }, 
    {    5,     6,     7,     8,     9 }, 
    {    4,     3,     2,     1,     0 }  
};

//determinar a posição de cada led para fazer a manipulação
static inline int posicao_leds(int lin, int col) {
    if (lin >= 1 && lin <= MATRIX_DIM &&
        col >= 1 && col <= MATRIX_DIM) {
        return Leds_Matrix_postion[lin - 1][col - 1];
    }
    return -1;
}

// faz as modoficações para definir o brilho como float
static inline uint32_t color_to_pio_format(ws2812b_color_t color, float brightness) {
    float r = fmaxf(0.0f, fminf(1.0f, color.r * brightness));
    float g = fmaxf(0.0f, fminf(1.0f, color.g * brightness));
    float b = fmaxf(0.0f, fminf(1.0f, color.b * brightness));
    unsigned char R_val = (unsigned char)(r * 255.0f);
    unsigned char G_val = (unsigned char)(g * 255.0f);
    unsigned char B_val = (unsigned char)(b * 255.0f);
    return ((uint32_t)(G_val) << 24) | ((uint32_t)(R_val) << 16) | ((uint32_t)(B_val) << 8);
}

static void update_matrix() {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        pio_sm_put_blocking(pio_instance, pio_sm, pixel_buffer[i]);
    }
    busy_wait_us(50);
}

// Helper to set a pixel using PHYSICAL coordinates (1-based row/col)
static void led_active_position(int lin, int col, uint32_t color) {
    int logical_index = posicao_leds(lin, col);
    if (logical_index != -1) {
        pixel_buffer[logical_index] = color;
    }
}

//inicia a matriz
void led_matrix_init() {
    uint offset = pio_add_program(pio_instance, &led_matrix_program);
    led_matrix_program_init(pio_instance, pio_sm, offset, MATRIX_WS2812_PIN);
    led_matrix_clear();
}

//apaga os leds da matriz
void led_matrix_clear() {
    uint32_t pio_black = color_to_pio_format(COLOR_BLACK, 1.0f);
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        pixel_buffer[i] = pio_black;
    }
    update_matrix();
}

// Desenha um pedestre andando em cor verde
void led_matrix_ped_walk() {
    uint32_t pio_green = color_to_pio_format(COLOR_GREEN, 0.25f);
    uint32_t pio_black = color_to_pio_format(COLOR_BLACK, 1.0f);

    //apaga antes para evitar erro de cores
    for (int i = 0; i < MATRIX_SIZE; ++i) { pixel_buffer[i] = pio_black; }

    led_active_position(1, 3, pio_green);
    led_active_position(3, 1, pio_green);
    led_active_position(1, 5, pio_green);
    led_active_position(2, 3, pio_green);
    led_active_position(3, 3, pio_green);
    led_active_position(4, 3, pio_green);
    led_active_position(2, 2, pio_green);
    led_active_position(2, 4, pio_green);
    led_active_position(5, 2, pio_green);
    led_active_position(5, 4, pio_green);

    update_matrix();
}

// Desenha um pedestre parado em vermelho
void led_matrix_ped_dont_walk(bool flash_state) {
    uint32_t pio_red = flash_state ? color_to_pio_format(COLOR_RED, 0.25f) : color_to_pio_format(COLOR_BLACK, 1.0f);
    uint32_t pio_black = color_to_pio_format(COLOR_BLACK, 1.0f);
    
    //apaga antes para evitar erro de cores
    for (int i = 0; i < MATRIX_SIZE; ++i) { pixel_buffer[i] = pio_black; }

    led_active_position(1, 3, pio_red); 
    led_active_position(2, 3, pio_red); 
    led_active_position(3, 3, pio_red);
    led_active_position(4, 3, pio_red); 
    led_active_position(2, 2, pio_red); 
    led_active_position(2, 4, pio_red); 
    led_active_position(5, 2, pio_red); 
    led_active_position(5, 4, pio_red); 

    update_matrix();
}