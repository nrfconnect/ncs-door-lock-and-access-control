.. _nfc_custom_integration:

Integrating a third-party NFC chip
###################################

.. contents::
   :local:
   :depth: 2

This guide explains how to integrate an NFC transceiver from a vendor other than STMicroelectronics into the |REPO_NAME| and the Aliro stack.
The STMicroelectronics ST25R200 and ST25R300 implementations shipped in this repository are examples of integrations — they show one way to map Aliro session semantics onto a vendor SDK, but they are not the only supported hardware paths.

For the reference RFAL architecture and stack interaction, see :ref:`nfc_integration`.

.. _nfc_custom_integration_prerequisites:

Prerequisites
*************

Before starting your port, make sure you understand the scope of the work and have the required components in place.

This guide assumes familiarity with the following:

* Aliro specification — In particular, the NFC transport protocol (ISO14443-A) and the Access Protocol message exchange flow.
  Your NFC implementation must handle the complete Aliro message exchange over NFC without requiring modifications to the protocol layer.
  See :ref:`aliro_application_interactions` for layering, stack event processing, and the NFC data path.
* NFC transport backend contract used in the add-on — The application code calls the ``NfcTransport`` functions, which forward each call to the NFC transport backend.
  The backend reports NFC reader events through callbacks and never calls the Aliro stack directly.
  You provide your own backend and keep ``NfcTransport`` and its callers unchanged.
  See :ref:`nfc_custom_integration_door_lock_addon` for details.
* nRF Connect SDK and Zephyr build system — You configure the port through Kconfig options and adapt CMake files.

NFC hardware requirements
=========================

Your NFC silicon and its driver or SDK must be able to provide the following capabilities, which the Aliro stack relies on:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Capability
     - Required
     - Notes
   * - ISO14443-A protocol support
     - Yes
     - Must support NFC-A Poller (reader/PCD) mode for Aliro User Device communication.
       The stack assumes standard ISO14443-A framing and collision resolution.
   * - APDU command/response exchange
     - Yes
     - Must handle Application Protocol Data Units (APDUs) as defined in ISO7816-4.
       Your SDK must support variable-length APDU transmission and reception.
   * - Contactless polling
     - Yes
     - Must continuously poll for NFC devices and signal presence/absence events.
       Polling parameters should be configurable for power optimization.
   * - Session lifecycle management
     - Yes
     - Your driver must signal NFC field events (field on/off, device detected, communication lost) so the application can manage Aliro sessions appropriately.
   * - Power management
     - Recommended
     - Ability to control RF field power and enter low-power states when no User Device is present.
   * - Interrupt-driven operation
     - Recommended
     - Efficient interrupt handling reduces CPU usage during NFC polling and communication.

If your SDK cannot satisfy one of the required capabilities, the corresponding Aliro authentication flow cannot complete, and the stack may terminate the NFC session.

Choosing the NFC transport backend
==================================

The :file:`subsys/aliro/nfc_transport` module contains the following components:

* The ``NfcTransport`` glue code, which connects an NFC backend to the Aliro stack.
* The reference backend, which is based on the ST RF Abstraction Layer (RFAL).

Select the backend with the ``CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND`` Kconfig choice:

* ``CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND_RFAL`` (default) — Builds the in-tree backend from :file:`subsys/aliro/nfc_transport/rfal_backend/`.
* ``CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND_EXTERNAL`` — Builds no in-tree backend, so that an out-of-tree module can provide it.

The following files are relevant when you write your own backend:

* :file:`subsys/aliro/nfc_transport/include/nfc_transport/nfc_transport_backend.h`: Backend contract that your implementation must fulfill.
* :file:`subsys/aliro/nfc_transport/src/nfc_transport.cpp`: Aliro glue code that calls into the backend.
* :file:`subsys/aliro/nfc_transport/rfal_backend/nfc_transport_rfal.cpp`: Reference RFAL backend that you can use as a template.

After you replace the RFAL backend with your own, build and run the reference application to verify that your backend builds, links, and works with the Aliro stack.
This allows you to test your integration incrementally as described in the :ref:`porting workflow <nfc_custom_integration_sequence>`.

Solution architecture
*********************

The ``NfcTransport`` glue connects the Aliro stack to an NFC transport backend.
You implement the backend, which adapts your vendor NFC driver or SDK to the backend contract defined in :file:`nfc_transport/nfc_transport_backend.h`.

The table below summarizes each layer, its location in the codebase, and its responsibility.
The sections that follow divide the port into stack integration and application integration so you can implement and test each part independently.

.. list-table::
   :header-rows: 1

   * - Component
     - Location
     - Responsibility
   * - Aliro stack
     - nRF Connect SDK Aliro stack (precompiled binary library)
     - Processes Aliro Access Protocol messages, manages authentication state machine, creates/destroys NFC sessions via ``CreateSession()`` and ``DestroySession()``, implements APDU chaining, and sends data via ``Interface::Session::Send()``.
   * - ``Interface::Session``
     - :file:`applications/*/src/aliro/interface_impl/session.cpp`
     - Routes ``Send()`` calls to appropriate transport based on the ``ConnectionHandle`` type.
       For NFC handles, it forwards to ``NfcTransport::Send()``.
   * - ``NfcTransport`` glue
     - :file:`subsys/aliro/nfc_transport/src/nfc_transport.cpp`
     - Forwards application calls to the backend and translates backend callbacks into ``AliroStack::CreateSession()``, ``HandleSessionData()``, and ``DestroySession()`` calls.
   * - NFC transport backend (the reference implementation that can be replaced with your own)
     - :file:`subsys/aliro/nfc_transport/rfal_backend/` (reference) or an external NFC transport module
     - Vendor-specific NFC driver integration, polling control, and data exchange.
       Reports device detection, received data, and device loss through callbacks.
   * - NFC Hardware Abstraction Layer
     - Your vendor SDK or driver (replaces RFAL)
     - Low-level NFC transceiver control, ISO14443-A protocol implementation, RF field management, and APDU handling.
   * - Application ``main``
     - :file:`applications/*/src/main.cpp`
     - Initializes NFC transport through ``NfcTransport::Init()`` and starts polling during application startup.

Integration steps
*****************

Integration of a third-party NFC chip consists of the following steps:

#. :ref:`Integration with the Aliro stack <nfc_custom_integration_aliro_stack>`
#. :ref:`Integration with the door lock add-on <nfc_custom_integration_door_lock_addon>`

After completing both steps, follow the :ref:`recommended bring-up sequence <nfc_custom_integration_sequence>` to implement and verify the port incrementally.

.. rst-class:: numbered-step

.. _nfc_custom_integration_aliro_stack:

Integration with the Aliro stack
*********************************

Your NFC transport backend never calls the Aliro stack directly.
It implements the functions declared in :file:`nfc_transport/nfc_transport_backend.h` and reports NFC reader events through the callbacks registered with ``Backend::Init()``.
The ``NfcTransport`` glue translates these callbacks into Aliro stack calls, so the backend does not depend on the Aliro stack.

Backend callbacks
=================

The backend contract in the :file:`nfc_transport_backend.h` header file declares the callbacks that the backend must invoke.
Invoke each callback under the conditions listed in the following table:

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Callback
     - When to invoke it
     - Resulting stack call
   * - ``mOnDeviceDetected()``
     - An ISO-DEP (ISO/IEC 14443-4) device is activated.
       If the callback returns an error, deactivate the device and resume polling.
     - ``AliroStack::CreateSession(ConnectionHandle::Nfc())``
   * - ``mOnDataReceived(Data data)``
     - The active device sends a response to a ``Send()`` call.
       The stack does not keep a reference to the buffer, so the buffer must remain valid only until the callback returns.
     - ``AliroStack::HandleSessionData(ConnectionHandle::Nfc(), data)``
   * - ``mOnDeviceLost()``
     - Communication with the active device is lost, for example after a failed data exchange or an empty response.
     - ``AliroStack::DestroySession(ConnectionHandle::Nfc())``

.. note::
   Invoke all callbacks from the Aliro workqueue context.
   The stack also calls ``Send()`` and ``Terminate()`` from this context.
   If your driver reports events from an interrupt or from its own thread, submit them to the Aliro workqueue before invoking the callbacks.

Backend functions
=================

The backend must define the following functions in the ``Aliro::NfcTransport::Backend`` namespace:

.. list-table::
   :header-rows: 1
   :widths: 25 30 45

   * - Function
     - When it is called
     - Contract and behavior
   * - ``Init(const Callbacks &callbacks)``
     - During application startup, before NFC polling begins.
     - Stores the callbacks, initializes NFC hardware, configures ISO14443-A parameters, sets up interrupts and GPIO.
       The function returns ``ALIRO_NO_ERROR`` on success or appropriate error code on failure.
   * - ``Start()``
     - When the application is ready to accept NFC transactions.
     - Begins NFC polling for ISO14443-A devices.
       Configures polling intervals and field parameters.
       Returns ``ALIRO_NO_ERROR`` on success.
   * - ``Stop()``
     - When NFC functionality should be temporarily disabled.
     - Stops NFC polling, disables RF field, enters low-power state if supported.
       Returns ``ALIRO_NO_ERROR`` on success.
   * - ``Send(Data data)``
     - When the Aliro stack needs to send data to the User Device, called through ``Interface::Session::Send()``.
     - Transmits the provided data buffer as APDU to the active NFC device and reports the response through ``mOnDataReceived``.
       Returns ``ALIRO_INVALID_STATE`` if no device is active.
   * - ``Terminate()``
     - When the current NFC session should be ended.
       It is called through ``Interface::Session::HandleTermination()``.
     - Deactivates the active device and resumes polling for the next User Device.
       The operation can be asynchronous.
       The function returns ``ALIRO_NO_ERROR`` on success.

The RFAL backend contains the internal ``RfalNotifyCallback()`` method that is called when the NFC state changes.
Use it as a reference to implement the NFC state machine that invokes the backend callbacks.

Protocol flow
=============

The typical NFC transaction flow between the Aliro stack and your NFC transport backend is as follows:

#. Application calls ``NfcTransport::Init()``, which registers the callbacks with ``Backend::Init()``.
#. Application calls ``NfcTransport::Start()`` and the backend continuously polls for User Device presence.
#. When a User Device is detected, the backend invokes ``mOnDeviceDetected()`` and the Aliro stack creates an NFC session.
#. Received NFC data is reported through ``mOnDataReceived()`` and forwarded to the Aliro stack.
#. The stack sends responses through ``Interface::Session::Send()``, which routes to ``Backend::Send()``.
#. Based on the authentication result, access is granted or denied through application interface callbacks.
#. The session ends through ``Interface::Session::HandleTermination()``, which calls ``Backend::Terminate()``, or when the backend invokes ``mOnDeviceLost()``.

.. rst-class:: numbered-step

.. _nfc_custom_integration_door_lock_addon:

Integration with the door lock add-on
**************************************

The door lock add-on provides the application framework that initializes the NFC transport and integrates it with the broader system.
The application code in :file:`applications/*/src/aliro/init.cpp` and :file:`applications/*/src/aliro/interface_impl/session.cpp` calls only the ``NfcTransport`` functions, so you do not need to modify it.
Instead, provide the backend and select it in the build configuration.

Backend skeleton
================

The backend is a source file that defines the functions declared in :file:`nfc_transport/nfc_transport_backend.h`, as in the following skeleton:

.. code-block:: cpp

   #include <nfc_transport/nfc_transport_backend.h>

   namespace Aliro::NfcTransport::Backend {

   namespace {
   Callbacks sCallbacks;
   } // namespace

   AliroError Init(const Callbacks &callbacks)
   {
       sCallbacks = callbacks;
       // Initialize the NFC reader
       return ALIRO_NO_ERROR;
   }

   AliroError Start() { return ALIRO_ERROR_NOT_IMPLEMENTED; }
   AliroError Stop() { return ALIRO_ERROR_NOT_IMPLEMENTED; }
   AliroError Send(Data data) { return ALIRO_ERROR_NOT_IMPLEMENTED; }
   AliroError Terminate() { return ALIRO_ERROR_NOT_IMPLEMENTED; }

   } // namespace Aliro::NfcTransport::Backend

.. _nfc_custom_integration_build_system:

Build system integration
========================

Provide the backend from your own module:

#. Select the ``CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND_EXTERNAL`` Kconfig option.

   .. code-block:: cfg

      CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND_EXTERNAL=y
      CONFIG_NFC_DRIVER_STM=n

#. In your module, build the sources that implement the backend contract from :file:`nfc_transport_backend.h`:

   .. code-block:: cmake

      # In your module
      zephyr_library()
      zephyr_library_sources(src/nfc_transport_backend.cpp)

Performance considerations
==========================

Several factors affect NFC performance, and you should consider them during integration:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Factor
     - Consideration
   * - Polling intervals
     - Balance responsiveness with power consumption.
   * - Field management
     - Control RF field on/off cycles efficiently.
   * - Sleep modes
     - Use low-power states when no User Device is present.
   * - Interrupt handling
     - Minimize CPU wake-ups through efficient event processing.

For detailed power consumption analysis of the reference RFAL implementation and optimization guidelines, see :ref:`nfc_power_measurements`.

.. _nfc_custom_integration_sequence:

Recommended bring-up sequence
*****************************

To minimize integration risk and enable incremental testing, complete the following steps:

.. rst-class:: numbered-step

Basic integration
=================

#. Create a skeleton backend with all required functions returning appropriate errors.
#. Select your backend in the build system as described in :ref:`nfc_custom_integration_build_system`.
#. Verify the application builds and links without runtime errors.

.. rst-class:: numbered-step

Hardware initialization
=======================

#. Implement ``Init()`` with basic hardware setup.
#. Add logging to verify hardware communication.
#. Implement ``Start()`` and ``Stop()`` with basic field control.
#. Test hardware initialization and field control.

.. rst-class:: numbered-step

Basic polling
=============

#. Implement basic ISO14443-A polling; use ``NfcTransportRfal::Execute()`` in :file:`rfal_backend/nfc_transport_rfal.cpp` as a reference.
#. Add device detection logging without session creation.
#. Test the polling loop and device detection events.
#. Verify power consumption and polling intervals.

.. rst-class:: numbered-step

Session management
==================

#. Invoke ``mOnDeviceDetected()`` on device detection.
#. Invoke ``mOnDeviceLost()`` on device removal.
#. Test the session lifecycle without data exchange.
#. Verify proper cleanup and state management.

.. rst-class:: numbered-step

Data exchange
=============

#. Implement the sending method for data transmission.
#. Add the data reception flow and forward received data to the Aliro stack.
#. Validate against the :ref:`Aliro Test Harness <setting_up_the_aliro_test_harness>` if available.
#. Perform interoperability testing with various User Devices.
#. Debug any protocol or timing issues.

.. rst-class:: numbered-step

Optimization
============

#. Optimize polling intervals and power management settings.
#. Optimize interrupt handling and worker thread usage.

For additional troubleshooting information, refer to your NFC chip vendor's documentation and the Aliro specification for protocol requirements.
