#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "include/common.h"

// Configuración inicial
#define CLOCK_PIN 16
#define DATA_PIN 17
#define SAMPLES_PER_READING 10
static int32_t tare_offset = 0;
static int32_t calibration_value = 0;
static float known_weight = 4650.0f;  // Ajusta esto al peso que uses para calibrar (en gramos)

// Función para obtener el promedio de varias lecturas
int32_t get_average_reading(hx711_t *hx, int num_readings) {
    int32_t sum = 0;
    for (int i = 0; i < num_readings; i++) {
        sum += hx711_get_value(hx);
        sleep_ms(10);
    }
    return sum / num_readings;
}

// Función de tara ajustada
void perform_tare(hx711_t *hx) {
    printf("\nRealizando tara...\n");
    printf("Asegúrate de que no haya peso en la balanza\n");
    sleep_ms(2000);
    
    // Promedio de 10 lecturas para mayor estabilidad
    int32_t sum = 0;
    for(int i = 0; i < 10; i++) {
        sum += hx711_get_value(hx);
        sleep_ms(100);
    }
    tare_offset = sum / 10;
    printf("Tara completada. Valor de offset: %li\n", tare_offset);
}

// Función de calibración ajustada
void calibrate(hx711_t *hx) {
    printf("\nIniciando calibración...\n");
    printf("Coloca el peso conocido de %.1f gramos\n", known_weight);
    sleep_ms(3000);
    
    // Promedio de 10 lecturas con el peso conocido
    int32_t sum = 0;
    for(int i = 0; i < 10; i++) {
        sum += hx711_get_value(hx);
        sleep_ms(100);
    }
    calibration_value = sum / 10;
    
    printf("Calibración completada.\n");
    printf("Valor sin peso: %li\n", tare_offset);
    printf("Valor con peso: %li\n", calibration_value);
}

// Función para obtener el peso ajustada
float get_weight(hx711_t *hx) {
    // Promedio de 3 lecturas para estabilidad
    int32_t sum = 0;
    for(int i = 0; i < 3; i++) {
        sum += hx711_get_value(hx);
        sleep_ms(10);
    }
    int32_t value = sum / 3;
    
    // Fórmula de conversión ajustada
    float weight = 0;
    if(calibration_value != tare_offset) {
        weight = ((float)(value - tare_offset) * known_weight) / 
                (float)(calibration_value - tare_offset);
    }
    
    return weight;
}

int main(void) {
    stdio_init_all();
    
    while (!tud_cdc_connected()) {
        sleep_ms(100);
    }
    
    printf("\nIniciando sistema de pesaje HX711...\n");
    
    // Configuración del HX711
    hx711_config_t hxcfg;
    hx711_get_default_config(&hxcfg);
    hxcfg.clock_pin = CLOCK_PIN;
    hxcfg.data_pin = DATA_PIN;
    
    hx711_t hx;
    
    // Inicialización del HX711 como en el ejemplo original
    hx711_init(&hx, &hxcfg);
    
    // Encendido y configuración del HX711
    hx711_power_up(&hx, hx711_gain_128);
    hx711_wait_settle(hx711_rate_80);
    
    printf("HX711 inicializado correctamente\n");
    
    // Menú principal
    char cmd;
    bool running = true;
    
    while (running) {
        printf("\n=== Menú de Calibración y Pesaje ===\n");
        printf("t: Realizar tara (ajuste a cero)\n");
        printf("c: Calibrar con peso conocido\n");
        printf("p: Realizar pesaje\n");
        printf("m: Monitoreo continuo\n");
        printf("q: Salir\n");
        printf("Seleccione una opción: ");
        
        cmd = getchar();
        
        switch (cmd) {
            case 't':
                perform_tare(&hx);
                break;
                
            case 'c':
                calibrate(&hx);
                break;
                
            case 'p':
                printf("\nPeso actual: %.2f g\n", get_weight(&hx));
                break;
                
            case 'm':
                printf("\nIniciando monitoreo continuo (presiona cualquier tecla para detener)...\n");
                while (!getchar_timeout_us(1000)) {
                    printf("Peso: %.2f g\r", get_weight(&hx));
                    sleep_ms(200);
                }
                break;
                
            case 'q':
                running = false;
                break;
                
            default:
                printf("\nOpción no válida\n");
                break;
        }
    }
    
    // Limpieza y cierre
    hx711_close(&hx);
    printf("\nSistema finalizado\n");
    
    return EXIT_SUCCESS;
}