.. _aliro_testing_ble_nordic_uart:

.. include:: /include/testing_nus_head.txt

.. code-block:: console

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=bt_nus

.. include:: /include/testing_nus_tail.txt
