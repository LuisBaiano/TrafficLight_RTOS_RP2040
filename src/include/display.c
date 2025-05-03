#include "display.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"


#define ICON_X_START    40  
#define ICON_Y_START    36  
#define ICON_BODY_W     48  
#define ICON_BODY_H     16  
#define ICON_LIGHT_D    10  
#define ICON_LIGHT_PAD   2

// Desenha um semaforo no inicio
static void draw_trafficlight(ssd1306_t *ssd) {

    uint8_t light_y = 36 + (ICON_BODY_H + 1) / 2 - (ICON_LIGHT_D + 1) / 2;
    uint8_t luz_vermelha    = ICON_X_START + ICON_LIGHT_PAD;
    uint8_t luz_amarela = luz_vermelha + ICON_LIGHT_D + ICON_LIGHT_PAD;
    uint8_t Luz_verde  = luz_amarela + ICON_LIGHT_D + ICON_LIGHT_PAD;
    ssd1306_rect(ssd, ICON_X_START, ICON_Y_START, ICON_BODY_W, ICON_BODY_H, true, false);
    ssd1306_rect(ssd, luz_vermelha,    light_y, ICON_LIGHT_D, ICON_LIGHT_D, true, false);
    ssd1306_rect(ssd, luz_amarela, light_y, ICON_LIGHT_D, ICON_LIGHT_D, true, false);
    ssd1306_rect(ssd, Luz_verde,  light_y, ICON_LIGHT_D, ICON_LIGHT_D, true, false);
}

void display_init(ssd1306_t *ssd) { 
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    ssd1306_init(ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
    printf("Display OLED inicializado.\n");
}

// tela de inicio
void display_startup_screen(ssd1306_t *ssd) {
    ssd1306_fill(ssd, false);
    uint8_t center_x_approx = ssd->width / 2;
    uint8_t start_y = 8;
    uint8_t line_height = 10;
    const char *line1 = "EMBARCATECH";
    const char *line2 = "PROJETO";
    const char *line3 = "SEMAFORO RTOS";
    ssd1306_draw_string(ssd, line1, center_x_approx - (strlen(line1)*8)/2, start_y);
    ssd1306_draw_string(ssd, line2, center_x_approx - (strlen(line2)*8)/2, start_y + line_height);
    ssd1306_draw_string(ssd, line3, center_x_approx - (strlen(line3)*8)/2, start_y + 2*line_height);
    draw_trafficlight(ssd);
    ssd1306_send_data(ssd);
    sleep_ms(2500);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}
