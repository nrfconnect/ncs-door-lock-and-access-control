.. _index:

|REPO_NAME|
########################

In combination with the |NCS|, the |REPO_NAME| provides a complete reference for building Aliro- and Matter-compatible locks and access control readers for both residential and commercial applications.
The reference integrates multiple wireless technologies, including Bluetooth Low Energy (Bluetooth LE), Ultra-Wideband (UWB), NFC, Thread, and Wi-Fi.
This allows you to choose the appropriate technology for your specific use case.

The |REPO_NAME| natively integrates the UWB SDK for Qorvo's QM35825 UWB SoC and the NFC driver for STMicroelectronics' ST25R300.
However, platform abstraction APIs allow for easy integration of a UWB SoC and an NFC transceiver from any silicon vendor.

Aliro standardizes the interaction that lets a phone or wearable act as a digital key at an opening.
Matter delivers reliable command-and-control functionality, such as remote locking and unlocking, user provisioning, and integration with broader home automation platforms.
The reference can support Aliro alone, Matter alone, or both protocols, depending on the specific use case and product requirements.

Aliro includes several key features that distinguish it from existing access protocols:

* Interoperability and compatibility - Ensures seamless interaction between access readers, such as electronic locks and access control readers, and User Devices like a smartphone and wearables.
  The standardized solution allows manufacturer-independent devices and readers to work together without compromising security.
* Flexibility - Does not dictate how your digital door lock or access control reader connects to the rest of your ecosystem.
* Protocol support - Supports various transport protocols, including mandatory Near Field Communication (NFC), Bluetooth® LE or Bluetooth LE with Ultra-Wideband (UWB).

.. note::
  Support for the Aliro mode, which uses a combination of Bluetooth LE and UWB transports, is currently `experimental <Software maturity_>`_.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   door_lock_app_arch.rst
   reference_application_interactions.rst
   uwb_disambiguation.rst
   other_addons.rst
   hardware_requirements.rst
   software_requirements.rst
   building_and_running.rst
   nfc_power_measurements.rst
   matter_power_measurements.rst
   testing.rst
   firmware_update.rst
   troubleshooting.rst
   matter_door_lock_app.rst
   release_notes.rst
   known_issues_and_limitations.rst
