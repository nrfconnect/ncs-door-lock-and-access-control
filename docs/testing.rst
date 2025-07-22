.. _testing:

Testing and troubleshooting
###########################

.. contents::
   :local:
   :depth: 2

This page will guide you through the testing instructions for the |app_name|.

.. _testing_environment:

Test environment
****************

Test environment consists of two major components:

* The Nordic Semiconductor’s :ref:`development kit (DK) <hw_requirements>`, which serves as the Reader in the door lock component.
  It must be attached to an NFC card reader expansion board.
* The Aliro Test Harness, which acts as the User Device and simulates unlocking of the door lock.

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

    Provision the Test Harness with the following Reader Public Key:
    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    ...

   .. code-block:: console

    Provision the Test Harness with the following Reader Group Identifier:
    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    ...

#. Open your project's JSON configuration and locate the ``dut_reader_public_key``, ``th_access_credential_public_key``, ``dut_reader_group_identifier``, and ``dut_reader_group_sub_identifier`` fields.

   .. figure:: /images/th_config.png
      :scale: 70%
      :alt: Test harness project configuration.

      Test harness project configuration.

#. Set up the test harness by inputting the 65-byte long Reader public key into the ``dut_reader_public_key`` field.

#. Install the 16-byte long Reader group identifier and reader group sub-identifier in the Reader device.
   Both values can be provided to the DUT using the following ``dl install`` shell commands:

   .. code-block:: console

      uart:~$ dl install group_id <16-byte reader_group_identifier in hex without 0x>
      uart:~$ dl install group_sub_id <16-byte reader_group_sub_identifier in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl install group_id 00113344667799AA00113344667799AA
      uart:~$ dl install group_sub_id 113344667799AA00113344667799AA00

   Executing the same commands without specifying values will return the stored value.
   For example:

   .. code-block:: console

      uart:~$ dl install group_id
      00000000: 00 11 33 44 66 77 99 aa  00 11 33 44 66 77 99 aa |..3Dfw.. ..3Dfw..|

   Alternatively, you can set the Reader group identifier retrieved from DUT in the test harness project configuration, next to the ``dut_reader_group_identifier`` field.

#. Set the ``th_access_credential_public_key`` in the DUT using the following ``dl provisioning`` shell command:

   .. code-block:: console

      uart:~$ dl provisioning AC_key <65-byte public key in hex in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl provisioning AC_key 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa

   Executing the same command without specifying the public key will return the stored value.
   For example:

   .. code-block:: console

      uart:~$ dl provisioning AC_key
      00000000: 04 74 2d f7 36 d0 fc 9b  e9 78 c4 5b 00 e8 fd f7 |.t-.6... .x.[....|
      00000010: ce a6 84 ea 10 5a e5 74  c1 50 5a 2c 24 ab 61 98 |.....Z.t .PZ,$.a.|
      00000020: e3 12 5b 7f 1b 7e 1d 13  4c 55 ec e6 96 81 ba 8e |..[..~.. LU......|
      00000030: cc 18 a3 83 6d c5 19 9c  75 9f 31 e8 cc f1 7e 3e |....m... u.1...~>|
      00000040: fa                                               |.                |

.. _testing_verification:

Verification and testing process
********************************

Perform tests to ensure your devices are functioning correctly.
You can see the full list of available tests in the `Aliro Certification Tool`_ repository.
From the :guilabel:`Test Suites` list, choose the tests with the `Reader` in the name, for example, :guilabel:`BLE Reader`.
This will allow you to test the Bluetooth LE transport.

.. figure:: /images/th_test_cases.png
   :scale: 70%
   :alt: Test suites list.

   Test suites list.

Thanks to this, the Test Harness device will be operating as the Aliro User Device and communicate with the Reader device.

For verification, execute the following tests, based on the `Aliro Certification Test Tool for GTE#2`_ tag:

.. tabs::

   .. tab:: NFC Reader

            +-----------------------------------------------------------+-----------------------------------------------------------+
            | Test case                                                 | Description                                               |
            +===========================================================+===========================================================+
            | NFC_RDR_STANDARD_NO_CERT                                  | Verify conformance of Reader UT in AUTH1 command.         |
            +-----------------------------------------------------------+-----------------------------------------------------------+
            | NFC_RDR_NEG_AUTH0_EXTRA_TAG                               | Verify conformance of Reader UT in AUTH1 command.         |
            +-----------------------------------------------------------+-----------------------------------------------------------+
            | NFC_RDR_NEG_AUTH1_EXTRA_TAG                               | Verify conformance of Reader UT in AUTH1 command.         |
            +-----------------------------------------------------------+-----------------------------------------------------------+
            | NFC_RDR_NEG_AUTH0_WRONG_VALUE                             | Verify conformance of Reader UT in AUTH1 command.         |
            +-----------------------------------------------------------+-----------------------------------------------------------+
            | NFC_RDR_NEG_SEL_RSP_NO_COMMON_EXPEDITED_PROTOCOL_VERSION  | Verify conformance of Reader UT in SELECT command.        |
            +-----------------------------------------------------------+-----------------------------------------------------------+
            | NFC_RDR_EXCHANGE_RDR_DESCRIPTOR_TAG                       | Verify conformance of Reader UT.                          |
            +-----------------------------------------------------------+-----------------------------------------------------------+

   .. tab:: Bluetooth LE

            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | Test case                                                 | Description                                                              |
            +===========================================================+==========================================================================+
            | TH BLEUWB_RDR_EXPEDITED_STANDARD_PHASE                    | Verify conformance of Reader UT in standard phase expedited transaction. |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_RANGING_SUSPEND                             | Verify conformance of Reader UT in ranging suspend functionality.        |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_NEG_FAILED_L2CAP                            | Verify conformance of Reader UT in L2CAP connection failure handling.    |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_NEG_FAILED_SPSM_L2CAP                       | Verify conformance of Reader UT in SPSM L2CAP failure handling.          |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_NEG_TIMEOUT_BEFORE_AUTH0                    | Verify conformance of Reader UT in timeout handling before AUTH0.        |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_TIMEOUT_EXTENSION                           | Verify conformance of Reader UT in timeout extension handling.           |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_NEG_M2_MISMATCH_PARAMETER                   | Verify conformance of Reader UT in M2 parameter mismatch handling.       |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_NEG_M4_MISMATCH_PARAMETER                   | Verify conformance of Reader UT in M4 parameter mismatch handling.       |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | TH BLEUWB_RDR_ADVERTISEMENT_FORMAT                        | Verify conformance of Reader UT in advertisement format.                 |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+

Running the test
================

Complete the following steps for the required tests:

#. Navigate to the project you created in the Test Harness web interface and click :guilabel:`Go To Test-Run`.

#. Click on :guilabel:`Add New Test`.

#. In the test suites, select either :guilabel:`NFC Reader` or :guilabel:`BLE Reader` and under the :guilabel:`Test Cases` section check the box of the test you wish to run.

#. Choose the operator and click :guilabel:`Start`.

#. Complete the test:

   .. tabs::

      .. tab:: NFC

         a. Position the Reader and Test Harness hardware close to each other, and align them to ensure optimal NFC communication.
         #. A notification will appear asking you to set the Reader DUT in the appropriate mode and to place the devices next to each other for automatic NFC detection.
            Select :guilabel:`Ok` and click :guilabel:`Submit`.

      .. tab:: Bluetooth LE

         a. Ensure the Reader is advertising and ready to accept Bluetooth LE connections from the Test Harness.
            You should see a notification requesting Bluetooth LE visibility for automatic detection.
         #. Confirm that the Reader is advertising by checking the DUT console for logs indicating that advertising has started.

            .. code-block:: console

               <dbg> L2CAP server registered with PSM: 0x0080

         #. Select :guilabel:`Ok` and click :guilabel:`Submit`.
            Restarting the Murata device, if prompted, is optional.

            .. note::
               At the start of each Bluetooth LE test, the firmware is uploaded to the Murata device, which may take some time.
               The Murata module (`LBUA0VG2BP-EVK-P`_) is used by the Test Harness to establish and handle Bluetooth LE communication with the device under test.

#. Depending on the test you executed, you should see results similar to the following examples:

   .. tabs::

      .. tab:: NFC_RDR_STANDARD_NO_CERT

            .. figure:: /images/NFC_RDR_STANDARD_NO_CERT_selection.png
               :scale: 50%
               :alt: Tests selection view.

               Tests selection view.

            The Reader device will select the Test Harness User Device and initiate the Aliro Access Protocol commands exchange.

            In the DUT's serial console, you will see logs that indicate the state of the operation and the data payloads transmitted and received by the Reader.
            If the results show as ``passed``, you will see the following output in the Test Harness web interface:

            .. figure:: /images/test_results_NFC_RDR_STANDARD_NO_CERT.png
               :scale: 50%
               :alt: Example of the advanced test results.

               Example of the advanced test results.

            After test execution is complete, you can check DUT logs to verify the communication and data exchange between the Reader and the test harness.
            The logs will provide detailed information about the test execution and authorization process (signature verification).
            When all installation and provisioning data provided in :ref:`testing_environment_configuration` are correct then you will see the following output in the DUT serial console:

            .. code-block:: console

               <inf> door_lock_app: ACCESS GRANTED

            .. note::
               When access is granted, the device also signals this event by turning on a dedicated LED. For details, see the :ref:`access decision indicator <access_decision_indicator>` section.

            When the provided access credential public key is incorrect the following output will be displayed:

            .. code-block:: console

               <inf> door_lock_app: ACCESS DENIED

      .. tab:: BLEUWB_RDR_EXPEDITED_STANDARD_PHASE

            .. figure:: /images/BLEUWB_RDR_EXPEDITED_STANDARD_PHASE_selection.png
               :scale: 50%
               :alt: Tests selection view.

               Tests selection view.

            The Reader device will advertise over Bluetooth LE and the Test Harness will connect as a Bluetooth LE central device, initiating the Aliro Access Protocol commands exchange over Bluetooth LE.

            In the DUT's serial console, you will see logs that indicate the state of the Bluetooth LE connection, protocol execution, and the data payloads transmitted and received by the Reader.
            Additionally, the UWB connection will be configured and established.
            If the results show as ``passed``, you will see the following output in the Test Harness web interface:

            .. figure:: /images/test_results_BLEUWB_RDR_EXPEDITED_STANDARD_PHASE.png
               :scale: 50%
               :alt: Basic test results view.

               Basic test results view.


Additional CLI commands
=======================

To check the revision of the Aliro library on your device, run the following command in the device shell:

.. code-block:: console

   uart:~$ dl info
   Aliro version: v0.2.0-22-g7da4b2e
   NFC reader: ST25R100
