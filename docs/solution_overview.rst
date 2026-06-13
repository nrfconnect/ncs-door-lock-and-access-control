.. _solution_overview:

Solution overview
#################

.. contents::
   :local:
   :depth: 2

Nordic Semiconductor's nRF Door Lock and Access Control solution gives you the flexibility to deliver the product based on your specification — using a single, unified development platform.
There are two primary market verticals - commercial and smart home - each with different requirements and user expectations.

Comparison
**********

See the table below for a high-level comparison of the different configurations of the solution and to choose the one that best fits your product's use case and requirements:

.. list-table:: Door Lock solution comparison
   :header-rows: 1
   :widths: 25 35 25 25

   * - Solution
     - Key features
     - Best for
     - Supported devices
   * - Commercial (Aliro)
     - * Phone or wearable unlock (NFC tap)
       * Hands-free proximity unlock with Ultra-Wideband (UWB)
     - Office buildings, hotels, co-working spaces, and enterprise campuses where employees and guests use phones or wearables as digital keys.
     - * `nRF52840 DK`_
       * `nRF5340 DK`_ (supports UWB)
       * `nRF54L15 DK`_
       * `nRF54LM20 DK`_ (supports UWB)
   * - Smart Home (Matter)
     - * Works with Apple, Google, Samsung, and Alexa ecosystems
       * Remote control and automation
       * PIN keypad and scheduled guest access
     - Residential locks with full smart home ecosystem integration and keypad-based entry, at a competitive price point.
     - * `nRF54LM20 DK`_
       * `nRF5340 DK`_
   * - Smart Home (Aliro + Matter)
     - * Phone or wearable unlock (NFC tap)
       * Hands-free proximity unlock with Ultra-Wideband (UWB)
       * Works with Apple
       * Remote control and automation
       * PIN keypad and scheduled guest access
     - Residential locks combining the modern phone-as-a-key experience with full smart home ecosystem support.
     - * `nRF54LM20 DK`_
       * `nRF5340 DK`_

Commercial use (Aliro only)
***************************

This solution is ideal if your product is meant for office buildings, hotels, co-working spaces, gyms, hospitals, and enterprise campuses - anywhere employees, guests, or members need to walk up to a door and have it open seamlessly using their phone, smartwatch, or wearable digital credential.

Aliro only solution focuses on the needs of commercial buildings, which do not integrate with smart-home ecosystems — they integrate with enterprise IT systems (badge management, HR systems, building management platforms).
It delivers everything required: a fast, secure, interoperable user experience while keeping the product's bill of materials and certification scope focused on the specific commercial use case.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Technology
     - Features
   * - Aliro
     - An industry standard (supported by Apple, Google, Samsung, and others) that lets any phone or wearable work with any certified reader.
       This means your locks work with iPhones, Android phones, and smartwatches out of the box.
   * - NFC (Tap to unlock)
     - Reliable, fast, and works even when the phone battery is low.
   * - Bluetooth LE + UWB (Hands-free unlock)
     - UWB - a short-range wireless technology that provides precise location and distance measurements.
       It allows unlocking the door when the user is within a certain proximity.
   * - Expedited-fast authentication
     - After the first time a user authenticates, every subsequent unlock is dramatically faster.
       For high-traffic doors at rush hour, this means no queues.
   * - Step-up authentication
     - For sensitive areas (server rooms, executive floors), the reader can require additional credential verification — supporting tiered access policies tied to the user's role in the organization.
   * - Hardware-backed security (PSA Crypto)
     - All cryptographic keys are stored in protected hardware, making it virtually impossible to extract them - meeting the security requirements of enterprise customers.

Smart home use (Matter only, Matter with Aliro)
***********************************************

This solution is ideal if your product is meant for residential use — homeowners and renters who want a smart, connected lock that integrates with their smart home ecosystem — Apple Home, Google Home, Samsung SmartThings, or Amazon Alexa.
From entry-level keypad locks to premium phone-as-a-key flagships, all share the same underlying platform.

* Matter only solution - Omits NFC, UWB, and Aliro, which significantly reduces bill-of-materials cost, PCB complexity, and certification scope — delivering a competitively priced lock that still offers everything mainstream customers actually use (keypad entry with full smart-home integration).

* Matter and Aliro solution - Adds Aliro on top of Matter, which delivers the modern, frictionless walk-up-and-unlock functionality, combined with the remote control, automation, and ecosystem freedom that Matter provides.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Technology
     - Features
   * - Matter
     - The smart-home standard.
       One product, certified once, works with every major ecosystem.
   * - Thread
     - The low-power wireless mesh network that Matter uses.
       It runs on a coin-cell battery for years, and devices automatically relay messages to extend range throughout the home.
   * - Matter user and credential management
     - Built-in support for multiple users, PIN codes, and scheduled access (allowing users to create specific access rules)
   * - Bluetooth LE for setup and local control
     - Used during initial setup (commissioning), and optionally to give users an offline way to lock and unlock from their phone if the internet is down.
   * - Aliro (optional - depends on the use case)
     - Adds the "phone-as-a-key" experience — tap-to-unlock through NFC and hands-free proximity unlock through UWB.
       The user's phone or smartwatch becomes the key, with no additional application required.
   * - Matter-based Aliro credential provisioning
     - When Aliro is included, its secure keys are delivered to the lock through the Matter ecosystem during normal setup.
   * - Over-the-air firmware updates (Matter OTA + Bluetooth LE)
     - Lets you push security patches and new features to deployed locks throughout their lifetime.
