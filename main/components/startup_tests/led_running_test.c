/**
 * @file led_running_test.c
 * @brief Running LED Test Implementation
 */

#include "led_running_test.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <inttypes.h>

static const char *TAG = "led_running_test";

esp_err_t led_running_test_single_cycle(led_color_t color, uint32_t delay_ms)
{
    if (!led_is_initialized())
    {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t led_count = led_get_count();

    for (uint16_t i = 0; i < led_count; i++)
    {
        // Clear previous LED
        if (i > 0)
        {
            led_clear_pixel(i - 1);
        }
        else
        {
            led_clear_pixel(led_count - 1);
        }

        // Set current LED
        led_set_pixel(i, color);
        led_show();

        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }

    return ESP_OK;
}

esp_err_t led_running_test_multiple_cycles(led_color_t color, uint32_t delay_ms, uint8_t cycles)
{
    ESP_LOGI(TAG, "Running light test: %d cycles, %" PRIu32 "ms delay", cycles, delay_ms);

    for (uint8_t cycle = 0; cycle < cycles; cycle++)
    {
        ESP_LOGI(TAG, "Running light cycle %d/%d", cycle + 1, cycles);
        esp_err_t ret = led_running_test_single_cycle(color, delay_ms);
        if (ret != ESP_OK)
        {
            return ret;
        }
    }

    // Clear all LEDs after test
    led_clear_all();
    led_show();

    ESP_LOGI(TAG, "Running light test completed");
    return ESP_OK;
}

esp_err_t led_running_test_rainbow(uint32_t delay_ms, uint8_t cycles)
{
    ESP_LOGI(TAG, "Rainbow running light test: %d cycles", cycles);

    // Define rainbow colors
    led_color_t rainbow_colors[] = {
        LED_COLOR_RED,
        led_color_rgb(255, 165, 0), // Orange
        LED_COLOR_YELLOW,
        LED_COLOR_GREEN,
        LED_COLOR_CYAN,
        LED_COLOR_BLUE,
        led_color_rgb(128, 0, 128), // Purple (dimmer magenta)
    };

    uint8_t color_count = sizeof(rainbow_colors) / sizeof(rainbow_colors[0]);
    uint16_t led_count = led_get_count();

    for (uint8_t cycle = 0; cycle < cycles; cycle++)
    {
        ESP_LOGI(TAG, "Rainbow cycle %d/%d", cycle + 1, cycles);

        for (uint16_t i = 0; i < led_count; i++)
        {
            // Clear previous LED
            if (i > 0)
            {
                led_clear_pixel(i - 1);
            }
            else
            {
                led_clear_pixel(led_count - 1);
            }

            // Set current LED with rainbow color
            led_color_t current_color = rainbow_colors[i % color_count];
            led_set_pixel(i, current_color);
            led_show();

            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }

    // Clear all LEDs after test
    led_clear_all();
    led_show();

    ESP_LOGI(TAG, "Rainbow running light test completed");
    return ESP_OK;
}
