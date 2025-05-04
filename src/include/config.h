#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include "FreeRTOSConfig.h" 
#include "buttons.h"
#include "buzzer.h"
#include "led_matrix.h"


#define LED_RED_PIN     13 
#define LED_GREEN_PIN   11 
#define LED_BLUE_PIN    12 

// Botões
#define BUTTON_A_PIN    5 // troca de modo
#define BUTTON_B_PIN    6 

// Buzzer
#define BUZZER_PIN_1 10
#define BUZZER_PIN_2 21

// LED Matrix
#define MATRIX_WS2812_PIN 7
#define MATRIX_SIZE       25
#define MATRIX_DIM        5

// Display
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define DISPLAY_ADDR 0x3C
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64


// --- Tempos definidos (ms) conforme solicitado no enunciado ---
// modo normal
#define TIME_CARS_GREEN_MS         7000
#define TIME_CARS_YELLOW_MS        2000
#define TIME_ALL_RED_MS            4000
#define TIME_PEDS_WALK_MS          6000
#define TIME_PEDS_FLASH_MS         3000
#define TIME_PEDS_FLASH_INTERVAL_MS 500

// modo noturno
#define TIME_NIGHT_FLASH_ON_MS     500
#define TIME_NIGHT_FLASH_OFF_MS    500

// --- tempos do buzzer e frequências ---

// Modo Normal - Verde (1 beep curto por segundo)
#define BUZZER_WALK_FREQ           440 
#define BUZZER_WALK_ON_MS        150   // Beep curto LIGADO
#define BUZZER_WALK_OFF_MS       850   // Pausa longa DESLIGADO (150 + 850 = 1000ms = 1s)

// Modo Normal - Amarelo/Pedestre Piscando (beep rápido intermitente)
#define BUZZER_FLASH_FREQ          659  
#define BUZZER_FLASH_ON_MS       150   // LIGADO curto
#define BUZZER_FLASH_OFF_MS      150   // DESLIGADO curto (150+150 = 300ms ciclo rápido)

// PARE para Pedestre
#define BUZZER_STOP_FREQ         1200 
#define BUZZER_STOP_ON_MS        500   // Ligado 
#define BUZZER_STOP_OFF_MS       1500  // Desligado - Total 2000ms ciclo

// Modo Noturno (beep lento a cada 2s)
#define BUZZER_NIGHT_FREQ          1440 
#define BUZZER_NIGHT_ON_MS       200   // Beep LIGADO um pouco mais longo
#define BUZZER_NIGHT_OFF_MS      1800  // Pausa DESLIGADO longa (200 + 1800 = 2000ms = 2s)

// --- tempos de delay das tarefas ---
#define DEBOUNCE_TIME_US           20000
#define BUTTON_TASK_DELAY_MS       20
#define DISPLAY_UPDATE_DELAY_MS    250
#define RGB_LED_TASK_DELAY_MS      50
#define MATRIX_TASK_DELAY_MS       100
#define BUZZER_TASK_BASE_DELAY_MS  50


// prioridades
#define PRIORIDADE_CONTROLLER     (tskIDLE_PRIORITY + 4)
#define PRIORIDADE_BUTTONS        (tskIDLE_PRIORITY + 3)
#define PRIORIDADE_RGB_LED        (tskIDLE_PRIORITY + 2)
#define PRIORIDADE_MATRIX         (tskIDLE_PRIORITY + 2)
#define PRIORIDADE_BUZZER         (tskIDLE_PRIORITY + 1)
#define PRIORIDADE_DISPLAY        (tskIDLE_PRIORITY + 0)

//tamanho das stacks
#define STACK_MULTIPLIER_DEFAULT  2
#define STACK_MULTIPLIER_DISPLAY  4
#define STACK_SIZE_DEFAULT        (configMINIMAL_STACK_SIZE * STACK_MULTIPLIER_DEFAULT)
#define STACK_SIZE_DISPLAY        (configMINIMAL_STACK_SIZE * STACK_MULTIPLIER_DISPLAY)

#endif // HARDWARE_CONFIG_H