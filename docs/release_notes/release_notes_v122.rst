.. _release_notes_v122:

Release notes for |REPO_NAME| v1.2.2
####################################

This patch release is based on the |NCS| v3.4.1 long-term support (LTS) release and integrates the `ncs-matter Add-on`_ v1.0.1.
The ncs-matter Add-on v1.0.1 fixes the storage location of the Matter Device Attestation Certificate (DAC) private key on the nRF54L Series SoCs, which affects firmware updates of the :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`.
It is recommended to update all devices running these applications on the nRF54L Series SoCs to this release.
For the required and recommended changes when updating, see :ref:`migration_1.2.2`.

The following updates were introduced in this release.

* Added:

  * The ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_SUSPEND_RANGING_WHEN_UNSECURED`` Kconfig option to the Access Manager in the :ref:`doc_aliro_access_control_application` and :ref:`doc_aliro_matter_door_lock_application`.
    When enabled, the application suspends all active UWB ranging sessions when the lock enters the ``Unsecured`` state.
  * Logs indicating when the DFU over Bluetooth LE SMP advertising is started and stopped.

* Updated:

  * The `ncs-matter Add-on`_ to v1.0.1.
    For the complete list of changes, see the `ncs-matter Add-on v1.0.1 release notes`_.
  * Aliro stack libraries.

* Fixed:

  * The Matter DAC private key storage on the nRF54L Series SoCs in the :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`.
    With the ncs-matter Add-on v1.0.0, used by the |REPO_NAME| v1.2.0 and v1.2.1, the DAC private key was stored in PSA Internal Trusted Storage (ITS) instead of the CRACEN Key Management Unit (KMU).
  * The Aliro stack now sends the ``LOAD_CERT`` command when the Expedited-fast transaction fails and the Reader certificate is provisioned, both over NFC and Bluetooth LE.
    Previously, the Aliro stack skipped the ``LOAD_CERT`` command and continued directly with the Expedited-standard transaction, so the User Device could not verify the Reader certificate.
