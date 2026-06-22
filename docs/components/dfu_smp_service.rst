.. _door_lock_dfu_smp_service:
.. _dfu_smp_service:

DFU SMP service
###############

.. contents::
   :local:
   :depth: 2

The Door Lock DFU Simple Management Protocol (SMP) service is a wrapper around the |NCS| `GATT DFU SMP Service`_.
It controls on-demand SMP advertising so a client can connect over Bluetooth LE and upload a firmware image.
For details on the `SMP protocol`_, see the |NCS| documentation.

For transport context and how SMP DFU fits alongside other Bluetooth LE services, see :ref:`door_lock_app_ble_smp`.
For step-by-step update procedures, see :ref:`aliro_dfu_bluetooth_smp` (|ALIRO_APP_NAME|) and :ref:`dfu_ble_smp` (|MATTER_ALIRO_APP_NAME|).

Source
======

The module is located in the :file:`subsys/dfu_smp_service/` directory.
The public API is declared in the :file:`subsys/dfu_smp_service/include/dfu_smp_service/dfu_smp_service.h` file.

Shell and development kit controls
==================================

When shell commands are enabled (``CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_SHELL=y``, default when the shell is available), use the ``dfu_smp on`` command to start SMP advertising and the ``dfu_smp off`` command to stop it.

.. code-block:: console

   uart:~$ dfu_smp on
   uart:~$ dfu_smp off

In reference applications, a development kit button can also toggle SMP advertising when the ``dfu_smp`` snippet is enabled.
See the User interface section in :ref:`aliro_access_control_application` (**Button 1**) or :ref:`matter_ui` (**Button 3**).

In Matter release builds, the UART shell is disabled by default, so use the button or re-enable the shell if you rely on ``dfu_smp`` commands.

Kconfig options
===============

Configure the DFU SMP service through Kconfig options in :file:`prj.conf` as listed below.
The ``dfu_smp`` snippet sets the wrapper options and the MCUmgr and Bluetooth LE transport options in :file:`snippets/dfu_smp/dfu_smp.conf`.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_DFU_SMP_SERVICE``
     - Enables the Door Lock DFU SMP service wrapper.
       Requires MCUboot (``CONFIG_BOOTLOADER_MCUBOOT``).
   * - ``CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_SHELL``
     - Enables shell commands to start and stop SMP advertising (default ``y`` when ``SHELL`` is enabled).
   * - ``CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_LOG_LEVEL``
     - Log level for the module.

Option definitions and defaults are in :file:`subsys/dfu_smp_service/Kconfig`.

.. _door_lock_dfu_smp_service_usage:

Usage
=====

Enable the module by building the application with the ``dfu_smp`` snippet.

Pass the snippet to ``west build`` using the application-specific symbol:

.. code-block:: console

   west build -p -b <build_target> applications/aliro-access-control-app -- \
       -Daliro-access-control-app_SNIPPET=dfu_smp

   west build -p -b <build_target> applications/matter-aliro-door-lock-app -- \
       -Dmatter-aliro-door-lock-app_SNIPPET=dfu_smp

To combine with other snippets, separate their names with semicolons, for example:

.. code-block:: console

   -Daliro-access-control-app_SNIPPET='uwb_qm35;dfu_smp'

After flashing, enable SMP advertising, then follow the update steps in :ref:`aliro_firmware_update` or :ref:`firmware_update`.
