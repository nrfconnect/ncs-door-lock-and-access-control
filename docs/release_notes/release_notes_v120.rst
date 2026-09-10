.. _release_notes_v120:

Release notes for |REPO_NAME| v1.2.0
####################################

This release upgrades the Add-on to the |NCS| v3.4.0 long-term support (LTS) release, providing a stable, long-term foundation for the next five years.
It also migrates the Matter integration to the `ncs-matter Add-on`_ with Matter v1.6 support and moves all applications to a shared DTS-based partition layout.
In addition, common Aliro interface implementations have been extracted into a shared subsystem, and the deprecated native Qorvo QM35 UWB integration has been removed in favor of the vendor-agnostic UWB platform abstraction

.. note::

   For access to a native integration with a third-party UWB vendor's solution, contact Nordic Sales.

The following updates were introduced in this release.

* Added:

  * Persistent storage for Credential Issuers learned from certificates.
  * Verification of the validity periods of stored Access Documents.
  * Configuration that enables the Aliro Expedited-fast and Step-up phases by default in the :ref:`doc_aliro_access_control_application`.
  * Configuration that enables Device Firmware Update over Bluetooth LE SMP by default in the :ref:`doc_aliro_access_control_application`.
  * Optional logging of the UWB ranging session number and active session count (``CONFIG_DOOR_LOCK_ALIRO_UWB_RANGING_SESSION_LOG``).

* Updated:

  * Integrated |NCS| v3.4.0 and updated the Aliro stack libraries.
  * Migrated the Matter integration to the `ncs-matter Add-on`_ and updated the generated data model to the Matter v1.6 Door Lock cluster.
  * Migrated all applications from Partition Manager to a shared DTS-based partition layout.
  * Extracted the common Aliro interface implementations (OS, Logging, Reader, UWB, Crypto, and Credential Issuer Certificate) from the applications into the shared :file:`subsys/aliro/interface_impl` module.
  * Improved Access Document processing. 
    The application now requests the Access Document when it is missing and processes it before the provisioned Access Credentials.
  * Improved Validity Iterations processing. 
    The application now removes expired Access Documents before storing the current set.
  * Extended Access Document storage to include a validity period and expected update time, with migration support for the new format.
  * Enabled debug logging for the DFU SMP service snippet.
  * Unified the time representation at the boundary between the Aliro stack and the application by using the ``Timestamp`` type and the new ``ValidityPeriod`` structure.
  * Extended Access Document verification to cover the validity period, the expected update time, and the Credential Issuer certificate validity period.

* Removed:

  * The deprecated native Qorvo QM35 UWB integration.
    UWB is now provided solely through the vendor-agnostic platform abstraction.
    Integrate a UWB SoC as described in :ref:`uwb_custom_integration`.

* Fixed:

  * Incorrect access authorization for disabled users and credentials without an assigned user.
  * Credential-owner resolution that did not skip unoccupied user slots.
  * Synchronization of credentials between the Matter and Aliro backends when Matter runs out of credential slots.
  * Using an incorrect Credential Issuer index when removing Validity Iterations.
  * ``GetAliroReaderConfig`` now returns a null Group Resolving Key instead of an error when the key is not set.
  * Hardened the Aliro Crypto interface implementation.
    The ephemeral PSA key is destroyed on export failure and the result of the key-attribute query is verified.
