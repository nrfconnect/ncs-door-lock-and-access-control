.. _release_notes:

Compatibility matrix and release notes
######################################

.. contents::
   :local:
   :depth: 2

This page provides a compatibility matrix with various versions of the components used in |REPO_NAME| and outlines changes introduced with each release of the Add-on.

Compatibility matrix
********************

The following table shows tested and verified combinations of |REPO_NAME| releases with various versions of the components used in the Add-on.

.. list-table:: |REPO_NAME| compatibility matrix
   :header-rows: 1
   :widths: 12 12 12 12 12 12

   * - |REPO_NAME| version
     - |NCS| version
     - Aliro protocol version
     - Matter protocol version
     - Qorvo UWB SDK and QM35825 FW version
     - STM NFC RFAL library version
   * - 0.1.0
     - 2.9.0
     - 0.9.0
     - —
     - —
     - —
   * - 0.2.0
     - 2.9.0
     - 0.9.0
     - —
     - —
     - 3.0.0
   * - 0.3.0
     - 2.9.0
     - 0.9.0
     - —
     - 0.3.0
     - 3.0.0
   * - 0.4.0
     - 3.1.0
     - 0.9.0
     - 1.4.0
     - 0.4.0
     - 4.0.2
   * - 0.5.0
     - 3.1.0
     - 0.9.0
     - 1.4.0
     - 0.4.0
     - 4.0.2
   * - 0.6.0
     - 3.2.0
     - 1.0.0
     - 1.5.0
     - 0.6.0
     - 4.0.2
   * - 1.0.0
     - 3.2.0
     - 1.0.0
     - 1.5.0
     - 0.6.0
     - 4.0.2
   * - 1.0.1
     - 3.2.0
     - 1.0.0
     - 1.5.0
     - 0.6.0
     - 4.0.2

Release notes
*************

The following list outlines the release notes for each release of the |REPO_NAME|.

v1.0.1
******

This is a bugfix release that fixes the Device Firmware Update process for the QM35.
Specifically, it fixes the QM35 firmware version mapping script path used during sysbuild when building with the ``uwb_qm35_dfu`` snippet.

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Fixed:

    * Incorrect path to the QM35 version mapping script (``map_qm35_version.py``) in sysbuild.

v1.0.0
******

This milestone release represents the first production-ready, fully certifiable |REPO_NAME| with Aliro v1.0 certification compliance and Matter support, enabling you to develop and certify interoperable access control products for everything from commercial buildings to residential smart homes.

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Added:

    * Full compliance with Aliro v1.0 certification program.
    * Production support for ST25R300 NFC reader module.
    * Experimental support for the Aliro mode, which uses a combination of Bluetooth LE and UWB transports, with support for the QM35825 UWB module.
    * NFC power profiling and optimization documentation.
    * Event-based RFAL worker implementation for reduced NFC power consumption.
    * Encrypted Access Document storage on external flash using Zephyr NVS.
    * Unified interface and APIs for transport (NFC, BLE, and UWB), session management, and crypto operations.
    * BLE advertising arbiter for improved multi-component control.
    * CLI commands for setting Reader private key and retrieving Bluetooth address.
    * Time Synchronization cluster support.
    * Access Document validity period verification in Matter builds.
    * Release build configuration.
    * QM35 firmware update support for nRF54LM20 platform.
    * Documentation of the architecture and integration of the door lock and access control application with the Aliro stack.

  * Updated:

    * Renamed the repository to "nRF Door Lock and Access Control Add-on".
    * Enhanced persistent storage architecture with optimized efficiency.
    * Improved NFC discovery mechanism with better reliability.
    * Optimized RFAL NFC configuration for power consumption.
    * Refactored crypto interface with simplified PSA implementation.
    * Restructured the Kconfig options across the repository.
    * Enhanced documentation structure.

  * Fixed:

    * Stack overflow issue in StepUp verification process.
    * Incorrect reader_group_identifier_key usage for Kpersistent.
    * NFC session handling during ENVELOPE chaining.
    * Power consumption anomalies in IDLE mode on nRF5340DK.
    * BT advertising issues with DFU SMP and NUS services.
    * L2CAP channel allocation problems.
    * SELECT response handling during Step-up phase.
    * Various build and compilation issues across platforms.

v0.6.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Added:

    * Compliance with the Aliro specification confirmed against Aliro Test Harness (`aliro-sve-v1.0`_ tag).
    * Credential Issuer Certificate verification using the Credential Issuer CA Public Key.
    * Reader Certificate provisioning.
    * Presenting Reader Certificate using the ``LOAD_CERT`` command.
    * Support for injecting proprietary NFC technologies.
    * The UWB Suspend and Resume functionality.
    * Support for testing Aliro over NFC and Aliro over Bluetooth LE + UWB with the Apple Home ecosystem.
    * Access Document storage in non-volatile memory.
    * Reference implementation of QM35825 firmware update using:
      * DFU over Matter
      * DFU over Bluetooth LE

  * Updated:

    * Integrated nRF Connect SDK v3.2.0.
    * Integrated Matter v1.5.0.
    * Integrated QM35825 firmware and software v0.6.0.
    * Improved NFC discovery mechanism.
    * Improved support for Aliro Expedited-Fast phase and Aliro Step-up phase.
    * Updated default UWB session termination to allow User Device–controlled session closure.
    * Improved system robustness by avoiding fatal errors when X-NUCLEO-NFC09A1 shield or QM35825 UWB module fail to initialize, preserving operability for potential firmware upgrades.
    * Changed default configuration to communicate with X-NUCLEO-NFC09A1 shield and QM35825 UWB module using a single SPI bus.
    * Extended documentation with:
      * Instructions for flashing the QM35825 device using nRF5340 DK.
      * Guidelines for testing with Aliro Test Harness.
      * Instructions for testing with the Apple Home ecosystem using Bluetooth LE + UWB.

  * Fixed:

    * Build issues when compiling the project on Windows PCs.

v0.5.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Added support for:

    * nRF54LM20A platform.
    * Aliro Expedited-fast phase.
    * Aliro Step-up phase.
    * Aliro Access Document reception and parsing in the application.
    * Bluetooth LE Nordic UART Service (NUS) for remote control of the door lock.
    * Device Firmware Upgrade (DFU) over Bluetooth LE Simple Management Protocol (SMP).
    * Building QM35825 UWB libraries from sources for registered users.

  * Extended the documentation with:

    * A guide on how to test the application with the Apple smart home ecosystem.
    * Instructions on how to test Matter over-the-air (OTA) updates and DFU over Bluetooth LE SMP.
    * Explanation of the new Aliro features and their usage.

  * Fixed:

    * Compliance issues with the Apple smart home ecosystem.

v0.4.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Added:

    * Matter support.
    * Integration of the Matter door lock cluster server in the |APP_NAME|.
    * Support for provisioning of Aliro door lock reader and user credentials using Matter.

  * Updated:

    * nRF Connect SDK revision to v3.1.0.
    * STM RFAL drivers to the STSW-ST25RFAL005 revision.
    * QM35 libraries to latest internal Qorvo delivery.
    * Moved CLI and persistent storage implementation from Aliro stack to the application layer.

  * Fixed:

    * An issue where the watchdog was expiring for an NFC session upon test execution (AL-335).
    * An issue where occasional timeout was occuring in the Reader when executing the RD-NFC-STDTXN-2.0 test from the test harness (AL-239).
    * An issue where an undefined access decision was occuring when executing the RD-NFC-STDTXN-2.0 test harness case in a loop (AL-282).

v0.3.1
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Fixed:

    * An issue where ranging measurements with the QM35825 module were not delivered to the application.
      The QM35825 module's ranging measurements are now correctly delivered to the application.
      The distance between the Reader and User Device can now be displayed in the logs.
    * An issue with Access Manager not supporting multiple Aliro sessions.
      Access Manager now supports multiple Aliro and UWB sessions.
    * An issue where only one User Device could be provisioned to the Reader.
      The application now allows provisioning multiple User Devices to the Reader.

  * Changed:

    * The Access Manager can now autonomously terminate the Aliro and UWB ranging sessions.
    * The ``dl provisioning AC_key`` shell command was extended to support multiple Access Credential public keys.

v0.3.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Added:

    * The EXCHANGE command implementation.
    * Support for Bluetooth LE transport protocol.
    * Support for ultra wideband (UWB) interface.
    * Reference implementation of the UWB module QM35825 example.
      The Aliro UWB adapter is provided as precompiled libraries.
    * Access Manager interface and its reference implementation.
    * Support for LED indicator of access decision.

v0.2.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  The following updates were introduced in this release.

  * Added:

    * Experimental support for the following development platforms:

      * `nRF52840 DK`_
      * `nRF5340 DK`_

    * A platform logger implementation.
    * A new NFC transport interface integration.
    * Simplified Aliro stack API integration.

  * Improved the implementation of the following components in the RFAL platform abstraction layer:

      * Timers
      * Semaphores
      * Threading

v0.1.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

.. toggle::

  See the following section for the list of implemented features.

  * Added experimental support for the following:

    * Access Protocol (the expedited standard transaction only).
    * Transport Protocol over Near Field Communication (NFC) transport.
    * Trusted Framework implementation with the |NCS| PSA API as a cryptography backend.
    * The nRF54L15 hardware platform.
    * STM NFC reader transceivers: ST25R200, ST25R3911, ST25R3916, ST25R3916B, ST25R200.
    * ST Microelectronics R/F Abstraction Layer driver with Zephyr Platform Abstraction Layer integration.
    * |APP_NAME| that leverages Aliro stack and supports CLI-based provisioning of the Access Credential public key, the reader's group identifier, and the group sub-identifier.
    * Sample applications that uses RFAL driver.
