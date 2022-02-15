#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>


#include <stdio.h>
#include "uart.h"
#include "semphr.h"
#include <avr/interrupt.h>

#define mainPHOTORESISTOR_TASK_PRIORITY   2
#define mainTHERMISTOR_TASK_PRIORITY 3
#define mainPOTENTIOMETER_TASK_PRIORITY 1

#define thermistorInput     0
#define photoresistorInput  1
#define potentiometerInput  2

static TaskHandle_t currentHandle;
static SemaphoreHandle_t semaphore = NULL;
static SemaphoreHandle_t mutex = NULL;

static void vPhotoresistor(void* pvParameters);
static void vThermistor(void* pvParameters);
static void vPotentiometer(void* pvParameters);
static uint16_t readADC(uint8_t mux);

static uint16_t readADC(uint8_t mux)
{
    if(mux > 2)
        return 0;
    ADMUX = _BV(REFS0) | mux;
    ADCSRA |= _BV(ADSC);
    xSemaphoreTake(semaphore, portMAX_DELAY);
    uint16_t v = ADC;
    return v;
}

static void vPhotoresistor(void* pvParameters)
{
    while(1)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        uint16_t res = readADC(photoresistorInput);
        printf("ADC value (photoresistor): %" PRIu16 "\r\n", res);
        xSemaphoreGive(mutex);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void vThermistor(void* pvParameters)
{
    while(1)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        uint16_t res = readADC(thermistorInput);
        printf("ADC value (thermistor): %" PRIu16 "\r\n", res);
        xSemaphoreGive(mutex);
        vTaskDelay(800 / portTICK_PERIOD_MS);
    }
}

static void vPotentiometer(void* pvParameters)
{
    while(1)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        uint16_t res = readADC(potentiometerInput);
        printf("ADC value (potentiometer): %" PRIu16 "\r\n", res);
        xSemaphoreGive(mutex);
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
}


static void adc_init()
{
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  DIDR0 = _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D); 
  ADCSRA |= _BV(ADEN); // włącz ADC
  ADCSRA |= _BV(ADIE);
}

ISR(ADC_vect)
{
    xSemaphoreGiveFromISR(semaphore, NULL);
}

int main(void)
{
    // Create task.
    
    xTaskHandle photoresistor_handle;
    xTaskHandle thermistor_handle;
    xTaskHandle potentiometer_handle;
    currentHandle = photoresistor_handle;
    adc_init();
    uart_init();
    stdin = stdout = stderr = &uart_file;
    sei();
    semaphore = xSemaphoreCreateBinary();
    mutex = xSemaphoreCreateMutex();
    if(semaphore == NULL)
        printf("Semaphore was not created :c\r\n");
    
    
    xTaskCreate
        (
         vPhotoresistor,
         "photoresistor",
         configMINIMAL_STACK_SIZE + 32,
         NULL,
         mainPHOTORESISTOR_TASK_PRIORITY,
         &photoresistor_handle
        );

    xTaskCreate
        (
         vThermistor,
         "thermistor",
         configMINIMAL_STACK_SIZE + 32,
         NULL,
         mainTHERMISTOR_TASK_PRIORITY,
         &thermistor_handle
        );

    xTaskCreate
        (
         vPotentiometer,
         "potentiometer",
         configMINIMAL_STACK_SIZE + 32,
         NULL,
         mainPOTENTIOMETER_TASK_PRIORITY,
         &potentiometer_handle
        );

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

void vApplicationIdleHook(void)
{

}

