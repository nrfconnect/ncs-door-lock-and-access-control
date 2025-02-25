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

The |APP_NAME| v0.1.0
*********************

AL-148: The RD-NFC-STDTXN-1.0 test case fails when the NFC module ST X-NUCLEO-NFC05A1 is in use
  Testing RD-NFC-STDTXN-1.0 with the NFC module NFC05A1 results in failure.
  The issue arises from an error indicated by the test harness, which detects the presence of an invalid TLV tag in the payload received from the Device Under Test (DUT).

  **Workaround:** Switch to X-NUCLEO-NFC09A1, a newer, recommended revision of the NFC ST module.
  Attach X-NUCLEO-NFC09A1 shield to the nRF54L15 DK, rebuild the firmware with the ``CONFIG_ST25R200_DRV`` Kconfig option enabled, and re-flash the nRF54L15 DK.

AL-158: Access control is insufficient due to relying only on signature verification
  The reader device does not have the capability to configure additional access rules.
  Access Manager is not implemented.

AL-159: Cannot provision multiple user devices
  The system allows provisioning of only one user device to the reader device.

AL-161: The RD-NFC-STDTXN-2.0 [X-NUCLEO-NFC08A1] test exhibits a delay in the transaction initiation step
  During the RD-NFC-STDTXN-2.0 test execution, there is a noticeable delay of a few seconds after the transaction initiation step.
