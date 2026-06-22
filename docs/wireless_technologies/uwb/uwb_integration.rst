.. _uwb_integration:

UWB integration in the reference applications
#############################################

.. contents::
   :local:
   :depth: 2

This page explains how Ultra-wideband (UWB) is integrated into the |REPO_NAME|: the layered architecture, how the Aliro stack drives the ``UltraWideBand`` facade, and how the reference Qorvo QM35825 implementation is built and selected at compile time.

For how UWB fits into access control and proximity unlock, see :ref:`wireless_technologies_uwb`.
For the Bluetooth LE side of the transport, see :ref:`aliro_ble_transport`.

Architecture overview
*********************

The UWB integration follows a layered architecture that separates the Aliro protocol logic from the hardware-specific implementation.
The diagram below shows the example implementation based on the Qorvo QM35825 module, which can be replaced with any compatible UWB radio.

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
All hardware access goes through the ``UltraWideBand`` facade (:file:`subsys/aliro/uwb/uwb.h` and :file:`subsys/aliro/uwb/uwb.cpp`), which forwards to a vendor-specific ``UltraWideBandImpl`` class.
The reference applications wire the Aliro stack, the facade, and the :ref:`aliro_access_manager` together.

Aliro stack interaction
=======================

When the Bluetooth LE + UWB transport is enabled, the Aliro stack and your implementation exchange data through interface functions in :file:`applications/*/src/aliro/interface_impl/` and callbacks registered in :file:`applications/*/src/main.cpp` before ``AliroInit()``.

.. list-table::
   :header-rows: 1
   :widths: 18 38 44

   * - Direction
     - Interface call
     - Action
   * - Inbound
     - ``Aliro::Interface::Uwb::HandleBleMessage()``
     - Forwards UWB setup traffic (M1–M4 messages over Bluetooth LE) to ``UltraWideBandInstance().HandleBleMessage()``.
   * - Inbound
     - ``Aliro::Interface::Session::StartRangingSession()``
     - Delegates to Access Manager, which calls ``UltraWideBandInstance().ConfigureRangingSession()`` once URSK, session ID, and protocol version are available.
   * - Outbound
     - ``mRangingData``
     - Delivers distance samples to ``AccessManager::HandleRangingSessionData()``.
   * - Outbound
     - ``mRangingSessionStateChanged``
     - Reports session lifecycle transitions to ``AccessManager::HandleRangingSessionStateChanged()``.
   * - Outbound
     - ``mBleMessageTransmit``
     - Sends UWB setup responses over Bluetooth LE through ``AliroStack::SendBleMessage()``.

The Access Manager evaluates reported distance against its access policy to decide when to unlock and relock.
For the full stack interface contract and porting workflow, see :ref:`uwb_custom_integration`.
For sequence diagrams, see :ref:`aliro_application_interactions`.

Example UWB implementation (Qorvo QM35825)
******************************************

.. note::

   |QM35_EXPERIMENTAL_NOTE|

The |REPO_NAME| includes a reference ``UltraWideBandImpl`` for the Qorvo QM35825 UWB SoC in :file:`subsys/aliro/uwb/qm35_impl/`.
It maps Aliro UWB session semantics onto the Qorvo firmware through the ``aliro_uwb_adapter`` library.
This library, along with the Cherry, QOSAL, and qmrom components, is provided by the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository.
The implementation keeps a list of active sessions so each Aliro Bluetooth LE session maps to its own UWB ranging session.

Optional QM35 features such as front/back disambiguation are documented in :ref:`uwb_disambiguation`.

Enable the Qorvo QM35 implementation by applying the ``uwb_qm35`` snippet (sets ``CONFIG_QM35_UWB_ALIRO_ZEPHYR``).
This requires the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository in your workspace; see :ref:`aliro_building_and_running_qm35_src`.

The snippet also enables:

* Bluetooth LE + UWB transport (``CONFIG_DOOR_LOCK_BLE_UWB``).
* Platform UWB module (``CONFIG_DOOR_LOCK_ALIRO_UWB``) and Aliro stack UWB support (``CONFIG_NCS_ALIRO_BLE_UWB``).
* The board overlay that wires the UWB module over SPI.

The Qorvo QM35 implementation is supported on the nRF5340 and nRF54LM20 SoCs.
For the supported UWB module, expansion board, and wiring, see :ref:`hw_requirements_uwb_module`.

Build-time implementation selection
===================================

The build selects the UWB implementation in :file:`subsys/aliro/uwb/CMakeLists.txt`:

.. code-block:: cmake

   if(CONFIG_QM35_UWB_ALIRO_ZEPHYR)
     add_subdirectory(qm35_impl)    # Qorvo reference implementation
   else()
     add_subdirectory(custom_impl)  # Starting point for a third-party radio
   endif()

The shared ``UltraWideBand`` facade (:file:`uwb.cpp`) is always built.
Only one ``UltraWideBandImpl`` (from :file:`qm35_impl/` or :file:`custom_impl/`) is compiled and linked.

Integrating a third-party UWB radio
***********************************

When the Qorvo Kconfig option is disabled, the build selects the skeleton in :file:`subsys/aliro/uwb/custom_impl/`.
Its methods return ``-ENOSYS``, allowing the reference application to build without ranging.

Replace the stub ``UltraWideBandImpl`` methods with calls into your vendor driver or SDK.
See :ref:`uwb_custom_integration` for the full porting guide, Kconfig checklist, and recommended bring-up sequence.

Related documentation
*********************

* :ref:`wireless_technologies_uwb` — UWB in the add-on, access policy, and subpage overview.
* :ref:`uwb_custom_integration` — Port a third-party UWB module.
* :ref:`uwb_disambiguation` — QM35 front/back detection (optional).
* :ref:`aliro_application_interactions` — Sequence diagrams for UWB session establishment.
* :ref:`hw_requirements_uwb_module` — Supported module, expansion board, and wiring.
