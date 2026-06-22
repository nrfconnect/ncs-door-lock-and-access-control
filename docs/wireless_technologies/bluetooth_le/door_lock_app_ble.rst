.. _addon_architecture_ble:
.. _wireless_technologies_ble:

Bluetooth LE
############

.. contents::
   :local:
   :depth: 2

In addition to its role as a transport for Aliro and as a commissioning channel for Matter, Bluetooth LE is also used in the |APP_NAME| to provide standalone services that are independent of both Aliro and Matter.
These services allow remote control and maintenance of the lock without requiring a connection to a Matter ecosystem or an Aliro User Device.

The following general-purpose Bluetooth LE services are available:

* :ref:`door_lock_app_ble_nus` — Remote lock and unlock over the Nordic UART Service (NUS), a UART-style GATT command channel.
* :ref:`door_lock_app_ble_smp` — Wireless firmware updates using the Simple Management Protocol (SMP) over Bluetooth LE.

.. toctree::
   :maxdepth: 2
   :caption: Subpages:

   door_lock_app_ble_nus.rst
   door_lock_app_ble_smp.rst
