.. _known_issues:

Known issues
############

.. contents::
   :local:
   :depth: 2

Known issues listed on this page are valid for the current state of development.
Refer to the following sections for more information on specific items.

A known issue can list one or both of the following entries:

* **Affected platforms:**

  If a known issue does not have any specific platforms listed, it is valid for all hardware platforms.

* **Workaround:**

  Some known issues have a workaround.
  Sometimes, they are discovered later and added over time.

The |APP_NAME| v0.3.1
*********************

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

AL-333: The NFC transport lost error occurrs in test cases for granting access fo the door lock
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

The application crashes when the UWB module QM35825, used with Qorvo Arduino Interface Board, is connected to the nRF5340 DK and the X-NUCLEO-NFC09A1 board.
  This issue arises because the QM35825 module forces the SPI MISO line to remain low at all times, which prevents the X-NUCLEO-NFC09A1 board from initializing properly.

  **Workaround 1:** Use a different SPI bus to connect the X-NUCLEO-NFC09A1 board.

  **Workaround 2:** For test purposes, use the ``uwb_qm35`` snippet to disable the NFC transport protocol.
  This way, the X-NUCLEO-NFC09A1 board initialization is not required.

The QM35825 ranging measurement is not working with the test harness.
  Currently, there is no workaround for this issue.

The current implementation of the Access Manager does not support multiple Aliro sessions.
  Even though, the Aliro stack supports multiple sessions, the Access Manager implementation is limited to one session at a time.

AL-333: The NFC transport lost error occurrs in test cases for granting access fo the door lock
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
