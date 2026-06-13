.. _testing_verification_and_testing:
.. _testing_verification_th:

.. include:: /include/testing_verification_head.txt

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp applications/matter-aliro-door-lock-app -- -Dmatter-aliro-door-lock-app_SNIPPET=uwb_qm35 -DCONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE=y -DCONFIG_DOOR_LOCK_STEP_UP_PHASE=y -DCONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA=y

.. include:: /include/testing_verification_tail.txt
