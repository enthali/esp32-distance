Known Issues and Limitations
=============================

*Welcome, brave explorer! Every project has its dragons — here are ours.
Don't panic if you encounter these: you're not alone, and you'll probably
survive.*


QEMU / Emulation
----------------

HTTPS not available in emulator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:Origin: :need:`REQ_SYS_SIM_2` (QEMU network simulation)

:Description:
   The QEMU network stack bridges traffic through a UART tunnel.
   The ``cert_handler`` component exists but TLS handshake behaviour in
   QEMU has not been validated. HTTPS testing is therefore not supported
   in the emulator — you might get an encrypted error message, which is
   somehow worse than a plain-text one.

:Workaround:
   Use HTTP (port 80) when testing in QEMU. HTTPS validation requires
   real hardware.

QEMU web interface requires HTTP proxy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:Origin: :need:`REQ_SYS_SIM_2` (host browser access to emulated network)

:Description:
   The emulated ESP32 lives on a virtual network (``192.168.100.0/24``).
   Accessing ``http://192.168.100.2`` from the Codespaces browser requires
   an HTTP proxy — the forwarded port 8080 goes through a Python proxy
   rather than directly to the ESP32, because the virtual TUN device is
   not directly routable from the browser.

:Workaround:
   Use the proxy URL ``http://localhost:8080`` (or the Codespaces
   forwarded URL for port 8080). See :doc:`qemu-emulator` for details.


Hardware / Peripherals
-----------------------

*No known issues at this time. Found a new dragon? See* `Reporting New Issues`_ *below.*


Networking
----------

HTTPS not implemented (HTTP only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:Origin: :need:`REQ_SYS_SEC_1` (HTTPS support marked as future/open)

:Description:
   The web interface runs on HTTP only — your credentials travel in plain
   text like postcards from the 1980s. We have a ``cert_handler`` component
   gathering dust in the garage, but TLS on ESP32 involves certificate
   management, memory pressure, and a surprising number of opinions.
   **Not suitable for production** unless you enjoy living dangerously.

:Workaround:
   Use only on trusted networks (your home lab, not the coffee shop).
   For production deployments: implement HTTPS using the existing
   ``cert_handler`` component, or deploy behind a VPN.

Captive portal auto-detection unreliable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:Origin: :need:`REQ_WEB_3` (captive portal WiFi setup page)

:Description:
   When you connect to the ESP32's WiFi access point, modern phones and
   tablets stubbornly refuse to show the magical "Sign in to network"
   popup. Our DNS server is... let's call it "minimalist" (it doesn't
   exist). iOS, Android, and Windows all have different ideas about what
   makes a proper captive portal, and we didn't bribe any of them
   sufficiently. You end up manually typing ``http://192.168.4.1`` into
   your browser like it's 1995.

:Workaround:
   Navigate directly to ``http://192.168.4.1`` — document this address
   prominently for your users and save everyone the confusion.


Build System
-------------

*No known issues at this time. Found a new dragon? See* `Reporting New Issues`_ *below.*


.. _Reporting New Issues:

Reporting New Issues
---------------------

Found a new dragon? A quirk that makes you question your life choices?
We want to know — and whoever finds a bug gets to keep it (briefly),
before filing a GitHub Issue.

**How to report:**

1. Check if it's already documented here (maybe we're aware of the chaos)
2. Open a GitHub Issue with a descriptive title
3. Include: ESP-IDF version, hardware variant, build configuration
4. Include reproduction steps — "it doesn't work" does not help
5. Bonus points for a fix or workaround

----

.. note::
   Remember: if everything worked perfectly, it wouldn't be called a
   *project* — it would be called a *product*. And products are boring.

*Last Updated: 2026-03-17*
