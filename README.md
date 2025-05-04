# Semáforo Inteligente com Acessibilidade e Modo Noturno usando FreeRTOS na BitDogLab

## Índice

- [Semáforo Inteligente com Acessibilidade e Modo Noturno usando FreeRTOS na BitDogLab](#semáforo-inteligente-com-acessibilidade-e-modo-noturno-usando-freertos-na-bitdoglab)
  - [Índice](#índice)
  - [Objetivos](#objetivos)
  - [Descrição do Projeto](#descrição-do-projeto)
  - [Funcionalidades Implementadas](#funcionalidades-implementadas)
  - [Requisitos Técnicos Atendidos](#requisitos-técnicos-atendidos)
  - [Como Executar](#como-executar)
    - [Requisitos de Hardware](#requisitos-de-hardware)
    - [Requisitos de Software](#requisitos-de-software)
    - [Passos](#passos)
  - [Estrutura do Código](#estrutura-do-código)
  - [Demonstrativo em Vídeo](#demonstrativo-em-vídeo)

## Objetivos

*   Aplicar os conceitos de Sistemas Operacionais de Tempo Real (RTOS), especificamente FreeRTOS, em um microcontrolador RP2040.
*   Desenvolver um sistema multitarefa real, com tarefas dedicadas ao controle de periféricos e à lógica principal.
*   Implementar um semáforo de cruzamento funcional com ciclos para veículos e pedestres.
*   Criar um modo de operação noturno com comportamento de alerta distinto.
*   Integrar múltiplos periféricos da placa BitDogLab: GPIO (LEDs, Botões com IRQ), PIO (Matriz de LEDs WS2812), I2C (Display OLED SSD1306) e PWM (Buzzer).
*   Implementar recursos de acessibilidade através de feedback sonoro distinto para cada fase do semáforo, utilizando o buzzer.
*   Gerenciar a comunicação e sincronização entre tarefas utilizando *apenas* variáveis globais voláteis (conforme requisito específico), com proteção básica via seções críticas.
*   Desenvolver o projeto em um ambiente VS Code configurado para o RP2040 com código modularizado e comentado.

## Descrição do Projeto

Este projeto implementa um "Semáforo Inteligente" na placa BitDogLab usando o RP2040 e o sistema operacional de tempo real FreeRTOS. O sistema simula um cruzamento gerenciando o fluxo de veículos e pedestres com dois modos de operação principais: Normal e Noturno.

**Modo Normal:**
*   **Veículos:** O semáforo para veículos, representado pelos LEDs RGB (pinos Vermelho, Verde, Azul/Amarelo), segue o ciclo padrão: Verde -> Amarelo -> Vermelho -> Vermelho+Amarelo -> Verde.
*   **Pedestres:** O semáforo para pedestres, representado pela Matriz de LEDs 5x5 WS2812 (controlada via PIO), é sincronizado com o semáforo de veículos:
    *   Exibe um símbolo de "Pare" (mão/bloco vermelho) quando os veículos têm sinal verde ou amarelo.
    *   Exibe um símbolo de "Ande" (boneco verde) por um período quando os veículos estão com sinal vermelho.
    *   Exibe o símbolo de "Pare" piscando por um período antes de liberar os veículos novamente.
*   **Acessibilidade Sonora:** O buzzer emite padrões sonoros distintos para indicar a fase do pedestre (som contínuo para "Ande", bipes rápidos para "Pare" piscando).

**Modo Noturno:**
*   **Veículos:** O semáforo para veículos exibe apenas a luz Amarela (usando pino Azul) piscando intermitentemente.
*   **Pedestres:** A matriz de LEDs para pedestres permanece apagada.
*   **Acessibilidade Sonora:** O buzzer emite um bip lento e espaçado indicando o modo noturno de alerta.

**Controle e Feedback:**
*   O **Botão A** é usado para alternar entre os modos Normal e Noturno, com detecção via interrupção (IRQ) e tratamento de debounce.
*   O **Display OLED** (128x64, via I2C) exibe o modo de operação atual ("NORMAL" ou "NOTURNO") e o estado detalhado da fase do cruzamento (ex: "Carro: Verde / Ped:Pare"). *(Nota: A funcionalidade do display está presente no código, mas pode ser comentada para testes individuais das outras tarefas).*
*   Logs de inicialização e mudança de modo são enviados via `printf` (visível em terminal serial USB).

**Arquitetura FreeRTOS:**
O sistema utiliza múltiplas tarefas FreeRTOS, cada uma responsável por um periférico ou pela lógica central:
*   `vIntersectionControllerTask`: Gerencia as fases do semáforo e a temporização principal.
*   `vButtonTask`: Monitora o Botão A para mudança de modo.
*   `vRgbLedTask`: Controla o LED RGB (semáforo de veículos).
*   `vLedMatrixTask`: Controla a Matriz de LEDs (semáforo de pedestres).
*   `vBuzzerTask`: Gera os sons de acessibilidade.
*   `vDisplayTask`: Atualiza o display OLED (se habilitado).

## Funcionalidades Implementadas

```
✅ Ciclo de semáforo para veículos (Verde -> Amarelo -> Vermelho -> Vermelho+Amarelo) usando LEDs RGB/individuais.
✅ Ciclo de semáforo para pedestres (Ande -> Pare Piscando -> Pare) usando Matriz de LEDs WS2812 (via PIO).
✅ Sincronização entre os ciclos de veículos e pedestres.
✅ Implementação de Modo Noturno (Amarelo piscante para veículos, matriz apagada, som lento).
✅ Leitura do Botão A (GPIO 5) via Interrupção (IRQ) com Debounce para alternar entre Modo Normal e Noturno.
✅ Uso de flag global volátil (`g_flagModoNoturno`, `g_intersectionPhase`) para comunicação entre tarefas (conforme restrição).
✅ Proteção básica de acesso às flags globais com `taskENTER/EXIT_CRITICAL()`.
✅ Geração de sons distintos no buzzer (via PWM) para fases de pedestre ("Ande", "Pare Piscando") e Modo Noturno.
✅ Exibição do modo e estado atual no display OLED SSD1306 (I2C). *(Funcionalidade comentada na versão de teste)*.
✅ Uso de múltiplas tarefas FreeRTOS dedicadas (Controller, Button, RGB LED, Matrix, Buzzer, Display).
✅ Código estruturado em módulos (`main.c`, `hardware_management/` para drivers/config) e comentado.
✅ Mensagens de log de inicialização e mudança de modo via `printf` (stdio/USB).
```

## Requisitos Técnicos Atendidos

*(Baseado nos requisitos originais do projeto de Semáforo)*

1.  **Implementar Semáforo Inteligente:** Sim, implementa a lógica com dois modos.
2.  **Dois Modos Distintos:** Sim, Normal e Noturno com comportamentos diferentes.
3.  **Utilizar Apenas Tarefas FreeRTOS:** Sim, a lógica é distribuída em múltiplas tarefas. (Nota: Botões usam ISR para detecção inicial).
4.  **Sem Filas, Semáforos ou Mutexes:** Sim, a comunicação principal é via flags globais voláteis. (Nota: `taskENTER/EXIT_CRITICAL` usado para proteção mínima).
5.  **Representado por:**
    *   **Matriz de LEDs:** Sim, usada para pedestres (Walk/Don't Walk piscando).
    *   **LED RGB:** Sim, usado para semáforo de veículos (R/G/Y).
    *   **Display:** Sim, exibe modo/estado (pode ser comentado para teste).
    *   **Buzzer:** Sim, usado para feedback sonoro de acessibilidade.
6.  **Modo Normal:** Sim, implementa o ciclo Veicular e Pedestre com sons correspondentes.
7.  **Modo Noturno:** Sim, implementa Amarelo piscante (veicular) e som lento, com matriz apagada.
8.  **Alternar com Botão A:** Sim, implementado com IRQ, debounce e flag global.
9.  **Flag Global Modificada por Tarefa:** Sim, `g_flagModoNoturno` (pela ButtonTask) e `TrafficLight_states trafficLight_state` (pela ControllerTask).
10. **Sinais Sonoros Distintos:** Sim, padrões diferentes para Pedestre Anda, Pedestre Pisca e Modo Noturno.
11. **Requisito Implícito (Task por Periférico):** Sim, a estrutura final dedica tarefas para Botão, LED RGB, Matriz, Buzzer, Display, além da tarefa Controladora.

## Como Executar

### Requisitos de Hardware

*   Placa de desenvolvimento **BitDogLab** (com RP2040).
*   Cabo Micro-USB para conexão e alimentação/programação.

### Requisitos de Software

*   **VS Code** com a extensão Pico-W-Go ou configuração manual do toolchain ARM e Pico SDK.
*   **Pico SDK** versão compatível instalada e configurada (ex: 1.5.1+).
*   **FreeRTOS Kernel:** O código assume que o diretório `FreeRTOS-Kernel` existe na raiz do projeto (idealmente como um submódulo git: `git submodule update --init`).
*   **Git** (opcional, para clonar).
*   Um **Terminal Serial** (ex: Monitor Serial do VS Code, Putty, Minicom) configurado para a porta serial da Pico e **115200 baud** (verifique `pico_enable_stdio_uart` / `usb` no CMakeLists e a configuração em `pico/stdlib.h` se usar baudrate diferente).

### Passos

1.  **Obter o Código:** Clone ou baixe o repositório/arquivos do projeto.
2.  **Obter FreeRTOS:** Se ainda não tiver, inicialize o submódulo:
    ```bash
    git submodule update --init --recursive
    ```
    Ou copie manualmente a pasta `FreeRTOS-Kernel` para a raiz do projeto.
3.  **Configurar o Projeto (CMakeLists.txt):**
    *   Verifique o caminho para `FREERTOS_KERNEL_PATH` no `CMakeLists.txt` e ajuste se necessário (o padrão assume estar na raiz).
    *   Certifique-se de que `PICO_BOARD` está definido corretamente (provavelmente `pico_w`).
4.  **Compilar (Build):**
    *   Via VS Code: Use a função de build.
    *   Via Linha de Comando:
        ```bash
        mkdir build
        cd build
        cmake ..
        make
        ```
5.  **Carregar o Firmware:**
    *   Coloque a BitDogLab em modo BOOTSEL (pressione BOOTSEL ao conectar o USB).
    *   Copie o arquivo `main.uf2` (ou o nome do seu projeto `.uf2`) da pasta `build` para o drive `RPI-RP2`.
    *   A placa reiniciará.
6.  **Visualizar Logs:** Abra o terminal serial na porta COM correta com 115200 baud.
7.  **Testar:**
    *   Observe o comportamento dos LEDs RGB e da Matriz de LEDs.
    *   Ouça os padrões do buzzer.
    *   Pressione o Botão A (GPIO 5) para alternar entre os modos Normal e Noturno.
    *   *(Se habilitado)* Observe as informações no display OLED.

## Estrutura do Código

```
📂 src/
├── include/ # Headers públicos
│ └── hardware_management/ # Headers dos módulos de hardware
| |   FreeRTOSConfig.h # Configuração específica do FreeRTOS
│ ├── buttons.h
│ ├── buzzer.h
│ ├── debouncer.h
│ ├── display.h
│ ├── led_matrix.h
│ └── hardware_config.h # Configuração centralizada de pinos/constantes
│ ├── hardware_management/ # Implementação dos módulos de hardware
│ │ ├── buttons.c
│ │ ├── buzzer.c
│ │ ├── debouncer.c
│ │ ├── display.c # Contém init/startup (Task comentada aqui)
│ │ └── led_matrix.c
│ └── main.c # Ponto de entrada, init, criação de tasks, loop tasks
├── pio/ # Programas PIO Assembly
│ └── led_matrix.pio
├── lib/ # Bibliotecas
│ └── ssd1306/
│ ├── ssd1306.c
│ ├── ssd1306.h
│ └── font.h # (se separado)
├── CMakeLists.txt # Configuração do build CMake
├── pico_sdk_import.cmake # Padrão do SDK
```

## Demonstrativo em Vídeo

*(Insira aqui o link para um vídeo demonstrando o projeto funcionando, se disponível)*
