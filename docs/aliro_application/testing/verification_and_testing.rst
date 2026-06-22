.. _aliro_testing_verification_and_testing:
.. _aliro_testing_verification_th:

Verification and testing
########################

.. contents::
   :local:
   :depth: 2

Once you have successfully provisioned Aliro credentials, perform tests to ensure your device is functioning correctly.

.. include:: /include/testing_verification_head.txt

* Tests that should be executed (but might be skipped):

  * ``NFC_RDR_NEG_STEPUP_AD_SCHEDULE_IN_ACCESS_RULE_AND_READER``
  * ``NFC_RDR_NEG_STEPUP_AD_SCHEDULE_TIME_VERIFY_REQUIRED``
  * ``NFC_RDR_NEG_STEPUP_AD_TIME_VERIFICATION_REQUIRED``
  * ``BLEUWB_RDR_ADVERTISEMENT_FORMAT``

* Tests that should not be executed (but might be selected):

  * ``BLEUWB_RDR_CONTROL_FLOW_RDR_DESCRIPTOR_TAG``

.. include:: /include/testing_verification_command.txt

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=uwb_qm35 -DCONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE=y -DCONFIG_DOOR_LOCK_STEP_UP_PHASE=y -DCONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA=y

.. include:: /include/testing_verification_tail.txt
