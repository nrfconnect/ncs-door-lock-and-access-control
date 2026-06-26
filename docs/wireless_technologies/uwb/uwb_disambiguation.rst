.. _uwb_disambiguation:

UWB front/back disambiguation
#############################

.. contents::
   :local:
   :depth: 2

The front/back disambiguation algorithm determines whether a user is standing in front of or behind the door lock, so that the door is opened only for users on the front side.

.. note::
   This feature is available only with the QM35825 integration.
   The disambiguation algorithm is Qorvo intellectual property and depends on QM35-specific radar and diagnostic measurements.
   It is not included in third-party UWB ports and cannot be reused with other UWB radios.
   See :ref:`uwb_custom_integration`.

.. note::
   The disambiguation algorithm depends heavily on the antenna design and placement of the UWB module in the device.
   To improve the reliability, tune the parameters in the calibration files (:file:`subsys/aliro/uwb/qm35_impl/calibration/`).
   You might also need to adapt the Kconfig options in :file:`subsys/aliro/disambiguation/Kconfig` and :file:`subsys/aliro/uwb/qm35_impl/front_back_detection/Kconfig` to meet your specific requirements.

Overview
********

The integration combines three measurement sources from the QM35 UWB module and feeds them into the disambiguation algorithm:

* Distance — from the standard Aliro CCC ranging session (controlee reports).
* PDOA and RSSI — from CCC session diagnostic reports (controlee reply frame).
* CIR (Channel Impulse Response) — from a dedicated Cherry radar session that is started alongside ranging when the user is close enough.

The algorithm outputs a FRONT or BACK side classification per session.
The core processing is provided by the Qorvo disambiguation component (``Q_ALIRO_DISAMBIGUATION``).
Its public API is defined in :file:`aliro_disambiguation.h` in the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository.

The implementation is split into two layers:

* Algorithm library — :file:`subsys/aliro/disambiguation/` exposes ``Disambiguation::Disambiguator`` (a thread-safe singleton), which buffers samples and calls ``aliro_disambiguation()``.
* QM35 integration — :file:`subsys/aliro/uwb/qm35_impl/front_back_detection/` implements ``FrontBackDetection``, which subscribes to session events, feeds the disambiguator, and runs periodic ``Disambiguator::Process()`` calls while ranging is active.

Enable the QM35 integration by setting ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION``.
This option selects ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION`` (algorithm library), ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR`` (CIR source), and ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS`` (PDOA/RSSI source).

Data flow
=========

#. Session events (status changes, controlee distance reports, and diagnostic reports) are dispatched through :file:`subsys/aliro/uwb/qm35_impl/session_event_hub.cpp` to all registered subscribers, including ``FrontBackDetection``, ``UwbRadar``, and ``UwbDiagnostics``.
#. Distance — ``FrontBackDetection`` forwards controlee distance reports to ``Disambiguator::AddDistanceMeasurement()``.
   Distances above 500 cm (``kUwbMaximumReportableDistanceCm``) and frames with an error status are fed as error samples.
#. PDOA and RSSI — ``UwbDiagnostics`` enables AoA and RSSI reporting on the UWB adapter.
   ``FrontBackDetection`` reads the PDOA and RSSI from the controlee reply frame (``frame_report[1]``) of each diagnostic report and forwards them to ``Disambiguator::AddPdoaMeasurement()``.
#. Radar CIR samples are forwarded from :file:`subsys/aliro/uwb/qm35_impl/radar/radar.cpp` through the ``onRadarMeasurement`` callback registered in :file:`subsys/aliro/uwb/qm35_impl/uwb_impl.cpp` to ``FrontBackDetection::HandleRadarMeasurement()``, which calls ``Disambiguator::AddCirMeasurement()``.
#. Periodic processing — ``FrontBackDetection`` calls ``Disambiguator::Process()`` on a delayable work item every ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION_PROCESSING_INTERVAL_MS`` (default 48 ms) for every session whose ranging state is ``Ranging`` or ``RangingResumed``.
   The work item reschedules itself after each run.
#. Access decision — on each ranging measurement, the Access Manager evaluates distance (``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` and exit margin) and, when open is not yet allowed for the session, consults the last FRONT/BACK result for that session (see :ref:`uwb_disambiguation_access_manager`).

``Disambiguator::Process()`` only produces a result once enough samples are buffered: at least ``ALIRO_DISAMBIGUATION_WINDOW_SIZE`` (40) CIR (radar), distance, and PDOA samples for the session.
Until then it returns ``-EBUSY`` and no result is stored.
With the default 48 ms processing interval, the first FRONT or BACK result typically appears about two seconds after the radar session starts and samples begin to accumulate.

Radar lifecycle
===============

The radar session is managed in :file:`subsys/aliro/uwb/qm35_impl/radar/radar.cpp`.
``UwbRadar`` subscribes to session events directly and reacts to controlee distance reports and session state changes.

* Started — when a valid controlee distance report is less than or equal to ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_ACTIVATION_DISTANCE_CM`` (default 150 cm).
  Reports with an error status or with a distance above 500 cm do not trigger a start.
* Stopped — ``UwbRadar::Stop()`` runs when any of the following occurs:

  * The CCC ranging session transitions from ``ACTIVE`` to ``IDLE``.
  * The CCC ranging session transitions from ``IDLE`` to ``DEINIT``.
  * A ranging session error (``ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR``) or a radar session error (``CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR``) occurs.
  * ``UnlockAction()`` stops the radar for UWB sessions via ``UltraWideBandImpl::StopRadarSession()`` (see :ref:`uwb_disambiguation_access_manager`).

  The radar session is not stopped when the user simply moves beyond the activation distance.
  ``UwbRadar::Stop()`` also cancels any pending (scheduled but not yet started) radar start and always invokes the ``mOnSessionStopped`` callback.
  ``LockAction()`` does not stop the radar session.

Front/back processing lifecycle
===============================

Managed in :file:`subsys/aliro/uwb/qm35_impl/front_back_detection/front_back_detection.cpp`.

* Started / restarted — when the CCC ranging session transitions from ``IDLE`` to ``ACTIVE``:

  * If the session's ranging state is ``Idle`` (new ranging session), the disambiguator state for that session is reset with ``Disambiguator::ResetSession()`` and periodic processing is scheduled.
  * If the session's ranging state is ``RangingSuspended`` (resumed ranging), periodic processing is scheduled without resetting the session state.

  Processing is also scheduled when controlee distance reports arrive while processing is not yet enabled.
* Stopped — ``FrontBackDetection::CancelProcessing()`` disables the periodic work item and synchronously cancels any pending run.
  It is invoked:

  * Directly, when the CCC ranging session transitions from ``ACTIVE`` to ``IDLE``.
  * From the radar ``mOnSessionStopped`` callback registered in :file:`subsys/aliro/uwb/qm35_impl/uwb_impl.cpp`, which runs whenever ``UwbRadar::Stop()`` runs.

  ``CancelProcessing()`` does not clear the last FRONT/BACK result stored in the disambiguator.

.. _uwb_disambiguation_access_manager:

Access Manager integration
**************************

When ``CONFIG_DOOR_LOCK_BLE_UWB`` is enabled, the Access Manager applies a distance gate before allowing unlock from UWB ranging.
When ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION`` is additionally enabled, it also applies a front/back gate.
Both gates are evaluated in ``AccessManagerImpl::EvaluateUwbOpenAllowed()``.

.. list-table::
   :header-rows: 1
   :widths: 18 26 36 20

   * - Gate
     - When evaluated
     - Pass condition
     - Kconfig (defaults)
   * - Distance (enter)
     - Every ranging report while open is not yet allowed
     - Distance ≤ ``MAX_ALLOWED_DISTANCE_CM``
     - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` (100 cm)
   * - Distance (exit)
     - Every ranging report while open is already allowed
     - Distance > ``MAX_ALLOWED_DISTANCE_CM + EXIT_MARGIN_CM``
     - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_EXIT_MARGIN_CM`` (50 cm)
   * - Front/back
     - Every ranging report, only before the first unlock
     - Last disambiguation result for the session is FRONT
     - Requires ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION``
   * - Unlock
     - When all gates pass
     - ``UnlockAction()`` runs; UWB radar session stops
     - —
   * - Lock
     - When distance exit condition is met
     - ``LockAction()`` runs
     - Unless ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK`` (lock simulator timer)

See :ref:`aliro_access_manager` for distance threshold details.

Implementation details
======================

``AccessManagerImpl::DisambiguationAllowsOpen()`` resolves the session index through ``UltraWideBandImpl::GetDisambiguationSessionIdx()`` and reads the last result with ``Disambiguator::TryGetLastResult()``.
If no result is available yet, or the last result is BACK, the front/back gate blocks open.

On unlock, ``SetOpenAllowed()`` calls ``UnlockAction(false)``, which invokes ``UltraWideBandImpl::StopRadarSession()`` for UWB sessions.
That stops the radar and triggers ``FrontBackDetection::CancelProcessing()`` through the radar ``mOnSessionStopped`` callback.
``LockAction()`` does not stop the radar or front/back processing directly.

Configuration parameters
************************

The following tables list Kconfig options for the QM35 integration layer, the Cherry radar session, diagnostics, the core disambiguation algorithm, and UWB access distance thresholds.
Set them in :file:`prj.conf` unless noted otherwise.

Integration and timing
======================

Options in :file:`subsys/aliro/uwb/qm35_impl/front_back_detection/Kconfig`:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Default
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION``
     - disabled
     - Enables QM35 front/back detection integration.
       Selects the disambiguation library (``DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION``), radar support (``DOOR_LOCK_ALIRO_UWB_QM35_RADAR``), and diagnostics (``DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS``).
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION_PROCESSING_INTERVAL_MS``
     - 48
     - Interval between ``Disambiguator::Process()`` runs while ranging is active.

Radar session
=============

Options in :file:`subsys/aliro/uwb/qm35_impl/radar/Kconfig`:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Default
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_ANT_SET_ID``
     - 3
     - Antenna set used by the Cherry radar session (range 2-3).
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_CHANNEL``
     - 9
     - UWB RF channel for the radar session (range 5-9, typically 5 or 9).
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_ACTIVATION_DISTANCE_CM``
     - 150
     - Ranging distance at or below which the radar start is scheduled (range 30-500).

Cherry radar session parameters that are not exposed through Kconfig (burst period, sweep layout, preamble, session ID, and similar) are fixed as ``constexpr`` constants in :file:`subsys/aliro/uwb/qm35_impl/radar/radar.cpp`.

Diagnostics
===========

Options in :file:`subsys/aliro/uwb/qm35_impl/diagnostics/Kconfig`:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Default
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_DEFAULT_HANDLER``
     - enabled
     - Enables a default diagnostic report handler that converts the raw AoA data to degrees and logs it.
       Front/back detection consumes the diagnostic reports regardless of this option.

Algorithm parameters
====================

Core algorithm tuning options are defined in :file:`subsys/aliro/disambiguation/Kconfig`.
At runtime they are loaded into ``kFrontBackDetectionParams`` in :file:`subsys/aliro/uwb/qm35_impl/front_back_detection/front_back_detection.cpp` and passed to ``Disambiguator::Init()``.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Default
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_MAX_CIR``
     - 800
     - Maximum absolute CIR; above this the previous FRONT/BACK decision is reused instead of computing ``p_ratio``.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_P_RATIO``
     - 7500 (4-port), 6000 (2-port)
     - Minimum ``p_ratio`` for FRONT.
       The value is divided by 10000 at runtime (e.g. 7500 → 0.75).
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_NB_BLOCKS``
     - 10
     - Maximum number of noise blocks allowed for a FRONT decision.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_NOISE_THRESHOLD``
     - 15
     - CIR amplitude below which a block is classified as noise.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_SHADOW_THRESHOLD``
     - 8
     - Noise threshold in the shadow (back) region.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_BLIND_DISTANCE_CM``
     - 30
     - Minimum distance for the algorithm to engage; below this the result is unreliable.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_SECURE_BUBBLE_CM``
     - 150 (4-port), 80 (2-port)
     - Maximum distance (cm) at which the disambiguation algorithm starts processing.
       Has no effect when approaching purely from the back side.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_FRONT_TO_BACK_CONFIRM``
     - 8
     - Consecutive BACK results required to transition the side decision from FRONT to BACK.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_BACK_TO_FRONT_CONFIRM``
     - 6
     - Consecutive FRONT results required to transition the side decision from BACK to FRONT.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_PDOA_OFFSET_DEG``
     - 0
     - PDOA calibration offset in degrees, subtracted from raw PDOA before processing.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_DISTANCE_OFFSET_CM``
     - 0
     - Distance calibration offset in centimeters, subtracted from raw range before processing.

The Access Manager front/back gate uses the ``Result::IsFront()`` side classification, not the algorithm's internal secure-bubble door-open result.
``Disambiguator::Process()`` stores ``results.side`` and ignores the ``disambiguation()`` door-open return value, so ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_SECURE_BUBBLE_CM`` does not influence the FRONT/BACK decision consumed by the Access Manager.

Post-processing in :file:`subsys/aliro/disambiguation/src/disambiguator.cpp` (``Disambiguator::Process()``):

* Temporal hysteresis — FRONT→BACK requires ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_FRONT_TO_BACK_CONFIRM`` (default 8) consecutive BACK results; BACK→FRONT requires ``CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_BACK_TO_FRONT_CONFIRM`` (default 6) consecutive FRONT results.

Distance and access thresholds
==============================

UWB unlock/lock distance limits are configured under the Access Manager menu in :file:`applications/aliro-access-control-app/src/aliro/access_manager/Kconfig` and :file:`applications/matter-aliro-door-lock-app/src/aliro/access_manager/Kconfig`.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Default
     - Description
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM``
     - 150
     - Maximum UWB ranging distance (cm) to enter the open-allowed state.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_EXIT_MARGIN_CM``
     - 30
     - Additional margin (cm) above the maximum distance before the door locks again (exit range).
       Set to 0 to disable the exit margin.

See also :ref:`aliro_access_manager`.

Building
********

Front/back detection requires the QM35 UWB module and is enabled with ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION``.
For UWB build and flash steps, see :ref:`aliro_access_control_application` (|ALIRO_APP_NAME|) or :ref:`aliro_matter_access_control_application` (|MATTER_ALIRO_APP_NAME|).

For example, to build the Aliro access control application for the nRF54LM20 DK with QM35 UWB and front/back detection enabled, run:

.. code-block:: bash

   west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp applications/aliro-access-control-app -- \
       -Daliro-access-control-app_SNIPPET=uwb_qm35 \
       -DCONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION=y

Replace the application path with :file:`applications/matter-aliro-door-lock-app` (and the matching ``matter-aliro-door-lock-app_SNIPPET``) for the Matter variant.

Enabling ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION`` automatically pulls in the disambiguation library, radar, and diagnostics, so no other Kconfig options need to be set to get the default behavior.

Testing
*******

#. Build and flash a UWB-capable variant with front/back detection enabled, then open a serial terminal (see :ref:`aliro_access_control_application` or :ref:`aliro_matter_access_control_application`).
#. Start an Aliro UWB ranging session with a User Device and approach the lock from the front.
   As the radar starts (within the activation distance), the ``FrontBackDetection`` module logs one line per processed result:

   .. code-block:: console

      [side] FRONT | dist: 65cm | pratio_u6: 420000 | cir: 312 | blk: 3 | pdoa:+58.123

   The ``[side]`` field shows ``FRONT`` or ``BACK``, followed by the distance, ``p_ratio`` (scaled by 1,000,000), absolute CIR, noise-block count, and mean PDOA in degrees.
#. Verify the gating behavior:

   * Approaching from the front within ``MAX_ALLOWED_DISTANCE_CM`` should produce ``FRONT`` results and allow the door to open once both the distance and front/back gates pass.
   * Approaching from the back should produce ``BACK`` results and block unlock even when distance is within ``MAX_ALLOWED_DISTANCE_CM``.
   * Moving beyond ``MAX_ALLOWED_DISTANCE_CM + EXIT_MARGIN_CM`` after unlock should trigger lock (or auto-relock when the lock simulator is configured).

#. To increase log verbosity, raise the module log levels (``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION_LOG_LEVEL``, ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_LOG_LEVEL``, and ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_LOG_LEVEL``) and enable ``CONFIG_DOOR_LOCK_ALIRO_UWB_SESSION_LOGGING`` for session state transitions.

Related documentation
*********************

* :ref:`wireless_technologies_uwb` — How UWB fits into the add-on, including access policy overview.
* :ref:`aliro_access_manager` — Distance thresholds and exit margin Kconfig options.
* :ref:`uwb_integration` — QM35825 UWB integration architecture and stack interactions.
* :ref:`uwb_custom_integration` — Porting a third-party UWB module (does not include this feature).
