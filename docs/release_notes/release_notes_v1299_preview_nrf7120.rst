.. _release_notes_v1299_preview_nrf7120:

Release notes for |REPO_NAME| v1.2.99-preview-nrf7120
#####################################################

This preview release is based on the |NCS| v3.5.0-preview4 and the `ncs-matter Add-on`_ v1.0.99-preview-nrf7120.
It introduces experimental support for the nRF7120 platform, including Matter over Wi-Fi in :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`, and Wi-Fi connectivity with lock control over CoAP in the :ref:`doc_aliro_access_control_application`.

.. note::

   The nRF7120 DK support is experimental and provided for evaluation purposes only.
   It is not intended for production use.

The following updates were introduced in this release.

* Added experimental support for the following:

  * The nRF7120 platform (``nrf7120dk/nrf7120/cpuapp``) in the :ref:`doc_aliro_access_control_application`, :ref:`doc_matter_door_lock_application`, and :ref:`doc_aliro_matter_door_lock_application`.
  * Matter over Wi-Fi on the nRF7120 platform in the :ref:`doc_matter_door_lock_application` and :ref:`doc_aliro_matter_door_lock_application`.
  * Wi-Fi station in the :ref:`doc_aliro_access_control_application` on the nRF7120 platform (``CONFIG_DOOR_LOCK_WIFI``).
  * The :ref:`door_lock_coap_service` component, enabled with the ``coap_service`` snippet.
  * Integration of the :ref:`door_lock_coap_service` with the :ref:`doc_aliro_access_control_application`, which allows reading, observing, and changing the lock state over CoAP on the nRF7120 platform.

* Updated:

  * Integrated |NCS| v3.5.0-preview4, the `ncs-matter Add-on`_ v1.0.99-preview-nrf7120, and updated the Aliro stack libraries.
  * Migrated all applications from Trusted Storage to Secure Storage.
  * Migrated the Matter applications to build-time ZAP generation.
  * OpenThread is now built from sources in the Matter applications.

* Removed:

  * Support for the nRF52840 platform.

Limitations
***********

The nRF7120 platform support in this preview release has the following limitations:

* nRF7120 platform is not supported for production use.
* Matter support is limited to Matter over Wi-Fi.
* MCUboot, Device Firmware Update (DFU), and Matter OTA are not supported.
* The :ref:`door_lock_coap_service` is based on an experimental CoAP library and uses plain UDP without authentication or encryption.

For more information about the Matter-specific limitations, see the `ncs-matter Add-on v1.0.99-preview-nrf7120 release notes`_.
