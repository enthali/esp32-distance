/**
 * @file netif_uart_tunnel_sim.h
 * @brief UART-based IP tunnel network interface for QEMU simulator
 * 
 * Implements a lwIP network interface that tunnels IP packets over UART1.
 * This allows full TCP/IP stack functionality in QEMU without network device emulation.
 * 
 * Architecture:
 * - ESP32 (QEMU): lwIP stack -> netif -> UART1 -> TCP socket
 * - Host: TCP socket -> TUN device -> Linux network stack
 * 
 * Framing Protocol:
 * - Simple length-prefixed frames: [LENGTH:2 bytes][DATA:N bytes]
 * - Length is big-endian uint16_t
 * - Maximum frame size: 1500 bytes (MTU)
 * 
 * UART Configuration:
 * - UART1 used for packet transport (UART0 reserved for console)
 * - Baud rate: 115200 (configurable in implementation)
 * - Pins: TX=GPIO4, RX=GPIO5 (QEMU default UART1 mapping)
 * 
 * Requirements Traceability:
 * - REQ_NETIF_TUNNEL_1: QEMU UART Network Bridge
 * - REQ_NETIF_TUNNEL_2: Packet Encapsulation
 * - REQ_NETIF_TUNNEL_5: Conditional Compilation
 * 
 * Typical Usage:
 * @code
 * netif_uart_tunnel_config_t config = {
 *     .hostname = "esp32-qemu",
 *     .ip_addr = {192, 168, 100, 2},
 *     .netmask = {255, 255, 255, 0},
 *     .gateway = {192, 168, 100, 1}
 * };
 * 
 * esp_err_t ret = netif_uart_tunnel_init(&config);
 * if (ret != ESP_OK) {
 *     ESP_LOGE(TAG, "Failed to init tunnel: %s", esp_err_to_name(ret));
 *     return ret;
 * }
 * 
 * // Network stack is now ready - use TCP/IP APIs normally
 * // ...
 * 
 * netif_uart_tunnel_deinit();
 * @endcode
 * 
 * @note This is a simulator-only component (_sim suffix)
 * @author ESP32 Distance Project
 * @date 2025
 */

#ifndef NETIF_UART_TUNNEL_SIM_H
#define NETIF_UART_TUNNEL_SIM_H

#include "esp_err.h"
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuration for UART tunnel network interface
 */
typedef struct {
    const char* hostname;          ///< Hostname for the interface
    uint8_t ip_addr[4];           ///< Static IP address (e.g., {192,168,100,2})
    uint8_t netmask[4];           ///< Netmask (e.g., {255,255,255,0})
    uint8_t gateway[4];           ///< Gateway IP (e.g., {192,168,100,1})
} netif_uart_tunnel_config_t;

/**
 * @brief Initialize UART tunnel network interface
 * 
 * Implements REQ_NETIF_TUNNEL_1: QEMU UART Network Bridge
 * 
 * Creates and configures a lwIP network interface that tunnels IP packets
 * over UART1. This allows full TCP/IP stack functionality in QEMU.
 * Starts a background task to handle packet reception from UART.
 * 
 * This function must be called after NVS and event loop initialization,
 * but before any network operations.
 * 
 * @param config Network configuration (IP, netmask, gateway)
 * 
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if config is NULL or contains invalid values
 * @return ESP_ERR_NO_MEM if memory allocation fails
 * @return ESP_FAIL if UART or lwIP initialization fails
 * 
 * @note This function is thread-safe and can be called from any task.
 * @note Only call once; subsequent calls will return ESP_ERR_INVALID_STATE.
 */
esp_err_t netif_uart_tunnel_init(const netif_uart_tunnel_config_t *config);

/**
 * @brief Deinitialize UART tunnel network interface
 * 
 * Implements REQ_NETIF_TUNNEL_1: Resource cleanup for network bridge
 * 
 * Stops the receive task and cleans up resources. This function will wait
 * for the receive task to terminate gracefully. Active network connections
 * will be closed by lwIP stack.
 * 
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_STATE if interface was not initialized
 * @return ESP_FAIL if cleanup fails
 * 
 * @note This function is thread-safe and can be called from any task.
 * @note Calling multiple times is safe (idempotent after first call).
 * @note All active TCP/UDP connections will be terminated.
 */
esp_err_t netif_uart_tunnel_deinit(void);

/**
 * @brief Get the esp_netif handle for the UART tunnel interface
 * 
 * Implements REQ_NETIF_TUNNEL_1: Access to lwIP network interface
 * 
 * Returns the esp_netif handle for the UART tunnel interface.
 * Useful for registering event handlers or querying interface status.
 * 
 * Example uses:
 * - Registering IP change event handlers
 * - Querying IP address and netmask
 * - Checking link status
 * 
 * @return esp_netif handle if interface is initialized
 * @return NULL if netif_uart_tunnel_init() has not been called yet
 * 
 * @note This function is thread-safe and can be called from any task.
 * @note The returned handle remains valid until netif_uart_tunnel_deinit() is called.
 */
esp_netif_t* netif_uart_tunnel_get_handle(void);

#ifdef __cplusplus
}
#endif

#endif // NETIF_UART_TUNNEL_SIM_H
