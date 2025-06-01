#ifndef FUNCAO_ATIVIDADE_3_H
#define FUNCAO_ATIVIDADE_3_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"   // para __wfi()
#include "hardware/adc.h"    // Biblioteca para manipulação do ADC no RP2040
#include "pico/multicore.h"

// ======= DEFINIÇÕES E CONSTANTES =======

// Tempo para debounce do botão (em ms)
#define DEBOUNCE_MS 40
// Delay usado em algumas rotinas (em ms)
#define DELAY_MS 500

// Definição dos pinos dos botões
#define BOTAO_A 5
#define BOTAO_B 6
#define BOTAO_JOYSTICK 22

// Definição dos pinos dos LEDs RGB externos
#define LED_VERMELHO 13
#define LED_AZUL 12
#define LED_VERDE 11

#define NUM_BOTOES 3  // Quantidade de botões usados no sistema

#define TAM_FILA 25   // Tamanho máximo da fila de eventos

#define ACAO_INSERIR 1
#define ACAO_REMOVER 2

// ======= PROTÓTIPOS DAS FUNÇÕES =======

// Callback que trata interrupções nos botões
void gpio_callback(uint gpio, uint32_t events);

// Inicializa um pino com as configurações desejadas (direção e resistores de pull)
void inicializar_pino(uint pino, uint direcao, bool pull_up, bool pull_down);

// Função executada no núcleo 1 para tratar eventos dos LEDs
void tratar_eventos_leds();

// Imprime no console o estado atual da fila de eventos
void imprimir_fila();

// Adiciona um evento na fila de eventos, dado o id do botão
void adicionar_evento(int id);

// Variáveis externas usadas em outros arquivos
extern const uint BOTOES[NUM_BOTOES];
extern const uint LEDS[NUM_BOTOES];
extern volatile bool core1_pronto;

extern volatile bool eventos_pendentes[NUM_BOTOES];
extern volatile bool estado_leds[NUM_BOTOES];

#endif
