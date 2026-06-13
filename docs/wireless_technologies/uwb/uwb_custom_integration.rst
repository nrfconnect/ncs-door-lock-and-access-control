.. _uwb_custom_integration:

Integrating a third-party UWB chip
##################################

.. contents::
   :local:
   :depth: 2

This guide explains how to integrate a UWB radio from a vendor other than Qorvo into the |REPO_NAME| and the Aliro stack.
The Qorvo QM35825 (QM35) implementation shipped in this repository is an example integration — it shows one way to map Aliro session semantics onto a vendor SDK, but it is not the only supported hardware path.

.. _uwb_custom_integration_prerequisites:

Prerequisites
*************

Before starting your port, make sure you understand the scope of the work and have the required components in place.

This guide assumes familiarity with the following:

* Aliro specification — In particular the Bluetooth LE message layout (Protocol Header, Message ID, Length, Payload) and the ranging session setup flow (M1-M4 exchange).
  Several integration points require you to forward complete Aliro messages unchanged, so a working understanding of this layout is essential.
  See :ref:`aliro_application_interactions` for the relevant sequence diagrams.
* Facade pattern used in the add-on — The platform code separates a stable public API (``UltraWideBand``) from a vendor-specific implementation (``UltraWideBandImpl``).
  You implement only the private methods; you do not modify the facade or the stack interface wiring.
  See :ref:`uwb_custom_integration_door_lock_addon` for details.
* nRF Connect SDK and Zephyr build system — You configure the port through Kconfig options and adapt one or two CMake files.

Vendor SDK requirements
=======================

Your UWB silicon and its driver or SDK must be able to provide the following capabilities, which the Aliro stack and the reference ``AccessManager`` rely on:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Capability
     - Required
     - Notes
   * - Ranging session configuration from URSK, session ID, and protocol version
     - Yes
     - The stack extracts these values from the Bluetooth LE session and passes them to ``ConfigureRangingSession()``.
       Your SDK must accept an externally supplied URSK rather than generating its own key material.
   * - Distance reporting
     - Yes
     - You deliver distance samples through the ``mRangingData`` callback.
       The reference ``AccessManager`` expects a 16-bit big-endian value in centimeters, but you can convert in your implementation or override the extraction logic.
   * - Session lifecycle events
     - Yes
     - Your driver must signal session state transitions (for example, ranging started, suspended, or destroyed) so you can forward them through ``mRangingSessionStateChanged``.
   * - Forwarding opaque UWB setup messages
     - Yes
     - During setup, the vendor stack must accept and produce the UWB-related payloads carried inside Aliro BLE messages, without requiring direct access to the Bluetooth LE link.
   * - Multiple concurrent sessions
     - Recommended
     - If your hardware supports more than one User Device at a time, your implementation must map each ``ConnectionHandle`` to a separate vendor session context.
   * - BLE/UWB time synchronization
     - Optional
     - Implement ``BleTimeSync()`` only if your vendor requires explicit time alignment after the Bluetooth LE connection (Aliro spec §19.4.4.1).

If your SDK cannot satisfy one of the required capabilities, the corresponding Aliro ranging flow cannot complete, and the stack may terminate the Bluetooth LE session.

Starting point
==============

The :file:`subsys/aliro/uwb/custom_impl/` directory is the starting point for your port.
It contains skeleton :file:`uwb_impl.h` and :file:`uwb_impl.cpp` files whose methods return ``-ENOSYS``.

When the QM35 Kconfig options are disabled, the build system selects this directory automatically, so you can build, link, and run the reference application before writing any vendor code.
This is the basis for the first step of the :ref:`porting workflow <uwb_custom_integration_sequence>`.

To confirm your environment is ready, build a reference application with the :ref:`specific options set <uwb_custom_integration_door_lock_addon>` and verify that it links and logs ``UWB is not implemented``.

Solution architecture
*********************

UWB integration spans two cooperating layers:

* Aliro stack — Protocol engine that owns Bluetooth LE and UWB session state machines.
  It calls application-provided hooks through ``Aliro::Interface::*`` when UWB setup traffic or ranging session creation is required.
* Door lock add-on (``ncs-door-lock-and-access-control``) — Reference application and platform services.
  It implements those hooks, owns the ``UltraWideBand`` facade, and forwards access-policy decisions through ``AccessManager``.

Neither layer communicates with your UWB silicon directly.
Your code sits in ``UltraWideBandImpl``, which translates between the Aliro-facing API and your vendor driver or SDK.
To learn more about the interactions between the Aliro stack and the application, see the :ref:`aliro_application_interactions` documentation page.

The sections below divide this flow into stack integration and application integration, so you can implement and test each part independently.

Layer responsibilities
======================

The following table summarizes each component in the UWB integration flow, its location in the codebase, and its specific responsibility.

.. list-table::
   :header-rows: 1

   * - Component
     - Location
     - Responsibility
   * - Aliro stack state machine
     - nRF Connect SDK Aliro stack (precompiled binary library)
     - Enters ``UwbRanging`` state, delegates UWB Ranging Service and Notification/Ranging Bluetooth LE messages to ``Interface::Uwb::HandleBleMessage()`` (full Aliro message buffers), and requests ranging session creation through ``Interface::Session::StartRangingSession()``.
   * - ``Interface::Uwb``
     - :file:`include/aliro/interface.h`
     - Single stack entry point for inbound BLE/UWB traffic.
       The stack forwards only UWB Ranging Service and Notification/Ranging messages as complete Aliro messages.
   * - ``Interface::Session``
     - :file:`include/aliro/interface.h`
     - Calls ``AccessManager::StartRangingSession()`` when the stack has URSK and session ID and needs the platform to configure hardware ranging. As a result the ``AccessManager`` calls ``UltraWideBandInstance().ConfigureRangingSession()``.
   * - ``UltraWideBand`` facade
     - :file:`subsys/aliro/uwb/uwb.h`, :file:`subsys/aliro/uwb/uwb.cpp`
     - Stable public API used by the application; forwards to private ``UltraWideBandImpl`` methods.
   * - ``UltraWideBandImpl``
     - :file:`subsys/aliro/uwb/custom_impl/` (your code) or :file:`qm35_impl/` (an example)
     - Vendor-specific driver and SDK glue, session bookkeeping, and callback invocation.
   * - ``AccessManager``
     - :file:`applications/*/src/aliro/access_manager/`
     - Applies access policy by evaluating distance thresholds, deciding unlock and lock actions, and handling UWB session lifecycle.
   * - Application ``main``
     - :file:`applications/*/src/main.cpp`
     - Registers UWB callbacks and initializes the module before ``AliroInit()``.

Integration steps
*****************

Integration of a third-party UWB chip consists of the following steps:

1. :ref:`Integration with the Aliro stack <uwb_custom_integration_aliro_stack>`
2. :ref:`Integration with the door lock add-on <uwb_custom_integration_door_lock_addon>`

After completing both integration steps, follow the :ref:`recommended bring-up sequence <uwb_custom_integration_sequence>` to implement and verify the port incrementally.

.. _uwb_custom_integration_aliro_stack:

1. Integration with the Aliro stack
***********************************

The Aliro stack remains vendor-agnostic.
It never includes specific UWB driver headers.
All hardware access goes through two interface functions implemented in the application tree.

Stack interface contract
========================

When the ``CONFIG_NCS_ALIRO_BLE_UWB`` Kconfig option is enabled (automatically enabled when the ``CONFIG_DOOR_LOCK_BLE_UWB`` Kconfig option is selected), the stack expects the following implementations (guarded by the same Kconfig symbol):

.. list-table::
   :header-rows: 1
   :widths: 25 30 45

   * - Function
     - When it is called
     - Contract and behavior
   * - ``Aliro::Interface::Uwb::HandleBleMessage()``
     - On UWB Ranging Service messages and Notification messages with the Ranging Message ID.
     - The ``data`` buffer is a complete Aliro message, not a bare payload.
       It must include, in order: Protocol Header (1 byte), Message ID (1 byte), Length (16-bit, big-endian, payload size), and Payload — the Aliro BLE message layout from the Aliro specification.
       The ``length`` argument is the total byte count of that complete message (four-byte header plus payload).
       Your implementation should pass this buffer unchanged to the vendor stack (the Qorvo example copies the full message into ``aliro_uwb_session_message_handle()``).
       Return ``0`` on success or a negative ``errno`` value on failure.
       Depending on the internal state of the Aliro session machine state, any error returned by this function may eventually cause the stack to terminate the BLE session.
   * - ``Aliro::Interface::Session::StartRangingSession()``
     - When the stack enters the ``UwbRanging`` HSM state, after ``sessionId``, ``ursk``, and ``protocolVersion`` are extracted from the Bluetooth LE session.
     - Delegates to ``AccessManager``, which ultimately calls ``UltraWideBandInstance().ConfigureRangingSession()``.

Both functions have reference implementations under the :file:`applications/*/src/aliro/interface_impl/` directory.
The following example shows the reference ``HandleBleMessage()`` implementation in :file:`interface_impl/uwb.cpp`:

.. code-block:: cpp

   return ::Aliro::Uwb::UltraWideBandInstance()
       .HandleBleMessage(data, length, sessionContext);

.. note::

   In most ports you only adapt :file:`uwb_impl.cpp` (and :file:`uwb_impl.h`).
   You normally do not modify :file:`interface_impl/uwb.cpp` or :file:`interface_impl/session.cpp`.
   Keep that wiring as-is and implement ranging setup in ``UltraWideBandImpl::_ConfigureRangingSession()``.

Outbound path: UWB and Bluetooth LE inter-operation
===================================================

When your UWB stack needs to send a response to the User Device (for example M2 after M1), it must not call Bluetooth LE APIs directly.
Instead, invoke the ``mBleMessageTransmit`` callback registered during ``UltraWideBand::Init()``.

The reference application code wires this callback to the Aliro stack public API:

.. code-block:: cpp

   .mBleMessageTransmit = [](UltraWideBand::SessionContextHandle sessionContext,
                              const uint8_t *data, size_t length) {
       Aliro::AliroStack::Instance().SendBleMessage(sessionContext, data, length);
   },

``SendBleMessage()`` accepts the same full Aliro message layout as ``HandleBleMessage()`` (Protocol Header, Message ID, Length, Payload).
The stack parses the buffer, extracts the protocol type and message ID, encrypts the payload, and transmits it over BLE through ``Interface::Session::Send()``.
Messages you send through this API should therefore use either UWB Ranging Service or Notification with Ranging Message ID — the same types the stack forwards inbound through ``HandleBleMessage()``.
Passing only a payload without the Aliro header is invalid and will be rejected by the stack.

Session context handle
======================

Both interface functions receive an ``Aliro::ConnectionHandle`` (typedef ``UltraWideBand::SessionContextHandle``).
This opaque handle identifies the Aliro Bluetooth LE session and must be stored in your implementation so that:

* Incoming Bluetooth LE payloads from ``HandleBleMessage()`` can be routed to the correct UWB session object.
* Outbound ``mBleMessageTransmit`` calls can be directed to the Bluetooth LE session the stack is expecting.
* Ranging callbacks (``mRangingData``, ``mRangingSessionStateChanged``) correlate measurements with the correct access-control context.

Inbound path: UWB to the access policy layer
=============================================

Your implementation reports two event streams through the ``Callbacks`` structure defined in the :file:`subsys/aliro/uwb/uwb.h` file:

.. list-table::
   :header-rows: 1
   :widths: 35 25 40

   * - Callback
     - Payload type
     - Purpose
   * - ``mRangingSessionStateChanged``
     - ``Aliro::RangingSessionState``
     - Report session lifecycle transitions.
   * - ``mRangingData``
     - ``Aliro::UwbRangingData``
     - Deliver distance samples.

``mRangingSessionStateChanged``
-------------------------------

Notify state transitions using ``Aliro::RangingSessionState`` (:file:`include/aliro/types.h`):

.. code-block:: cpp

   enum class RangingSessionState : uint8_t {
       Uninitialized = 0x00,
       Initialized,
       Idle,
       Ranging,
       RangingSuspended,
       RangingResumed,
       Destroyed,
   };

The ``AccessManager`` acts on ``Destroyed`` and ``RangingSuspended`` to update reader state and prevent unwanted unlock toggling.

``mRangingData``
----------------

Delivers distance samples as ``Aliro::UwbRangingData`` (a data structure that contains a pointer to the data and its length).

The reference ``AccessManager`` reads a 16-bit big-endian distance in centimeters from the first two bytes (see ``ExtractDistanceFromUwbData()`` in the :file:`access_manager_impl.cpp` file).

If your vendor reports distance in another format, do one of the following:

* Convert in ``UltraWideBandImpl`` before invoking the callback.
* Override ``ExtractDistanceFromUwbData()`` in your ``AccessManagerImpl``.

.. _uwb_custom_integration_door_lock_addon:

2. Integration in the door lock add-on
**************************************

Platform UWB code is located under the :file:`subsys/aliro/uwb/` directory.
The build selects the implementation as follows (:file:`subsys/aliro/uwb/CMakeLists.txt`):

.. code-block:: cmake

   if(CONFIG_QM35_UWB_ALIRO_ZEPHYR)
     add_subdirectory(qm35_impl)    # Qorvo example implementation
   else()
     add_subdirectory(custom_impl)  # Your starting point
   endif()

To use a third-party chip:

* Do not enable the ``CONFIG_QM35_UWB_ALIRO_ZEPHYR`` Kconfig option.
* Do not apply the ``uwb_qm35`` snippet.

Enable only the following options:

.. code-block:: kconfig

   CONFIG_DOOR_LOCK_BLE_UWB=y
   CONFIG_DOOR_LOCK_ALIRO_UWB=y

The ``UltraWideBand`` facade pattern
====================================

Following the same design as ``AccessManager``, the add-on exposes a singleton facade:

* ``UltraWideBand`` — Declares the public methods (``Init()``, ``HandleBleMessage()``, …).
* ``UltraWideBandImpl`` — Implements the private methods (``_Init()``, ``_HandleBleMessage()``, …).
* ``UltraWideBandInstance()`` — Returns the facade reference used throughout the application.

External code should call ``UltraWideBandInstance()`` and should not call ``UltraWideBandImpl`` methods directly.

The public ``UltraWideBand`` API in :file:`subsys/aliro/uwb/uwb.h` defines the contract between the reference application and your implementation.

To use it:

* Implement ``UltraWideBandImpl`` in :file:`custom_impl/uwb_impl.h` and :file:`custom_impl/uwb_impl.cpp`, using the files in that directory as a skeleton.
  Replace each ``-ENOSYS`` stub with real driver logic.
* Implement only the methods declared in the facade.
  Keep any additional chip-specific types and helpers private to ``UltraWideBandImpl``.

The :ref:`implementation methods table <uwb_custom_integration_implementation_methods>` lists every method
in the contract and the behavior each one must provide.

.. _uwb_custom_integration_implementation_methods:

Implementation methods
======================

Each row below maps a public ``UltraWideBand`` method to the private ``UltraWideBandImpl`` method you implement, and describes the behavior it must provide.
Methods marked *Optional* can remain no-op or ``-ENOSYS`` stubs if your hardware does not need them.

.. list-table::
   :header-rows: 1
   :widths: 22 38 40

   * - Public method (via facade)
     - Private implementation method
     - Expected behavior
   * - ``Init(callbacks)``
     - ``_Init()``
     - Initialize hardware, store ``callbacks``, set ``_IsInitialized()`` to true when ready.
       Return ``0`` on success.
       The reference ``main.cpp`` treats ``-ENOSYS`` as "UWB not implemented" and continues without ranging.
   * - ``Deinit()``
     - ``_Deinit()``
     - Release sessions and shut down the radio.
   * - ``BleTimeSync()``
     - ``_BleTimeSync()``
     - Optional: trigger CCC Procedure 0 BLE/UWB time sync (Aliro spec §19.4.4.1).
       Implement if your UWB vendor requires explicit time alignment after Bluetooth LE connection.
   * - ``HandleBleMessage(data, length, handle)``
     - ``_HandleBleMessage()``
     - Pass the full decrypted Aliro message (header + payload) to your vendor stack.
       This is the critical path during session setup (M1-M4 exchange over BLE).
       Outbound replies must use the same message layout when invoking ``mBleMessageTransmit`` / ``SendBleMessage()``.
   * - ``ConfigureRangingSession(id, ursk, version, handle)``
     - ``_ConfigureRangingSession()``
     - Create a per-session context keyed by ``handle``, program URSK and protocol version into the vendor stack.
       Called from ``AccessManager`` when the Aliro stack enters ranging.
   * - ``InitiateRangingSession(handle)``
     - ``_InitiateRangingSession()``
     - Start or arm ranging after configuration.
       Currently not used by the reference applications.
   * - ``TerminateRangingSession(handle)``
     - ``_TerminateRangingSession()``
     - Tear down the UWB session when the Bluetooth LE session ends or access process completes.
   * - ``SuspendRangingSession(handle)``
     - ``_SuspendRangingSession()``
     - Pause active ranging (Aliro suspend and resume flow).
       Currently not used by the reference applications.
   * - ``ResumeRangingSession(handle)``
     - ``_ResumeRangingSession()``
     - Resume after suspend.
       Currently not used by the reference applications.
   * - ``GetFirmwareVersion()``
     - ``_GetFirmwareVersion()``
     - Optional: human-readable firmware string for logging or shell commands.
       Return ``nullptr`` if unavailable.
   * - ``IsInitialized()``
     - ``_IsInitialized()``
     - Return whether ``Init()`` completed successfully.
   * - ``StopRadarSession()``
     - ``_StopRadarSession()``
     - Optional: stop any background sensing your hardware uses.
       The ``custom_impl`` stub leaves this as a no-op.

.. note::

   Currently, UWB setup and suspend/resume in the reference applications are driven by BLE messages from the User Device (relayed through the Aliro stack and ``HandleBleMessage()``), not by explicit calls to ``InitiateRangingSession()``, ``SuspendRangingSession()``, or ``ResumeRangingSession()``.
   Those methods remain part of the public API for integrators who need the application to initiate setup or control ranging directly.

Using the Qorvo QM35 implementation as an example
=================================================

The QM35 port (:file:`subsys/aliro/uwb/qm35_impl/`) demonstrates a complete production-style integration.
Study it for patterns, not as a mandatory structure:

* Vendor SDK boundary — Qorvo host driver libraries and APIs wrap the communication with the QM35825 firmware.
  Your port replaces this layer with your vendor's equivalent while keeping the same ``UltraWideBandImpl`` method signatures.
* Session list — QM35 keeps a Zephyr ``sys_slist_t`` of ``SessionContext`` objects mapping ``ConnectionHandle`` to vendor session pointers.
  Replicate this pattern if your driver supports multiple concurrent sessions.
* Callback translation — ``SessionHandlerCallback()`` maps vendor session events to ``RangingSessionState`` and encodes distance into two big-endian bytes before calling ``mRangingData``.
* Bluetooth LE relay — ``TransmitBleMessage()`` invokes ``mBleMessageTransmit`` with raw bytes from the vendor stack.

UWB integration in the reference applications
=============================================

The reference applications already contain the glue code that is used for the integration with the Aliro stack, the Access Manager, and the UWB implementation.
The entries follow the data path, from initialization through to the unlock decision.

Initialization — :file:`applications/*/src/main.cpp`
  Register callbacks and call ``Init()`` **before** ``AliroInit()``:

  .. code-block:: cpp

     #ifdef CONFIG_DOOR_LOCK_BLE_UWB
     constexpr Aliro::Uwb::UltraWideBand::Callbacks uwbCallbacks = { /* ... */ };
     int uwbError = Aliro::Uwb::UltraWideBandInstance().Init(uwbCallbacks);
     #endif

  The reference applications connect callbacks as follows:

  * ``mRangingData`` → ``AccessManagerInstance().HandleRangingSessionData()``
  * ``mRangingSessionStateChanged`` → ``AccessManagerInstance().HandleRangingSessionStateChanged()``
  * ``mBleMessageTransmit`` → ``AliroStack::Instance().SendBleMessage()``

Aliro stack interface — :file:`applications/*/src/aliro/interface_impl/uwb.cpp`
  Implements ``Interface::Uwb::HandleBleMessage()`` by forwarding to ``UltraWideBandInstance().HandleBleMessage()``.

Session lifecycle — :file:`applications/*/src/aliro/interface_impl/session.cpp`
  Forwards stack session events to ``AccessManager``: ranging start on ``StartRangingSession()``, cleanup (including UWB teardown) on ``HandleTermination()``.

Access policy and ranging sessions — :file:`applications/*/src/aliro/access_manager/access_manager_impl.cpp`
  * ``AddRangingSession()`` calls ``UltraWideBandInstance().ConfigureRangingSession()``
  * ``RemoveRangingSession()`` calls ``UltraWideBandInstance().TerminateRangingSession()``
  * ``EvaluateUwbOpenAllowed()`` and ``ExtractDistanceFromUwbData()`` define the distance-based unlock policy

CMake integration
=================

The build is wired across three places:

* Subsystem library — :file:`subsys/aliro/uwb/custom_impl/CMakeLists.txt` compiles your :file:`uwb_impl.cpp`.
  The parent :file:`subsys/aliro/uwb/CMakeLists.txt` always builds the shared facade :file:`uwb.cpp`.
* Application interface — :file:`applications/*/src/aliro/interface_impl/CMakeLists.txt` adds :file:`uwb.cpp` when the ``CONFIG_NCS_ALIRO_BLE_UWB`` Kconfig option is set.
* Include path — Application code includes :file:`uwb_impl.h`.
  With the custom implementation selected, Zephyr resolves that header from :file:`custom_impl/`.

To use your own directory instead of ``custom_impl``, either change the ``else()`` branch in :file:`subsys/aliro/uwb/CMakeLists.txt`, or edit the files in place.

Kconfig checklist
=================

.. list-table::
   :header-rows: 1

   * - Option
     - Required for custom UWB
     - Notes
   * - ``CONFIG_NCS_ALIRO_BLE_UWB``
     - Yes (via ``CONFIG_DOOR_LOCK_BLE_UWB``)
     - Enables stack UWB HSM and interface symbols.
   * - ``CONFIG_DOOR_LOCK_BLE_UWB``
     - Yes
     - Application-level Bluetooth with UWB integration.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB``
     - Yes
     - Builds ``subsys/aliro/uwb/`` platform module.
   * - ``CONFIG_QM35_UWB_ALIRO_ZEPHYR``
     - No
     - Selects Qorvo example implementation instead of ``custom_impl``.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_*``
     - No
     - QM35-only features (DFU, disambiguation, radar).

.. _uwb_custom_integration_sequence:

Recommended bring-up sequence
*****************************

Implement the methods in the order below.
The first stages — initializing the hardware, configuring sessions, and relaying Bluetooth LE messages — form the *minimum viable port*, the smallest set of methods needed for an end-to-end ranging session.
With these in place, the Aliro stack can drive a complete UWB setup exchange through your implementation.

The later stages refine reporting and validate the integration.
Each stage adds a verification point, so you can confirm progress before moving on.

1. Build with stubs — Enable the ``CONFIG_DOOR_LOCK_BLE_UWB`` Kconfig option without QM35 snippets, as described in :ref:`uwb_custom_integration_prerequisites`.
   Confirm the application links and logs ``UWB is not implemented`` (``-ENOSYS`` from ``_Init()``).

#. Initialize the hardware — Implement ``_Init()``, ``_Deinit()``, and ``_IsInitialized()``.
   Verify ``IsInitialized()`` returns true after init, and optionally surface ``GetFirmwareVersion()`` in a log or shell command.

#. Configure and tear down sessions — Implement ``_ConfigureRangingSession()`` and ``_TerminateRangingSession()``.
   Confirm that ``StartRangingSession()`` from the stack succeeds and that a session is cleanly removed on Bluetooth LE disconnect.

#. Relay Bluetooth LE messages — Implement ``_HandleBleMessage()`` and wire ``mBleMessageTransmit``.
   Use logging to confirm M1-M4 traffic flows through your module and back to the User Device.

#. Deliver ranging reports — Fire ``mRangingData`` with a 16-bit big-endian centimeter distance.
   Tune ``AccessManager`` thresholds through ``SetMaxAllowedDistance()`` and confirm the distance reaches the access policy layer.

#. Emit state notifications — Call ``mRangingSessionStateChanged`` so the ``AccessManager`` updates reader state on suspend and destroy.

#. Regression test — Run an Aliro Bluetooth LE with UWB session against a certified User Device or test harness.

Related documentation
*********************

For more background on UWB integration, see the following resources:

* :ref:`uwb_interface` — High-level UWB interface overview in the door lock architecture guide.
* :ref:`aliro_application_interactions` — Sequence diagrams for UWB session establishment.
* :ref:`uwb_disambiguation` — QM35-specific front and back detection (not required for generic ports).
