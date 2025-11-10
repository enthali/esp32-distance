/**
 * @file netif_uart_tunnel_sim.c
 * @brief UART-based IP tunnel network interface implementation
 * 
 * This module implements a lwIP network interface driver that tunnels IP packets
 * over UART1. It provides full TCP/IP stack functionality in QEMU without
 * requiring network device emulation.
 * 
 * Implementation Details:
 * - UART1 configured at 115200 baud (can be increased for better throughput)
 * - RX task reads frames from UART and injects into lwIP
 * - TX path: lwIP calls output function, we send to UART
 * - Simple framing: [LENGTH:2][DATA:N]
 * 
 * @author ESP32 Distance Project
 * @date 2025
 */

#include "netif_uart_tunnel_sim.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "esp_event.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/etharp.h"
#include "lwip/err.h"
#include "lwip/tcpip.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "netif_uart_tunnel";

// Debug counters
static uint32_t s_rx_count = 0;
static uint32_t s_tx_count = 0;

// UART configuration
#define UART_NUM UART_NUM_1
#define UART_TX_PIN 17
#define UART_RX_PIN 16
#define UART_BAUD_RATE 115200
#define UART_BUF_SIZE 2048

// Framing protocol
#define MAX_FRAME_SIZE 1500  // Standard MTU
#define FRAME_HEADER_SIZE 2  // 2 bytes for length

// Task configuration
#define RX_TASK_STACK_SIZE 4096
#define RX_TASK_PRIORITY 2  // Lower than display_logic (3) to avoid blocking
#define UART_READ_TIMEOUT_MS 100  // Timeout for uart_read_bytes

// Module state
static esp_netif_t *s_netif_handle = NULL;
static TaskHandle_t s_rx_task_handle = NULL;
static bool s_initialized = false;
static esp_netif_driver_base_t *s_driver_base = NULL;

/**
 * @brief UART RX task - receives frames and injects into lwIP
 */
static void uart_rx_task(void *arg)
{
    uint8_t *frame_buffer = malloc(MAX_FRAME_SIZE + FRAME_HEADER_SIZE);
    if (frame_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate RX buffer");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UART RX task started");
    
    // Flush any garbage data in UART buffer
    uart_flush(UART_NUM);
    ESP_LOGI(TAG, "UART buffer flushed, waiting for stable connection...");
    //TODO: why is here a task delay?
    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait 1 second for things to settle

    while (1) {
        // Read frame length (2 bytes, big-endian) with timeout
        uint8_t len_buf[2];
        int len = uart_read_bytes(UART_NUM, len_buf, 2, pdMS_TO_TICKS(UART_READ_TIMEOUT_MS));
        
        if (len != 2) {
            // No data or timeout - yield to other tasks
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        uint16_t frame_len = (len_buf[0] << 8) | len_buf[1];
        
        if (frame_len == 0 || frame_len > MAX_FRAME_SIZE) {
            ESP_LOGW(TAG, "Invalid frame length: %d - flushing UART to resync", frame_len);
            uart_flush(UART_NUM);
            vTaskDelay(pdMS_TO_TICKS(100));  // Wait a bit before trying again
            continue;
        }
        
        ESP_LOGD(TAG, "RX: Got valid length header: %d bytes", frame_len);

        // Read frame data
        len = uart_read_bytes(UART_NUM, frame_buffer, frame_len, pdMS_TO_TICKS(1000));
        
        if (len != frame_len) {
            ESP_LOGW(TAG, "Failed to read complete frame: got %d, expected %d", len, frame_len);
            continue;
        }

        ESP_LOGD(TAG, "RX: Complete frame received: %d bytes", frame_len);
        
        // Dump first 64 bytes for debugging (verbose level)
        ESP_LOGV(TAG, "RX: First bytes (hex):");
        int dump_len = (frame_len < 64) ? frame_len : 64;
        for (int i = 0; i < dump_len; i += 16) {
            char hex_str[80];
            int pos = 0;
            for (int j = 0; j < 16 && (i + j) < dump_len; j++) {
                pos += sprintf(hex_str + pos, "%02x ", frame_buffer[i + j]);
            }
            ESP_LOGV(TAG, "  [%02d]: %s", i, hex_str);
        }

        // Inject packet into lwIP using esp_netif_receive helper
        if (s_netif_handle != NULL) {
            ESP_LOGD(TAG, "RX: Injecting %d bytes into lwIP via esp_netif_receive...", frame_len);
            
            // Get the lwIP netif from esp_netif
            struct netif *lwip_netif = esp_netif_get_netif_impl(s_netif_handle);
            if (lwip_netif == NULL) {
                ESP_LOGE(TAG, "Failed to get lwIP netif");
                continue;
            }
            
            // Debug: Check netif state
            ESP_LOGV(TAG, "RX: netif flags=0x%02x, up=%d, link_up=%d", 
                     lwip_netif->flags,
                     (lwip_netif->flags & NETIF_FLAG_UP) ? 1 : 0,
                     (lwip_netif->flags & NETIF_FLAG_LINK_UP) ? 1 : 0);
            
            // Feed to esp_netif - it takes ownership of the buffer contents
            esp_err_t err = esp_netif_receive(s_netif_handle, frame_buffer, frame_len, NULL);
            if (err != ESP_OK) {
                ESP_LOGW(TAG, "esp_netif_receive failed: %s", esp_err_to_name(err));
            } else {
                s_rx_count++;
                ESP_LOGD(TAG, "RX: Packet successfully queued (count=%lu). TX count=%lu", s_rx_count, s_tx_count);
            }
        }
    }

    free(frame_buffer);
    vTaskDelete(NULL);
}

/**
 * @brief Free RX buffer - no-op since we use stack-allocated buffer
 * 
 * This callback is called by esp_netif after processing received data.
 * Since our frame_buffer in uart_rx_task is stack-allocated (not heap),
 * we don't need to free anything here.
 */
static void driver_free_rx_buffer(void *h, void *buffer)
{
    // No-op: frame_buffer is stack-allocated in uart_rx_task
    // esp_netif calls this after copying data out of our buffer
}

/**
 * @brief lwIP linkoutput function - called by lwIP to transmit Ethernet frames
 * 
 * This is the LOW-LEVEL output function called by lwIP's Ethernet layer.
 * It receives a pbuf chain and must transmit the complete Ethernet frame.
 * 
 * Called in tcpip_thread context!
 */
static err_t netif_linkoutput(struct netif *netif, struct pbuf *p)
{
    if (p->tot_len > MAX_FRAME_SIZE) {
        ESP_LOGE(TAG, "TX: Frame too large: %d bytes", p->tot_len);
        return ERR_IF;
    }

    s_tx_count++;
    ESP_LOGD(TAG, "TX: *** LINKOUTPUT CALLED *** count=%lu, tot_len=%d", s_tx_count, p->tot_len);

    // Allocate temporary buffer for the complete frame
    uint8_t *frame_buf = (uint8_t *)malloc(p->tot_len);
    if (frame_buf == NULL) {
        ESP_LOGE(TAG, "TX: Failed to allocate frame buffer");
        return ERR_MEM;
    }

    // Copy pbuf chain into contiguous buffer
    uint16_t copied = pbuf_copy_partial(p, frame_buf, p->tot_len, 0);
    if (copied != p->tot_len) {
        ESP_LOGW(TAG, "TX: pbuf copy incomplete: %d/%d", copied, p->tot_len);
        free(frame_buf);
        return ERR_BUF;
    }

    // Send frame length header (2 bytes, big-endian)
    uint8_t len_buf[2];
    len_buf[0] = (p->tot_len >> 8) & 0xFF;
    len_buf[1] = p->tot_len & 0xFF;
    
    int written = uart_write_bytes(UART_NUM, (const char*)len_buf, 2);
    if (written != 2) {
        ESP_LOGE(TAG, "TX: Failed to write length header");
        free(frame_buf);
        return ERR_IF;
    }

    // Send frame data
    written = uart_write_bytes(UART_NUM, (const char*)frame_buf, p->tot_len);
    free(frame_buf);
    
    if (written != p->tot_len) {
        ESP_LOGE(TAG, "TX: Incomplete write: %d/%d", written, p->tot_len);
        return ERR_IF;
    }

    ESP_LOGD(TAG, "TX: Frame sent successfully: %d bytes", p->tot_len);
    return ERR_OK;
}

/**
 * @brief esp_netif transmit function - sends IP packets to UART
 * 
 * Called by esp_netif layer (legacy, kept for compatibility).
 */
static esp_err_t netif_transmit(void *h, void *buffer, size_t len)
{
    s_tx_count++;
    ESP_LOGD(TAG, "TX: *** TRANSMIT CALLED *** count=%lu, len=%d", s_tx_count, len);
    
    // STEP 2: Enable TX (UART now initialized)
    if (len > MAX_FRAME_SIZE) {
        ESP_LOGE(TAG, "Frame too large: %d bytes", len);
        return ESP_ERR_INVALID_SIZE;
    }

    ESP_LOGD(TAG, "TX: Sending %d bytes...", len);

    // Send frame length (big-endian)
    uint8_t len_buf[2];
    len_buf[0] = (len >> 8) & 0xFF;
    len_buf[1] = len & 0xFF;
    
    int written = uart_write_bytes(UART_NUM, (const char*)len_buf, 2);
    ESP_LOGD(TAG, "TX: Length header written: %d bytes", written);
    
    // Send frame data
    written = uart_write_bytes(UART_NUM, (const char*)buffer, len);
    ESP_LOGD(TAG, "TX: Frame data written: %d bytes", written);
    
    return ESP_OK;
}

/**
 * @brief Initialize UART hardware
 */
static esp_err_t init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    esp_err_t ret = uart_param_config(UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, 
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set pin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "UART1 initialized for IP tunnel at %d baud", UART_BAUD_RATE);
    
    return ESP_OK;
}

esp_err_t netif_uart_tunnel_init(const netif_uart_tunnel_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Config cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (s_initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // STEP 1: Enable UART init only (no TX/RX operations yet)
    esp_err_t ret = init_uart();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGD(TAG, "UART initialized successfully (TX/RX disabled for testing)");

    // Create esp_netif instance with custom driver
    // Use IP-only configuration (Layer 3, like PPP/TUN)
    esp_netif_inherent_config_t netif_cfg = ESP_NETIF_INHERENT_DEFAULT_ETH();
    netif_cfg.if_desc = "uart_tunnel";
    netif_cfg.route_prio = 10;
    netif_cfg.flags = ESP_NETIF_FLAG_AUTOUP;  // IP-only, no Ethernet/ARP
    netif_cfg.if_key = "UART_TUN";
    
    esp_netif_config_t cfg = {
        .base = &netif_cfg,
        .driver = NULL,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH,
    };
    
    s_netif_handle = esp_netif_new(&cfg);
    if (s_netif_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create netif");
        uart_driver_delete(UART_NUM);
        return ESP_FAIL;
    }

    // Set static IP configuration
    esp_netif_dhcpc_stop(s_netif_handle);
    
    esp_netif_ip_info_t ip_info;
    memcpy(&ip_info.ip.addr, config->ip_addr, 4);
    memcpy(&ip_info.netmask.addr, config->netmask, 4);
    memcpy(&ip_info.gw.addr, config->gateway, 4);
    
    ret = esp_netif_set_ip_info(s_netif_handle, &ip_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set IP info: %s", esp_err_to_name(ret));
        esp_netif_destroy(s_netif_handle);
        uart_driver_delete(UART_NUM);
        return ret;
    }

    // Set hostname
    if (config->hostname != NULL) {
        esp_netif_set_hostname(s_netif_handle, config->hostname);
    }

    // Attach driver with transmit function
    // Create a driver base structure
    s_driver_base = (esp_netif_driver_base_t *)malloc(sizeof(esp_netif_driver_base_t));
    if (s_driver_base == NULL) {
        ESP_LOGE(TAG, "Failed to allocate driver structure");
        esp_netif_destroy(s_netif_handle);
        uart_driver_delete(UART_NUM);
        return ESP_ERR_NO_MEM;
    }
    
    // Set up driver callbacks
    s_driver_base->post_attach = NULL;  // No post-attach callback needed
    
    // Attach driver to netif
    ret = esp_netif_attach(s_netif_handle, s_driver_base);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach driver: %s", esp_err_to_name(ret));
        free(s_driver_base);
        s_driver_base = NULL;
        esp_netif_destroy(s_netif_handle);
        uart_driver_delete(UART_NUM);
        return ret;
    }
    
    // Register transmit function and free_rx_buffer callback
    esp_netif_driver_ifconfig_t driver_cfg = {
        .handle = s_driver_base,
        .transmit = netif_transmit,
        .driver_free_rx_buffer = driver_free_rx_buffer,
    };
    
    ret = esp_netif_set_driver_config(s_netif_handle, &driver_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set driver config: %s", esp_err_to_name(ret));
        free(s_driver_base);
        s_driver_base = NULL;
        esp_netif_destroy(s_netif_handle);
        uart_driver_delete(UART_NUM);
        return ret;
    }
    ESP_LOGD(TAG, "Driver attached and configured successfully");

    // Bring interface up
    esp_netif_action_connected(s_netif_handle, 0, 0, NULL);
    
    // Set as default interface for routing
    struct netif *lwip_netif = esp_netif_get_netif_impl(s_netif_handle);
    if (lwip_netif != NULL) {
        // Set our MAC address (ESP32_MAC: 02:00:00:00:00:02)
        lwip_netif->hwaddr_len = 6;
        lwip_netif->hwaddr[0] = 0x02;
        lwip_netif->hwaddr[1] = 0x00;
        lwip_netif->hwaddr[2] = 0x00;
        lwip_netif->hwaddr[3] = 0x00;
        lwip_netif->hwaddr[4] = 0x00;
        lwip_netif->hwaddr[5] = 0x02;
        ESP_LOGD(TAG, "Set netif MAC address: 02:00:00:00:00:02");
        
        // Configure as Ethernet interface
        lwip_netif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_BROADCAST;
        
        // Set lwIP callbacks (CRITICAL - without these, netif->input is NULL!)
        lwip_netif->input = tcpip_input;           // RX path: feeds packets to TCP/IP thread
        lwip_netif->output = etharp_output;        // TX path: IPv4 with ARP
        lwip_netif->linkoutput = netif_linkoutput; // LOW-LEVEL TX: called with pbuf to send frame
        ESP_LOGD(TAG, "Set lwIP netif callbacks (input, output, linkoutput)");
        
        netif_set_default(lwip_netif);
        ESP_LOGD(TAG, "Set as default netif for routing");        // Add static ARP entry for gateway (point-to-point, no ARP needed)
        // Gateway MAC: 02:00:00:00:00:01 (HOST_MAC from bridge)
        ip4_addr_t gw_ip;
        struct eth_addr gw_mac = {{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}};
        memcpy(&gw_ip.addr, config->gateway, 4);
        
        err_t arp_err = etharp_add_static_entry(&gw_ip, &gw_mac);
        if (arp_err == ERR_OK) {
            ESP_LOGD(TAG, "Added static ARP entry for gateway");
        } else {
            ESP_LOGW(TAG, "Failed to add static ARP entry: %d", arp_err);
        }
    }

    // STEP 3: Start RX task with low priority and timeout
    BaseType_t task_ret = xTaskCreate(uart_rx_task, "uart_rx", RX_TASK_STACK_SIZE, 
                                      NULL, RX_TASK_PRIORITY, &s_rx_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create RX task");
        esp_netif_destroy(s_netif_handle);
        uart_driver_delete(UART_NUM);
        return ESP_FAIL;
    }
    ESP_LOGD(TAG, "UART RX task started (priority %d, timeout %dms)", 
             RX_TASK_PRIORITY, UART_READ_TIMEOUT_MS);

    s_initialized = true;
    
    ESP_LOGI(TAG, "UART tunnel initialized: %d.%d.%d.%d/%d.%d.%d.%d gw %d.%d.%d.%d",
             config->ip_addr[0], config->ip_addr[1], config->ip_addr[2], config->ip_addr[3],
             config->netmask[0], config->netmask[1], config->netmask[2], config->netmask[3],
             config->gateway[0], config->gateway[1], config->gateway[2], config->gateway[3]);

    return ESP_OK;
}

esp_err_t netif_uart_tunnel_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

        // STEP 3: Stop RX task
    if (s_rx_task_handle != NULL) {
        vTaskDelete(s_rx_task_handle);
        s_rx_task_handle = NULL;
        ESP_LOGI(TAG, "UART RX task stopped");
    }

    // Destroy netif
    if (s_netif_handle != NULL) {
        esp_netif_destroy(s_netif_handle);
        s_netif_handle = NULL;
    }

    if (s_driver_base != NULL) {
        free(s_driver_base);
        s_driver_base = NULL;
    }

    // STEP 1: Cleanup UART (now initialized)
    uart_driver_delete(UART_NUM);
    ESP_LOGI(TAG, "UART driver deleted");

    s_initialized = false;
    ESP_LOGI(TAG, "UART tunnel deinitialized");

    return ESP_OK;
}

esp_netif_t* netif_uart_tunnel_get_handle(void)
{
    return s_netif_handle;
}
