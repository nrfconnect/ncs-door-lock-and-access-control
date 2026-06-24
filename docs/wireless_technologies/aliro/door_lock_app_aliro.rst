.. _addon_architecture:
.. _wireless_technologies_aliro:

Aliro
#####

.. contents::
   :local:
   :depth: 2

Aliro is a standardized access protocol defined by the `Connectivity Standards Alliance`_ (CSA).
It specifies how access readers communicate with user devices such as smartphones and wearables to unlock points of entry.
In access control applications, Aliro delivers a phone-as-a-key experience: users authenticate with a digital credential stored on their device, and the Reader grants or denies access based on the result.
Aliro supports multiple transports — mandatory Near Field Communication (NFC) and optional Bluetooth® LE with Ultra-Wideband (UWB) — so products can offer tap-to-unlock, hands-free proximity unlock, or both.
For transport-specific details, see :ref:`wireless_technologies_nfc` and :ref:`wireless_technologies_uwb`.
For protocol-level background, see the `Aliro Technology`_ page.

.. note::
   This reference implements Aliro over NFC and over Bluetooth LE combined with UWB.
   It does not support the Aliro Bluetooth LE-only transport variant.

Overview
********

In the |REPO_NAME|, Aliro is the access protocol that runs on top of the transport layers.
The Aliro stack implements session management, cryptographic operations, and communication with the User Device.
The application provides the platform-specific backends — NFC, Bluetooth LE, UWB, and crypto — and uses the Access Manager to translate authentication and ranging results into lock actions.
Aliro is well suited for interoperable access control for the following reasons:

* Interoperability — certified Readers and User Devices from different manufacturers work together without custom pairing.
* Flexibility — Aliro does not dictate how the Reader connects to the rest of your ecosystem.
* Multiple unlock modes — NFC for deliberate tap-to-unlock; Bluetooth LE + UWB for hands-free proximity unlock.
* Enterprise-grade security — End-to-end encryption of the Aliro session data and device-to-device authentication.
* Optional advanced phases — Expedited-fast authentication for high-traffic doors and Step-up authentication for tiered access policies.

Role in the add-on
==================

The |REPO_NAME| ships the Aliro stack as a binary library in :file:`lib/aliro`.
The stack exposes a public API for the application and defines interfaces for the platform backends listed below.
The application implements these interfaces in :file:`applications/*/src/aliro/interface_impl/` and wires them together at startup.
Platform abstraction APIs allow replacing the default NFC and UWB implementations with hardware from other vendors.

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Component
     - Role
   * - Aliro stack
     - Access Protocol logic, session state machines, and Aliro-specific cryptography.
   * - NFC backend
     - Mandatory Aliro transport for tap-to-unlock.
       See :ref:`nfc_integration`.
   * - Bluetooth LE backend
     - Carries the Aliro Access Protocol for the Bluetooth LE + UWB transport.
       See :ref:`aliro_ble_transport`.
   * - UWB backend
     - Secure ranging for hands-free unlock.
       It is used in combination with Bluetooth LE for authentication and ranging session setup.
       See :ref:`uwb_integration`.
   * - Crypto backend
     - PSA Crypto for hardware-backed key storage and cryptographic operations.
   * - Access Manager
     - Access control logic that combines authentication results and UWB distance measurements.
       See :ref:`aliro_access_manager`.
      
To learn how the application, Aliro stack, and transport backends interact, see :ref:`aliro_integration` and :ref:`aliro_application_interactions`.

Supported transports in this |REPO_NAME|
========================================

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Transport
     - Description
   * - NFC
     - Mandatory Aliro transport.
       Supported on all :ref:`supported development kits <hw_requirements_development_kit>`.
   * - Bluetooth LE + UWB
     - Optional Aliro transport for hands-free unlock.
       Supported on the `nRF5340 DK`_ and `nRF54LM20 DK`_ with an external UWB module.
   * - Bluetooth LE only
     - Not supported in this reference.

Advanced protocol features
==========================

The reference applications optionally support Aliro protocol extensions defined by the CSA:

* Expedited-fast phase — faster re-authentication after the first successful unlock using a stored persistent key.
* Step-up phase — additional authorization using Access Documents and Credential Issuer verification.
  
See :ref:`aliro_advanced_features` for enablement, configuration, and provisioning details.

.. toctree::
   :maxdepth: 2
   :caption: Subpages:

   aliro_integration.rst
   aliro_ble_transport.rst
   aliro_access_manager.rst
   aliro_advanced_features.rst