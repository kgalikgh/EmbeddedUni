/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <avr/io.h>


#include <stdio.h>
#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainBLINK_TASK_PRIORITY   2

#define mainREAD_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vRead(void* pvParameters);

static void vBlink(void* pvParameters);

static QueueHandle_t queue;
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
    xTaskHandle read_handle;
    xTaskHandle blink_handle;

    queue = xQueueCreate(16, sizeof(uint16_t));
    xTaskCreate
        (
         vRead,
         "read",
         configMINIMAL_STACK_SIZE + 128,
         NULL,
         mainREAD_TASK_PRIORITY,
         &read_handle
        );

    xTaskCreate
        (
         vBlink,
         "blink",
         configMINIMAL_STACK_SIZE + 128,
         NULL,
         mainBLINK_TASK_PRIORITY,
         &blink_handle
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

static void vRead(void* pvParameters)
{
    uint16_t input;
    uart_init();
    stdin = stdout = stderr = &uart_file;

    while(1)
    {
       printf("Ready: ");
       scanf("%" SCNu16, &input);
       printf("%"PRIu16 "\r\n", input);
       xQueueSend(queue, (void*) &input, ( TickType_t ) 0);
    }
}

static void vBlink(void* pvParameters)
{
   DDRB |= _BV(PB4);
   uint16_t output;
   while(1)
   {
       if(xQueueReceive(queue, (void*) &output, ( TickType_t ) 100) == pdTRUE)
       {
            PORTB |= _BV(PB4);
            vTaskDelay(output / portTICK_PERIOD_MS);
            PORTB &= ~_BV(PB4);
            vTaskDelay(500 / portTICK_PERIOD_MS);
       }
   } 
}
