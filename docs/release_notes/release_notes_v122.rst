.. _release_notes_v122:

Release notes for |REPO_NAME| v1.2.2
####################################

This patch release is based on the |NCS| v3.4.1 long-term support (LTS) release and integrates the `ncs-matter Add-on`_ v1.0.1.
The ncs-matter Add-on v1.0.1 fixes the storage location of the Matter Device Attestation Certificate (DAC) private key on the nRF54L Series SoCs, which affects firmware updates of the :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`.
It is recommended to update all devices running these applications on the nRF54L Series SoCs to this release.

The following updates were introduced in this release.

* Added:

  * The ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_SUSPEND_RANGING_WHEN_UNSECURED`` Kconfig option to the Access Manager in the :ref:`doc_aliro_access_control_application` and :ref:`doc_aliro_matter_door_lock_application`.
    When enabled, the application suspends all active UWB ranging sessions when the lock enters the ``Unsecured`` state.
  * Logs indicating when the DFU over Bluetooth LE SMP advertising is started and stopped.

* Updated:

  * Updated the `ncs-matter Add-on`_ to v1.0.1.
    For the complete list of changes, see the `ncs-matter Add-on v1.0.1 release notes`_.
  * Updated the Aliro stack libraries.

* Fixed:

  * The Matter DAC private key storage on the nRF54L Series SoCs in the :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`.
    With the ncs-matter Add-on v1.0.0, used by the |REPO_NAME| v1.2.0 and v1.2.1, the DAC private key was stored in PSA Internal Trusted Storage (ITS) instead of the CRACEN Key Management Unit (KMU).
  * The Aliro stack now sends the ``LOAD_CERT`` command when the Expedited-fast transaction fails and the Reader certificate is provisioned, both over NFC and Bluetooth LE.
    Previously, the Aliro stack skipped the ``LOAD_CERT`` command and continued directly with the Expedited-standard transaction, so the User Device could not verify the Reader certificate.

Migration notes
***************

If you have deployed devices running the :ref:`doc_matter_door_lock_application` or :ref:`doc_aliro_matter_door_lock_application` on the nRF54L Series SoCs, or you plan to update devices running the legacy |NCS| Matter door lock sample to these applications, complete the following steps:

1. Update the devices to the |REPO_NAME| v1.2.2 firmware using :ref:`matter_ota` or :ref:`dfu_ble_smp` **before** performing a factory reset.
   Updating the firmware after the factory reset does not restore a lost DAC private key.
#. Keep the ``CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_FALLBACK`` Kconfig option enabled (default) in the firmware you ship to devices that first booted |REPO_NAME| v1.2.0 or v1.2.1.
   Disabling it on these devices stops the boot until they are re-provisioned with new factory data.
#. Follow the migration path that matches the firmware that was first running on your devices, as described in the `ncs-matter Add-on v1.0.1 migration paths`_.
   In case of the |REPO_NAME|, the paths are the following:

   * Path A - Devices that store the DAC private key in KMU because they were first programmed with the in-tree |NCS| Matter firmware, and were later updated to |REPO_NAME| v1.2.0 or v1.2.1.
     This includes devices first programmed with |REPO_NAME| v1.1.x or earlier, and devices first programmed with firmware based on the legacy |NCS| :doc:`Matter door lock <nrf:samples/matter/lock/README>` sample.
   * Path B - Devices first programmed with |REPO_NAME| v1.2.0 or v1.2.1.
   * Path C - New devices first programmed with |REPO_NAME| v1.2.2.

   Devices that store the DAC private key in KMU and are updated directly to |REPO_NAME| v1.2.2, without running v1.2.0 or v1.2.1 in between, do not require any changes.

#. Validate the updated firmware on a test device as described in the `ncs-matter Add-on v1.0.1 recommended validation`_ section, including Matter commissioning after the firmware update and factory reset.

For the complete description of the required and recommended changes, see the `ncs-matter Add-on v1.0.1 migration guide`_.
