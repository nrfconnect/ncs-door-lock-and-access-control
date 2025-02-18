.. _testing:

Testing and troubleshooting
###########################

This page will guide you through the testing instructions for the |app_name|.

.. _testing_environment:

Test environment
****************

Test environment consists of two major components:

* The `nRF54L15 DK`_, which serves as the Reader in the door lock component.
  It must be attached to an NFC card reader expansion board.
* The Aliro Test Harness, which acts as the user device and simulates unlocking of the door lock.

See the :ref:`hw_requirements` page for more details.

.. _testing_environment_configuration:

Configuring test environment
****************************

This section provides instructions on setting up the Test Harness, selecting tests, and executing them.
In case you do not have access to `Aliro Certification Tool`_ repository, see the :ref:`hw_requirements_test_harness` section for further guidance.

.. note::
   All examples related to the `Aliro Certification Tool`_ are based on the ``dryrun/test_event4-2024-aliro_specification_v0.9.0-v1.0`` tag.

#. Follow the `Test harness usage instructions`_ in the `Aliro Certification Tool`_ repository.

#. Obtain the long-term public key and reader group identifier of the Reader device under test (DUT).
   You can retrieve this information from the serial console of a DUT.
   See more information on :ref:`building, flashing, and accessing the serial console<building_and_running>`.

#. After flashing the door lock firmware with Aliro support onto the device, observe the provisioned key and reader group identifier of the Reader printed on the DUT's serial console:

   .. code-block:: console

    I: Reader long term public key:
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX                      |x
    Provision the Test Harness with the following byte string:
    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    ...

   .. code-block:: console

    I: Reader Identifier:
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx
    I: XX XX XX XX XX XX XX XX |xxxxxxxx

    Provision the Test Harness with the following Reader Group Identifier:
    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    ...

#. Open your project's JSON configuration and locate the ``dut_reader_public_key`` and ``dut_reader_group_identifier`` fields.

#. Input the 65-byte long Reader public key and the 16-byte long reader group identifier that you retrieved earlier.

.. figure:: /images/th_config.png
   :scale: 70%
   :alt: Test harness project configuration.

   Test harness project configuration.

.. _testing_verification:

Verification and testing process
********************************

Perform tests to ensure your devices are functioning correctly.
You can see the full list of available test on the `Aliro Certification Tool`_ repository.
Note, that NFC Reader tests start with the ``RD`` prefix.
This means that the Test Harness device will be operating as Aliro user device.

For verification, execute the following tests:

1. RD-NFC-CONTROLFLOW-2.0 test, which allows you to verify if the communication between the Reader and the user device terminates correctly when sending the ``CONTROL_FLOW`` command.
2. RD-NFC-STDTXN-2.0 test, which allows you to verify if the Reader properly implements the Aliro authentication protocol.

Running the test
================

Complete the following steps for the required tests:

#. Navigate to the project you created in the Test Harness web interface and click :guilabel:`Go To Test-Run`.
#. Click on :guilabel:`Add New Test`.
#. In the test suites, select :guilabel:`NFC Reader` and under the :guilabel:`Test Cases` section check the box of the test you wish to run.
#. Choose the operator and click :guilabel:`Start`.
#. Position the Reader and Test Harness hardware close to each other, and align them to ensure optimal NFC communication.
#. A notification will appear asking you to set the Reader DUT in the NFC polling mode and to place the devices next to each other for automatic detection.
   Select :guilabel:`Ok` and click :guilabel:`Submit`.
#. Depending on the test you executed, you should see the following results:

   .. tabs::

      .. tab:: RD-NFC-CONTROLFLOW-2.0

            .. figure:: /images/control_flow_test_selection.png
               :scale: 50%
               :alt: Tests selection view.

               Tests selection view.

            The Reader device selects the Test Harness user device, which then terminates the communication and expects the ``CONTROL_FLOW`` command from the Reader in response. 

            In the DUT's serial console, you will see logs that indicate the state of the operation and the data payloads transmitted and received by the Reader.
            If the results show as ``passed``, you will see the following output in the Test Harness web interface:

            .. figure:: /images/test_results_control_flow.png
               :scale: 50%
               :alt: Basic test results view.

               Basic test results view.

      .. tab:: RD-NFC-STDTXN-2.0

            .. figure:: /images/stdtxn_test_selection.png
               :scale: 50%
               :alt: Tests selection view.

               Tests selection view.

            The Reader device will select the Test Harness user device and initiate the Aliro Access Protocol commands exchange.

            In the DUT's serial console, you will see logs that indicate the state of the operation and the data payloads transmitted and received by the Reader.
            If the results show as ``passed``, you will see the following output in the Test Harness web interface:

            .. figure:: /images/test_results_stdtxn.png
               :scale: 50%
               :alt: Example of the advanced test results.

               Example of the advanced test results.
