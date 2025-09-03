.. _release_notes:

Release notes
#############

.. contents::
   :local:
   :depth: 2

This page outlines changes introduced with each release of the |APP_NAME|.

v0.4.0
******

.. note::
  |EXPERIMENTAL_NOTE|

Changelog
=========

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
