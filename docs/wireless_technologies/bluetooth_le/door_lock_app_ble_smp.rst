.. _door_lock_app_ble_smp:

Device Firmware Update over SMP
###############################

.. contents::
   :local:
   :depth: 2

Device Firmware Update (DFU) over SMP provides wireless firmware updates using the `Simple Management Protocol <SMP protocol_>`_ over a Bluetooth LE GATT connection.
In the |APPS_NAME|, the Door Lock DFU SMP wrapper controls on-demand SMP advertising so a client can connect and upload a firmware image over Bluetooth LE without a wired connection.
Advertising is not started automatically at boot; enable it with a development kit button or the ``dfu_smp on`` shell command before a client connects.
SMP DFU works independently of Matter network connectivity, which is useful when a device is uncommissioned or the network is unavailable.
In Matter builds, the module also coordinates SMP DFU with Matter OTA; see :ref:`door_lock_dfu_smp_service`.

Role in the add-on
******************

SMP DFU is a parallel maintenance path on the default Bluetooth identity, alongside other general-purpose Bluetooth LE services such as NUS.
Aliro Bluetooth LE sessions use a separate identity; see :ref:`aliro_ble_transport` for how the two coexist and how to size ``CONFIG_BT_MAX_CONN``.

Module details
**************

See :ref:`door_lock_dfu_smp_service` for enabling, Kconfig options, and shell commands.
For the underlying GATT service definition, refer to `GATT DFU SMP Service`_ in the |NCS| documentation.

Testing
*******

For step-by-step update procedures, including enabling SMP advertising and uploading an image with an SMP client, see :ref:`aliro_dfu_bluetooth_smp` (|ALIRO_APP_NAME|) and :ref:`dfu_ble_smp` (|MATTER_ALIRO_APP_NAME|).
