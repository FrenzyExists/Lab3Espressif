#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define BLINK_GPIO GPIO_NUM_2 // Use the GPIO pin that the LED is connected to.

void app_main(void) {
    // Configure the GPIO pin for the LED as an output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BLINK_GPIO), // Pin to be configured
        .mode = GPIO_MODE_OUTPUT,            // Set as output mode
        .pull_up_en = GPIO_PULLUP_DISABLE,   // Disable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // Enable pull-down resistor
        .intr_type = GPIO_INTR_DISABLE        // Disable GPIO interrupt
    };
    // Initialize GPIO with the given settings
    gpio_config(&io_conf);

    while (1) {
        // Turn the LED on (GPIO level high)
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1000 ms

        // Turn the LED off (GPIO level low)
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1000 ms
    }
}