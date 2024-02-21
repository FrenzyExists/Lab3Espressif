#include <stdio.h>
#include "math.h"
#include "string.h"
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <freertos/semphr.h>


#define LED_PIN         GPIO_NUM_2
#define SOFT_BUTTON_PIN GPIO_NUM_22
#define HARD_BUTTON_PIN GPIO_NUM_23

#define A GPIO_NUM_27
#define B GPIO_NUM_25
#define C GPIO_NUM_33
#define D GPIO_NUM_32
#define E GPIO_NUM_18
#define F GPIO_NUM_19
#define G GPIO_NUM_5

#define S7_1 GPIO_NUM_21
#define S7_2 GPIO_NUM_4



volatile bool blinkEnabled = true;
bool blinky = true;
const int increment = 1; // delta count
volatile int myCount = 0;

const int digitalBinary[10] = {
   // abcdefg
    0b0000001,  // 0
    0b1001111,  // 1
    0b0010010,  // 2
    0b0000110,  // 3
    0b1001100,  // 4
    0b0100100,  // 5
    0b0100000,  // 6
    0b0001111,  // 7
    0b0000000,  // 8
    0b0001100   // 9
};


SemaphoreHandle_t buttonSemaphore;
QueueHandle_t xQueue;


void blinkTask(void *pvParameters) {
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    for (;;) {
        if (blinkEnabled) {
            gpio_set_level(LED_PIN, blinky); // Turn LED on
            blinky = !blinky;
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            vTaskDelay(50 / portTICK_PERIOD_MS); // Delay 100ms when blinking is disabled
        }
    }
}


void IRAM_ATTR buttonISR(void *arg) {
    int whoPressedMe = (int *) arg;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (whoPressedMe == 1) {
        xSemaphoreGiveFromISR(buttonSemaphore, &xHigherPriorityTaskWoken);
    } else if (whoPressedMe == 2) {
        xQueueSendFromISR(xQueue, &increment, &xHigherPriorityTaskWoken);
    }
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}


void toggleBlinking(void *pvParameters) {
for (;;) {
        // Wait for the button semaphore
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE) {
            // Perform task actions here
            printf("Button pressed. Task executed.\n");
            blinkEnabled = !blinkEnabled;
        }
    }
}


void countTask(void *pvParameters) {
    int pinNumber;
    for(;;) {
        printf("Count: %d\n", myCount);
        if(xQueueReceive(xQueue, &pinNumber, portMAX_DELAY)) {
            myCount += 1;
            printf("My Count Count: %d\n", myCount);
        }
        vTaskDelay(15.51);
    }
}

void sevenSegmentTask(void *pvParameters) {

    for (;;) {
        gpio_set_level(A, 1);
        gpio_set_level(B, 1);
        gpio_set_level(C, 1);
        gpio_set_level(D, 1);
        gpio_set_level(E, 1);
        gpio_set_level(F, 1);
        gpio_set_level(G, 1);

        gpio_set_level(S7_1, 0);
        gpio_set_level(S7_2, 1);

        gpio_set_level(A, (digitalBinary[(int)floor(myCount / 10)] >> 6) & 1);
        gpio_set_level(B, (digitalBinary[(int)floor(myCount / 10)] >> 5) & 1);
        gpio_set_level(C, (digitalBinary[(int)floor(myCount / 10)] >> 4) & 1);
        gpio_set_level(D, (digitalBinary[(int)floor(myCount / 10)] >> 3) & 1);
        gpio_set_level(E, (digitalBinary[(int)floor(myCount / 10)] >> 2) & 1);
        gpio_set_level(F, (digitalBinary[(int)floor(myCount / 10)] >> 1) & 1);
        gpio_set_level(G, (digitalBinary[(int)floor(myCount / 10)] >> 0) & 1);

        vTaskDelay(1 / portTICK_PERIOD_MS); // Delay for 1 second

        gpio_set_level(A, 1);
        gpio_set_level(B, 1);
        gpio_set_level(C, 1);
        gpio_set_level(D, 1);
        gpio_set_level(E, 1);
        gpio_set_level(F, 1);
        gpio_set_level(G, 1);

        gpio_set_level(S7_1, 1);
        gpio_set_level(S7_2, 0);


        gpio_set_level(A, (digitalBinary[myCount % 10] >> 6) & 1);
        gpio_set_level(B, (digitalBinary[myCount % 10] >> 5) & 1);
        gpio_set_level(C, (digitalBinary[myCount % 10] >> 4) & 1);
        gpio_set_level(D, (digitalBinary[myCount % 10] >> 3) & 1);
        gpio_set_level(E, (digitalBinary[myCount % 10] >> 2) & 1);
        gpio_set_level(F, (digitalBinary[myCount % 10] >> 1) & 1);
        gpio_set_level(G, (digitalBinary[myCount % 10] >> 0) & 1);

        vTaskDelay(1/portTICK_PERIOD_MS); // Delay for 1 second
    }
}

void initSevenSegment() {
    gpio_set_direction(A,   GPIO_MODE_OUTPUT);
    gpio_set_direction(B,   GPIO_MODE_OUTPUT);
    gpio_set_direction(C,   GPIO_MODE_OUTPUT);
    gpio_set_direction(D,   GPIO_MODE_OUTPUT);
    gpio_set_direction(E,   GPIO_MODE_OUTPUT);
    gpio_set_direction(F,   GPIO_MODE_OUTPUT);
    gpio_set_direction(G,   GPIO_MODE_OUTPUT);

    gpio_set_direction(S7_1,   GPIO_MODE_OUTPUT);
    gpio_set_direction(S7_2,   GPIO_MODE_OUTPUT);
}


void app_main() {
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SOFT_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(SOFT_BUTTON_PIN, GPIO_INTR_NEGEDGE); // Trigger interrupt on falling edge

    gpio_set_direction(HARD_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(HARD_BUTTON_PIN, GPIO_INTR_NEGEDGE); // Trigger interrupt on falling edge

    // seven-segment
    initSevenSegment();


    xQueue =  xQueueCreate(10, sizeof(int));
    // Create button semaphore
    buttonSemaphore = xSemaphoreCreateBinary();

    gpio_install_isr_service(0);
    gpio_isr_handler_add(SOFT_BUTTON_PIN, buttonISR, (void *)1);
    gpio_isr_handler_add(HARD_BUTTON_PIN, buttonISR, (void *)2);

    xTaskCreatePinnedToCore(&toggleBlinking,  "TOGGLE_BLINKY", 0x8FFUL, NULL, 0, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(&blinkTask,       "BLINKY",        0x8FFUL, NULL, 0, NULL, tskNO_AFFINITY); 
    xTaskCreatePinnedToCore(&sevenSegmentTask, "SEVEN_SEGMENT", 0xFFFFUL, NULL, 0, NULL, tskNO_AFFINITY); 
    xTaskCreatePinnedToCore(&countTask,        "COUNT",         0x8FFUL, NULL, 0, NULL, tskNO_AFFINITY); 
}



/**
 * is it a one-time payment
 * 
 * si tiene cama
 * si tiene estufa y nevera
 * 
 * tel y
 * 
 * has no wifi, parking lot
 * how parking lot works
 * que tantos supermercados, tiendas, stuff like that
 * 
*/