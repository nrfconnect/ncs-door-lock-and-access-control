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

The following table shows a high-level comparison of the different configurations of the solution. 
Use it to choose the one that best fits your product's use case and requirements:

.. list-table:: Door Lock solution comparison
   :header-rows: 1
   :widths: 20 25 20 20 20

   * - Solution
     - Key features
     - Best for
     - Supported SOCs
     - Recommended SOCs
   * - :ref:`Commercial (Aliro)<doc_aliro_access_control_application>`
     - * Phone or wearable unlock (NFC tap)
       * Hands-free proximity unlock with Ultra-Wideband (UWB)
     - Office buildings, hotels, co-working spaces, and enterprise campuses where employees and guests use phones or wearables as digital keys.
     - * nRF52840 (NFC only; not recommended for new designs)
       * nRF5340
       * nRF54L15 (NFC only)
       * nRF54LM20A
       * nRF54LM20B
     - * nRF54L15 (NFC only)
       * nRF54LM20A
       * nRF54LM20B
   * - :ref:`Smart Home (Matter)<doc_matter_door_lock_application>`
     - * Works with Apple, Google, Samsung, and Alexa ecosystems
       * Remote control and automation
       * PIN- and schedule-based access
     - Residential locks with full smart home ecosystem integration and keypad-based entry, at a competitive price point.
     - * nRF52840 (not recommended for new designs)
       * nRF5340
       * nRF54L15
       * nRF54LM20A
       * nRF54LM20B
     - * nRF54L15
       * nRF54LM20A
       * nRF54LM20B
   * - :ref:`Smart Home (Matter + Aliro)<doc_aliro_matter_door_lock_application>`
     - * Phone or wearable unlock (NFC tap)
       * Hands-free proximity unlock with Ultra-Wideband (UWB)
       * For Matter, it works with the Apple, Google, Samsung, and Alexa ecosystems.
         For Aliro, it works with Aliro-compatible digital wallets (for example, Apple Wallet and Samsung Wallet).
       * Remote control and automation
       * PIN- and schedule-based access
     - Residential locks combining the modern phone-as-a-key experience with full smart home ecosystem support.
     - * nRF52840 (NFC only; not recommended for new designs)
       * nRF5340
       * nRF54L15 (NFC only)
       * nRF54LM20A
       * nRF54LM20B
     - * nRF54LM20A
       * nRF54LM20B

Commercial use (Aliro only)
***************************

This solution is ideal for products aimed at office buildings, hotels, co-working spaces, gyms, hospitals, and university or enterprise campuses — anywhere employees, guests, or members need to walk up to a door and open it seamlessly with their phone, smartwatch, or wearable digital credential.

Unlike residential products, commercial access control readers do not integrate with smart-home ecosystems.
They integrate with enterprise IT systems such as badge management, HR systems, and building management platforms.
These back-end integrations are typically cloud-based, using Ethernet or Wi-Fi as the back-end protocol, and are not standardized — each solution is proprietary to its vendor.
Note also that using a smartphone as a digital key requires a separate business agreement with each digital wallet provider (for example, Apple Wallet and Samsung Wallet).

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Feature
     - Description
   * - Aliro
     - An industry standard (supported by Apple, Google, Samsung, and others) that lets any phone or wearable work with any certified reader.
       This means your locks work with iPhones, Android phones, and smartwatches out of the box.
   * - NFC (tap to unlock)
     - Reliable, fast, and works when the phone's battery is low.
   * - Bluetooth LE (hands-free unlock)
     - Carries the Aliro session with the user's phone or wearable, coordinating authentication and session management for proximity-based unlock.

       It also supports wireless firmware updates over Simple Management Protocol (SMP).
   * - UWB (hands-free unlock)
     - A short-range wireless technology that provides precise location and distance measurements.
       It allows unlocking the door when the user is within a certain proximity.
   * - Expedited-fast phase authentication
     - After the first time a user authenticates, every subsequent unlock is faster.
   * - Step-up authentication
     - For sensitive areas (server rooms, executive floors), the Reader can require additional credential verification.
       This supports tiered access policies based on the user's role in the organization.

Smart home use (Matter only, Matter with Aliro)
***********************************************

Use this solution if your product is meant for residential use — homeowners and renters who want a smart, connected lock that integrates with their smart home ecosystem.
From entry-level keypad locks to premium phone-as-a-key flagships, all share the same underlying platform.

* Matter only solution - Omits NFC, UWB, and Aliro, which significantly reduces bill-of-materials cost, PCB complexity, and certification scope.
  It delivers a competitively priced lock that still meets the needs of mainstream customers - keypad entry with full smart-home integration.

* Matter and Aliro solution - Adds Aliro on top of Matter, which delivers the modern, frictionless walk-up-and-unlock functionality, combined with the remote control, automation, and ecosystem freedom.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Features
     - Description
   * - Matter
     - The smart-home standard.
       One product, certified once, works with every major ecosystem.
   * - Thread
     - The low-power wireless mesh network that Matter uses.
       It runs on a coin-cell battery for years, and devices automatically relay messages to extend range throughout the home.
   * - Matter user and credential management
     - Built-in support for multiple users, PIN codes, and scheduled access.
       This allows users to create specific access rules.
   * - Bluetooth LE for setup and local control
     - Used during initial setup (commissioning), and optionally to give users an offline way to lock and unlock from their phone.

       It also supports wireless firmware updates over Simple Management Protocol (SMP).
   * - Aliro (optional - depends on the use case)
     - Adds the "phone-as-a-key" experience — tap-to-unlock through NFC and hands-free proximity unlock through UWB.
       The user's phone or smartwatch becomes the key, with no additional application required.
   * - Matter-based Aliro credential provisioning
     - When Aliro is included, its secure keys are delivered to the lock through the Matter ecosystem during normal setup.
   * - Over-the-air firmware updates (Matter OTA + Bluetooth LE)
     - Lets you push security patches and new features to deployed locks throughout their lifetime.
