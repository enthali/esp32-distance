Switching Development Modes
===========================

The project supports two build targets that are selected at compile time:

- **Emulator mode** – runs in QEMU without physical hardware
- **Hardware mode** – runs on a real ESP32 device (default)

The switch is controlled by the ``CONFIG_TARGET_EMULATOR`` Kconfig flag, which
is defined in ``main/Kconfig.projbuild``.

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Setting
     - Emulator mode
     - Hardware mode
   * - ``CONFIG_TARGET_EMULATOR``
     - ``y``
     - not set (default)
   * - ``CONFIG_EMULATOR_MOCK_DATA``
     - ``y`` (default)
     - n/a
   * - Flash target
     - QEMU via ``run_qemu.sh``
     - Physical ESP32 via USB

How to Switch
-------------

Option 1: menuconfig (recommended)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   idf.py menuconfig

Navigate to **Build Target Configuration** and toggle:

- **Build for QEMU emulator** – enables emulator mode
- **Use mocked data in emulator** – replaces hardware drivers with simulated data

Save and exit, then rebuild:

.. code-block:: bash

   idf.py build

Option 2: Edit sdkconfig directly
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open ``sdkconfig`` and find the relevant lines:

**Enable emulator mode:**

.. code-block:: text

   CONFIG_TARGET_EMULATOR=y
   CONFIG_EMULATOR_MOCK_DATA=y

**Disable emulator mode (hardware build):**

.. code-block:: text

   # CONFIG_TARGET_EMULATOR is not set

Then rebuild:

.. code-block:: bash

   idf.py build

Option 3: One-liner on the command line
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Emulator build
   idf.py -DCONFIG_TARGET_EMULATOR=y build

   # Hardware build (default, no extra flag needed)
   idf.py build

.. note::
   After switching modes you need a full rebuild because different source files
   are compiled for each target.  A ``idf.py fullclean`` is only required if
   you see unexpected linker errors after switching.

What changes between modes
--------------------------

When ``CONFIG_TARGET_EMULATOR=y`` is set:

- Hardware-specific peripheral drivers (distance sensor, LED controller, display)
  are replaced by mock implementations that generate simulated data.
- The ``netif_uart_tunnel`` component is enabled to bridge networking through
  QEMU's UART interface (required for HTTP/WebSocket access from the host).
- Physical GPIO and bus assignments are ignored.

When running on hardware (default):

- Real peripheral drivers are compiled in.
- The UART network tunnel is not needed and is excluded from the build.
- Flash/JTAG upload is used to deploy the firmware.

Typical Workflow
----------------

.. code-block:: text

   ┌─────────────────────────────────────────────────────────┐
   │  Development cycle                                      │
   │                                                         │
   │  1. Switch to emulator mode  (menuconfig or sdkconfig)  │
   │  2. idf.py build                                        │
   │  3. ./tools/qemu/run_qemu.sh   (or Run QEMU task)       │
   │  4. Test & iterate                                      │
   │                                                         │
   │  5. Switch to hardware mode  (menuconfig or sdkconfig)  │
   │  6. idf.py build                                        │
   │  7. idf.py flash              (or Web Flasher task)     │
   └─────────────────────────────────────────────────────────┘

See also :doc:`qemu-emulator` for details on running and debugging in QEMU,
and :doc:`web-flasher` for flashing physical hardware.
