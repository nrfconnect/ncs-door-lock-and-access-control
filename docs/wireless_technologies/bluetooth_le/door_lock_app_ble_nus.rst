.. _door_lock_app_ble_nus:

Nordic UART Service (NUS)
#########################

.. contents::
   :local:
   :depth: 2

The `Nordic UART Service (NUS)`_ is a Bluetooth LE GATT service that exposes RX and TX characteristics, acting as a UART-style command channel over Bluetooth LE.
In the |APPS_NAME|, the Door Lock NUS wrapper receives line-based commands from a paired peer and dispatches them to registered callbacks.
The reference applications register ``Lock`` and ``Unlock`` commands for proprietary remote lock control, independent of Aliro and Matter.

Role in the add-on
******************

NUS is a parallel control path on the default Bluetooth identity, alongside other general-purpose Bluetooth LE services such as SMP DFU.
Aliro Bluetooth LE sessions use a separate identity; see :ref:`aliro_ble_transport` for how the two coexist and how to size ``CONFIG_BT_MAX_CONN``.

Module details
**************

See :ref:`door_lock_nus_service` for enabling, Kconfig options, and security details.

Testing
*******

For step-by-step testing, including pairing and sending ``Lock`` and ``Unlock`` commands, see :ref:`aliro_testing_ble_nordic_uart` (|ALIRO_APP_NAME|) and :ref:`aliro_matter_testing_ble_nordic_uart` (|MATTER_ALIRO_APP_NAME|).
