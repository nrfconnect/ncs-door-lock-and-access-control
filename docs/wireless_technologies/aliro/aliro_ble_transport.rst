.. _aliro_ble_transport:
.. _door_lock_app_arch_bluetooth_le:

Bluetooth LE transport
######################

.. contents::
   :local:
   :depth: 2

In the Aliro Bluetooth LE with Ultra-Wideband (UWB) transport, Bluetooth LE carries the Access Protocol and the UWB ranging session setup (the M1–M4 message exchange).
UWB then performs secure ranging. 
See :ref:`wireless_technologies_uwb` for how distance measurements drive unlock and lock actions.

.. note::
   This reference does not support the Aliro Bluetooth LE-only transport variant.
   Aliro over Bluetooth LE with UWB is supported on the `nRF5340 DK`_ and `nRF54LM20 DK`_ with an external UWB module.

Bluetooth identity and advertising
**********************************

Aliro uses a dedicated Bluetooth local identity (``CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_BT_ID``) for advertising and connections.
This keeps Aliro sessions separate from other Bluetooth LE services on the default identity, such as Matter, `Nordic UART Service (NUS)`_ or Device Firmware Update over Simple Management Protocol (DFU SMP).

The advertising payload follows the Aliro specification and is built from the reader group identifier and reader group sub-identifier.
The payload is updated automatically when either value changes.

.. note::
   To override values from the console, use the ``reader group_id`` and ``reader group_sub_id`` subcommands.
   See :ref:`aliro_testing_cli_ref` for command syntax.

Kconfig options
****************

When Aliro over Bluetooth LE and UWB runs alongside other Bluetooth LE services, the stack must support multiple simultaneous connections across both identities.

Each Aliro session uses one connection on the Aliro identity.
Services such as NUS or SMP DFU may use additional connections on the default identity.
Set ``CONFIG_BT_MAX_CONN`` to cover both: at least the configured Aliro session count plus any connections required on the default identity.

The Aliro session limits are ``CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_MAX_SESSIONS`` and ``CONFIG_DOOR_LOCK_ALIRO_L2CAP_SERVER_MAX_SESSIONS``.
Both default to ``CONFIG_BT_MAX_CONN`` and are defined in the :file:`subsys/aliro/aliro_service/Kconfig` and :file:`subsys/aliro/l2cap_server/Kconfig` files.
The build verifies that ``CONFIG_BT_MAX_CONN`` is greater than or equal to both limits.

For example, with ``CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_MAX_SESSIONS`` set to ``2`` and NUS enabled for one shell connection, set ``CONFIG_BT_MAX_CONN`` to at least ``3``.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_BT_MAX_CONN``
     - Maximum simultaneous Bluetooth LE connections across all identities.
   * - ``CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_MAX_SESSIONS``
     - Maximum concurrent Aliro Bluetooth LE sessions handled by the Aliro service.
   * - ``CONFIG_DOOR_LOCK_ALIRO_L2CAP_SERVER_MAX_SESSIONS``
     - Maximum concurrent Aliro L2CAP server sessions.

You can configure other Bluetooth LE parameters — buffer sizes, MTU, GATT database, L2CAP channels, TX power, and PHY — in the application-specific :file:`Kconfig.ble.defconfig`.
