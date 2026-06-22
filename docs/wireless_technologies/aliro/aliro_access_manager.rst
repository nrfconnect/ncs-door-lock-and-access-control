.. _aliro_access_manager:

Aliro Access Manager
####################

.. contents::
   :local:
   :depth: 2

This page describes the application-side access control components in the |REPO_NAME|.
The Aliro Access Manager evaluates authentication results and UWB distance measurements to grant or deny access and drive lock actions.
When the Access Manager decides to unlock or lock, it invokes application callbacks.
In the |REPO_NAME|, those callbacks call the lock simulator.
In a product, you would replace them with your real lock driver.

For a high-level description of how UWB ranging feeds into access decisions, see :ref:`wireless_technologies_uwb`.

The Access Manager interface (:file:`access_manager.h`) provides a unified API for handling access control logic in the application code.
The default implementation (:file:`access_manager`) is designed to cover typical access control scenarios.
It makes access decisions based on the proximity of the Aliro User Device, as measured by UWB ranging, and the stored public keys, and - for NFC sessions - successful tap-to-unlock authentication.
Separate application callbacks signal access granted or denied (for logging) and unlock or lock actions.

.. _aliro_access_manager_kconfig:
.. _addon_architecture_kconfig_default:

Configuring Access Manager
**************************

Access Manager behavior is controlled through Kconfig options set in :file:`prj.conf`.
The following table lists the options you are most likely to adjust for UWB proximity policy, credential storage, and session termination.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM``
     - Specifies the maximum allowed distance, in centimeters, measured by UWB ranging for granting access to the User Device.
       If the measured distance exceeds this value, access is denied.
       If the Aliro User Device is within this distance, access is granted.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_EXIT_MARGIN_CM``
     - * Specifies an additional margin, in centimeters, added to ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` to determine when the door should be locked, referred to as the exit range.
         This margin prevents rapid open and close toggling when the measured distance fluctuates around the maximum allowed distance threshold.

         * Unlock behavior applies when the distance is less than or equal to ``MAX_ALLOWED_DISTANCE_CM``.
         * Lock behavior applies when the distance exceeds ``MAX_ALLOWED_DISTANCE_CM + EXIT_MARGIN_CM``.
         * The exit margin applies per UWB ranging session based on the previous session state.

       Set this option to ``0`` to disable the exit margin, causing the door to lock immediately when the distance exceeds ``MAX_ALLOWED_DISTANCE_CM``.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``
     - Sets the maximum number of public keys that can be stored in the Access Manager.
       This option determines the size of the statically allocated memory for the Access Manager cache.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION``
     - Selects the method used by the Aliro Reader to terminate the UWB ranging session.
       This mechanism is important when the User Device does not terminate the ranging session on its own.

       Available suboptions include:

       * ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_DISABLED``
         Prevents the Access Manager from automatically terminating UWB ranging sessions.
         This is the default behavior.
         Sessions terminate only when explicitly requested by the User Device or when the Bluetooth LE connection is lost.
         This option is useful when the User Device controls the session lifecycle.

       * ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED``
         Terminates the UWB ranging session immediately after access is granted for the session.

       * ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT``
         Terminates the UWB ranging session after a specified timeout.
         The timeout, in milliseconds, is defined by the
         ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_SESSION_TIMEOUT_MS`` Kconfig option.
         By default, the timeout is set to ``10000`` milliseconds.
