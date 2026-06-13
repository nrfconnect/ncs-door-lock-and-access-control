.. _aliro_firmware_update:

Firmware update
###############

.. contents::
   :local:
   :depth: 2

This page will guide you through the available firmware update mechanisms for the |ALIRO_APP_NAME|.

.. _aliro_dfu_bluetooth_smp:

DFU over Bluetooth LE SMP
*************************

The |ALIRO_APP_NAME| supports firmware updates through Bluetooth® LE using the Simple Management Protocol (SMP).
This allows for field updates of deployed devices without requiring physical access.

Prerequisites
=============

Before starting the DFU update process, ensure that:

* You have built the new application with a higher version number than the one that is currently running on the device to be updated.
* You have the update image file (``app_update.bin``) generated in the build directory.
* The device is built with DFU support enabled (``CONFIG_DOOR_LOCK_DFU_BLE_SMP=y``).
* You have the necessary DFU tools installed (nRF Connect for Mobile or mcumgr CLI).

Enabling Bluetooth LE SMP advertising
=====================================

The application needs to advertise the SMP DFU service for clients to connect:

1. **Build with DFU configuration**:

   .. code-block:: bash

      west build -p -b build_target applications/aliro-access-control-app -- -DCONFIG_DOOR_LOCK_DFU_BLE_SMP=y

2. **Enable DFU advertising** (if not automatically enabled):

   .. code-block:: console

      uart:~$ dfu adv start

3. **Verify DFU service** is advertising by checking nearby Bluetooth devices.

Updating firmware
=================

Using nRF Connect for Mobile
-----------------------------

1. **Install nRF Connect for Mobile** on your smartphone or tablet.

2. **Scan for devices** and connect to your door lock device.

3. **Select the DFU service** in the service list.

4. **Upload the firmware file** (``app_update.bin``).

5. **Monitor the progress** and wait for completion.

6. **Device will automatically reboot** with the new firmware.

Using mcumgr CLI
----------------

For instructions on using the mcumgr CLI tool for DFU updates, refer to the `MCU Bluetooth LE programming`_.

.. _aliro_qm35_firmware_upgrade:

QM35825 firmware upgrade
*************************

The application also supports firmware updates for the QM35825 UWB module. This allows updating both the main application firmware and the QM35 UWB module firmware.

Prerequisites
=============

Before updating QM35 firmware:

* The application is built with QM35 DFU support (``uwb_qm35_dfu_app`` snippet).
* QM35 firmware image is included in the build.
* The QM35 module is properly connected and initialized.

QM35 firmware update process
=============================

The QM35 firmware update process includes:

1. **Version comparison** - Compare stored firmware with running QM35 firmware
2. **Automatic update** - Update if version differs or is higher (configurable)  
3. **Verification** - Verify successful firmware installation
4. **Fallback** - Retry firmware update if QM35 initialization fails

Enabling QM35 firmware update
==============================

To enable QM35 firmware update support:

1. **Build with QM35 DFU support**:

   .. code-block:: bash

      west build -p -b build_target applications/aliro-access-control-app -- -DCONFIG_DOOR_LOCK_BLE_UWB=y -S uwb_qm35_dfu_app

2. **The QM35 firmware** is automatically managed during application initialization.

QM35 DFU configuration options
===============================

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

By default, the firmware binary is taken from ``${ZEPHYR_NRFCONNECT_SDK_QORVO_MODULE_DIR}/firmware/qm35825.bin``.
To use a different binary, specify its absolute path with the ``QM35_IMAGE_PATH`` build argument:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -DQM35_IMAGE_PATH='/path/to/qm35825.bin'

QM35 firmware update monitoring
================================

Monitor QM35 firmware updates through logs:

.. code-block:: console

   [00:00:02.123,456] <inf> uwb: QM35 FW version comparison: stored=1.2.3 vs running=1.2.2
   [00:00:02.234,567] <inf> uwb: QM35 firmware update started
   [00:00:05.345,678] <inf> uwb: QM35 firmware chunk 1/10 transferred
   [00:00:10.456,789] <inf> uwb: QM35 firmware update completed successfully
   [00:00:11.567,890] <inf> uwb: QM35 FW revision: 1.2.3rc1_[...]

Check current QM35 firmware version:

.. code-block:: console

   uart:~$ uwb qm35_fw_version