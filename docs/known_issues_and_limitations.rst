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
   * - Reader Descriptor
     - --
   * - Mailbox
     - --
   * - Time concept
     - --
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
   * - Pass Through Message ID in the BLE Aliro Message
     - --
   * - Expedited-Fast Phase authentication based on Credential Issuer CA Public Key
     - --

Known issues
************

Known issues listed on this page are valid for the current state of development.
Refer to the following sections for more information on specific items.

A known issue can list one or both of the following entries:

* **Affected platforms:**

  If a known issue does not have any specific platforms listed, it is valid for all hardware platforms.

* **Workaround:**

  Some known issues have a workaround.
  Sometimes, they are discovered later and added over time.

The |APP_NAME| v1.0.0
*********************

.. toggle::

  AL-717: Matter onboarding with NFC is not supported
    Matter onboarding using NFC does not work.

    **Workaround:**
    Use QR code scanning mechanism for Matter device commissioning.

  AL-718: Access Document is not stored on the nRF52840 platform
    Access Document (AD) is not stored persistently on the nRF52840 platform when using the  ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` Kconfig build option.
    As a result, Kpersistent is also not stored persistently on the nRF52840 device when the ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig option is enabled together with ``CONFIG_DOOR_LOCK_STEP_UP_PHASE``.

    **Workaround:**
    Implement a custom persistent storage mechanism for the Access Document on the nRF52840 platform.

    **Affected platforms:** nRF52840 DK

The |APP_NAME| v0.6.0
*********************

.. toggle::

  The application crashes when using UWB and NFC on the same SPI bus has been resolved.
    The `QM35825`_ UWB module and the X-NUCLEO NFC reader board can now be used together on the same SPI bus.
    Use the ``uwb_qm35`` snippet when building the application to configure both NFC and UWB modules to share the same SPI bus.
    The snippet configures both devices to use different chip select (CS) pins on the same SPI bus.

The |APP_NAME| v0.5.0
*********************

.. toggle::

  AL-493: Expedited-fast phase does not work after the Step-up phase
    When the Step-up phase is completed, the Kpersistent key is not preserved and the Reader does not proceed to the Expedited-fast phase.

  AL-377: Error messages visible in the Reader's serial console during ranging session
    When the Reader is close to the Test Harness during a ranging session, the following error messages may appear in the Reader's serial console:

    .. code-block::

        uwb: Controlee report error status: 0x22 on slot id: 0
        uwb: Controlee report error status: 0x21 on slot id: 0

  The application crashes when the UWB module QM35825, used with Qorvo Arduino Interface Board, is connected to the nRF5340 DK and the X-NUCLEO-NFC09A1 board.
    This issue arises because the QM35825 module forces the SPI MISO line to remain low at all times, which prevents the X-NUCLEO-NFC09A1 board from initializing properly.

    **Workaround 1:** Use a different SPI bus to connect the X-NUCLEO-NFC09A1 board.

    **Workaround 2:** For test purposes, use the ``uwb_qm35`` snippet to disable the NFC transport protocol.
    This way, the X-NUCLEO-NFC09A1 board initialization is not required.

  AL-333: The NFC transport lost error occurs in test cases for granting access to the door lock
    When executing test cases that determine if access to the door lock is granted or denied, the ``NFC transport lost`` error occurs.
    The expected outcome is ``ACCESS GRANTED`` or the ``Provided User Device public key not found in Access Manager database`` log in the serial output of the DUT.

  AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
    Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
    The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach X-NUCLEO-NFC09A1 shield to the nRF54L15 DK, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and re-flash the nRF54L15 DK.

  AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
    During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.

The |APP_NAME| v0.4.0
*********************

.. toggle::
    
  AL-377: Error messages visible in the Reader's serial console during ranging session.
    When the Reader is close to the Test Harness during a ranging session, the following error messages may appear in the Reader's serial console:

    .. code-block::

        uwb: Controlee report error status: 0x22 on slot id: 0
        uwb: Controlee report error status: 0x21 on slot id: 0

  The application crashes when the UWB module QM35825, used with Qorvo Arduino Interface Board, is connected to the nRF5340 DK and the X-NUCLEO-NFC09A1 board.
    This issue arises because the QM35825 module forces the SPI MISO line to remain low at all times, which prevents the X-NUCLEO-NFC09A1 board from initializing properly.

    **Workaround 1:** Use a different SPI bus to connect the X-NUCLEO-NFC09A1 board.

    **Workaround 2:** For test purposes, use the ``uwb_qm35`` snippet to disable the NFC transport protocol.
    This way, the X-NUCLEO-NFC09A1 board initialization is not required.

  AL-333: The NFC transport lost error occurrs in test cases for granting access to the door lock
    When executing test cases that determine if access to the door lock is granted or denied, the ``NFC transport lost`` error occurrs.
    The expected outcome is ``ACCESS GRANTED`` or the ``Provided User Device public key not found in Access Manager database`` log in the serial output of the DUT.

  AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
    Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
    The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach X-NUCLEO-NFC09A1 shield to the nRF54L15 DK, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and re-flash the nRF54L15 DK.

  AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
    During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.

The |APP_NAME| v0.3.1
*********************

.. toggle::

  AL-377: Error messages visible in the Reader's serial console during ranging session.
    When the Reader is close to the Test Harness during a ranging session, the following error messages may appear in the Reader's serial console:

    .. code-block::

        uwb: Controlee report error status: 0x22 on slot id: 0
        uwb: Controlee report error status: 0x21 on slot id: 0

  The application crashes when the UWB module QM35825, used with Qorvo Arduino Interface Board, is connected to the nRF5340 DK and the X-NUCLEO-NFC09A1 board.
    This issue arises because the QM35825 module forces the SPI MISO line to remain low at all times, which prevents the X-NUCLEO-NFC09A1 board from initializing properly.

    **Workaround 1:** Use a different SPI bus to connect the X-NUCLEO-NFC09A1 board.

    **Workaround 2:** For test purposes, use the ``uwb_qm35`` snippet to disable the NFC transport protocol.
    This way, the X-NUCLEO-NFC09A1 board initialization is not required.

  AL-333: The NFC transport lost error occurrs in test cases for granting access to the door lock
    When executing test cases that determine if access to the door lock is granted or denied, the ``NFC transport lost`` error occurrs.
    The expected outcome is ``ACCESS GRANTED`` or the ``Provided User Device public key not found in Access Manager database`` log in the serial output of the DUT.

  AL-335: Watchdog expires for an NFC session upon test execution
    When executing the RD-NFC-STDTXN-2.0 test, the ``Watchdog expired for NFC session`` event occasionally occurs.
    This issue is caused by missing AUTH1 response from the test harness, despite logs indicating that the AUTH1 response was sent.

  AL-239: Occasional timeout occurs in the Reader when executing the RD-NFC-STDTXN-2.0 test from the test harness
    When executing the RD-NFC-STDTXN-2.0 test case, the Reader might report an RX timeout error code in the serial console.
    For more details, see `Test Harness issue #191`_.

  AL-282: An undefined access decision occurs when executing the RD-NFC-STDTXN-2.0 test harness case in a loop
    When executing the RD-NFC-STDTXN-2.0 test case multiple times in a row (with valid credentials provisioned), the Reader might not report the access decision.
    As a result, the ``ACCESS GRANTED`` or ``ACCESS DENIED`` logs are not displayed in the serial output of the Reader under test.

  AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
    Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
    The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach X-NUCLEO-NFC09A1 shield to the nRF54L15 DK, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and re-flash the nRF54L15 DK.

  AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
    During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.


The |APP_NAME| v0.3.0
*********************

.. toggle::

  The application crashes when the UWB module QM35825, used with Qorvo Arduino Interface Board, is connected to the nRF5340 DK and the X-NUCLEO-NFC09A1 board.
    This issue arises because the QM35825 module forces the SPI MISO line to remain low at all times, which prevents the X-NUCLEO-NFC09A1 board from initializing properly.

    **Workaround 1:** Use a different SPI bus to connect the X-NUCLEO-NFC09A1 board.

    **Workaround 2:** For test purposes, use the ``uwb_qm35`` snippet to disable the NFC transport protocol.
    This way, the X-NUCLEO-NFC09A1 board initialization is not required.

  The QM35825 ranging measurement is not working with the test harness.
    Currently, there is no workaround for this issue.

  The current implementation of the Access Manager does not support multiple Aliro sessions.
    Even though, the Aliro stack supports multiple sessions, the Access Manager implementation is limited to one session at a time.

  AL-333: The NFC transport lost error occurrs in test cases for granting access to the door lock
    When executing test cases that determine if access to the door lock is granted or denied, the ``NFC transport lost`` error occurrs.
    The expected outcome is ``ACCESS GRANTED`` or the ``Invalid Signature`` log in the serial output of the DUT.

  AL-335: Watchdog expires for an NFC session upon test execution
    When executing the RD-NFC-STDTXN-2.0 test, the ``Watchdog expired for NFC session`` event occasionally occurs.
    This issue is caused by missing AUTH1 response from the test harness, despite logs indicating that the AUTH1 response was sent.

  AL-239: Occasional timeout occurs in the Reader when executing the RD-NFC-STDTXN-2.0 test from the test harness
    When executing the RD-NFC-STDTXN-2.0 test case, the Reader might report an RX timeout error code in the serial console.
    For more details, see `Test Harness issue #191`_.

  AL-282: An undefined access decision occurs when executing the RD-NFC-STDTXN-2.0 test harness case in a loop
    When executing the RD-NFC-STDTXN-2.0 test case multiple times in a row (with valid credentials provisioned), the Reader might not report the access decision.
    As a result, the ``ACCESS GRANTED`` or ``ACCESS DENIED`` logs are not displayed in the serial output of the Reader under test.

  AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
    Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
    The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach X-NUCLEO-NFC09A1 shield to the nRF54L15 DK, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and re-flash the nRF54L15 DK.

  AL-159: Cannot provision multiple User Devices
    The system allows provisioning of only one User Device to the Reader device.

  AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
    During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.

The |APP_NAME| v0.2.0
*********************

.. toggle::

  AL-239: Occasional timeout occurs in the Reader when executing the RD-NFC-STDTXN-2.0 test from the test harness
    When executing the RD-NFC-STDTXN-2.0 test case, the Reader might report an RX timeout error code in the serial console.
    For more details, see `Test Harness issue #191`_.

  AL-282: An undefined access decision occurs when executing the RD-NFC-STDTXN-2.0 test harness case in a loop
    When executing the RD-NFC-STDTXN-2.0 test case multiple times in a row (with valid credentials provisioned), the Reader might not report the access decision.
    As a result, the ``ACCESS GRANTED`` or ``ACCESS DENIED`` logs are not displayed in the serial output of the Reader under test.

  AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
    Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
    The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach the X-NUCLEO-NFC09A1 shield to the supported Nordic development kit, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and reflash the DK.

  AL-158: Access control is insufficient due to relying only on signature verification
    The Reader device does not have the capability to configure additional access rules.
    Access Manager is not implemented.

  AL-159: Cannot provision multiple User Devices
    The system allows provisioning of only one User Device to the Reader device.

  AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
    During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach the X-NUCLEO-NFC09A1 shield to the supported Nordic development kit, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and reflash the DK.

The |APP_NAME| v0.1.0
*********************

.. toggle::

  AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
    Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
    The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

    **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
    Attach X-NUCLEO-NFC09A1 shield to the nRF54L15 DK, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and re-flash the nRF54L15 DK.

  AL-158: Access control is insufficient due to relying only on signature verification
    The Reader device does not have the capability to configure additional access rules.
    Access Manager is not implemented.

  AL-159: Cannot provision multiple User Devices
    The system allows provisioning of only one User Device to the Reader device.

  AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
    During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.
