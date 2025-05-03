#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

// Project includes
#include "config.h"
#include "buttons.h"
#include "buzzer.h"
#include "display.h"
#include "led_matrix.h"

//define os estados possiveis para os semaforos (carro e pedestre)
typedef enum {
    CARS_GREEN_LIGHT,
    CARS_YELLOW_LIGHT,
    CARS_PED_RED_LIGHT,
    CARS_RED_PEDS_WALK,
    CARS_RED_PEDS_FLASH,
    CARS_REDYELLOW,
    CARS_NIGHT_FLASHING
} TrafficLight_states;

// --- Global State ---
volatile bool g_flagModoNoturno = false; //flag para alternar entre o modo noturno e o normal
volatile TrafficLight_states trafficLight_state = CARS_PED_RED_LIGHT; //inicia com ambos os sinais em vermelho
static ssd1306_t display;

// --- System Initialization ---
void init_system_all() {
    stdio_init_all();
    sleep_ms(1000);
    printf("Sistema de semáforo inicializado!\n");

    buttons_init();
    buzzer_init();
    led_matrix_init();
    display_init(&display);
    gpio_init(LED_RED_PIN); gpio_set_dir(LED_RED_PIN, GPIO_OUT); gpio_put(LED_RED_PIN, 1);
    gpio_init(LED_GREEN_PIN); gpio_set_dir(LED_GREEN_PIN, GPIO_OUT); gpio_put(LED_GREEN_PIN, 0);
    gpio_init(LED_BLUE_PIN); gpio_set_dir(LED_BLUE_PIN, GPIO_OUT); gpio_put(LED_BLUE_PIN, 0);
}

// --- Tarefas por cada periférico ---

void vDisplayUpdateTask(void *pvParameters) {
    ssd1306_t *ssd = &display;
    char mode_str[15];
    char state_str[40];
    printf("TASK: Display Update started (in main.c).\n");

    while (true) {

        bool is_night_mode = g_flagModoNoturno;
        TrafficLight_states current_state = trafficLight_state;
    

        if (is_night_mode) {
            strcpy(mode_str, "MODO: NOTURNO");
            strcpy(state_str, "Carro: Amarelo Piscando.");
        } else {
            strcpy(mode_str, "MODO: NORMAL ");
            switch(current_state) {
                case CARS_GREEN_LIGHT:         strcpy(state_str, "Carro: Verde    \nPed: Pare"); break;
                case CARS_YELLOW_LIGHT:        strcpy(state_str, "Carro: Amarelo  \nPed: Pare"); break;
                case CARS_PED_RED_LIGHT:       strcpy(state_str, "Carro: Vermelho \nPed: Pare"); break;
                case CARS_RED_PEDS_WALK:       strcpy(state_str, "Carro: Vermelho \nPed: Siga"); break;
                case CARS_RED_PEDS_FLASH:      strcpy(state_str, "Carro: Vermelho \nPed: Piscando"); break;
                case CARS_REDYELLOW:           strcpy(state_str, "Carro: Verm.+Amar."); break;
                default:                       strcpy(state_str, "Erro no semaforo"); break;
            }
        }
        //limpa display
        ssd1306_fill(ssd, false);
        ssd1306_rect(ssd, 0, 0, 127, 63, true, 0);
        ssd1306_draw_string(ssd, mode_str, 5, 8);
        ssd1306_hline(ssd, 1, 126, 31, true);
        ssd1306_draw_string(ssd, state_str, 5, 36);
        ssd1306_send_data(ssd);

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_DELAY_MS));
    }
}

// Tarefa do botão A que alterna entre modo normal(dia) e noturno.
void vButtonTask(void *pvParameters) {
    while (true) {
        
        if (button_a_pressed()) { 
    
            g_flagModoNoturno = !g_flagModoNoturno;
        
            printf("Modo Noturno: %s\n", g_flagModoNoturno ? "ON" : "OFF");
            buzzer_play_tone(440, 30);
        }
        vTaskDelay(pdMS_TO_TICKS(BUTTON_TASK_DELAY_MS));
    }
}

void vGeneralControlTask(void *pvParameters) {
    TrafficLight_states current_state = CARS_PED_RED_LIGHT;
    trafficLight_state = current_state;

    uint32_t current_state_duration_ms = TIME_ALL_RED_MS;

    while (true) {
        // --- Aguarda o tempo do estado atual ---
        vTaskDelay(pdMS_TO_TICKS(current_state_duration_ms));

        // --- Verifica se o modo noturno está ativo ---
        bool night_mode_active = g_flagModoNoturno;

        TrafficLight_states next_state;

        if (night_mode_active) {
            // Se ainda não está piscando, ativa o modo noturno
            if (current_state != CARS_NIGHT_FLASHING) {
                next_state = CARS_NIGHT_FLASHING;
            } else {
                // Continua piscando o amarelo
                current_state = CARS_NIGHT_FLASHING;
                trafficLight_state = current_state;
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }
        } else {
            // Se estava no modo noturno, reinicia o ciclo normal
            if (current_state == CARS_NIGHT_FLASHING) {
                current_state = CARS_PED_RED_LIGHT;
                current_state_duration_ms = TIME_ALL_RED_MS;
            }

            // Ciclo normal do semáforo
            switch (current_state) {
                case CARS_GREEN_LIGHT:
                    current_state_duration_ms = TIME_CARS_YELLOW_MS;
                    next_state = CARS_YELLOW_LIGHT;
                    break;

                case CARS_YELLOW_LIGHT:
                    current_state_duration_ms = TIME_ALL_RED_MS;
                    next_state = CARS_PED_RED_LIGHT;
                    break;

                case CARS_PED_RED_LIGHT:
                    current_state_duration_ms = TIME_PEDS_WALK_MS;
                    next_state = CARS_RED_PEDS_WALK;
                    break;

                case CARS_RED_PEDS_WALK:
                    current_state_duration_ms = TIME_PEDS_FLASH_MS;
                    next_state = CARS_RED_PEDS_FLASH;
                    break;

                case CARS_RED_PEDS_FLASH:
                    current_state_duration_ms = TIME_CARS_REDYELLOW_MS;
                    next_state = CARS_REDYELLOW;
                    break;

                case CARS_REDYELLOW:
                    current_state_duration_ms = TIME_CARS_GREEN_MS;
                    next_state = CARS_GREEN_LIGHT;
                    break;

                default:
                    printf("Erro na mudança dos estados\n");
                    current_state = CARS_PED_RED_LIGHT;
                    current_state_duration_ms = TIME_ALL_RED_MS;
                    next_state = CARS_PED_RED_LIGHT;
                    break;
            }
        }

        current_state = next_state;
        trafficLight_state = current_state;
    }
}



// Tarefa do led RGB que representa as luzes do semaforo dos carros
void vRgbLedTask(void *pvParameters) {
    bool yellow_flash_state = false;
    uint32_t last_flash_time = 0;

    while(true) {
        TrafficLight_states tf_state = trafficLight_state;

        uint32_t current_tick_time = xTaskGetTickCount();
        // muda de acordo com o estado 
        switch(tf_state) {
            case CARS_GREEN_LIGHT:
                gpio_put(LED_RED_PIN, 0); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 1);
                break;
            case CARS_YELLOW_LIGHT:
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 1);
                break;
            case CARS_PED_RED_LIGHT:
            case CARS_RED_PEDS_WALK:
            case CARS_RED_PEDS_FLASH:
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 0);
                break;
            case CARS_REDYELLOW:
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 1); gpio_put(LED_GREEN_PIN, 1); // Amarelo +  Azul
                break;
            case CARS_NIGHT_FLASHING:
                //Pisca a luz amarela no modo noturno
                if ((current_tick_time - last_flash_time) >= pdMS_TO_TICKS(TIME_NIGHT_FLASH_ON_MS)) {
                    yellow_flash_state = !yellow_flash_state;
                    gpio_put(LED_BLUE_PIN, 0);
                    gpio_put(LED_GREEN_PIN, yellow_flash_state);
                    gpio_put(LED_RED_PIN, yellow_flash_state); 
                    last_flash_time = current_tick_time;
                }
                break;
            default: // Default to safe state: Red
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 0);
                break;
        }
        // This task yields control periodically
        vTaskDelay(pdMS_TO_TICKS(RGB_LED_TASK_DELAY_MS));
    }
}

// Task: LED Matrix Controller (Pedestrian Light)
void vLedMatrixTask(void *pvParameters) {
    printf("TASK: LED Matrix started.\n");
    bool ped_flash_state = false;
    uint32_t last_ped_flash_time = 0;
    TrafficLight_states last_known_phase = CARS_GREEN_LIGHT; // Init to force update

    while(true) {
        TrafficLight_states tf_state = trafficLight_state;

        uint32_t current_tick_time = xTaskGetTickCount();

        // Reset flashing state if phase changes, prevent stale flash
        if (tf_state != last_known_phase) {
            ped_flash_state = (tf_state == CARS_RED_PEDS_FLASH); // Start flash cycle correctly
            last_ped_flash_time = current_tick_time;
            last_known_phase = tf_state; // Update last known phase
            printf("MATRIX: Phase changed to %d\n", tf_state);
        }

        switch(tf_state) {
            case CARS_RED_PEDS_WALK:
                led_matrix_draw_walk_symbol(); // Show solid green walk symbol
                break;
            case CARS_RED_PEDS_FLASH:
                // Non-blocking flash based on time
                // Check if half the interval has passed to toggle the state
                if ((current_tick_time - last_ped_flash_time) >= pdMS_TO_TICKS(TIME_PEDS_FLASH_INTERVAL_MS / 2)) {
                    ped_flash_state = !ped_flash_state;
                    led_matrix_draw_dont_walk_symbol(ped_flash_state); // Draw red hand or blank
                    last_ped_flash_time = current_tick_time;
                }
                // Note: if just entered this state, draw initial state immediately
                else if (ped_flash_state != false) { // Draw initial 'ON' state if needed
                    led_matrix_draw_dont_walk_symbol(true);
                }
                break;
            case CARS_NIGHT_FLASHING:
                led_matrix_clear(); // Matrix off during night mode
                break;
            case CARS_GREEN_LIGHT:
            case CARS_YELLOW_LIGHT:
            case CARS_PED_RED_LIGHT:
            case CARS_REDYELLOW:
            default:
                led_matrix_draw_dont_walk_symbol(true); // Solid Don't Walk
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(MATRIX_TASK_DELAY_MS)); // Base polling delay
    }
}

// Task: Buzzer Controller
void vBuzzerTask(void *pvParameters) {
    printf("TASK: Buzzer Controller started.\n");
    uint32_t last_sound_tick = 0;
    bool is_sound_playing = false; // Track if PWM is currently active
    TrafficLight_states last_known_phase_buz = CARS_GREEN_LIGHT; // Track phase changes

    while(true) {
         TrafficLight_states phase = trafficLight_state;

         uint32_t current_tick = xTaskGetTickCount();
         uint32_t on_duration_ticks = 0;
         uint32_t off_duration_ticks = 0;
         uint freq = 0;

         // Reset state tracking if phase changes
         if (phase != last_known_phase_buz) {
              printf("BUZZER: Phase changed to %d\n", phase);
              if (is_sound_playing) { // Turn off sound from previous phase
                  buzzer_play_tone(0, 0); // Stop PWM
              }
              is_sound_playing = false; // Reset playing flag
              last_sound_tick = current_tick; // Reset timer
              last_known_phase_buz = phase;
         }

         // Determine parameters for the current phase
         switch(phase) {
            case CARS_RED_PEDS_WALK:
                 freq = BUZZER_WALK_FREQ;
                 on_duration_ticks = pdMS_TO_TICKS(BUZZER_WALK_ON_MS);
                 off_duration_ticks = pdMS_TO_TICKS(BUZZER_WALK_OFF_MS);
                 break;
             case CARS_RED_PEDS_FLASH:
                 freq = BUZZER_FLASH_FREQ;
                 on_duration_ticks = pdMS_TO_TICKS(BUZZER_FLASH_ON_MS);
                 off_duration_ticks = pdMS_TO_TICKS(BUZZER_FLASH_OFF_MS);
                 break;
             case CARS_NIGHT_FLASHING:
                 freq = BUZZER_NIGHT_FREQ;
                 on_duration_ticks = pdMS_TO_TICKS(BUZZER_NIGHT_ON_MS);
                 off_duration_ticks = pdMS_TO_TICKS(BUZZER_NIGHT_OFF_MS);
                 break;
             default: // Silent phases
                 freq = 0;
                 // Keep is_sound_playing = false
                 break;
         }

        // Non-blocking pattern playback logic
        if (freq > 0) { // Should make sound in this phase
            if (!is_sound_playing) { // Currently silent, check if time for ON period
                 if ((current_tick - last_sound_tick) >= off_duration_ticks) {
                     buzzer_play_tone(freq, 0); // Start tone (non-blocking PWM start)
                     is_sound_playing = true;
                     last_sound_tick = current_tick; // Record start time
                 }
            } else { // Currently playing, check if time for OFF period
                 if ((current_tick - last_sound_tick) >= on_duration_ticks) {
                     buzzer_play_tone(0, 0); // Stop tone (non-blocking PWM stop)
                     is_sound_playing = false;
                     last_sound_tick = current_tick; // Record start time of OFF period
                 }
            }
        } else { // Should be silent
             if (is_sound_playing) { // Turn it off if it was left on
                 buzzer_play_tone(0, 0);
                 is_sound_playing = false;
             }
             // Allow timer to naturally reset when entering a sound-producing phase again
        }

        // Determine appropriate delay before next check
        // Aim for checking slightly faster than the shortest ON/OFF period component
        uint32_t shortest_interval_ms = 50; // Minimum check interval
        if(freq > 0) {
             uint32_t on_ms = BUZZER_WALK_ON_MS; // Find minimum of relevant timings
            uint32_t off_ms= BUZZER_WALK_OFF_MS;
            if(phase == CARS_RED_PEDS_FLASH) {on_ms = BUZZER_FLASH_ON_MS; off_ms= BUZZER_FLASH_OFF_MS;}
            else if(phase == CARS_NIGHT_FLASHING){on_ms = BUZZER_NIGHT_ON_MS; off_ms= BUZZER_NIGHT_OFF_MS;}
            uint32_t min_phase_interval = (on_ms < off_ms) ? on_ms : off_ms;
             if(min_phase_interval < shortest_interval_ms * 2 && min_phase_interval > 0)
                shortest_interval_ms = (min_phase_interval / 2 > 10) ? (min_phase_interval / 2) : 10; // Check at half the min interval
        }


        vTaskDelay(pdMS_TO_TICKS(shortest_interval_ms)); // Base check delay
    }
}

// --- Main ---
int main() {
    init_system_all();
    display_startup_screen(&display);
    printf("Creating tasks...\n");
    xTaskCreate(vGeneralControlTask, "CtrlTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_CONTROLLER, NULL);
    xTaskCreate(vButtonTask, "ButtonTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_BUTTONS, NULL);
    xTaskCreate(vRgbLedTask, "RgbLedTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_RGB_LED, NULL);
    xTaskCreate(vLedMatrixTask, "MatrixTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_MATRIX, NULL);
    xTaskCreate(vBuzzerTask, "BuzzerTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_BUZZER, NULL);
    xTaskCreate(vDisplayUpdateTask, "DisplayTask", STACK_SIZE_DISPLAY, NULL , PRIORIDADE_DISPLAY, NULL);
    vTaskStartScheduler();
    while(1);
}

