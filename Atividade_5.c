#include "funcao_atividade_.h"
#include "funcoes_neopixel.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

// Estrutura para armazenar cores RGB
typedef struct {
    uint8_t r, g, b;
} CorRGB;

// Array com diversas cores para uso nos LEDs NeoPixel
CorRGB cores[] = {
    {255, 0, 0}, {255, 64, 0}, {255, 128, 0}, {255, 192, 0},
    {255, 255, 0}, {192, 255, 0}, {128, 255, 0}, {0, 255, 0},
    {0, 255, 128}, {0, 255, 255}, {0, 128, 255}, {0, 0, 255},
    {128, 0, 255}, {255, 0, 255}, {255, 0, 128}, {255, 255, 255}
};

const size_t TOTAL_CORES = sizeof(cores) / sizeof(CorRGB);
volatile uint index_neo = 0;  // Índice para controle do LED NeoPixel atual

int main() {
    // Inicializa o driver NeoPixel no pino configurado (definido em funcoes_neopixel.h)
    npInit(LED_PIN);
    npClear();   // Limpa a matriz NeoPixel (apaga todos LEDs)
    npWrite();   // Atualiza os LEDs físicos

    stdio_init_all();  // Inicializa USB Serial para saída no terminal
    sleep_ms(500);     // Pequena pausa para estabilizar a comunicação

    // Inicializa todos os botões com pull-ups e interrupções na borda de descida
    for (int i = 0; i < NUM_BOTOES; i++) {
        inicializar_pino(BOTOES[i], GPIO_IN, true, false);
        gpio_set_irq_enabled_with_callback(BOTOES[i], GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    }

    // Inicializa os LEDs externos como saída e desliga-os
    for (int i = 0; i < NUM_BOTOES; i++) {
        gpio_init(LEDS[i]);
        gpio_set_dir(LEDS[i], GPIO_OUT);
        gpio_put(LEDS[i], 0);
    }

    // Inicializa ADC para o joystick (se necessário)
    adc_init();
    adc_gpio_init(26);  // pino ADC para eixo X do joystick
    adc_select_input(0);

    multicore_launch_core1(tratar_eventos_leds);  // inicia a função no núcleo 1

    // Aguarda o núcleo 1 estar pronto antes de continuar
    while (!core1_pronto) {
        tight_loop_contents();
    }

    // Loop principal: pode adicionar tarefas adicionais aqui, mas controle dos LEDs fica no core1
    while (true) {
        // Pode ser usado para ler joystick e alterar comportamentos, mas para esta atividade o botão joystick controla reset na interrupção

        sleep_ms(10);  // evita loop excessivo
    }

    return 0;
}
