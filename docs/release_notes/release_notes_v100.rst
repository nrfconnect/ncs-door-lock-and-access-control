.. _release_notes_v100:

Release notes for |REPO_NAME| v1.0.0
####################################

This milestone release represents the first production-ready, fully certifiable |REPO_NAME| with Aliro v1.0 certification compliance and Matter support, enabling you to develop and certify interoperable access control products for everything from commercial buildings to residential smart homes.

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
