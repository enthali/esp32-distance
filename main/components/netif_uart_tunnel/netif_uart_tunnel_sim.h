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
 * Creates and configures a lwIP network interface that tunnels IP packets
 * over UART1. This allows full TCP/IP stack functionality in QEMU.
 * 
 * @param config Network configuration (IP, netmask, gateway)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t netif_uart_tunnel_init(const netif_uart_tunnel_config_t *config);

/**
 * @brief Deinitialize UART tunnel network interface
 * 
 * Stops the receive task and cleans up resources.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t netif_uart_tunnel_deinit(void);

/**
 * @brief Get the esp_netif handle for the UART tunnel interface
 * 
 * Useful for registering event handlers or querying interface status.
 * 
 * @return esp_netif handle or NULL if not initialized
 */
esp_netif_t* netif_uart_tunnel_get_handle(void);

#ifdef __cplusplus
}
#endif

#endif // NETIF_UART_TUNNEL_SIM_H
