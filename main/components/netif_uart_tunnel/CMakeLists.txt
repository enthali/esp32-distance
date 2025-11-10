# Component is only needed for QEMU builds
# However, we must register it unconditionally because CMake scans components
# before CONFIG values are available. The conditional is in the source selection.
if(CONFIG_TARGET_EMULATOR)
    idf_component_register(
        SRCS "netif_uart_tunnel_sim.c"
        INCLUDE_DIRS "."
        REQUIRES driver esp_netif lwip
    )
else()
    # Register empty component for hardware builds
    # Must still list REQUIRES so headers are available during dependency scan
    idf_component_register(
        INCLUDE_DIRS "."
        REQUIRES driver esp_netif lwip
    )
endif()
