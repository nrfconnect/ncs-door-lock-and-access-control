.. _release_notes_v110:

Release notes for |REPO_NAME| v1.1.0
####################################

This release refactors the reference firmware into three dedicated applications aligned with the :ref:`solution_overview`, upgrades to |NCS| v3.3.0, and adds production support for the nRF54LM20A and the nRF54LM20B platform and Aliro over Bluetooth LE and UWB.

.. note::

  |QM35_EXPERIMENTAL_NOTE|

The following updates were introduced in this release.

* Added:

  * Platforms and transports:

    * Production support for the nRF54LM20A and the nRF54LM20B platform.
    * Production support for Aliro over Bluetooth LE and UWB transport mode.
      Platform abstraction APIs let you integrate a UWB SoC from any silicon vendor.
      The Add-on provides a validated turnkey path through native integration of the UWB SDK for Qorvo’s QM35825 UWB SoC which remains experimental.
    * Support for up to five concurrent Aliro Bluetooth LE/UWB sessions (``CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS``).
    * Bluetooth LE Dynamic Tag generation with an expiry timestamp.

  * QM35825 UWB (experimental support):

    * Integration with the public Qorvo Aliro SDK for the QM35825 UWB module.
    * Experimental support for UWB diagnostic data and radar sessions using the QM35825 UWB module (see :ref:`uwb_integration`).
    * Experimental implementation of the front/back disambiguation algorithm using the QM35825 UWB module.

  * Documentation:

    * Dedicated documentation for the :ref:`doc_aliro_access_control_application`, :ref:`doc_matter_door_lock_application`, and :ref:`doc_aliro_matter_door_lock_application`.
    * Technical guides on integrating third-party UWB and NFC modules (see :ref:`uwb_custom_integration` and :ref:`nfc_custom_integration`).
    * Testing guide for the Matter and Aliro Door Lock Application with Samsung SmartThings and Samsung Wallet (Aliro over NFC; see :ref:`testing_with_samsung_ecosystem`).

* Updated:

  * Refactored the reference firmware into three dedicated applications:

    * :ref:`doc_aliro_access_control_application`
    * :ref:`doc_matter_door_lock_application`
    * :ref:`doc_aliro_matter_door_lock_application`

  * Extracted common software modules for the Aliro and Matter applications into the :file:`subsys/` directory.
  * Integrated the |NCS| v3.3.0.
  * Integrated Qorvo QM35825 firmware and software v1.1.0.
  * Refined the Reader Status reporting implementation.
  * Optimized Aliro Bluetooth LE/UWB session handling for reduced power consumption.
  * Optimized the RAM usage on the network core of the nRF5340 platform.
  * Optimized Bluetooth LE connection and advertising management by adding separate Bluetooth identities for the Aliro service and optional Bluetooth LE features (see :ref:`aliro_ble_transport`).
  * Enhanced documentation structure.

* Fixed:

  * Synchronization of credentials between the Matter and Aliro backends.
  * Support for Matter diagnostic logs.
  * ``ValidityInfo`` verification in Access Documents (Matter builds).
