#include "funcao_atividade_.h"
#include "funcoes_neopixel.h" 

// Fila circular para armazenar eventos
int fila[TAM_FILA];
int inicio = 0;      // índice do primeiro elemento na fila
int fim = 0;         // índice para inserir o próximo elemento
int quantidade = 0;  // quantidade atual de elementos na fila
int contador = 0;    // contador geral para numerar eventos

// Armazena o tempo do último toque para debounce de cada botão
absolute_time_t ultimo_toque[NUM_BOTOES];

// Arrays com os pinos dos botões e LEDs
const uint BOTOES[NUM_BOTOES] = {BOTAO_A, BOTAO_B, BOTAO_JOYSTICK};
const uint LEDS[NUM_BOTOES]   = {LED_VERMELHO, LED_AZUL, LED_VERDE};

// Flags para controle dos eventos e estados dos LEDs
volatile bool eventos_pendentes[NUM_BOTOES] = {false, false, false};
volatile bool estado_leds[NUM_BOTOES] = {false, false, false};
volatile bool core1_pronto = false;

// Função chamada quando ocorre interrupção num pino de botão
void gpio_callback(uint gpio, uint32_t events) {
    // Verifica qual botão gerou a interrupção
    for (int i = 0; i < NUM_BOTOES; i++) {
        if (gpio == BOTOES[i] && (events & GPIO_IRQ_EDGE_FALL)) {
            // Envia para o core 1 qual botão foi pressionado
            multicore_fifo_push_blocking(i);  
        }
    }
}

// Inicializa o pino com direção e configurações de pull-up/pull-down
void inicializar_pino(uint pino, uint direcao, bool pull_up, bool pull_down) {
    gpio_init(pino);
    gpio_set_dir(pino, direcao);

    if (direcao == GPIO_IN) {
        if (pull_up) {
            gpio_pull_up(pino);
        } else if (pull_down) {
            gpio_pull_down(pino);
        } else {
            gpio_disable_pulls(pino);
        }
    }
}

// Função que roda no núcleo 1, responsável pelo tratamento dos LEDs
void tratar_eventos_leds() {
    core1_pronto = true;  // sinaliza que núcleo 1 está pronto

    while (true) {
        // Espera bloqueante para receber qual botão foi pressionado do núcleo 0
        uint32_t id1 = multicore_fifo_pop_blocking();

        sleep_ms(DEBOUNCE_MS);  // tempo para evitar múltiplas detecções (debounce)

        // Verifica se o botão ainda está pressionado após debounce
        if (!gpio_get(BOTOES[id1])) {
            // Confere se outro botão também está pressionado
            bool outro_pressionado = false;
            for (int i = 0; i < NUM_BOTOES; i++) {
                if (i != id1 && !gpio_get(BOTOES[i])) {
                    outro_pressionado = true;
                    break;
                }
            }
            if (outro_pressionado) {
                // Espera soltar o botão antes de continuar
                while (!gpio_get(BOTOES[id1])) tight_loop_contents();
                continue;
            }

            // Se botão A foi pressionado (id1 == 0), incrementa fila e acende LED NeoPixel
            if (id1 == 0 && index_neo < LED_COUNT) {
                // Gera cores aleatórias para o LED
                uint8_t r = numero_aleatorio(1, 255);
                uint8_t g = numero_aleatorio(1, 255);
                uint8_t b = numero_aleatorio(1, 255);
                npAcendeLED(index_neo, r, g, b);
                index_neo++;

                // Insere na fila o próximo evento, se não estiver cheia
                if (quantidade < TAM_FILA) {
                    fila[fim] = contador++;  // insere evento com número atual
                    fim = (fim + 1) % TAM_FILA; // avança índice circular
                    quantidade++;
                    imprimir_fila();         // mostra fila no console
                }

            } else if (id1 == 1 && index_neo > 0) {  // Botão B: decrementa fila e apaga LED NeoPixel
                index_neo--;
                npAcendeLED(index_neo, 0, 0, 0);  // apaga o LED correspondente

                // Remove da fila o evento mais antigo
                if (quantidade > 0) {
                    int valor = fila[inicio];
                    inicio = (inicio + 1) % TAM_FILA;
                    quantidade--;
                    imprimir_fila();
                }
            } else if (id1 == 2) {
                // BOTÃO DO JOYSTICK: zera contador e limpa fila e NeoPixel
                contador = 0;
                quantidade = 0;
                inicio = 0;
                fim = 0;
                index_neo = 0;
                npClear();
                npWrite();
                imprimir_fila();
            }

            // Atualiza LEDs RGB externos para indicar estado da fila
            gpio_put(LED_VERMELHO, (index_neo == LED_COUNT));  // vermelho se fila cheia
            gpio_put(LED_AZUL, (index_neo == 0));              // azul se fila vazia
            gpio_put(LED_VERDE, 0);                             // verde desligado (opcional)

            // Espera o botão ser solto antes de processar próximo evento
            while (!gpio_get(BOTOES[id1])) {
                tight_loop_contents();
            }
        }
    }
}

// Imprime no console os elementos presentes na fila
void imprimir_fila() {
    printf("Fila [tam=%d]: ", quantidade);
    int i = inicio;
    for (int c = 0; c < quantidade; c++) {
        printf("%d ", fila[i]);
        i = (i + 1) % TAM_FILA;
    }
    printf("\n");
}
