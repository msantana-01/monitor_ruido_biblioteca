#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"
#include <math.h>

// Definições dos pinos
#define MICROPHONE_PIN 28
#define BUZZER_PIN 21
#define SDA_PIN 14
#define SCL_PIN 15
#define LED_MATRIX_PIN 7
#define I2C_PORT i2c1
#define OLED_ADDRESS 0x3C
#define BUZZER_FREQUENCY 1000
#define LED_COUNT 25
#define MAX_BRIGHTNESS 0.1

// Variáveis globais
bool monitoring_enabled = true;
int alarm_count = 0;
bool showing_alarm_count = false;

struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
pixel_t leds[LED_COUNT];
PIO np_pio;
uint sm;
uint32_t pwm_wrap_value;
ssd1306_t oled;

// Protótipos de funções
void setup_microphone();
void setup_buzzer();
void setup_led_matrix();
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b);
void npWrite();
void npClear();
uint8_t smooth_brightness(uint16_t adc_value);
void play_alarm();
void setup_oled();
void display_sound_info(uint16_t adc_value, float voltage);
void setup_buttons();

// Configurações iniciais
void setup_microphone() {
    adc_init();
    adc_gpio_init(MICROPHONE_PIN);
    adc_select_input(2);
}

void setup_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_wrap_value = 125000000 / BUZZER_FREQUENCY - 1;
    pwm_set_wrap(slice_num, pwm_wrap_value);
    pwm_set_enabled(slice_num, true);
}

void setup_led_matrix() {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }
    ws2818b_program_init(np_pio, sm, offset, LED_MATRIX_PIN, 800000.f);
    npClear();
}

void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index].R = r * MAX_BRIGHTNESS;
    leds[index].G = g * MAX_BRIGHTNESS;
    leds[index].B = b * MAX_BRIGHTNESS;
}

void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i) npSetLED(i, 0, 0, 0);
    npWrite();
}

uint8_t smooth_brightness(uint16_t adc_value) {
    float normalized = (float)adc_value / 4095.0f;
    float smoothed = log1p(normalized * 10) / log1p(10);
    return (uint8_t)(smoothed * 255);
}

void play_alarm() {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    for (int i = 0; i < 3; i++) {
        pwm_wrap_value = 125000000 / 400 - 1;
        pwm_set_wrap(slice_num, pwm_wrap_value);
        pwm_set_gpio_level(BUZZER_PIN, pwm_wrap_value / 2);
        npClear();
        sleep_ms(100);

        pwm_wrap_value = 125000000 / 600 - 1;
        pwm_set_wrap(slice_num, pwm_wrap_value);
        pwm_set_gpio_level(BUZZER_PIN, pwm_wrap_value / 2);
        for (int j = 0; j < LED_COUNT; j++) npSetLED(j, 255, 0, 0);
        npWrite();
        sleep_ms(100);
    }
    pwm_set_gpio_level(BUZZER_PIN, 0);
    npClear();
}

void setup_oled() {
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    ssd1306_init(&oled, 128, 64, false, OLED_ADDRESS, I2C_PORT);
    ssd1306_config(&oled);
    ssd1306_fill(&oled, false);
    ssd1306_send_data(&oled);
}

void display_sound_info(uint16_t adc_value, float voltage) {
    char buffer[32];
    ssd1306_fill(&oled, false);
    snprintf(buffer, sizeof(buffer), "ADC: %d", adc_value);
    ssd1306_draw_string(&oled, buffer, 0, 0);
    snprintf(buffer, sizeof(buffer), "Voltagem %.2f V", voltage);
    ssd1306_draw_string(&oled, buffer, 0, 10);
    uint8_t bar_width = (adc_value * 128) / 4095;
    ssd1306_rect(&oled, 20, 0, bar_width, 10, true, true);
    ssd1306_send_data(&oled);
}

void setup_buttons() {
    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5);
    gpio_init(6);
    gpio_set_dir(6, GPIO_IN);
    gpio_pull_up(6);
}

int main() {
    stdio_init_all();
    setup_microphone();
    setup_buzzer();
    setup_oled();
    setup_led_matrix();
    setup_buttons();

    const float threshold_voltage = 1.68f;
    static bool button_a_prev = false;
    static bool button_b_prev = false;

    while (1) {
        // Verificação dos botões com detecção de borda
        bool button_a_current = (gpio_get(5) == 0);
        bool button_b_current = (gpio_get(6) == 0);

        if (button_a_current && !button_a_prev) {
            monitoring_enabled = !monitoring_enabled; // Alterna o estado do monitoramento
            while (gpio_get(5) == 0) {} // Espera soltar o botão
            sleep_ms(20); // Debounce
        }
        button_a_prev = button_a_current;

        if (button_b_current && !button_b_prev) {
            showing_alarm_count = !showing_alarm_count; // Alterna entre contador e monitoramento
            while (gpio_get(6) == 0) {} // Espera soltar o botão
            sleep_ms(20); // Debounce
        }
        button_b_prev = button_b_current;

        // Lógica principal de exibição
        if (showing_alarm_count) {
            // Exibe o contador de alarmes
            ssd1306_fill(&oled, false); // Limpa o display
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "Alarmes %d", alarm_count);
            ssd1306_draw_string(&oled, buffer, 0, 0);
            ssd1306_send_data(&oled);
        } 
        else if (monitoring_enabled) {
            // Monitoramento ativo
            uint16_t adc_value = adc_read();
            float voltage = (adc_value * 3.3f) / 4095.0f;
            display_sound_info(adc_value, voltage);

            uint8_t brightness = smooth_brightness(adc_value);
            for (int i = 0; i < LED_COUNT; i++) npSetLED(i, 0, brightness, 0);
            npWrite();

            if (voltage > threshold_voltage) {
                play_alarm();
                alarm_count++;
            }
        } 
        else {
            // Monitoramento desativado
            ssd1306_fill(&oled, false); // Limpa o display
            ssd1306_draw_string(&oled, "Monitoramento OFF", 0, 0); // Exibe a mensagem
            ssd1306_send_data(&oled);
            npClear(); // Desliga os LEDs
        }

        sleep_ms(100); // Intervalo de verificação
    }
}