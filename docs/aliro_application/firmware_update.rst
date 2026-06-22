.. _aliro_firmware_update:

Firmware update
###############

.. contents::
   :local:
   :depth: 2

The |ALIRO_APP_NAME| supports field firmware update over Bluetooth LE Simple Management Protocol (SMP) (:ref:`aliro_dfu_bluetooth_smp`).

When built with QM35 Ultra-Wideband (UWB) support, the update image can also bundle QM35825 module firmware (:ref:`aliro_qm35_firmware_upgrade`).
In the standalone application, SMP DFU is the only way to upload that QM35 image to the device in the field.
The application stores it in external flash and applies it to the module during initialization.

For build variants and flashing instructions, see :ref:`aliro_access_control_application`.

.. _aliro_dfu_bluetooth_smp:

DFU over Bluetooth LE SMP
*************************

The |ALIRO_APP_NAME| supports field firmware updates over Bluetooth LE using the `SMP protocol`_.

See :ref:`door_lock_app_ble_smp` for transport details and :ref:`door_lock_dfu_smp_service` for the reusable SMP DFU service.

Prerequisites
=============

.. include:: /include/firmware_update_dfu_smp_prerequisites.txt

Enabling Bluetooth LE SMP advertising
=====================================

SMP advertising is not started automatically after boot.
Press Button 1 to toggle it.

.. include:: /include/firmware_update_dfu_smp_shell_advertising.txt

Updating firmware
=================

Set the application version in the :file:`applications/aliro-access-control-app/VERSION` file before building the update image.

.. include:: /include/firmware_update_dfu_smp_updating.txt

.. _aliro_qm35_firmware_upgrade:

QM35825 firmware upgrade
*************************

The application can update QM35825 UWB module firmware in addition to the main application image.
The QM35 image is stored in external flash as an extra MCUboot image and is applied automatically during application initialization when a newer or different version is detected.

.. _aliro_flashing_qm35_using_nrf53_dk:

Initial QM35 module programming
===============================

Before first use of UWB on QM35 hardware, the module coprocessor must run firmware compatible with the host driver in your build.

You can program the module in either of the following ways:

Bundled with the application image
----------------------------------

Build with the ``uwb_qm35_dfu`` snippet together with ``uwb_qm35``, then flash the full image set with ``west flash --erase``.
See :ref:`aliro_qm35_firmware_update_enable` below.

Qorvo flash_app on nRF5340 DK
-----------------------------

To program the module directly before building the door lock application, use the Qorvo ``flash_app`` tool in the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository (see :ref:`aliro_building_and_running_qm35_src`):

#. Connect the QM35825 module to the nRF5340 DK.
#. Navigate to :file:`qm35-aliro-sdk/flash_app` and build:

   .. code-block:: bash

      west build -b nrf5340dk/nrf5340/cpuapp -p

#. Flash the programmer firmware:

   .. code-block:: bash

      west flash

#. Confirm on the serial console that flashing completed successfully and the reported firmware version matches your target.

In-field QM35 update
====================

In the standalone application, SMP DFU is the only way to upload the QM35 firmware image to the device in the field.
Before you start, ensure that:

* The application is built with the ``uwb_qm35_dfu`` sysbuild snippet (which enables the ``uwb_qm35_dfu_app`` application snippet).
* The application is built with the ``dfu_smp`` snippet.
* A QM35 firmware image is included in the build.
* The QM35 module is connected and initialized.

Update flow
-----------

At startup the application:

#. Compares the running QM35 firmware version with the version stored in external flash.
#. Starts an update when the configured version policy matches (higher or different).
#. Resets the QM35 into firmware update mode and transfers the firmware from external flash to the module.
#. Resets the QM35 and re-initializes the UWB stack.
#. Verifies the installation after transfer completes.

.. note::
   During the update, the QM35 is temporarily unavailable.

.. _aliro_qm35_firmware_update_enable:

Enabling QM35 firmware update
-----------------------------

Build with the ``uwb_qm35_dfu`` snippet together with ``uwb_qm35``.
To update both the main application and QM35 over SMP, also include the ``dfu_smp`` snippet.

.. code-block:: bash

   west build -p -b <build_target> applications/aliro-access-control-app -- \
     -DSNIPPET=uwb_qm35_dfu -Daliro-access-control-app_SNIPPET='uwb_qm35;dfu_smp'

The QM35 firmware image is managed automatically during application initialization.
Flash the full image set with ``west flash --erase`` (see :ref:`aliro_access_control_application`).

QM35 DFU configuration options
------------------------------

Control QM35 firmware update behavior using the following Kconfig options:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Configuration option
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU``
     - Enables QM35 firmware upgrade support.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_HIGHER``
     - Updates only if the new version is higher.
       This is the default behavior.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT``
     - Updates if the version differs.

.. note::
   Use the ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT`` option if you need to update from an older Qorvo firmware that reports a higher version number than the new firmware.
   For example, when downgrading from ``13.0.1rc2_10977260000`` to ``0.6.0rc1_12208268663``, the default ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_HIGHER`` option would block the update, since the target version is lower.

By default, the firmware binary is taken from ``${ZEPHYR_QM35_ALIRO_SDK_MODULE_DIR}/firmware/qm35825.bin``.
To use a different binary, specify its absolute path with the ``QM35_IMAGE_PATH`` build argument:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -DQM35_IMAGE_PATH='/path/to/qm35825.bin'

QM35 firmware update monitoring
-------------------------------

Monitor QM35 firmware updates through logs:

.. code-block:: console

   [00:00:02.123,456] <inf> uwb: QM35 FW version comparison: stored=1.2.3 vs running=1.2.2
   [00:00:02.234,567] <inf> uwb: QM35 firmware update started
   [00:00:05.345,678] <inf> uwb: QM35 firmware chunk 1/10 transferred
   [00:00:10.456,789] <inf> uwb: QM35 firmware update completed successfully
   [00:00:11.567,890] <inf> uwb: QM35 FW revision: 1.2.3rc1_[...]

Check the current QM35 firmware version with the shell command:

.. code-block:: console

   uart:~$ uwb qm35_fw_version
