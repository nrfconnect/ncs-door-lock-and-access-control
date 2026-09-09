.. _aliro_matter_access_control_application:
.. _building_and_running:

Application guide
#################

.. contents::
   :local:
   :depth: 2

This page describes hardware requirements, configuration, building, and verification for the |MATTER_ALIRO_APP_NAME|.
For firmware update procedures, see :ref:`firmware_update`.
For commissioning, provisioning, and feature testing, see :ref:`testing`.
For common issues, see :ref:`troubleshooting`.

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
For Aliro over Bluetooth LE and UWB, add an :ref:`external UWB module <hw_requirements_uwb_module>` on the `nRF5340 DK`_ or `nRF54LM20 DK`_.

To commission the device and control it remotely with Matter over Thread, see :ref:`testing_door_lock_provisioning_with_matter`.

See :ref:`hw_requirements` for wiring diagrams, VDDIO configuration on nRF54L-series DKs, and the full hardware setup.

Overview
********

The |MATTER_ALIRO_APP_NAME| combines a Matter door lock accessory with an Aliro reader that authenticates User Devices and drives a simulated bolt lock through the Access Manager.

You can exercise the application in the following ways:

* On a single DK using the serial shell and the development kit buttons and LEDs (see :ref:`matter_ui`).
* Through Matter over Thread using CHIP Tool or a commercial ecosystem (see :ref:`testing_door_lock_provisioning_with_matter`).
* With the Aliro Test Harness acting as the User Device for NFC or UWB flows (see :ref:`testing`).

.. _aliro_matter_access_control_application_features:

Application features
====================

The |MATTER_ALIRO_APP_NAME| supports the following capabilities:

* Matter door lock cluster — Remote lock control and credential management over Thread.
* Aliro over NFC — Default tap-to-unlock transport.
  See :ref:`nfc_integration`.
* Aliro over Bluetooth LE and UWB — Optional hands-free unlock.
  See :ref:`aliro_ble_transport` and :ref:`uwb_integration`.
* Matter Over-The-Air (OTA) update — Remote firmware update through the Matter fabric.
  See :ref:`matter_ota`.
* Device Firmware Update over SMP — Optional field updates over Bluetooth LE.
  See :ref:`firmware_update` and :ref:`door_lock_dfu_smp_service`.
* Nordic UART Service — Optional Bluetooth LE command channel.
  See :ref:`door_lock_nus_service`.
* Aliro credential provisioning through Matter — Provision reader credentials from a Matter controller or ecosystem app.
  See :ref:`testing_door_lock_provisioning_with_matter`.
* Advanced Aliro phases — Optional expedited-fast and step-up authentication.
  See :ref:`aliro_advanced_features`.

.. _matter_aliro_building_and_running_config_options:

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
       By default, the in-tree ``stub_impl`` is built as the backend for the UWB facade.
       Provide a functional UWB implementation as described in :ref:`uwb_custom_integration` to make the Aliro UWB transport operational.
   * - ``CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS``
     - Available when ``CONFIG_DOOR_LOCK_BLE_UWB`` is enabled.
       Sets the maximum number of concurrent Aliro Bluetooth LE and UWB sessions.
       Defaults to ``CONFIG_BT_MAX_CONN``.
       When changing the session count, set ``CONFIG_BT_MAX_CONN`` to at least that value plus any connections used on the default identity by Matter commissioning, SMP DFU, or NUS.

For Bluetooth LE transport and session-limit details, see :ref:`door_lock_app_arch_bluetooth_le`.

Debug and release builds
========================

Debug is the default configuration.

In release configuration, the application is built with the following characteristics:

* All logs are disabled.
* Power-management options are enabled.
* Unused peripherals are disabled using the board-specific ``*_release.overlay``.
* The device resets automatically on a fatal error.

Shell availability differs by build type:

* In release builds, the UART shell is disabled by default to save memory.
* In debug builds shell remains enabled for testing and CLI provisioning (see :ref:`Provisioning with CLI <testing_provisioning_cli>`).

.. list-table:: Matter Aliro Door Lock Application build configurations
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

       Enables additional features for verifying the application behavior, such as logs and shells.
   * - Release
     - :file:`prj_release.conf`
     - ``release``
     - All from `Requirements`_
     - Release version of the application.

       Enables only the necessary application functionality to optimize its performance.

To build in release mode:

.. code-block:: bash

   west build -p -b <build_target> applications/matter-aliro-door-lock-app -- -DFILE_SUFFIX=release

.. _matter_ui:

User interface
**************

The reference application exposes the development kit buttons and LEDs used for Matter commissioning, lock simulation, and optional Bluetooth LE services.

.. note::
   The button and LED numbering differs between development kits.
   On the nRF52840 and nRF5340 boards, the numbering starts from 1.
   On the nRF54L15 and nRF54LM20 boards, it starts from 0.

Development kit interface
=========================

LED 1:
   .. include:: /include/matter_state_led.txt

LED 2:
   .. include:: /include/matter_signalling_led.txt

Button 1:
   .. include:: /include/matter_button.txt

Button 2:
   Changes the lock state to the opposite one.

Button 3:
   When the application is built with the ``dfu_smp`` snippet, toggles Bluetooth LE advertising for DFU over SMP.

SEGGER J-Link USB port:
   Used for logs and the UART shell.

NFC reader expansion board:
   Connect the selected reader board to the Arduino-compatible headers on the DK (see :ref:`hw_requirements_nfc_reader`).

Building and running
********************

.. |sample path| replace:: :file:`ncs-door-lock-and-access-control/applications/matter-aliro-door-lock-app`

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

      west build -p -b nrf5340dk/nrf5340/cpuapp applications/matter-aliro-door-lock-app

   Example for the nRF54L15 DK with `X-NUCLEO-NFC09A1`_:

   .. code-block:: bash

      west build -p -b nrf54l15dk/nrf54l15/cpuapp applications/matter-aliro-door-lock-app -- -DCONFIG_ST25R200_DRV=y

#. Flash the firmware:

   .. code-block:: bash

      west flash

#. Verify that the application runs (see :ref:`matter_aliro_building_and_running_verify`).

Build variants
==============

The default quick-start build is Matter with Aliro over NFC only.
Use the variants below when you need Bluetooth LE with UWB, optional Bluetooth LE services, or release optimizations.

Replace ``<build_target>`` with your DK target from :ref:`hw_requirements_development_kit`.
For UWB hardware setup, see :ref:`hw_requirements_uwb_module` and :ref:`uwb_integration`.

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Variant
     - Example build command
   * - Matter and Aliro (NFC only)
     - ``west build -p -b <build_target> applications/matter-aliro-door-lock-app``
   * - Aliro over Bluetooth LE and UWB
     - ``west build -p -b <build_target> applications/matter-aliro-door-lock-app -- -DCONFIG_DOOR_LOCK_BLE_UWB=y``
       Builds the in-tree UWB stub by default.
       Provide a UWB backend as described in :ref:`uwb_custom_integration`.
   * - SMP DFU over Bluetooth LE
     - ``west build -p -b <build_target> applications/matter-aliro-door-lock-app -- -Dmatter-aliro-door-lock-app_SNIPPET=dfu_smp``
       See :ref:`firmware_update` and :ref:`door_lock_dfu_smp_service`.
   * - Nordic UART Service (NUS)
     - ``west build -p -b <build_target> applications/matter-aliro-door-lock-app -- -Dmatter-aliro-door-lock-app_SNIPPET=bt_nus``
       See :ref:`door_lock_nus_service`.
   * - Combined snippets
     - Separate snippet names with semicolons, for example, ``-Dmatter-aliro-door-lock-app_SNIPPET='dfu_smp;bt_nus'``.
   * - Release build
     - ``west build -p -b <build_target> applications/matter-aliro-door-lock-app -- -DFILE_SUFFIX=release``
       Combine with snippets when needed.

``CONFIG_DOOR_LOCK_BLE_UWB`` enables the Bluetooth LE and Aliro UWB transport.
With the default ``DOOR_LOCK_ALIRO_UWB_IMPL`` choice it builds the in-tree ``stub_impl`` stub, whose methods return ``-ENOSYS`` so the application builds and runs without ranging.
To get working ranging, plug in a UWB implementation as described in :ref:`uwb_custom_integration`.

This application supports one Bluetooth LE connection at a time on the default identity.
Matter commissioning, SMP DFU, and NUS cannot run concurrently.
See :ref:`door_lock_app_ble_smp` for how these services share the Bluetooth LE stack.

.. _matter_aliro_building_and_running_verify:

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

      You should also see Matter stack initialization logs similar to:

      .. code-block:: console

         Init CHIP stack
         [DL]OpenThread started: OK
         ...
         [ZCL]Door Lock server initialized

      With the default in-tree stub, UWB reports as not implemented and the application continues without ranging.

   .. tab:: Release configuration

      UART logs are disabled in release builds.
      Verify LED and button behavior described in :ref:`matter_ui`, then commission and exercise the device using the steps in :ref:`testing`.

Dependencies
************

This application uses the following |REPO_NAME| and |NCS| components:

* Matter application layer and OpenThread
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
