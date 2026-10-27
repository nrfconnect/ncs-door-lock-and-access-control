.. _door_lock_coap_service:
.. _coap_service:

CoAP service
############

.. contents::
   :local:
   :depth: 2

The Door Lock CoAP service exposes a single CoAP resource over IPv4 UDP for reading, observing, and changing the door lock state.
It is built on the Zephyr CoAP server.

.. note::
   The CoAP service requires an IP network connection and is supported only on the nRF7120 DK (``nrf7120dk/nrf7120/cpuapp``), which provides Wi-Fi connectivity.

Source
======

The module is located in the :file:`subsys/coap_service/` directory.
The public API is declared in the :file:`subsys/coap_service/include/coap_service/coap_service.h` file.
A minimal demo is in the :file:`samples/coap_service/` directory.

Resource
========

The service exposes the ``coap://<ip>:5683/door-lock`` resource by default:

* ``GET`` returns the lock state as plain text (for example ``secured`` or ``unsecured``).
  The resource supports CoAP Observe, and observers are notified on every state change.
* ``PUT`` with the ``application/cbor`` content format and a single CBOR boolean payload changes the state.
  ``true`` (``0xF5``) unlocks and ``false`` (``0xF4``) locks.

Security
========

The service uses plain UDP without authentication or encryption.
Any host that can reach the device can read and change the lock state.
Use it only on a trusted test network.

Kconfig options
===============

The ``coap_service`` snippet enables the module and its networking dependencies in :file:`snippets/coap_service/coap_service.conf`.

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

.. _door_lock_coap_service_usage:

Usage
=====

Build the application for the nRF7120 DK with the ``coap_service`` snippet:

.. code-block:: console

   west build -p -b nrf7120dk/nrf7120/cpuapp applications/aliro-access-control-app -- \
       -Daliro-access-control-app_SNIPPET=coap_service

After flashing, connect the device to Wi-Fi and send requests from a host on the same network.
The :file:`scripts/coap_test.py` script is a dependency-free CoAP client that requires only Python 3:

.. code-block:: console

   python3 scripts/coap_test.py <ip> get
   python3 scripts/coap_test.py <ip> unlock
   python3 scripts/coap_test.py <ip> lock
   python3 scripts/coap_test.py <ip> observe

The ``observe`` command prints every notification until you press :kbd:`Ctrl+C`, or until ``--count`` messages are received.

If you changed ``CONFIG_DOOR_LOCK_COAP_SERVICE_PORT`` or ``CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH``, pass the ``--port`` and ``--path`` options.
Run ``python3 scripts/coap_test.py --help`` for all options.
