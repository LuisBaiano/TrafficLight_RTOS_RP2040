#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "buttons.h"
#include "config.h"
#include "debouncer.h"
#include "pico/bootrom.h"


static volatile bool flag_button_a = false;
static volatile bool flag_button_b = false;
// static volatile bool flag_joystick_button = false;

static uint32_t last_press_time_a = 0;
static uint32_t last_press_time_b = 0;
// static uint32_t last_press_time_joy = 0;

static void buttons_irq_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        switch (gpio) {
            case BUTTON_A_PIN:
                if (check_debounce(&last_press_time_a, DEBOUNCE_TIME_US)) {
                    flag_button_a = true;
                }
                break;
            case BUTTON_B_PIN:
                if (check_debounce(&last_press_time_b, DEBOUNCE_TIME_US)) {
                    flag_button_b = true;
                    reset_usb_boot(0, 0);
                }
                break;
            default:
                break;
        }
    }
}

void buttons_init() {
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &buttons_irq_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &buttons_irq_callback);
}

bool button_a_pressed() {
    if (flag_button_a) {
        flag_button_a = false;
        return true;
    }
    return false;
}

bool button_b_pressed() {
    if (flag_button_b) {
        flag_button_b = false;
        return true;
    }
    return false;
}
