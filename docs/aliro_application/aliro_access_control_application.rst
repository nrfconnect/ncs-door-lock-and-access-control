.. _aliro_access_control_application:
.. _aliro_building_and_running:

Application guide
#################

.. contents::
   :local:
   :depth: 2

This page describes hardware requirements, configuration, building, and verification for the |ALIRO_APP_NAME|.
For firmware update procedures, see :ref:`aliro_firmware_update`.
For Test Harness setup and provisioning, see :ref:`aliro_testing`.
For common issues, see :ref:`aliro_troubleshooting`.

Requirements
************

The application supports the following development kits:

.. list-table:: Supported hardware platforms
   :widths: auto
   :header-rows: 1

   * - Hardware platforms
     - PCA
     - Board name
     - Build target
   * - `nRF54LM20 DK`_
     - PCA10184
     - `nrf54lm20dk`_
     - | ``nrf54lm20dk/nrf54lm20b/cpuapp``
       | ``nrf54lm20dk/nrf54lm20a/cpuapp``
   * - `nRF54L15 DK`_
     - PCA10156
     - `nrf54l15dk`_
     - ``nrf54l15dk/nrf54l15/cpuapp``
   * - `nRF5340 DK`_
     - PCA10095
     - `nrf5340dk`_
     - ``nrf5340dk/nrf5340/cpuapp``
   * - `nRF52840 DK`_
     - PCA10056
     - `nrf52840dk`_
     - ``nrf52840dk/nrf52840``

You also need an :ref:`NFC reader expansion board <hw_requirements_nfc_reader>` connected to the DK.
For Aliro over Bluetooth LE and UWB, add a :ref:`QM35825 UWB module <hw_requirements_uwb_module>` on the `nRF5340 DK`_ or `nRF54LM20 DK`_.

See :ref:`hw_requirements` for wiring diagrams, VDDIO configuration on nRF54L-series DKs, and the full hardware setup.

Overview
********

The |ALIRO_APP_NAME| implements an Aliro reader that authenticates User Devices and drives a simulated bolt lock through the Access Manager.

You can exercise the application in the following ways:

* On a single DK using the serial shell and :ref:`aliro_testing_provisioning_cli`.
* With the Aliro Test Harness acting as the User Device (see :ref:`aliro_testing`).

.. _aliro_access_control_application_features:

Application features
====================

The |ALIRO_APP_NAME| supports the following capabilities:

* Aliro over NFC — Default tap-to-unlock transport.
  See :ref:`nfc_integration`.
* Aliro over Bluetooth LE and UWB — Optional hands-free unlock.
  See :ref:`aliro_ble_transport` and :ref:`uwb_integration`.
* CLI provisioning — Provision reader credentials through shell.
  See :ref:`aliro_testing_provisioning_cli`.
* Device Firmware Update over SMP — Optional field updates over Bluetooth LE.
  See :ref:`aliro_firmware_update` and :ref:`door_lock_dfu_smp_service`.
* Nordic UART Service — Optional Bluetooth LE command channel.
  See :ref:`door_lock_nus_service`.
* Advanced Aliro phases — Optional expedited-fast and step-up authentication.
  See :ref:`aliro_advanced_features`.

This application does not integrate with Matter.
For a Matter-enabled variant, see :ref:`doc_aliro_matter_door_lock_application`.

.. _aliro_building_and_running_config_options:

Configuration
*************

The following Kconfig options are useful when customizing :file:`prj.conf` or build arguments.
They are not required for the default NFC-only build.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Configuration option
     - Description
   * - ``CONFIG_DOOR_LOCK_BLE_UWB``
     - Enables Aliro over Bluetooth LE and UWB.
       When disabled, the application uses NFC-only Aliro transport.
       Prefer the ``uwb_qm35`` snippet instead of enabling this option in isolation.
   * - ``CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS``
     - Available when ``CONFIG_DOOR_LOCK_BLE_UWB`` is enabled.
       Sets the maximum number of concurrent Aliro Bluetooth LE and UWB sessions.
       When changing the session count, set ``CONFIG_BT_MAX_CONN`` to at least that value plus any connections used on the default identity by other services.

For Bluetooth LE transport and session-limit details, see :ref:`aliro_ble_transport`.

Debug and release builds
========================

Debug is the default configuration.

In release configuration, the application is built with the following characteristics:

* Aliro stack logs are disabled.
* RFAL NFC driver logs are disabled.
* Power-management options are enabled.
* Unused peripherals are disabled using the board-specific ``*_release.overlay``.
* The device resets automatically on a fatal error.

The application keeps the UART shell enabled in release builds so you can provision keys through the CLI (see :ref:`Provisioning with CLI <aliro_testing_provisioning_cli>`).

.. list-table:: Aliro Access Control Application build configurations
   :widths: auto
   :header-rows: 1

   * - Configuration
     - File name
     - :makevar:`FILE_SUFFIX`
     - Supported board
     - Description
   * - Debug (default)
     - :file:`prj.conf`
     - No suffix
     - All from `Requirements`_
     - Debug version of the application.

       Enables additional features for verifying the application behavior, such as logs.
   * - Release
     - :file:`prj_release.conf`
     - ``release``
     - All from `Requirements`_
     - Release version of the application.

       Enables only the necessary application functionalities to optimize its performance.

To build in release mode:

.. code-block:: bash

   west build -p -b <build_target> applications/aliro-access-control-app -- -DFILE_SUFFIX=release

User interface
**************

The reference application exposes a minimal development kit interface.
Lock actions are driven by Aliro authentication through the Access Manager, not by lock/unlock buttons as in the Matter door lock sample.

Development kit interface
=========================

Button 1:
   When the application is built with the ``dfu_smp`` snippet, toggles Bluetooth LE advertising for DFU over SMP.
   You can also use the ``dfu_smp on`` and ``dfu_smp off`` shell commands.

LED 2:
   When ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR`` is enabled (default), shows the simulated lock state through the ``lock-sim-indicator`` devicetree alias:

   * On — Bolt is retracted (unlocked).
   * Off — Bolt is extended (locked).

   The LED stays on for the simulated movement time and any configured auto-relock period.
   See :ref:`aliro_lock_sim` for lock simulator timing options.

SEGGER J-Link USB port:
   Used for logs and the UART shell (provisioning commands).

Building and running
********************

.. |sample path| replace:: :file:`ncs-door-lock-and-access-control/applications/aliro-access-control-app`

.. include:: ../include/build_and_run.txt

#. Connect the DK to your computer using the DEBUGGER port and set the **POWER** switch to **ON**.

#. In the :file:`project-workspace` directory, navigate to the :file:`ncs-door-lock-and-access-control` folder.

#. Build the application for your DK and :ref:`NFC reader expansion board <hw_requirements_nfc_reader>`.
   Find ``<build_target>`` for your DK in the table in `Requirements`_ or in :ref:`hw_requirements_development_kit`.

   .. list-table::
      :header-rows: 1

      * - NFC reader expansion board
        - Extra CMake argument
        - Notes
      * - `X-NUCLEO-NFC12A1`_
        - —
        - Recommended for new designs.
      * - `X-NUCLEO-NFC09A1`_
        - ``-DCONFIG_ST25R200_DRV=y``
        - Supported, but not recommended for new products.

   Example for the nRF5340 DK with `X-NUCLEO-NFC12A1`_:

   .. code-block:: bash

      west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app

   Example for the nRF54L15 DK with `X-NUCLEO-NFC09A1`_:

   .. code-block:: bash

      west build -p -b nrf54l15dk/nrf54l15/cpuapp applications/aliro-access-control-app -- -DCONFIG_ST25R200_DRV=y

#. Flash the firmware:

   .. code-block:: bash

      west flash

#. Verify that the application runs (see :ref:`aliro_building_and_running_verify`).

Build variants
==============

The default quick-start build is Aliro over NFC only.
Use the variants below when you need Bluetooth LE with UWB, optional Bluetooth LE services, release optimizations, or QM35 firmware update support.

Replace ``<build_target>`` with your DK target from :ref:`hw_requirements_development_kit`.
For UWB hardware setup, see :ref:`hw_requirements_uwb_module` and :ref:`uwb_integration`.

.. note::

   |QM35_EXPERIMENTAL_NOTE|

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Variant
     - Example build command
   * - Aliro standalone (NFC only)
     - ``west build -p -b <build_target> applications/aliro-access-control-app``
   * - QM35825 UWB
     - ``west build -p -b <build_target> applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=uwb_qm35``
       See :ref:`aliro_building_and_running_qm35_src` for the required workspace setup.
   * - SMP DFU over Bluetooth LE
     - ``west build -p -b <build_target> applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=dfu_smp``
       See :ref:`aliro_firmware_update` and :ref:`door_lock_dfu_smp_service`.
   * - Nordic UART Service (NUS)
     - ``west build -p -b <build_target> applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=bt_nus``
       See :ref:`door_lock_nus_service`.
   * - Combined snippets
     - Separate snippet names with semicolons, for example ``-Daliro-access-control-app_SNIPPET='uwb_qm35;dfu_smp'``.
   * - QM35 firmware update (DFU)
     - Pass the sysbuild ``uwb_qm35_dfu`` snippet together with ``uwb_qm35``, for example ``-- -DSNIPPET=uwb_qm35_dfu -Daliro-access-control-app_SNIPPET=uwb_qm35``.
       You must add ``dfu_smp`` to the application snippet when you update the main application over SMP.
       See :ref:`aliro_firmware_update`.
   * - Release build
     - ``west build -p -b <build_target> applications/aliro-access-control-app -- -DFILE_SUFFIX=release``
       Combine with snippets when needed.
   * - QM35 front/back disambiguation
     - Add ``-DCONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION=y`` to a QM35 UWB build.
       See :ref:`uwb_disambiguation`.

The ``uwb_qm35`` snippet enables the ``CONFIG_DOOR_LOCK_BLE_UWB`` Kconfig option and configures the board overlay so the NFC and UWB modules share the same SPI bus.
Use the snippet rather than setting ``CONFIG_DOOR_LOCK_BLE_UWB`` alone, which enables the transport without the Qorvo QM35825 implementation.
The snippet uses the UWB stack and QM35 host driver from the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository, so you must first add it to your workspace (see :ref:`aliro_building_and_running_qm35_src`).

Example for the nRF5340 DK with QM35825 UWB:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- \
       -Daliro-access-control-app_SNIPPET=uwb_qm35

Example for the nRF54LM20 DK with QM35825 UWB:

.. code-block:: bash

   west build -p -b nrf54lm20dk/nrf54lm20a/cpuapp applications/aliro-access-control-app -- \
       -Daliro-access-control-app_SNIPPET=uwb_qm35

.. _aliro_building_and_running_qm35_src:

QM35 SDK repository
-------------------

QM35 support uses the UWB stack and QM35 host driver from the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository.
Add the repository to your workspace before building with the ``uwb_qm35`` snippet:

.. code-block:: bash

   west config manifest.group-filter -- +qm35-aliro-sdk
   west update

Before first use on QM35 hardware, program the module coprocessor firmware (see :ref:`aliro_flashing_qm35_using_nrf53_dk`).

.. _aliro_building_and_running_verify:

Flash and verify
================

After building, flash the firmware:

.. code-block:: bash

   west flash

Verify that the application runs correctly:

.. tabs::

   .. tab:: Debug configuration

      Connect to the DK with a terminal emulator that supports VT100/ANSI escape characters.
      The `Serial Terminal app`_ is recommended.
      See `Testing and optimization`_ in the |NCS| documentation for serial settings.

      .. note::
         |app_hwfc_enabled|

      Press **RESET** on the DK.
      You should see the Zephyr boot banner and a line similar to:

      .. code-block:: console

         Starting nRF Door Lock and Access Control Application

      When QM35 UWB is enabled, you should also see UWB initialization logs ending with:

      .. code-block:: console

         uwb: Initializing UWB device...
         uwb: UWB device initialized successfully.

      An ``Awake frame not received`` line from ``hsspi_helpers`` can appear during init and does not necessarily indicate a fault if initialization completes successfully.

      If UWB initialization fails on first boot, complete :ref:`aliro_flashing_qm35_using_nrf53_dk` before retesting.

   .. tab:: Release configuration

      Boot logs are reduced compared with debug builds.
      Connect to the serial console and verify that the ``dl`` shell command is available (see :ref:`Provisioning with CLI <aliro_testing_provisioning_cli>`).

Dependencies
************

This application uses the following |REPO_NAME| and |NCS| components:

* Aliro stack (binary library in :file:`lib/aliro`)
* :ref:`aliro_access_manager`
* :ref:`aliro_lock_sim`
* :ref:`nfc_integration` — NFC transport and RFAL driver
* PSA Crypto

When DFU is enabled, the application also uses `MCUboot`_ for image management.

The application depends on the following Zephyr facilities:

* `Logging`_
* `Kernel Services`_
* Bluetooth host stack (when Bluetooth LE features or UWB transport are enabled)
