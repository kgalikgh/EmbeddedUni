/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>


#include <stdio.h>
#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainLED_TASK_PRIORITY   2

#define mainSERIAL_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vRuler(void* pvParameters);

static void vButton(void* pvParameters);
static void vCos(void* pvParameters);
/******************************************************************************
 * Public function definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \fn int main(void)
 *
 * \brief Main function.
 *
 * \return
 ******************************************************************************/
int main(void)
{
    // Create task.
    xTaskHandle ruler_handle;
    xTaskHandle button_handle;

    xTaskCreate
        (
         vRuler,
         "ruler",
         configMINIMAL_STACK_SIZE,
         NULL,
         2,
         &ruler_handle
        );

    xTaskCreate
        (
         vButton,
         "button",
         configMINIMAL_STACK_SIZE,
         NULL,
         1,
         &button_handle
        );

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

/**************************************************************************//**
 * \fn static vApplicationIdleHook(void)
 *
 * \brief
 ******************************************************************************/
void vApplicationIdleHook(void)
{

}

/******************************************************************************
 * Private function definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \fn static void vBlinkLed(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
static void vCos(void* pvParameters)
{
    while(1)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
static void vRuler(void* pvParameters)
{
    DDRD = 0xff;
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    while(1)
    {
        short status = 1;
        for(int i = 0; i < 8; i++)
        {
            PORTD = status;
            status <<= 1;

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        status >>= 1;
        for(int i = 6; i >= 0; i--)
        {
            PORTD = status;
            status >>= 1;
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}


/**************************************************************************//**
 * \fn static void vSerial(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
uint8_t arr[100] = {0};
uint8_t counter = 0;
static void vButton(void* pvParameters)
{
    PORTC |= _BV(PC0);
    DDRB |= _BV(PB4);


    while(1)
    {
        if(arr[counter])
        {
            PORTB &= ~_BV(PB4);
        }
        else
        {
            PORTB |= _BV(PB4);
        }
        arr[counter] = PINC & _BV(PC0);
        if(++counter >= 100) counter = 0;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
