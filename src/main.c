#include "config.h"
#include "display.h"

/**
 * @brief Enumeração dos possíveis estados do semáforo para veículos e pedestres.
 */
typedef enum {
    CARS_GREEN_LIGHT,        /**< Carro: sinal verde. Pedestre: pare. */
    CARS_YELLOW_LIGHT,       /**< Carro: sinal amarelo. Pedestre: pare. */
    CARS_PED_RED_LIGHT,      /**< Ambos os sinais vermelhos. */
    CARS_RED_PEDS_WALK,      /**< Carro: vermelho. Pedestre: siga. */
    CARS_RED_PEDS_FLASH,     /**< Carro: vermelho. Pedestre: sinal piscante. */
    CARS_NIGHT_FLASHING      /**< Modo noturno: amarelo piscando. */
} TrafficLight_states;


volatile bool g_flagModoNoturno = false; //Flag global que indica se o modo noturno está ativado.
volatile TrafficLight_states trafficLight_state = CARS_PED_RED_LIGHT; //Estado atual do semáforo. inicia com ambos os sinais em vermelho
static ssd1306_t display; //controle do display

//Inicializa todos os sistemas: UART, botões, buzzer, matriz de LEDs, display e LEDs RGB.
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

/**
 * @brief Tarefa responsável por atualizar o conteúdo exibido no display OLED.
 *        Mostra o modo atual (Normal/Noturno) e o estado dos semáforos.
 */
void vDisplayUpdateTask() {
    ssd1306_t *ssd = &display;
    char mode_str[15];
    char state_str[40];
    printf("TASK: Display Update started (in main.c).\n");

    while (true) {
        bool is_night_mode = g_flagModoNoturno;
        TrafficLight_states current_state = trafficLight_state;

        // Define as strings a serem exibidas com base no modo e estado
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
                default:                       strcpy(state_str, "Erro no semaforo"); break;
            }
        }

        // Limpa e redesenha o display
        ssd1306_fill(ssd, false);
        ssd1306_rect(ssd, 0, 0, 127, 63, true, 0);
        ssd1306_draw_string(ssd, mode_str, 5, 8);
        ssd1306_hline(ssd, 1, 126, 31, true);
        ssd1306_draw_string(ssd, state_str, 5, 36);
        ssd1306_send_data(ssd);

        // Aguarda antes da próxima atualização
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_DELAY_MS));
    }
}

/**
 * @brief Tarefa responsável por monitorar o botão A e alternar entre modo normal e noturno.
 *        Toca um breve som no buzzer ao alternar o modo.
 */
void vButtonTask() {
    while (true) {
        // Verifica se o botão A foi pressionado
        if (button_a_pressed()) {
            // Inverte o estado do modo noturno
            g_flagModoNoturno = !g_flagModoNoturno;
            printf("Modo Noturno: %s\n", g_flagModoNoturno ? "ON" : "OFF");
            // Toca um tom curto para indicar a mudança
            buzzer_play_tone(440, 30);
        }
        // Aguarda antes de verificar novamente
        vTaskDelay(pdMS_TO_TICKS(BUTTON_TASK_DELAY_MS));
    }
}

/**
 * @brief Tarefa de controle geral que realiza a transição dos estados do semáforo.
 *        Avança pelos estados normais ou entra/mantém o estado noturno piscante.
 */
void vGeneralControlTask() {
    // Inicializa com um estado definido e sua duração
    TrafficLight_states current_state = CARS_PED_RED_LIGHT;
    trafficLight_state = current_state;
    uint32_t current_state_duration_ms = TIME_ALL_RED_MS;

    while (true) {
        // Aguarda a duração do estado atual
        vTaskDelay(pdMS_TO_TICKS(current_state_duration_ms));
        // Lê o estado do modo noturno
        bool night_mode_active = g_flagModoNoturno;
        TrafficLight_states next_state;

        // Lógica para modo noturno
        if (night_mode_active) {
            // Se não estiver no modo noturno, muda para ele
            if (current_state != CARS_NIGHT_FLASHING) {
                next_state = CARS_NIGHT_FLASHING;
                current_state_duration_ms = 100; // Duração pequena para o estado de transição (não usado diretamente no loop)
            } else {
                // Se já está no modo noturno, apenas continua nele
                current_state = CARS_NIGHT_FLASHING;
                trafficLight_state = current_state;
                vTaskDelay(pdMS_TO_TICKS(100)); // Pequeno delay para evitar busy-waiting
                continue; // Volta ao início do loop sem mudar o estado
            }
        } else { // Lógica para modo normal
            // Se estava saindo do modo noturno, volta para um estado inicial seguro
            if (current_state == CARS_NIGHT_FLASHING) {
                current_state = CARS_PED_RED_LIGHT;
                current_state_duration_ms = TIME_ALL_RED_MS;
                // Não define next_state aqui, pois o switch fará isso
            }

            // Determina o próximo estado e sua duração com base no estado atual
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
                    // Nota: TIME_CARS_REDYELLOW_MS parece um nome inadequado se leva ao VERDE.
                    // Deveria ser talvez TIME_BEFORE_CARS_GREEN_MS ou similar.
                    // Usando a duração configurada antes de ir para o verde.
                    current_state_duration_ms = TIME_CARS_REDYELLOW_MS;
                    next_state = CARS_GREEN_LIGHT;
                    break;
                default: // Estado de erro ou inesperado
                    printf("Erro na mudança dos estados\n");
                    current_state = CARS_PED_RED_LIGHT; // Volta para um estado seguro
                    current_state_duration_ms = TIME_ALL_RED_MS;
                    next_state = CARS_PED_RED_LIGHT;
                    break;
            }
        }

        // Atualiza o estado atual para o próximo estado calculado
        current_state = next_state;
        // Atualiza a variável global de estado para outras tarefas
        trafficLight_state = current_state;
    }
}

/**
 * @brief Tarefa responsável por controlar o LED RGB que representa o semáforo dos veículos.
 *        Acende as cores Verde, Amarelo ou Vermelho conforme o estado, ou pisca Amarelo no modo noturno.
 */
void vRgbLedTask() {
    bool yellow_flash_state = false; // Estado do pisca-pisca amarelo (ligado/desligado)
    uint32_t last_flash_time = 0; // Tempo do último pisca

    while(true) {
        // Lê o estado atual do semáforo
        TrafficLight_states tf_state = trafficLight_state;
        uint32_t current_tick_time = xTaskGetTickCount(); // Tempo atual em ticks

        // Controla o LED RGB com base no estado
        switch(tf_state) {
            case CARS_GREEN_LIGHT: // Verde
                gpio_put(LED_RED_PIN, 0); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 1);
                break;
            case CARS_YELLOW_LIGHT: // Amarelo (Vermelho + Verde)
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 1);
                break;
            case CARS_PED_RED_LIGHT: // Vermelho
            case CARS_RED_PEDS_WALK: // Vermelho
            case CARS_RED_PEDS_FLASH: // Vermelho
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 0);
                break;
            case CARS_NIGHT_FLASHING: // Amarelo Piscando
                // Verifica se passou tempo suficiente para inverter o estado do pisca
                if ((current_tick_time - last_flash_time) >= pdMS_TO_TICKS(TIME_NIGHT_FLASH_ON_MS)) {
                    yellow_flash_state = !yellow_flash_state; // Inverte o estado
                    gpio_put(LED_BLUE_PIN, 0); // Azul sempre desligado
                    // Acende ou apaga Vermelho e Verde juntos para formar Amarelo
                    gpio_put(LED_GREEN_PIN, yellow_flash_state);
                    gpio_put(LED_RED_PIN, yellow_flash_state);
                    last_flash_time = current_tick_time; // Atualiza o tempo do último pisca
                }
                break;
            default: // Estado inesperado, assume Vermelho por segurança
                gpio_put(LED_RED_PIN, 1); gpio_put(LED_BLUE_PIN, 0); gpio_put(LED_GREEN_PIN, 0);
                break;
        }

        // Aguarda antes da próxima verificação
        vTaskDelay(pdMS_TO_TICKS(RGB_LED_TASK_DELAY_MS));
    }
}

/**
 * @brief Tarefa que controla a matriz de LEDs indicando o estado do semáforo de pedestres.
 *        Mostra "Ande" (Walk), "Pare" (Don't Walk) ou pisca "Pare", ou apaga no modo noturno.
 */
void vLedMatrixTask() {
    printf("TASK: LED Matrix started.\n");
    bool ped_flash_state = false; // Estado do pisca-pisca do pedestre (ligado/desligado)
    uint32_t last_ped_flash_time = 0; // Tempo do último pisca do pedestre
    // Guarda a fase anterior para detectar mudanças e reiniciar o pisca
    TrafficLight_states last_known_phase = CARS_GREEN_LIGHT;

    while(true) {
        // Lê o estado atual do semáforo
        TrafficLight_states tf_state = trafficLight_state;
        uint32_t current_tick_time = xTaskGetTickCount(); // Tempo atual em ticks

        // Se a fase mudou, reinicia o estado do pisca e o tempo
        if (tf_state != last_known_phase) {
            // Garante que o pisca comece no estado correto (normalmente apagado no início do flash)
            ped_flash_state = (tf_state == CARS_RED_PEDS_FLASH) ? false : true; // Inicia piscando ou mostra estático
            last_ped_flash_time = current_tick_time; // Reseta o timer do pisca
            last_known_phase = tf_state; // Atualiza a última fase conhecida
            printf("MATRIX: Phase changed to %d\n", tf_state); // Log da mudança
        }

        // Controla a matriz de LEDs com base no estado
        switch(tf_state) {
            case CARS_RED_PEDS_WALK: // Pedestre: Siga (Walk)
                led_matrix_ped_walk();
                break;
            case CARS_RED_PEDS_FLASH: // Pedestre: Pisca Vermelho (Don't Walk Flashing)
                // Verifica se passou metade do intervalo do pisca para inverter o estado
                if ((current_tick_time - last_ped_flash_time) >= pdMS_TO_TICKS(TIME_PEDS_FLASH_INTERVAL_MS / 2)) {
                    ped_flash_state = !ped_flash_state; // Inverte o estado do pisca
                    // Mostra ou apaga o ícone "Don't Walk"
                    led_matrix_ped_dont_walk(ped_flash_state);
                    last_ped_flash_time = current_tick_time; // Atualiza o tempo do último pisca
                }
                // Garante que se o estado for true (mostrar ícone), ele permaneça visível até o próximo flip
                // (Esta condição pode ser redundante dependendo da implementação exata do `led_matrix_ped_dont_walk`)
                else if (ped_flash_state != false) {
                     led_matrix_ped_dont_walk(true);
                }
                break;
            case CARS_NIGHT_FLASHING: // Modo Noturno: Matriz apagada
                led_matrix_clear();
                break;
            case CARS_GREEN_LIGHT:    // Carro Verde => Pedestre: Pare
            case CARS_YELLOW_LIGHT:   // Carro Amarelo => Pedestre: Pare
            case CARS_PED_RED_LIGHT:  // Ambos Vermelhos => Pedestre: Pare
            default:                  // Estado Padrão/Erro => Pedestre: Pare
                led_matrix_ped_dont_walk(true); // Mostra "Don't Walk" estático
                break;
        }
        // Aguarda antes da próxima atualização
        vTaskDelay(pdMS_TO_TICKS(MATRIX_TASK_DELAY_MS));
    }
}

/**
 * @brief Tarefa que controla o buzzer para emitir sons de alerta para pedestres.
 *        Usa lógica simplificada baseada em ciclos ON/OFF para diferentes fases.
 */
void vBuzzerTask() {
    printf("TASK: Buzzer Controller started (Simplified).\n");

    // Guarda o tick do FreeRTOS quando a fase que precisa de som começou.
    uint32_t phase_start_tick = 0;
    // Guarda a última fase conhecida para detectar mudanças.
    TrafficLight_states last_known_phase_buz = trafficLight_state;
    // Flag interna para decidir se o som deve ser tocado nesta iteração.
    bool need_to_play_sound = false;
    // Define um tempo de espera fixo para o loop da tarefa (em ms).
    const uint32_t LOOP_DELAY_MS = 50;
    const TickType_t loop_delay_ticks = pdMS_TO_TICKS(LOOP_DELAY_MS);

    // Inicializa o contador de ticks de início da fase.
    phase_start_tick = xTaskGetTickCount();

    while(true) {
         // Lê a fase atual do semáforo.
         TrafficLight_states current_phase = trafficLight_state;
         // Obtém o tempo atual em ticks do FreeRTOS.
         uint32_t current_tick = xTaskGetTickCount();
         // Variáveis para guardar os parâmetros do som da fase atual.
         uint32_t freq = 0;
         uint32_t on_duration_ms = 0;
         uint32_t off_duration_ms = 0;
         uint32_t cycle_duration_ms = 0;

         // --- Resetar Timer na Mudança de Fase ---
         if (current_phase != last_known_phase_buz) {
              last_known_phase_buz = current_phase;
              phase_start_tick = current_tick;
              need_to_play_sound = false; // Começa silencioso
              buzzer_play_tone(0, 0); // Garante que o buzzer está desligado
         }

         // --- Determina os Parâmetros do Som para a Fase Atual ---
         switch(current_phase) {
             case CARS_RED_PEDS_WALK: // Pedestre Anda
                 freq = BUZZER_WALK_FREQ;
                 on_duration_ms = BUZZER_WALK_ON_MS;
                 off_duration_ms = BUZZER_WALK_OFF_MS;
                 break;
             case CARS_RED_PEDS_FLASH: // Pedestre Pisca Vermelho
                 freq = BUZZER_FLASH_FREQ;
                 on_duration_ms = BUZZER_FLASH_ON_MS;
                 off_duration_ms = BUZZER_FLASH_OFF_MS;
                 break;
             case CARS_NIGHT_FLASHING: // Noturno Piscante (som opcional)
                 freq = BUZZER_NIGHT_FREQ; // Pode ser 0 se não houver som noturno
                 on_duration_ms = BUZZER_NIGHT_ON_MS;
                 off_duration_ms = BUZZER_NIGHT_OFF_MS;
                 break;
             default: // Fases silenciosas
                 freq = 0;
                 break;
         }

        // --- Decide se o som deve estar LIGADO ou DESLIGADO ---
        if (freq > 0) { // Se a fase deve ter som
             cycle_duration_ms = on_duration_ms + off_duration_ms;
             if (cycle_duration_ms > 0) { // Evita divisão por zero
                 uint32_t elapsed_ticks_in_phase = current_tick - phase_start_tick;
                 // Converte ticks para ms de forma segura
                 uint32_t elapsed_ms_in_phase = (uint32_t)(((uint64_t)elapsed_ticks_in_phase * 1000) / configTICK_RATE_HZ);
                 // Calcula a posição dentro do ciclo ON/OFF
                 uint32_t time_in_current_cycle = elapsed_ms_in_phase % cycle_duration_ms;
                 // Verifica se está na porção ON do ciclo
                 need_to_play_sound = (time_in_current_cycle < on_duration_ms);
             } else {
                  // Ciclo inválido (on+off = 0), mas freq > 0? Assume som contínuo.
                  need_to_play_sound = true;
             }
        } else { // Fase silenciosa
             need_to_play_sound = false;
        }

        // --- Aciona o Buzzer ---
        // Liga ou desliga o PWM do buzzer sem bloquear.
        buzzer_play_tone(need_to_play_sound ? freq : 0, 0);

        // --- Delay Fixo ---
        vTaskDelay(loop_delay_ticks);
    }
}

/**
 * @brief Função principal: inicializa o sistema e cria todas as tarefas do FreeRTOS.
 *        Após a criação das tarefas, inicia o escalonador.
 * @return int (Nunca retorna, pois o escalonador assume o controle).
 */
int main() {
    // Inicializa hardware e periféricos
    init_system_all();
    // Mostra tela de inicialização no display
    display_startup_screen(&display);
    printf("Creating tasks...\n");

    // Cria as tarefas do sistema com suas prioridades
    xTaskCreate(vGeneralControlTask, "CtrlTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_CONTROLLER, NULL);
    xTaskCreate(vButtonTask, "ButtonTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_BUTTONS, NULL);
    xTaskCreate(vRgbLedTask, "RgbLedTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_RGB_LED, NULL);
    xTaskCreate(vLedMatrixTask, "MatrixTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_MATRIX, NULL);
    xTaskCreate(vBuzzerTask, "BuzzerTask", STACK_SIZE_DEFAULT, NULL, PRIORIDADE_BUZZER, NULL);
    xTaskCreate(vDisplayUpdateTask, "DisplayTask", STACK_SIZE_DISPLAY, NULL , PRIORIDADE_DISPLAY, NULL);

    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // O código abaixo nunca deve ser alcançado
    while(1);
    //return 0; // Inacessível
}