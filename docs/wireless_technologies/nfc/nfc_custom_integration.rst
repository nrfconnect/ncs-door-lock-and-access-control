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
* Direct integration pattern used in the add-on — The platform code uses direct class-to-class communication between ``AliroStack`` and the NFC transport implementation (``NfcTransportRfal``).
  You replace the transport class but keep the same interface methods and calling patterns.
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

Starting point
==============

Since NFC integration uses direct class replacement rather than a plugin architecture, you can replace the existing ``NfcTransportRfal`` implementation with your own class or implement a new one that matches your needs and satisfies the Aliro stack API.

The reference implementation can be found in:

* Header file: :file:`applications/*/src/aliro/platform/nfc/nfc_transport_rfal.h`
* Implementation file: :file:`applications/*/src/aliro/platform/nfc/nfc_transport_rfal.cpp`

When replacing the RFAL concrete implementation with your own, you can build, link, and run the reference application to verify your code is ready.
This allows you to test your integration incrementally as described in the :ref:`porting workflow <nfc_custom_integration_sequence>`.

Solution architecture
*********************

NFC integration uses direct communication between the Aliro stack and the NFC transport implementation.
Your code replaces the ``NfcTransportRfal`` class, which translates between the Aliro-facing API and your vendor driver or SDK.

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
     - Routes ``Send()`` calls to appropriate transport based on ``ConnectionHandle`` type. For NFC handles, forwards to ``NfcTransportRfal::Instance().Send()``.
   * - ``NfcTransportRfal`` (the reference implementation that can be replaced with your own)
     - :file:`applications/*/src/aliro/platform/nfc/` (replace with your code)
     - Vendor-specific NFC driver integration, session management, polling control, and data exchange. Calls ``AliroStack::CreateSession()`` and ``HandleSessionData()``.
   * - NFC Hardware Abstraction Layer
     - Your vendor SDK/driver (replaces RFAL)
     - Low-level NFC transceiver control, ISO14443-A protocol implementation, RF field management, and APDU handling.
   * - Application ``main``
     - :file:`applications/*/src/main.cpp`
     - Initializes NFC transport via ``NfcTransportRfal::Instance().Init()`` and starts polling during application startup.

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

The Aliro stack uses direct method calls to communicate with your NFC transport implementation.
It never includes specific NFC driver headers or vendor SDK dependencies.

Aliro Stack interface contract
==============================

Your NFC transport implementation must meet the Aliro stack API contract.
The basic requirements are:

* A public function that allows sending the data delivered by the Aliro stack to the User Device over NFC.
* A mechanism that allows informing the Aliro stack that the NFC data has been received from the User Device.

Apart from that, your NFC transport implementation must call these Aliro stack methods at appropriate times:

.. code-block:: cpp

   class AliroStack {
   public:
       // Session lifecycle - call when NFC device is detected
       void CreateSession(ConnectionHandle handle);

       // Session lifecycle - call when NFC device is removed or session ends
       void DestroySession(ConnectionHandle handle);

       // Data handling - call when NFC data is received from User Device
       void HandleSessionData(ConnectionHandle handle, Data data);
   };

Transport API
=============

Your NFC transport and the Aliro stack communicate in both directions.
Your code calls stack methods when NFC events occur (device present, APDU received, session ended).
The stack sends data back through ``Aliro::Interface::Session``, which application code routes to your transport in :file:`interface_impl/session.cpp`.

Calls from the transport to the stack
--------------------------------------

* When polling detects an ISO14443-A device, call ``AliroStack::Instance().CreateSession(ConnectionHandle::Nfc())``.
* When APDU data arrives from the User Device, call ``AliroStack::Instance().HandleSessionData(ConnectionHandle::Nfc(), receivedData)``.
* When the session ends (device removed, timeout, error), call ``AliroStack::Instance().DestroySession(ConnectionHandle::Nfc())``.

Required transport methods
--------------------------

The RFAL reference class defines the interface your transport should provide:

.. list-table::
   :header-rows: 1
   :widths: 25 30 45

   * - Method
     - When it is called
     - Contract and behavior
   * - ``Init()``
     - During application startup, before NFC polling begins.
     - Initialize NFC hardware, configure ISO14443-A parameters, set up interrupts and GPIO. Return ``ALIRO_NO_ERROR`` on success or appropriate error code on failure.
   * - ``Start()``
     - When the application is ready to accept NFC transactions.
     - Begin NFC polling for ISO14443-A devices. Configure polling intervals and field parameters. Return ``ALIRO_NO_ERROR`` on success.
   * - ``Stop()``
     - When NFC functionality should be temporarily disabled.
     - Stop NFC polling, disable RF field, enter low-power state if supported. Return ``ALIRO_NO_ERROR`` on success.
   * - ``Send(Data data)``
     - When the Aliro stack needs to send data to the User Device, called via ``Interface::Session::Send()``.
     - Transmit the provided data buffer as APDU to the currently connected NFC device. Handle APDU chaining if necessary. Return ``ALIRO_NO_ERROR`` on successful transmission.
   * - ``Terminate()``
     - When the current NFC session should be ended.
     - Terminate the active NFC session, return to polling state, prepare for next User Device detection. Return ``ALIRO_NO_ERROR`` on success.

Apart from that, the RFAL implementation contains the internal ``RfalNotifyCallback()`` method that is called when the NFC state changes.
You can use it as a reference to implement the NFC state machine that will call Aliro Stack public API methods required to drive the Aliro session.

Stack-to-transport calls
------------------------

The Aliro stack calls back to the application through the ``Aliro::Interface`` namespace.
The key callbacks your transport interacts with are as follows:

.. code-block:: cpp

   namespace Aliro::Interface::Session {
       // Called by stack to send data - routes to appropriate transport
       AliroError Send(ConnectionHandle handle, Data data);

       // Called by stack when session ends
       void HandleTermination(ConnectionHandle handle);
   }

The reference implementation in :file:`interface_impl/session.cpp` routes NFC calls to your transport:

.. code-block:: cpp

   AliroError Send(ConnectionHandle handle, Data data)
   {
       if (handle.IsNfc()) {
           return NfcTransportRfal::Instance().Send(data);  // ← Your implementation
       }
       // ... other transports
   }

.. note::

   In most ports you only need to replace the ``NfcTransportRfal`` class implementation.
   You normally do not modify :file:`interface_impl/session.cpp`.
   Keep that wiring as-is and implement NFC functionality in your transport class.

Protocol flow
=============

The typical NFC transaction flow between the Aliro stack and your NFC transport implementation is as follows:

#. Application calls ``Init()`` to prepare the NFC reader.
#. Application calls ``Start()`` and the NFC transport continuously polls for User Device presence.
#. When a User Device is detected, the NFC transport calls ``AliroStack::CreateSession(ConnectionHandle::Nfc())``.
#. Received NFC data is forwarded to the Aliro stack through ``AliroStack::HandleSessionData(ConnectionHandle::Nfc(), data)``.
#. The stack sends responses through ``Interface::Session::Send()``, which routes to the NFC transport ``Send()``.
#. Based on the authentication result, access is granted or denied through application interface callbacks.
#. The session ends through ``AliroStack::DestroySession()`` and the NFC transport ``Terminate()``.

.. rst-class:: numbered-step

.. _nfc_custom_integration_door_lock_addon:

Integration with the door lock add-on
**************************************

The door lock add-on provides the application framework that initializes your NFC transport and integrates it with the broader system.
You need to modify the following components to integrate your NFC implementation (``YourNfcTransport`` is the name of your NFC transport implementation):

Application initialization
==========================

Update the application's initialization code to use your NFC transport class.
The :file:`applications/*/src/aliro/init.cpp` contains the application initialization code:

.. code-block:: cpp

   // Replace NfcTransportRfal with your implementation
   #include "aliro/platform/nfc/your_nfc_transport.h"  // ← Your header

   int AliroInit()
   {
       // ... other initialization code ...

       AliroError ec = YourNfcTransport::Instance().Init();  // ← Your class
       if (ec != ALIRO_NO_ERROR) {
           LOG_ERR("NFC transport initialization failed");
       }

       // ... continue with other initialization ...
       return EXIT_SUCCESS;
   }

   int AliroStart()
   {
       AliroError ec = YourNfcTransport::Instance().Start();  // ← Your class
       if (ec != ALIRO_NO_ERROR) {
           LOG_ERR("NFC transport start failed");
           return EXIT_FAILURE;
       }

       // ... other start operations ...
       return EXIT_SUCCESS;
   }

   int AliroStop()
   {
       int rc = EXIT_SUCCESS;

       AliroError ec = YourNfcTransport::Instance().Stop();   // ← Your class
       if (ec != ALIRO_NO_ERROR) {
           LOG_ERR("NFC transport stop failed");
       }

       // ... other stop operations ...
       return rc;
   }

Session routing
===============

The session interface routes NFC calls to your implementation.
The :file:`applications/*/src/aliro/interface_impl/session.cpp` contains the session routing code:

.. code-block:: cpp

   #include "aliro/platform/nfc/your_nfc_transport.h"  // ← Your header

   AliroError Send(ConnectionHandle handle, Data data)
   {
       if (handle.IsNfc()) {
           return YourNfcTransport::Instance().Send(data);  // ← Your class
       }
       // ... rest unchanged
   }

   void HandleTermination(ConnectionHandle handle)
   {
       if (handle.IsNfc()) {
           YourNfcTransport::Instance().Terminate();  // ← Your class
       }
       // ... rest unchanged
   }

.. _nfc_custom_integration_implementation_methods:

Build system integration
========================

The reference NFC implementation is compiled unconditionally whenever the application is built with Aliro support.
To include your custom NFC implementation, you need to add your source files to the build system in the application platform folder and remove the RFAL-specific targets in :file:`applications/*/src/aliro/platform/nfc/`:

.. code-block:: cmake

   # Add your NFC implementation
   target_sources(app PRIVATE
       src/aliro/platform/nfc/your_nfc_transport.cpp
       # Add other source files as needed
   )

   target_include_directories(app PRIVATE
       src/aliro/platform/nfc/
       # Add your vendor SDK include paths
   )

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

#. Create a skeleton NFC transport class with all required methods returning appropriate errors.
#. Update the build system and application initialization to use your class.
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

#. Implement basic ISO14443-A polling; use ``NfcTransportRfal::Execute()`` as a reference.
#. Add device detection logging without session creation.
#. Test the polling loop and device detection events.
#. Verify power consumption and polling intervals.

.. rst-class:: numbered-step

Session management
==================

#. Add session creation on device detection.
#. Implement session destruction on device removal.
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
