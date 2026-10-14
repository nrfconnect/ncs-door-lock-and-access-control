:orphan:

.. _known_issues_and_limitations:

Known issues and limitations
############################

.. contents::
   :local:
   :depth: 2


This page describes the current limitations of the application and documents known issues that may affect functionality or platform support.

Current limitations
*******************

The following optional Aliro features are out-of-scope for the |APP_NAME|:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Out-of-scope Aliro feature
     - Supported instead
   * - Mailbox
     - --
   * - Access Document time verification in the |ALIRO_APP_NAME|
     - Time verification in the |MATTER_ALIRO_APP_NAME|, which obtains the current time from the Matter Time Synchronization cluster.
       See :ref:`aliro_step_up_time_verification`.
   * - Revocation Document
     - --
   * - Sending the Reader certificate in the AUTH1 command
     - Sending the Reader certificate through the LOAD_CERT command
   * - Extended length APDU
     - APDU Chaining
   * - ``key_slot`` value in the AUTH1 command ``command_parameter`` field
     - Access Credential Public Key
   * - Aliro schedules
     - Matter schedules
   * - Pass Through Message ID in the Bluetooth LE Aliro Message
     - --
   * - Expedited-Fast Phase authentication based on Credential Issuer CA Public Key
     - --
   * - When the application is built with Matter support, only one Aliro Bluetooth LE session is supported at a time
     - --

Known issues
************

Known issues are tagged with the releases they affect.
Use the release filter to view issues for a specific release.

A known issue can list one or both of the following entries:

* **Affected platforms:**

  If a known issue does not have any specific platforms listed, it is valid for all hardware platforms.

* **Workaround:**

  Some known issues have a workaround.
  Sometimes, they are discovered later and added over time.

.. known-issues-filter::
   :default: 1.2.1

.. rst-class:: v1-2-1 v1-2-0 v1-1-1 v1-1-0 v1-0-1 v1-0-0

AL-717: Matter onboarding with NFC is not supported in the |MATTER_ALIRO_APP_NAME|
   Matter onboarding using NFC does not work in the |MATTER_ALIRO_APP_NAME|.

   **Workaround:**
   Use the QR code scanning mechanism for Matter device commissioning.

.. rst-class:: v1-2-1 v1-2-0 v1-1-1 v1-1-0 v1-0-1 v1-0-0

AL-718: Access Document is not stored on the nRF52840 platform
   Access Document (AD) is not stored persistently on the nRF52840 platform when using the ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` Kconfig build option.
   As a result, Kpersistent is also not stored persistently on the nRF52840 device when the ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig option is enabled together with ``CONFIG_DOOR_LOCK_STEP_UP_PHASE``.

   **Workaround:**
   Implement a custom persistent storage mechanism for the Access Document on the nRF52840 platform.

   **Affected platforms:** nRF52840 DK

.. rst-class:: v1-2-0 v1-1-1 v1-1-0 v1-0-1 v1-0-0

AL-728: When built with Matter support, the ProductAppearance attribute is not supported
   The ProductAppearance attribute from the BasicInformation cluster is not supported when the application is built with Matter support.
   As a result, the ProductAppearance attribute cannot be read by the Matter controller.

   **Workaround:**
   Add the ProductAppearance attribute to the ZAP configuration file and regenerate the Matter data model.

.. rst-class:: v1-2-0 v1-1-1 v1-1-0

AL-856: Lock does not unlock via NFC after manual lock while the User Device remains in UWB range
   After UWB unlock and manual lock while the User Device remains in UWB range, NFC authentication may complete (ACCESS GRANTED) without actuating the lock.
   On the device, Aliro NFC authentication completes successfully (``ACCESS GRANTED``), but no physical unlock is triggered.

.. rst-class:: v1-2-1 v1-2-0

AL-931: Access Manager is not aware of the final unlock decision
   In the |MATTER_ALIRO_APP_NAME|, the Aliro Access Manager makes its access decision based only on the Access Credential public key, before the Matter user data is evaluated.
   When the public key belongs to a Matter user whose UserStatus is ``OccupiedDisabled``, the Access Manager still grants access over Aliro.
   The Matter validation runs afterwards, denies the credential with the ``DisabledUserDenied`` error, and the bolt is not actuated, so the door remains locked and no lock operation event is generated.

.. rst-class:: v1-1-1 v1-1-0 v1-0-1 v1-0-0

QM35825 UWB SoC integration is deprecated
   The Qorvo QM35825 UWB SoC integration has been deprecated and is no longer supported.

.. rst-class:: v1-1-1 v1-1-0

AL-851: Starting the UWB ranging session does not work reliably when testing with Apple ecosystem
   When testing |MATTER_ALIRO_APP_NAME| with Apple ecosystem, sometimes the UWB ranging session does not start after the lock is commissioned.

   **Workaround:**
   Reboot the iPhone and try again.

.. rst-class:: v1-1-0

AL-855: Aliro unlock does not include user identity in Apple Home notification
   When the lock running the |MATTER_ALIRO_APP_NAME| is unlocked via Aliro (NFC or UWB), no user identity is included in the Apple Home App notification.
   When the same lock is unlocked via Matter (for example, from the Apple Home app), the notification correctly includes the user who performed the unlock.

.. rst-class:: v1-0-1 v1-0-0

AL-727: Credentials created during Step-up phase are not forwarded to Matter
   User credentials provisioned in the Step-up phase (with Access Document) are not exported to Matter.
   As a result, they are not synchronized from the Matter controller's perspective.
