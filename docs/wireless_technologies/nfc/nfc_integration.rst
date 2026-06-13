.. _nfc_integration:

NFC integration in the reference applications
#############################################

.. contents::
   :local:
   :depth: 2

This page explains how NFC functionality is integrated into the |REPO_NAME|, covering the interaction between the Aliro stack and NFC hardware, the reference implementation using STMicroelectronics components, and guidelines for integrating alternative NFC reader chips.

The |REPO_NAME| includes a reference implementation using STMicroelectronics NFC transceivers and the RFAL library (``NfcTransportRfal``).
However, any ISO14443-A compatible NFC reader chip can be used by replacing the NfcTransportRfal class with your own implementation.
The Aliro Stack architecture is designed to be vendor-agnostic.

Architecture overview
*********************

The NFC integration in the |REPO_NAME| follows a layered architecture that separates protocol logic from hardware-specific implementation. The diagram below shows the reference implementation using STMicroelectronics RFAL, which can be replaced with any compatible NFC reader solution:

.. code-block:: none

   ┌─────────────────────────────────────┐
   │         AliroStack                  │
   │    (Protocol Implementation)        │
   │   HandleSessionData(), Send()       │
   └─────────────┬───────────────────────┘
                 │  Aliro Stack Public API
   ┌─────────────▼───────────────────────┐
   │    NFC Transport Implementation     │
   │     (Reference: NfcTransportRfal)   │
   │   Init(), Start(), Send(), etc.     │ ← Replaceable with any NFC chip
   └─────────────┬───────────────────────┘
                 │ Vendor-specific API
   ┌─────────────▼───────────────────────┐
   │   NFC Hardware Abstraction Layer    │
   │    (Reference: RFAL Library)        │ ← Replace with vendor SDK/driver
   └─────────────┬───────────────────────┘
                 │ SPI/I2C/Hardware Interface
   ┌─────────────▼───────────────────────┐
   │      NFC Reader Transceiver         │
   │   (Reference: ST25R200/ST25R300)    │ ← Replace with any ISO14443-A chip
   └─────────────────────────────────────┘

The Aliro stack communicates directly with an NFC transport implementation class.
The reference implementation (``NfcTransportRfal``) handles all NFC operations through the RFAL library.
This class can be replaced with any NFC reader implementation that can provide a similar interface.
There is no abstract transport interface for NFC - the integration uses concrete classes with direct method calls.

Reference NFC implementation (STMicroelectronics)
*************************************************

The |REPO_NAME| includes a reference implementation for STMicroelectronics NFC transceivers using the RFAL (RF Abstraction Layer) library. This implementation serves as an example and can be replaced with any NFC reader chip that supports ISO14443-A protocol.

RFAL integration
================

The RF Abstraction Layer (RFAL) provides a hardware-agnostic interface to STM NFC transceivers:

* **Hardware abstraction**: Unified API for different STM NFC chips (ST25R200, ST25R300)
* **Protocol support**: ISO14443-A/B, ISO15693, and proprietary modes
* **Power optimization**: Advanced power management and field control
* **Interrupt handling**: Efficient event-driven communication

The RFAL library is integrated through platform-specific adapters that:

* Map RFAL API calls to Zephyr kernel services
* Handle GPIO configuration for IRQ and enable pins
* Manage SPI communication with the NFC transceiver
* Implement timing and threading requirements

Platform abstraction layer
===========================

The platform abstraction layer bridges RFAL and the Zephyr RTOS:

.. code-block:: none

   RFAL Library
        ↓
   Platform Abstraction
        ↓
   ┌─────────────┬─────────────┬─────────────┐
   │    SPI      │    GPIO     │   Timers    │
   │  (Zephyr)   │  (Zephyr)   │  (Zephyr)   │
   └─────────────┴─────────────┴─────────────┘

Key platform services include:

* **SPI communication**: High-speed data transfer to NFC transceiver
* **GPIO management**: Control of IRQ, enable, and other control pins
* **Timer services**: Precise timing for protocol timeouts and delays
* **Threading**: Interrupt handling and worker thread coordination
* **Power management**: Integration with Zephyr power management subsystem

Supported transceivers
======================

Please see :ref:`hw_requirements_nfc_reader` for the list of the supported NFC readers and expansion boards.

Configuration and customization
********************************

The NFC integration can be configured through Kconfig options and device tree settings.

Kconfig options
===============

The following table lists the key RFAL Kconfig options available for NFC configuration:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_RFAL_FEATURE_LISTEN_MODE``
     - Enable or disable RFAL support for Listen Mode (card emulation).
   * - ``CONFIG_RFAL_FEATURE_WAKEUP_MODE``
     - Enable or disable RFAL support for Wake-Up mode (low power polling).
   * - ``CONFIG_RFAL_FEATURE_LOWPOWER_MODE``
     - Enable or disable RFAL support for Low Power mode.
   * - ``CONFIG_RFAL_FEATURE_NFCA``
     - Enable or disable RFAL support for NFC-A (ISO14443A) technology.
   * - ``CONFIG_RFAL_FEATURE_NFCB``
     - Enable or disable RFAL support for NFC-B (ISO14443B) technology.
   * - ``CONFIG_RFAL_FEATURE_NFCF``
     - Enable or disable RFAL support for NFC-F (FeliCa) technology.
   * - ``CONFIG_RFAL_FEATURE_NFCV``
     - Enable or disable RFAL support for NFC-V (ISO15693) technology.
   * - ``CONFIG_RFAL_FEATURE_T1T``
     - Enable or disable RFAL support for T1T (Topaz) tag format.
   * - ``CONFIG_RFAL_FEATURE_T2T``
     - Enable or disable RFAL support for T2T tag format.
   * - ``CONFIG_RFAL_FEATURE_T4T``
     - Enable or disable RFAL support for T4T tag format (required for Aliro).
   * - ``CONFIG_RFAL_FEATURE_ST25TB``
     - Enable or disable RFAL support for ST25TB proprietary tag format.
   * - ``CONFIG_RFAL_FEATURE_ST25xV``
     - Enable or disable RFAL support for ST25TV/ST25DV proprietary formats.
   * - ``CONFIG_RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG``
     - Enable or disable dynamic updates of analog configuration parameters.
   * - ``CONFIG_RFAL_FEATURE_DPO``
     - Enable or disable RFAL Dynamic Power Output support for power optimization.
   * - ``CONFIG_RFAL_FEATURE_ISO_DEP``
     - Enable or disable RFAL support for ISO-DEP (ISO14443-4) protocol (required for Aliro).
   * - ``CONFIG_RFAL_FEATURE_ISO_DEP_POLL``
     - Enable or disable RFAL support for ISO-DEP Poller mode (PCD) operation.
   * - ``CONFIG_RFAL_FEATURE_ISO_DEP_LISTEN``
     - Enable or disable RFAL support for ISO-DEP Listen mode (PICC) operation.
   * - ``CONFIG_RFAL_FEATURE_NFC_DEP``
     - Enable or disable RFAL support for NFC-DEP (NFCIP1/P2P) protocol.
   * - ``CONFIG_RFAL_WAKE_UP_MODE_STRICT``
     - Offers lower sensitivity for increased robustness against noise.
   * - ``CONFIG_RFAL_WAKE_UP_MODE_RELAXED``
     - Provides higher sensitivity, allowing detection of weaker NFC signals, but may increase sensitivity to noise.
   * - ``CONFIG_RFAL_WAKE_UP_MODE_DEFAULT``
     - Uses the default RFAL configuration, suitable for general use cases where no specific tuning is required.
   * - ``CONFIG_RFAL_WAKEUP_POLL_BEFORE``
     - Enable polling before entering Wake-Up mode for better device detection.
   * - ``CONFIG_RFAL_DISCOVERY_DEV_LIMIT``
     - Maximum number of NFC devices to discover in a single polling cycle (range: 1-10).
   * - ``CONFIG_RFAL_DISCOVERY_TOTAL_DURATION_MS``
     - Total duration of each polling cycle in milliseconds (range: 10-2000ms).
   * - ``CONFIG_RFAL_DISCOVERY_COMP_MODE_EMV``
     - Enable EMVCo compliance mode for payment card compatibility.
   * - ``CONFIG_RFAL_DISCOVERY_COMP_MODE_ISO``
     - Enable ISO/IEC 14443 compliance mode for general NFC compatibility.
   * - ``CONFIG_RFAL_DISCOVERY_MAX_BR``
     - Maximum bitrate for NFC communication (0=106kbps, 1=212kbps, 2=424kbps, 3=848kbps).
   * - ``CONFIG_RFAL_DISCOVERY_GBLEN``
     - Length of General Bytes for NFC-DEP P2P activation (range: 0-48 bytes).
   * - ``CONFIG_RFAL_WAKEUP_NPOLLS``
     - Number of polling cycles to perform before and after Wake-Up mode (range: 1-100).
   * - ``CONFIG_RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN``
     - ISO-DEP I-Block maximum length in bytes (range: 16-4096, default: 256).
   * - ``CONFIG_RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN``
     - ISO-DEP APDU maximum length in bytes (range: I-Block max to 8192, default: 512).
   * - ``CONFIG_RFAL_FEATURE_NFC_RF_BUF_LEN``
     - RF buffer length used by RFAL NFC layer (default: 258 bytes).
   * - ``CONFIG_RFAL_FEATURE_NFC_DEP_BLOCK_MAX_LEN_*``
     - NFC-DEP Block/Payload length selection (64, 128, 192, or 254 bytes).
   * - ``CONFIG_RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN``
     - NFC-DEP PDU maximum length for Protocol Data Unit exchange (default: 512 bytes).
   * - ``CONFIG_RFAL_MAX_TIMERS_NUM``
     - Maximum number of timers used by RFAL (default: 10).

Device tree configuration
=========================

Hardware connections are described in the device tree.
The NFC reader uses the X-NUCLEO-NFC expansion board compatible string.
See the following example of the device tree configuration for the nRF54LM20 platform:

.. code-block:: devicetree

  &spi21 {
      compatible = "nordic,nrf-spim";
      status = "okay";
      cs-gpios = <&gpio1 10 GPIO_ACTIVE_LOW>;

      pinctrl-0 = <&spi21_default>;
      pinctrl-1 = <&spi21_sleep>;
      pinctrl-names = "default", "sleep";

      nucleo_nfc@0 {
          compatible = "x-nucleo-nfc";
          reg = <0>;
          spi-max-frequency = <DT_FREQ_M(8)>;
          spi-cs-setup-delay-ns = <1000>;
          spi-cs-hold-delay-ns = <1000>;
          irq-gpios = <&gpio1 14 GPIO_ACTIVE_HIGH>;
          reset-gpios = <&gpio1 15 (GPIO_ACTIVE_LOW | GPIO_PULL_DOWN)>; // Optional
      };
  };

The ``x-nucleo-nfc`` compatible string supports STMicroelectronics NFC reader expansion boards including X-NUCLEO-NFC09A1 and X-NUCLEO-NFC12A1.
