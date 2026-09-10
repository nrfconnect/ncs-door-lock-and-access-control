.. _release_notes_v060:

Release notes for |REPO_NAME| v0.6.0
####################################

.. note::
  |EXPERIMENTAL_NOTE|

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
