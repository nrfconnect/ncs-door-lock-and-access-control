.. _door_lock_nus_service:
.. _nus_service:

NUS service
###########

.. contents::
   :local:
   :depth: 2

The Door Lock Nordic UART Service (NUS) wrapper is built on the |NCS| `Nordic UART Service (NUS)`_ GATT service.
It receives commands over Bluetooth LE and dispatches them to registered callbacks, providing a UART-style command channel for proprietary remote lock control.

For transport context and how NUS fits alongside other Bluetooth LE services, see :ref:`door_lock_app_ble_nus`.
For step-by-step pairing and command testing, see :ref:`aliro_testing_ble_nordic_uart` (|ALIRO_APP_NAME|) and :ref:`aliro_matter_testing_ble_nordic_uart` (|MATTER_ALIRO_APP_NAME|).

Source
======

The module is located in the :file:`subsys/nus_service/` directory.
The public API is declared in the :file:`subsys/nus_service/include/nus_service/nus_service.h` file.

Security
========

The reference applications start NUS advertising during application initialization and register ``Lock`` and ``Unlock`` commands for remote bolt control.
This path is independent of Aliro authentication and Matter network membership.

The service uses the default Bluetooth identity.
NUS cannot run concurrently with Matter commissioning or SMP DFU on that identity.
See :ref:`door_lock_app_ble_nus` for connection limits and the ``CONFIG_BT_MAX_CONN`` Kconfig option for further guidance.

Pairing is required.
Incoming commands are processed only when the connection security level is L2 or higher.

The module uses the advertising arbiter when multiple Bluetooth LE services share the default identity.

When the lock state changes, the reference applications can send short status strings (for example ``locked`` or ``unlocked``) back to the connected NUS client.

Kconfig options
===============

Configure the NUS service through Kconfig options in :file:`prj.conf` as listed below.
The ``bt_nus`` snippet sets the wrapper options and the Bluetooth LE manager authentication options in :file:`snippets/bt_nus/bt_nus.conf`.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_NUS_SERVICE``
     - Enables the Door Lock NUS service wrapper.
   * - ``CONFIG_DOOR_LOCK_NUS_SERVICE_MAX_COMMANDS``
     - Maximum number of registered commands (default ``2``).
   * - ``CONFIG_DOOR_LOCK_NUS_SERVICE_MAX_COMMAND_LEN``
     - Maximum length of a single command token in bytes (default ``10``).
   * - ``CONFIG_DOOR_LOCK_NUS_SERVICE_LOG_LEVEL``
     - Log level for the module.

Option definitions and defaults are in :file:`subsys/nus_service/Kconfig`.

.. _door_lock_nus_service_usage:

Usage
=====

Enable the module by building the application with the ``bt_nus`` snippet.

Pass the snippet to ``west build`` using the application-specific symbol:

.. code-block:: console

   west build -p -b <build_target> applications/aliro-access-control-app -- \
       -Daliro-access-control-app_SNIPPET=bt_nus

   west build -p -b <build_target> applications/matter-aliro-door-lock-app -- \
       -Dmatter-aliro-door-lock-app_SNIPPET=bt_nus

To combine with other snippets, separate their names with semicolons, for example:

.. code-block:: console

   -Daliro-access-control-app_SNIPPET='uwb_qm35;bt_nus'

After flashing, pair with the device and send commands using the steps in :ref:`aliro_testing_ble_nordic_uart` or :ref:`aliro_matter_testing_ble_nordic_uart`.
