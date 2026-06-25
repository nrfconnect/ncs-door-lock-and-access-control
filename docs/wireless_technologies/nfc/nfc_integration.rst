.. _nfc_integration:

NFC integration in the reference applications
#############################################

.. contents::
   :local:
   :depth: 2

This page explains how NFC is integrated into the |REPO_NAME| as the Aliro transport layer: the architecture, how the Aliro stack drives the NFC transport, and how the reference STMicroelectronics RFAL implementation is configured.

For NFC in access control and Aliro protocol requirements, see :ref:`wireless_technologies_nfc`.

Architecture overview
*********************

NFC integration separates Aliro protocol logic from hardware-specific driver code.
The following diagram shows the reference path through ``NfcTransportRfal`` and the RFAL library; both layers can be replaced when porting to another NFC reader.

.. code-block:: none

   ┌─────────────────────────────────────┐
   │         Aliro stack                 │
   │    (protocol and session logic)     │
   │   HandleSessionData(), Send()       │
   └─────────────┬───────────────────────┘
                 │  Aliro stack public API
   ┌─────────────▼───────────────────────┐
   │    NFC transport implementation     │
   │     (reference: NfcTransportRfal)   │ ← Replaceable with a custom class
   └─────────────┬───────────────────────┘
                 │  vendor-specific API
   ┌─────────────▼───────────────────────┐
   │   NFC hardware abstraction layer    │
   │    (reference: RFAL library)        │ ← Replace with a vendor SDK or driver
   └─────────────┬───────────────────────┘
                 │  SPI and GPIO
   ┌─────────────▼───────────────────────┐
   │      NFC reader transceiver         │
   │   (reference: ST25R200/ST25R300)    │ ← Any ISO14443-A poller
   └─────────────────────────────────────┘

The Aliro stack calls the NFC transport directly — there is no separate facade layer.
The reference ``NfcTransportRfal`` class routes operations through RFAL to ST25R-series transceivers.

Aliro stack interaction
=======================

The reference transport is located in :file:`applications/*/src/aliro/platform/nfc/nfc_transport_rfal.h` and :file:`nfc_transport_rfal.cpp`.
The application connects it to the Aliro stack in :file:`applications/*/src/aliro/interface_impl/session.cpp` and during startup in :file:`init.cpp`.

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * - Direction
     - Interface call
     - Action
   * - Outbound
     - ``Aliro::Interface::Session::Send()``
     - Forwards NFC data to ``NfcTransportRfal::Instance().Send()``.
   * - Outbound
     - ``Aliro::Interface::Session::HandleTermination()``
     - Calls ``NfcTransportRfal::Instance().Terminate()``.
   * - Inbound
     - User Device detected
     - ``AliroStack::Instance().CreateSession(ConnectionHandle::Nfc())``
   * - Inbound
     - APDU received
     - ``AliroStack::Instance().HandleSessionData(ConnectionHandle::Nfc(), data)``
   * - Inbound
     - Communication lost
     - ``AliroStack::Instance().DestroySession(ConnectionHandle::Nfc())``
   * - Lifecycle
     - ``NfcTransportRfal::Init()``
     - Initializes the RFAL platform abstraction layer.
   * - Lifecycle
     - ``NfcTransportRfal::Start()``
     - Starts NFC-A polling.
   * - Lifecycle
     - ``NfcTransportRfal::Stop()``
     - Stops NFC polling.

For application/stack layering and event processing over NFC, see :ref:`aliro_application_interactions`.

Reference implementation (STMicroelectronics RFAL)
**************************************************

The |REPO_NAME| includes a reference implementation for STMicroelectronics NFC transceivers using the RFAL (RF Abstraction Layer) library.
It can be replaced with any ISO14443-A-compatible NFC reader by swapping the transport class.

Select the transceiver in Kconfig with ``CONFIG_ST25R500_DRV`` (ST25R300, default) or ``CONFIG_ST25R200_DRV`` (ST25R200).
For supported boards and wiring, see :ref:`hw_requirements_nfc_reader`.

RFAL integration
================

The RF Abstraction Layer (RFAL) provides a hardware-agnostic interface to STM NFC transceivers:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Capability
     - Description
   * - Hardware abstraction
     - Unified API for different STM NFC chips (ST25R200, ST25R300).
   * - Protocol support
     - ISO14443-A/B, ISO15693, and proprietary modes.
   * - Power optimization
     - Advanced power management and field control.
   * - Interrupt handling
     - Efficient event-driven communication.

Platform adapters
=================

The RFAL library is integrated through platform-specific adapters in :file:`drivers/nfc/stm/`:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Service
     - Description
   * - SPI communication
     - High-speed data transfer to the NFC transceiver.
   * - GPIO management
     - Configuration of IRQ, enable, and other control pins.
   * - Timer services
     - Precise timing for protocol timeouts and delays.
   * - Threading
     - Interrupt handling and worker thread coordination.
   * - Power management
     - Integration with the Zephyr power management subsystem.

Configuration
*************

Set RFAL options in :file:`prj.conf` and describe hardware connections in the devicetree.

Aliro over NFC requires NFC-A poller mode with T4T and ISO-DEP support (marked in the table below).
The reference applications enable those features by default; change other options when tuning power, discovery mechanism, or buffer sizes.

Option definitions and defaults are in :file:`drivers/nfc/stm/Kconfig.rfal` and :file:`drivers/nfc/stm/nfc_configs/Kconfig`.

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
     - Enable or disable RFAL support for NFC-A (ISO14443A) technology (required for Aliro).
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

For power optimization on the reference implementation, see :ref:`nfc_power_measurements`.

Device tree configuration
=========================

The NFC reader connects over SPI using the ``x-nucleo-nfc`` compatible string.
The following example shows the devicetree configuration for the nRF54LM20 platform:

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

The ``x-nucleo-nfc`` compatible string supports X-NUCLEO-NFC09A1 and X-NUCLEO-NFC12A1 expansion boards.

Integrating a third-party NFC chip
**********************************

Replace ``NfcTransportRfal`` with a class that implements the same transport methods and forwards received APDUs to ``AliroStack::HandleSessionData()``.
For the porting guide — method reference, build changes, and bring-up sequence — see :ref:`nfc_custom_integration`.
