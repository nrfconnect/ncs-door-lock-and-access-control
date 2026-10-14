.. _release_notes_v121:

Release notes for |REPO_NAME| v1.2.1
####################################

This patch release is based on the |NCS| v3.4.1 long-term support (LTS) release and adds support for the optional Aliro Reader Descriptor.
During an Aliro session, the Reader uses this descriptor to provide the User Device with vendor, product, and firmware version information.
Support for PICS item RD22 enables the corresponding Aliro certification tests.

The following updates were introduced in this release.

* Added:

  * Support for the Aliro Reader Descriptor in the Aliro stack and in the :ref:`doc_aliro_access_control_application` and :ref:`doc_aliro_matter_door_lock_application`.
    For details, see :ref:`aliro_reader_descriptor`.
  * Support for the ``ProductAppearance`` attribute of the Matter Basic Information cluster in the :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`.

* Updated:

  * Integrated |NCS| v3.4.1 and updated the Aliro stack libraries.
  * The Access Manager now always issues an unlock request after successful NFC authentication.
    Concurrent NFC and UWB unlock requests are deduplicated based on the physical lock state instead of the UWB access decision.

* Fixed:

  * The NFC session is now terminated when an RFAL data exchange fails.
    Previously, corrupted frames could be forwarded to the Aliro stack, resulting in an invalid R-APDU status word.
  * Duplicate lock and unlock requests are now ignored when the physical lock is already in the requested state or transitioning to it.
    The Reader Status Changed message is no longer sent when an unlock request leaves the physical lock state unchanged.
