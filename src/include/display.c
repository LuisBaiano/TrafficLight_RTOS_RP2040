#include "display.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"

#define ICON_X_START         40
#define ICON_Y_START         44
#define ICON_BODY_W          48
#define ICON_BODY_H          16
#define ICON_LIGHT_SQUARE    10
#define ICON_LIGHT_PAD        2

/**
  * @brief Desenha um ícone de semáforo no display OLED.
  *
  * @param ssd Ponteiro para a estrutura de controle do display SSD1306.
  */
 static void draw_trafficlight(ssd1306_t *ssd) {
    uint8_t body_y_coord = ICON_Y_START;
    uint8_t body_x_coord = ICON_X_START;
    // Coordenada Y (vertical) das luzes, centralizadas dentro do corpo
    uint8_t light_y_coord = ICON_Y_START + (ICON_BODY_H - ICON_LIGHT_SQUARE) / 2;
    // Calcula o espaço horizontal
    uint8_t total_lights_width = 3 * ICON_LIGHT_SQUARE;
    uint8_t total_padding_space = (ICON_BODY_W > total_lights_width) ? (ICON_BODY_W - total_lights_width) : 0;
    uint8_t padding_each = total_padding_space / 4;
    // Calcula as coordenadas X (horizontal) para cada luz
    uint8_t luz_vermelha_x_coord = ICON_X_START + padding_each;
    uint8_t luz_amarela_x_coord  = luz_vermelha_x_coord + ICON_LIGHT_SQUARE + padding_each;
    uint8_t luz_verde_x_coord    = luz_amarela_x_coord + ICON_LIGHT_SQUARE + padding_each;
    // Desenha o retangulo externo (contorno)
    ssd1306_rect(ssd, body_y_coord, body_x_coord, ICON_BODY_W, ICON_BODY_H, true, false);
    // Desenha o quadrado das luzes (contorno)
    ssd1306_rect(ssd, light_y_coord, luz_vermelha_x_coord, ICON_LIGHT_SQUARE, ICON_LIGHT_SQUARE, true, false);
    ssd1306_rect(ssd, light_y_coord, luz_amarela_x_coord, ICON_LIGHT_SQUARE, ICON_LIGHT_SQUARE, true, false);
    ssd1306_rect(ssd, light_y_coord, luz_verde_x_coord, ICON_LIGHT_SQUARE, ICON_LIGHT_SQUARE, true, false);
}
/**
  * @brief Inicializa a comunicação I2C e o display OLED SSD1306.
  *        Configura os pinos SDA e SCL, inicializa o periférico I2C e
  *        envia os comandos de configuração para o display.
  *
  * @param ssd Ponteiro para a estrutura de controle do display SSD1306 a ser inicializada.
  */
 void display_init(ssd1306_t *ssd) {
     // Inicializa I2C na porta e velocidade definidas
     i2c_init(I2C_PORT, 400 * 1000);
     // Configura os pinos GPIO para a função I2C
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
     // Habilita resistores de pull-up internos para os pinos I2C
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
     // Inicializa a estrutura do driver SSD1306 com os parâmetros do display
    ssd1306_init(ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
     // Envia a sequência de comandos de configuração para o display
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
    printf("Display inicializado.\n");
}

/**
  * @brief Exibe uma tela de inicialização no display OLED.
  *        Mostra um texto de título e o ícone do semáforo por alguns segundos.
  *
  * @param ssd Ponteiro para a estrutura de controle do display SSD1306.
  */
 void display_startup_screen(ssd1306_t *ssd) {
     // Limpa o display
    ssd1306_fill(ssd, false);
     // Calcula posições aproximadas para centralizar o texto
    uint8_t center_x_approx = ssd->width / 2;
    uint8_t start_y = 8;
     uint8_t line_height = 10; // Espaçamento vertical entre linhas

     // Define as strings a serem exibidas
     const char *line1 = "EMBARCATECH";
     const char *line2 = "PROJETO";
     const char *line3 = "SEMAFORO RTOS";
    ssd1306_draw_string(ssd, line1, center_x_approx - (strlen(line1)*8)/2, start_y);
    ssd1306_draw_string(ssd, line2, center_x_approx - (strlen(line2)*8)/2, start_y + line_height);
    ssd1306_draw_string(ssd, line3, center_x_approx - (strlen(line3)*8)/2, start_y + 2*line_height);
    
    // Desenha o semáforo
    draw_trafficlight(ssd);
    ssd1306_send_data(ssd);
    // Mantém a tela visível por um tempo
    sleep_ms(2500);
    // Limpa o display após a tela de inicialização
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}