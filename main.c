#include <stdio.h>
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "lib/ssd1306.h"
#include "lib/neopixel.h"
#include "lib/buzzer.h"
#include "hardware/adc.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

// Pinos
#define RED_PIN       13
#define GREEN_PIN     11
#define BLUE_PIN      12
#define BUTTON_A      5
#define BUTTON_B      5
#define BUTTON_JOYSTICK 22
#define BUZZER_A    21
#define BUZZER_B    10
#define ADC_JOYSTICK_X 26
#define ADC_JOYSTICK_Y 27

static ssd1306_t ssd; // Estrutura do display OLED

typedef struct {
    float nivel_agua;   // em Porcentagem
    float volume_chuva; // em Porcentagem
} DadosSensor;

QueueHandle_t xQueueMatrizData;    // Para matriz LED
QueueHandle_t xQueueLEDData;       // Para LED RGB
QueueHandle_t xQueueDisplayData;   // Para OLED
QueueHandle_t xQueueBuzzerData;    // Para buzzer

void vJoystickTask()
{
    adc_gpio_init(ADC_JOYSTICK_Y);
    adc_gpio_init(ADC_JOYSTICK_X);
    adc_init();

    DadosSensor dados;
    
    while (true)
    {
        adc_select_input(0); // GPIO 26 = ADC0
        uint16_t y_pos = adc_read();
        dados.nivel_agua = (y_pos * 100.0f) / 4095.0f; // Transforma em porcentagem

        adc_select_input(1); // GPIO 27 = ADC1
        uint16_t x_pos = adc_read();
        dados.volume_chuva = (x_pos * 100.0f) / 4095.0f; // Transforma em porcentagem

        // Envia para as filas
        xQueueSend(xQueueLEDData, &dados, 0);
        xQueueSend(xQueueDisplayData, &dados, 0);
        xQueueSend(xQueueMatrizData, &dados, 0);
        xQueueSend(xQueueBuzzerData, &dados, 0);
        vTaskDelay(pdMS_TO_TICKS(100));              
    }
}

void vLedRGBTask(){

     // Configuração dos LEDs
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);

    DadosSensor dados;
    bool ligado = true; // Variavel para auxiliar o periférico piscar

    while(true){

        if (xQueueReceive(xQueueLEDData, &dados, portMAX_DELAY) == pdTRUE)
        {
            // Modo Alerta
            if (dados.nivel_agua >= 70 || dados.volume_chuva >= 80)
            {   
                gpio_put(GREEN_PIN, false);      // Desliga Verde
                
                gpio_put(RED_PIN, ligado);       // Alterna Vermelho
                ligado = !ligado;                 
            }
            // Modo Normal
            else
            {   
                gpio_put(RED_PIN, false); // Desliga Vermelho
                gpio_put(GREEN_PIN, true); //Liga Verde
                ligado = true;
            }

        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Espera 100ms

    }

}

void vMatrizLedTask(){
    
    npInit(LED_PIN);
    DadosSensor dados;
    bool ligado = true;

    while(true){
        if (xQueueReceive(xQueueMatrizData, &dados, portMAX_DELAY) == pdTRUE){
            
            // Modo Alerta
            if (dados.nivel_agua >= 70 || dados.volume_chuva >= 80){
                
                npFill(ligado,0,0);
                ligado = !ligado;
        
                // Desenha nuvem
                npSetLED(npGetIndex(1, 4), 1, 1, 1);
                npSetLED(npGetIndex(2, 4), 1, 1, 1);
                npSetLED(npGetIndex(3, 4), 1, 1, 1);

                npSetLED(npGetIndex(0, 3), 1, 1, 1);
                npSetLED(npGetIndex(1, 3), 1, 1, 1);
                npSetLED(npGetIndex(2, 3), 1, 1, 1);
                npSetLED(npGetIndex(3, 3), 1, 1, 1);
                npSetLED(npGetIndex(4, 3), 1, 1, 1);

                // Desenha raio
                npSetLED(npGetIndex(3, 2), 1, 1, 0);
                npSetLED(npGetIndex(2, 1), 1, 1, 0);
                npSetLED(npGetIndex(1, 0), 1, 1, 0);

            }
            // Modo Normal
            else{
                npClear();
                ligado = true;
            }
            npWrite(); // Escreve na matriz
        } 
      
        vTaskDelay(pdMS_TO_TICKS(100)); // Atualiza a cada 100

    }

}

void vDisplayOledTask(){

    DadosSensor dados;
    bool ligado = false; // Variavel para auxiliar o display piscar no modo alerta

    char nivel_agua[20];
    char volume_chuva[20];

    while(true){

        if (xQueueReceive(xQueueDisplayData, &dados, portMAX_DELAY) == pdTRUE){

            ssd1306_fill(&ssd, false); // Limpa a tela

            sprintf(nivel_agua, "Niv Agua: %1.0f%%", dados.nivel_agua); // Converte o float em string 
            sprintf(volume_chuva, "Vol Chuva: %1.0f%%", dados.volume_chuva); // Converte o float em string 
            
            // Modo alerta
            if (dados.nivel_agua >= 70 || dados.volume_chuva >= 80){
                ssd1306_rect(&ssd, 2, 2 , 124, 20, true, ligado);  
                ligado = !ligado;
                ssd1306_draw_string(&ssd, "=== ALERTA ===", 8, 10);
            }
            // Modo Normal
            else{
                ssd1306_draw_string(&ssd, "NORMAL", 40, 10);
            }
            ssd1306_draw_string(&ssd, nivel_agua, 4, 30); 
            ssd1306_draw_string(&ssd, volume_chuva, 4, 50); 
            ssd1306_send_data(&ssd);

        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay de 10ms por loop

    }

}

void vBuzzerTask() {

    buzzer_init(BUZZER_A);
    buzzer_init(BUZZER_B);

    DadosSensor dados;
    bool ligado = false;
    uint contador = 2;

    while (true) {
        if (xQueueReceive(xQueueBuzzerData, &dados, portMAX_DELAY) == pdTRUE) {

            bool condA = dados.nivel_agua >= 70; 
            bool condB = dados.volume_chuva >= 80;
            

            if (contador >= 2) { // A cada 200ms (2x 100ms)
                    contador = 0;
                    ligado = !ligado; // Inverte o estado
            }

            // Modo Alerta
            if (condA || condB) {
                
                // Liga os buzzers com base na variavel ligado e se atendem as suas condições
                if (ligado) {
                    if (condA) buzzer_turn_on(BUZZER_A, 1000);
                    if (condB) buzzer_turn_on(BUZZER_B, 2000);
                } else {
                    if (condA) buzzer_turn_off(BUZZER_A);
                    if (condB) buzzer_turn_off(BUZZER_B);
                }
            }
            // Modo Normal
            else {
                // Se nenhuma condição for verdadeira, desliga tudo
                buzzer_turn_off(BUZZER_A);
                buzzer_turn_off(BUZZER_B);
                ligado = true;
                contador = 0;
            }

            contador++; // Incrementa o contador

        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay de 100ms por loop
    }


}

//Callback para colocar em modo BOOTSEL
static void gpio_button_joystick_handler(uint gpio, uint32_t events){
    printf("\nHABILITANDO O MODO GRAVAÇÃO\n");

    // Adicionar feedback no display OLED
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "  HABILITANDO", 5, 25);
    ssd1306_draw_string(&ssd, " MODO GRAVACAO", 5, 38);
    ssd1306_send_data(&ssd);

    reset_usb_boot(0,0); // Reinicia no modo DFU
}


int main()
{
    stdio_init_all();

    // Ativa o BOOTSEL pelo botão JoyStick 
    display_init(&ssd);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_button_joystick_handler);

    // Cria a fila para compartilhamento de valores
    xQueueMatrizData = xQueueCreate(10, sizeof(DadosSensor));
    xQueueLEDData = xQueueCreate(10, sizeof(DadosSensor));
    xQueueDisplayData = xQueueCreate(10, sizeof(DadosSensor));
    xQueueBuzzerData = xQueueCreate(10, sizeof(DadosSensor));

    // Criação das tasks
    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 1, NULL);
    xTaskCreate(vDisplayOledTask, "Display Oled Task", 512, NULL, 1, NULL);
    xTaskCreate(vMatrizLedTask, "Matrix Leds Task", 512, NULL, 1, NULL);
    xTaskCreate(vLedRGBTask, "Leds RBG Task", 256, NULL, 1, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer Task", 256, NULL, 2, NULL);
    
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();

}
