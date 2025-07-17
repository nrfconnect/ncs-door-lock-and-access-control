.. _release_notes:

Release notes
#############

This page outlines changes introduced with each release of the |APP_NAME|.

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
  * STM NFC Reader transceivers: ST25R200, ST25R3911, ST25R3916, ST25R3916B, ST25R200.
  * ST Microelectronics R/F Abstraction Layer driver with Zephyr Platform Abstraction Layer integration.
  * |APP_NAME| that leverages Aliro stack and supports CLI-based provisioning of the Access Credential public key, the reader's group identifier, and the group sub-identifier.
  * Sample applications that uses RFAL driver.
