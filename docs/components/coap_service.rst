.. _door_lock_coap_service:

Door Lock CoAP service
######################

.. contents::
   :local:
   :depth: 2

The Door Lock Constrained Application Protocol (CoAP) service exposes a single CoAP resource over UDP and IPv4 for reading, observing, and changing the door lock state.
It is built on the Zephyr CoAP server.

.. note::
   The CoAP service requires an IP network connection and is supported only on the nRF7120 DK (``nrf7120dk/nrf7120/cpuapp``), which provides Wi-Fi connectivity.

Implementation
**************

The service is implemented as a reusable module in the :file:`subsys/coap_service/` directory.
The public API is declared in the :file:`subsys/coap_service/include/coap_service/coap_service.h` file.

API
***

The ``DoorLock::CoAPService`` namespace provides the following functions:

* ``Init(getStateCb, unlockCb, lockCb)`` registers callbacks for reading and changing the lock state.
  Call once before starting the service.
* ``Start()`` starts the CoAP service.
* ``Stop()`` stops the CoAP service.
* ``Notify(state)`` notifies registered observers of a reader-state change.

Configuration options
*********************

The ``coap_service`` snippet enables the module and its networking dependencies in :file:`snippets/coap_service/coap_service.conf`.
Configure the service using the following Kconfig options:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_COAP_SERVICE``
     - Enables the Door Lock CoAP service.
   * - ``CONFIG_DOOR_LOCK_COAP_SERVICE_PORT``
     - UDP port of the service (default ``5683``, port ``0`` is not supported).
   * - ``CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH``
     - Single URI path segment of the resource (default ``door-lock``, no slashes, at most 12 bytes unless ``CONFIG_COAP_EXTENDED_OPTIONS_LEN`` is enabled).
   * - ``CONFIG_DOOR_LOCK_COAP_SERVICE_LOG_LEVEL``
     - Log level for the module.

Option definitions and defaults are in :file:`subsys/coap_service/Kconfig`.

Door lock resource
******************

By default, the service exposes the ``coap://<ip>:5683/door-lock`` resource.
Clients interact with this resource using the following methods:

* ``GET`` returns the lock state as plain text (for example ``secured`` or ``unsecured``).
  The resource supports CoAP Observe, and observers are notified on every state change.
* ``PUT`` with the ``application/cbor`` content format and a single Concise Binary Object Representation (CBOR) boolean payload changes the state.
  ``true`` (``0xF5``) unlocks and ``false`` (``0xF4``) locks.

.. note::
   The service uses plain UDP without authentication or encryption.
   Any host that can reach the device can read and change the lock state.
   Use it only on a trusted test network.
