#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "buzzer.h"
#include "config.h"


// --- Internal Definitions ---
// Success Melody (Optional startup sound)
static const uint success_melody[]    = {523, 659, 784};
static const uint success_durations[] = {100, 100, 200}; // Shorter
static const size_t success_len       = sizeof(success_melody) / sizeof(success_melody[0]);

// --- Static Helper Functions ---
static void play_tone_internal(uint freq, uint duration_ms) {
    if (freq == 0) {
        if (duration_ms > 0) sleep_ms(duration_ms); // Use FreeRTOS vTaskDelay if called from task context!
        return;
    }

    gpio_set_function(BUZZER_PIN_MAIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN_MAIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN_MAIN);

    uint32_t clock = clock_get_hz(clk_sys);
    uint32_t divider16 = clock * 16 / freq;
    uint32_t wrap_val = 65535;
    uint clk_div = 1;

    while (divider16 >= 16 * wrap_val && clk_div < 256) {
        clk_div++;
        divider16 = clock * 16 / (freq * clk_div);
    }
    if (divider16 < 16) divider16 = 16;
    wrap_val = divider16 / 16;

    pwm_set_clkdiv_int_frac(slice_num, clk_div, 0);
    pwm_set_wrap(slice_num, wrap_val);
    pwm_set_chan_level(slice_num, channel, wrap_val / 2); // 50% duty cycle
    pwm_set_enabled(slice_num, true);

    if (duration_ms > 0) {
        sleep_ms(duration_ms); // Use FreeRTOS vTaskDelay if called from task context!
    }

    pwm_set_enabled(slice_num, false);
}

static void play_sequence_internal(const uint *melody, const uint *durations, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        play_tone_internal(melody[i], durations[i]);
        if (i < len - 1) {
            sleep_ms(30); // Shorter pause
        }
    }
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN_MAIN);
    pwm_set_enabled(slice_num, false);
}

// --- Public API Functions ---
void buzzer_init() {
    gpio_init(BUZZER_PIN_MAIN);
    gpio_set_dir(BUZZER_PIN_MAIN, GPIO_OUT);
    gpio_put(BUZZER_PIN_MAIN, 0);
    // buzzer_play_success(); // Play startup sound?
}

void buzzer_play_success() {
    play_sequence_internal(success_melody, success_durations, success_len);
}

void buzzer_play_tone(uint freq, uint duration_ms) {
    play_tone_internal(freq, duration_ms);
}