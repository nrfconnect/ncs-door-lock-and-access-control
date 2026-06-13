.. _uwb_integration:

UWB integration in the reference applications
#############################################

.. contents::
   :local:
   :depth: 2

This page explains how UWB functionality is integrated into the |REPO_NAME|, covering the interaction between the Aliro stack and the UWB hardware, the reference implementation based on the Qorvo QM35825 module, and how the same interfaces let you integrate a UWB radio from another vendor.

UWB is always used together with the Bluetooth® LE transport.
Bluetooth LE carries the Aliro Access Protocol and the UWB ranging session setup, while the UWB radio performs the secure ranging that measures the distance between the User Device and the reader.
The |REPO_NAME| ships an example implementation for the Qorvo QM35825 UWB SoC, but the Aliro stack and the platform UWB layer are vendor-agnostic, so any Aliro-capable UWB radio can be used by replacing this implementation.

Architecture overview
*********************

The UWB integration follows a layered architecture that separates the Aliro protocol logic from the hardware-specific implementation.
The diagram below shows the example implementation based on the Qorvo QM35825 module, which can be replaced with any compatible UWB radio:

.. code-block:: none

   ┌───────────────────────────────────────┐
   │ Aliro stack                           │
   │ (protocol and session state machines) │
   └─────────────┬─────────────────────────┘
                 │  Aliro stack interface
   ┌─────────────▼─────────────────────────┐
   │ Application glue                      │
   │ (interface_impl, AccessManager, main) │
   └─────────────┬─────────────────────────┘
                 │  UltraWideBandInstance()
   ┌─────────────▼─────────────────────────┐
   │ UltraWideBand facade                  │
   │ (subsys/aliro/uwb/uwb.cpp)            │
   └─────────────┬─────────────────────────┘
                 │  private methods prefixed with '_'
   ┌─────────────▼─────────────────────────┐
   │ UltraWideBandImpl                     │
   │ (example: qm35_impl)                  │ ← replaceable with custom driver integration
   └─────────────┬─────────────────────────┘
                 │  vendor SDK and SPI/GPIO
   ┌─────────────▼─────────────────────────┐
   │ UWB module                            │
   │ (example: Qorvo QM35825)              │ ← any Aliro-capable UWB radio
   └───────────────────────────────────────┘

The Aliro stack never includes UWB driver headers.
All hardware access goes through a stable public API, the ``UltraWideBand`` facade (:file:`subsys/aliro/uwb/uwb.h` and :file:`subsys/aliro/uwb/uwb.cpp`), which forwards to a vendor-specific ``UltraWideBandImpl`` class.
The reference applications provide the glue that wires the Aliro stack, the ``UltraWideBand`` facade, and the :ref:`Access Manager <wireless_technologies_aliro>` together.

Aliro stack interaction
=======================

When the Bluetooth LE + UWB transport is enabled, the Aliro stack drives UWB through two interface functions implemented in the application tree (:file:`applications/*/src/aliro/interface_impl/`):

* ``Aliro::Interface::Uwb::HandleBleMessage()`` forwards UWB setup traffic (the M1-M4 ranging session messages exchanged over Bluetooth LE) to ``UltraWideBandInstance().HandleBleMessage()``.
* ``Aliro::Interface::Session::StartRangingSession()`` is called once the stack has the ranging key material (URSK), session ID, and protocol version.
  It delegates to the Access Manager, which calls ``UltraWideBandInstance().ConfigureRangingSession()`` to configure the hardware.

The implementation reports back to the application through the callbacks registered in :file:`applications/*/src/main.cpp` before ``AliroInit()``:

* ``mRangingData`` delivers distance samples to ``AccessManager::HandleRangingSessionData()``.
* ``mRangingSessionStateChanged`` reports session lifecycle transitions to ``AccessManager::HandleRangingSessionStateChanged()``.
* ``mBleMessageTransmit`` sends outbound UWB setup responses back over Bluetooth LE through ``AliroStack::SendBleMessage()``.

The Access Manager evaluates the reported distance against its access policy to decide when to unlock and relock.
For the full sequence of interactions between the application, the Aliro stack, and the UWB radio, see :ref:`aliro_application_interactions`.

Example UWB implementation (Qorvo QM35825)
******************************************

The |REPO_NAME| includes a reference ``UltraWideBandImpl`` for the Qorvo QM35825 UWB SoC in :file:`subsys/aliro/uwb/qm35_impl/`.
It maps the Aliro UWB session semantics onto the Qorvo firmware through the ``nrfconnect-sdk-qorvo`` west add-on, which bundles the ``aliro_uwb_adapter``, Cherry, QOSAL, and qmrom components.
The implementation keeps a list of active sessions, so each Aliro Bluetooth LE session is mapped to its own UWB ranging session.

UWB builds require the ``nrfconnect-sdk-qorvo`` west project:

.. code-block:: bash

   west config manifest.group-filter -- +nrfconnect-sdk-qorvo
   west update

Enable the Qorvo QM35 implementation by applying the ``uwb_qm35`` snippet (sets ``CONFIG_QM35_UWB_ALIRO_ZEPHYR``).
The snippet also enables the Bluetooth LE + UWB transport (``CONFIG_DOOR_LOCK_BLE_UWB``), which in turn selects the platform UWB module (``CONFIG_DOOR_LOCK_ALIRO_UWB``) and the Aliro stack UWB support (``CONFIG_NCS_ALIRO_BLE_UWB``), and it adds the board overlay that wires the UWB module over SPI.
The Qorvo QM35 implementation is supported on the nRF5340 and nRF54LM20 SoCs.

For the supported UWB module and expansion board, and the required wiring, see :ref:`hw_requirements_uwb_module`.

Build-time implementation selection
===================================

The build selects the UWB implementation in :file:`subsys/aliro/uwb/CMakeLists.txt`:

.. code-block:: cmake

   if(CONFIG_QM35_UWB_ALIRO_ZEPHYR)
     add_subdirectory(qm35_impl)    # Qorvo reference implementation
   else()
     add_subdirectory(custom_impl)  # Starting point for a third-party radio
   endif()

The shared ``UltraWideBand`` facade (:file:`uwb.cpp`) is always built, while only one ``UltraWideBandImpl`` (from :file:`qm35_impl/` or :file:`custom_impl/`) is compiled and linked.

Integrating a third-party UWB radio
***********************************

Because the Aliro stack and the ``UltraWideBand`` facade are vendor-agnostic, you can replace the Qorvo reference implementation with any UWB radio that can perform Aliro ranging.
When the Qorvo Kconfig options are disabled, the build automatically selects the skeleton implementation in :file:`subsys/aliro/uwb/custom_impl/`, whose methods return ``-ENOSYS`` so that the reference application still builds and runs without ranging.

To implement your own integration, replace the stub ``UltraWideBandImpl`` methods in :file:`custom_impl/` with calls into your vendor driver or SDK.
For the complete porting guide, including the stack interface contract, the full list of implementation methods, the required Kconfig options, and a recommended bring-up sequence, see :ref:`uwb_custom_integration`.
