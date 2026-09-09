.. _aliro_firmware_update:

Firmware update
###############

.. contents::
   :local:
   :depth: 2

The |ALIRO_APP_NAME| supports field firmware update over Bluetooth LE Simple Management Protocol (SMP) (:ref:`aliro_dfu_bluetooth_smp`).
SMP DFU is enabled by default.
Set ``CONFIG_DOOR_LOCK_DFU_SMP_SERVICE=n`` to disable it.

For build variants and flashing instructions, see :ref:`aliro_access_control_application`.

.. _aliro_dfu_bluetooth_smp:

DFU over Bluetooth LE SMP
*************************

The |ALIRO_APP_NAME| supports field firmware updates over Bluetooth LE using the `SMP protocol`_.

See :ref:`door_lock_app_ble_smp` for transport details and :ref:`door_lock_dfu_smp_service` for the reusable SMP DFU service.

Prerequisites
=============

Before starting the DFU update process, ensure that:

* SMP DFU is enabled in your build (``CONFIG_DOOR_LOCK_DFU_SMP_SERVICE=y``, the default).

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
