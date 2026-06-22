.. _aliro_lock_sim:
.. _lock_simulator:

Lock simulator
##############

.. contents::
   :local:
   :depth: 2

The lock simulator is a software model of a door lock actuator included in the reference applications for development and testing.
It receives unlock and lock requests from the Access Manager callbacks and mimics real lock behavior: a configurable bolt travel delay, optional timed auto-relock, and LED feedback for secured and unsecured states.
On a production device, replace the simulator with a driver for your motor, solenoid, or other lock hardware.

Kconfig options
===============

Configure the simulator through Kconfig options in :file:`prj.conf` as listed below.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_MOVEMENT_TIME_MS``
     - Bolt travel time in milliseconds (default ``2000`` ms).
       Time for the simulated lock to move between locked and unlocked states.
   * - ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK``
     - Enables automatic relock after unlock (default enabled).
       When enabled, the Access Manager does not trigger lock on UWB exit; the simulator relocks on its timer instead.
   * - ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK_TIME_MS``
     - Duration the lock stays unlocked before auto-relock, in milliseconds (default ``5000`` ms).
       Available when ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK`` is enabled.
   * - ``CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR``
     - Enables visual lock-state feedback on a development kit LED (default enabled).

Indicator hardware
==================

When the indicator is enabled, set the ``lock-sim-indicator`` devicetree alias to the GPIO or LED used for feedback:

.. code-block:: dts

  /{
    aliases {
      lock-sim-indicator = &led2; // green LED2
    };
  };

The LED turns on when the lock is unsecured and off when secured.
It remains on for the simulated movement time plus the auto-relock duration.

Option definitions and defaults are in :file:`subsys/aliro/lock_sim/Kconfig`.
