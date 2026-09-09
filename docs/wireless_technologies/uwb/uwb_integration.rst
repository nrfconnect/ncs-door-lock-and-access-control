.. _uwb_integration:

UWB integration in the reference applications
#############################################

.. contents::
   :local:
   :depth: 2

This page explains how Ultra-wideband (UWB) is integrated into the |REPO_NAME|: the layered architecture, how the Aliro stack drives the ``UltraWideBand`` facade, and how to select the UWB implementation that the applications build against.

For how UWB fits into access control and proximity unlock, see :ref:`wireless_technologies_uwb`.
For the Bluetooth LE side of the transport, see :ref:`aliro_ble_transport`.

Architecture overview
*********************

The UWB integration follows a layered architecture that separates the Aliro protocol logic from the hardware-specific implementation.
The core repository provides the generic UWB abstraction only.
The specific radio driver is supplied by a UWB implementation that you selected at build time.

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
   │ (stub_impl stub or external module)   │ ← selected by DOOR_LOCK_ALIRO_UWB_IMPL
   └─────────────┬─────────────────────────┘
                 │  vendor SDK and SPI/GPIO
   ┌─────────────▼─────────────────────────┐
   │ UWB module                            │
   │ (any Aliro-capable UWB radio)         │
   └───────────────────────────────────────┘

The Aliro stack never includes UWB driver headers.
All hardware access goes through the ``UltraWideBand`` facade (:file:`subsys/aliro/uwb/uwb.h` and :file:`subsys/aliro/uwb/uwb.cpp`), which forwards to an ``UltraWideBandImpl`` class.
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

Selecting the UWB implementation
********************************

Setting the ``CONFIG_DOOR_LOCK_BLE_UWB=y`` Kconfig option enables the Bluetooth LE and UWB transport by selecting the following Kconfig options:

* ``CONFIG_DOOR_LOCK_ALIRO_UWB`` - The platform module in :file:`subsys/aliro/uwb/`.
* ``CONFIG_NCS_ALIRO_BLE_UWB`` - UWB support in the Aliro stack.

Select the specific ``UltraWideBandImpl`` with the ``DOOR_LOCK_ALIRO_UWB_IMPL`` Kconfig choice:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Choice option
     - Result
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_IMPL_STUB`` (default)
     - Builds the in-tree stub in :file:`subsys/aliro/uwb/stub_impl/`.
       Its methods return ``-ENOSYS``, so the reference applications build, link, and run without ranging.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_IMPL_EXTERNAL``
     - Does not build an in-tree implementation.
       An out-of-tree Zephyr module provides :file:`uwb_impl.h` and its sources instead.

The shared ``UltraWideBand`` facade (:file:`uwb.cpp`) is always built.
Only one ``UltraWideBandImpl`` — the in-tree ``stub_impl`` stub or the one supplied by an external UWB provider module — is compiled and linked.

Integrating a UWB radio
***********************

The default ``CONFIG_DOOR_LOCK_ALIRO_UWB_IMPL_STUB`` choice builds the stub in :file:`subsys/aliro/uwb/stub_impl/`, which lets you bring up your radio.

To ship a real backend, replace the stub ``UltraWideBandImpl`` methods with calls into your vendor driver or SDK, or select ``CONFIG_DOOR_LOCK_ALIRO_UWB_IMPL_EXTERNAL`` and provide the implementation from an external UWB provider module.
See :ref:`uwb_custom_integration` for the full porting guide, the external-provider contract, and the recommended bring-up sequence.

Related documentation
*********************

* :ref:`wireless_technologies_uwb` — UWB in the add-on, access policy, and subpage overview.
* :ref:`uwb_custom_integration` — Plug in a UWB implementation through the external-provider seam.
* :ref:`aliro_application_interactions` — Sequence diagrams for UWB session establishment.
* :ref:`hw_requirements_uwb_module` — UWB module hardware requirements.
