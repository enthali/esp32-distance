Known Issues and Limitations
=============================

*Welcome, brave explorer! Every project has its dragons—here are ours. Don't panic if you encounter these: you're not alone, and you'll probably survive.*

🔒 HTTPS: The Encryption That Isn't There Yet
----------------------------------------------

**What's Missing?** Security. Sweet, sweet HTTPS encryption.

**Current Reality:**

- HTTP web server works beautifully in both QEMU and hardware
- HTTPS/SSL/TLS? Not yet. We have a certificate handler component gathering dust in the garage
- Your credentials are transmitted in plain text like postcards from the 1980s

**Why Should You Care?**

- Web interface is completely unencrypted (HTTP only)
- Any credentials you send are readable by anyone on the WiFi
- **Not suitable for production** unless you enjoy living dangerously

**What Can You Do?**

- Use only on trusted networks (your home lab, not the coffee shop)
- For production: Implement HTTPS before deployment (certificate handler is there, just needs some love)
- Or deploy behind a VPN and pretend the problem doesn't exist

**Status:** Open - waiting for someone brave enough to tackle TLS on ESP32

📱 The Not-So-Captive Captive Portal
-------------------------------------

**The Problem:** Your phone thinks it's smarter than our DNS server (spoiler: there is no DNS server).

**What Happens?**

- You connect to the ESP32's WiFi access point
- Your phone/tablet stubbornly refuses to show the magical "Sign in to network" popup
- You stare at your screen wondering if WiFi is working
- Eventually you give up and manually type ``http://192.168.4.1`` into your browser

**Why?**

- Modern devices are *extremely* picky about captive portals
- Our DNS server is... let's call it "minimalist" (it doesn't exist)
- iOS, Android, and Windows all have different ideas about what makes a proper captive portal
- We didn't bribe any of them sufficiently

**Impact:**

- Reduced "wow factor" when showing off your IoT device
- Users must manually discover the magic IP address: ``192.168.4.1``
- You'll feel like it's 1995 and you're explaining IP addresses to your parents

**Status:** Open - low priority (most users figure it out after mild confusion)

**Pro Tip:** Just document ``http://192.168.4.1`` in big, friendly letters and save everyone the trouble.

🦄 Other Oddities and Surprises
--------------------------------

Found a new dragon? A quirk that makes you question your life choices? We want to know!

**How to Report:**

1. Check if it's already documented here (maybe we're aware of the chaos)
2. Open a GitHub Issue with a descriptive title
3. Include reproduction steps (be specific - "it doesn't work" doesn't help)
4. Tell us: ESP-IDF version, hardware variant, build configuration
5. Bonus points for including a fix or workaround

----

.. note::
   Remember: If everything worked perfectly, it wouldn't be called a "project," it would be called a "product." And products are boring.

*Last Updated: November 2025*

*Last Debugging Session That Made Us Add This Document: Too many to count*
