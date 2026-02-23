.. _testing_verification_and_testing:
.. _testing_verification_th:

Verification and testing
########################

.. contents::
   :local:
   :depth: 2

Once you have successfully provisioned Aliro credentials, either through CLI or with Matter, perform tests to ensure your device is functioning correctly.

Select tests for running
************************

You can see the full list of available tests in the `Aliro Certification Tool`_ repository.
From the :guilabel:`Test Suites` list, choose the tests with the `Reader` in the name, for example, :guilabel:`BLE Reader`.
This will allow you to test the Bluetooth LE + UWB transport.

.. note::
   You can upload the :file:`applications/doorlock/docs/certification_assets/Aliro PICS v0.9.3.r2.xml` file to the Test Harness to automatically select tests for execution.
   However, there is a known bug in this Test Harness revision that causes the PICS file to select some incorrect tests and skip some needed to execute.

   * Tests that should be executed (but might be skipped):

     * ``NFC_RDR_NEG_STEPUP_AD_SCHEDULE_IN_ACCESS_RULE_AND_READER``
     * ``NFC_RDR_NEG_STEPUP_AD_SCHEDULE_TIME_VERIFY_REQUIRED``
     * ``NFC_RDR_NEG_STEPUP_AD_TIME_VERIFICATION_REQUIRED``
     * ``BLEUWB_RDR_ADVERTISEMENT_FORMAT``

   * Tests that should not be executed (but might be selected):

     * ``BLEUWB_RDR_CONTROL_FLOW_RDR_DESCRIPTOR_TAG``

Always verify that the correct tests are chosen before executing them.
If you do not upload the PICS file, you can choose the tests manually.

.. figure:: /images/th_test_cases.png
   :scale: 70%
   :alt: Test suites list.

   Test suites list.

The Test Harness device will be operating as the Aliro User Device and communicate with the Reader device.
For verification, execute the following tests, based on the `aliro-sve-v1.0`_ tag.
The test results shown below were acquired on the nRF5340 DK build using the following command:

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp -- -Dapp_SNIPPET=uwb_qm35 -DCONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE=y -DCONFIG_DOOR_LOCK_STEP_UP_PHASE=y -DCONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA=y

.. tabs::

   .. tab:: NFC Reader

      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | Test case                                                 | Description                                                                             | Result | Comment      |
      +===========================================================+=========================================================================================+========+==============+
      | NFC_RDR_STANDARD_NO_CERT                                  | Verify conformance of Reader UT in AUTH1 command.                                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_AUTH0_EXTRA_TAG                               | Verify conformance of Reader UT in AUTH1 command.                                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_AUTH1_EXTRA_TAG                               | Verify conformance of Reader UT in AUTH1 command.                                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_AUTH0_WRONG_VALUE                             | Verify conformance of Reader UT in AUTH1 command.                                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_SEL_RSP_NO_COMMON_EXPEDITED_PROTOCOL_VERSION  | Verify conformance of Reader UT in SELECT command.                                      | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_AUTH1_WRONG_VALUES                            | Verify conformance of Reader UT in AUTH1 command.                                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_AUTH1_WRONG_UD_SIGNATURE                      | Verify conformance of Reader UT in AUTH1 command.                                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STANDARD_CERT_IN_LOAD_CERT_WITH_CHAINING          | Verify conformance of Reader during NFC standard "transaction using LOAD CERT protocol".| PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_FAST                                              | Verify conformance of Reader UT in expedited fast phase.                                | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_DEVICE_KEY_INFO_MISMATCH            | Verify rejection of Access Document not associated with the Access Credential.          | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_DOCTYPE_NOT_ALIROA                  | Verify rejection of Access Document with invalid docType.                               | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_INVALID_ACCESS_DATA_ELEMENT_VERSION | Verify rejection of Access Document with incorrect Access Data Element version.         | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_INVALID_HASH_ISSUER_AUTH            | Verify rejection of Access Document with an invalid Data Element digest.                | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_INVALID_SIGNATURE_ISSUER_AUTH       | Verify rejection of Access Document with invalid IssuerAuth signature.                  | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_ISSUER_CERTIFICATE_TIME_MISMATCH    | Verify rejection of Access Document with issuer certificate time mismatch.              | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_ISSUER_CERT_INVALID_SIGNATURE       | Verify rejection of Access Document with invalid issuer certificate signature.          | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_ISSUER_DOCTYPE_MISMATCH             | Verify rejection of Access Document with incorrect issuerAuth doctype.                  | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_NO_ACCESS_RULE_FOR_READER_ACTION    | Verify rejection of Access Document with no Access Rule for the intended action.        | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_NO_DATA_ELEMENTS                    | Verify rejection of Access Document with no data elements.                              | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_NO_ISSUER_CERT_NO_KEY_ID            | Verify rejection of Access Document with no Issuer Certificate or Key Identifier.       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_SCHEDULE_IN_ACCESS_RULE_AND_READER  | Verify rejection of Access Document with schedule in access rule and reader.            | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_SCHEDULE_TIME_VERIFY_REQUIRED       | Verify rejection of Access Document requiring schedule time verification.               | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_TIME_VERIFICATION_REQUIRED          | Verify rejection of Access Document requiring time verification.                        | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_UNKNOWN_CRITICAL_ACCESS_EXTENSION   | Verify rejection of Access Document with unknown critical access extension.             | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_UNKNOWN_READER_RULE                 | Verify rejection of Access Document with unknown reader rule.                           | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_NEG_STEPUP_AD_VALIDITY_ITERATION                  | Verify rejection of Access Document with validity iteration.                            | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STEPUP_AD_ACCESS_RULE                             | Verify parsing of Access Document with Access Rule.                                     | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STEPUP_AD_ISSUER_CERT_KEY_ID                      | Verify parsing of Access Document with Issuer Certificate Key ID.                       | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STEPUP_AD_ISSUER_CERT                             | Verify parsing of Access Document with Issuer Certificate.                              | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STEPUP_AD_KEY_ID                                  | Verify parsing of Access Document with Key Identifier.                                  | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STEPUP_AD_UNKNOWN_NON_ACCESS_EXTENSION            | Verify parsing of Access Document with unknown non-access extension.                    | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+
      | NFC_RDR_STEPUP_AD_UNKNOWN_NON_CRITICAL_ACCESS_EXTENSION   | Verify parsing of Access Document with unknown non-critical access extension.           | PASS   |              |
      +-----------------------------------------------------------+-----------------------------------------------------------------------------------------+--------+--------------+

   .. tab:: Bluetooth LE + UWB

      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | Test case                                 | Description                                                              | Result | Comment                     |
      +===========================================+==========================================================================+========+=============================+
      | BLEUWB_RDR_EXPEDITED_STANDARD_PHASE       | Verify conformance of Reader UT in standard phase expedited transaction. | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_RANGING_SUSPEND                | Verify conformance of Reader UT in ranging suspend functionality.        | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_RANGING_RESUME                 | Verify conformance of Reader UT in ranging resume functionality.         | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_NEG_FAILED_L2CAP               | Verify conformance of Reader UT in L2CAP connection failure handling.    | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_NEG_FAILED_SPSM_L2CAP          | Verify conformance of Reader UT in SPSM L2CAP failure handling.          | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_NEG_TIMEOUT_BEFORE_AUTH0       | Verify conformance of Reader UT in timeout handling before AUTH0.        | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_TIMEOUT_EXTENSION              | Verify conformance of Reader UT in timeout extension handling.           | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_NEG_M2_MISMATCH_PARAMETER      | Verify conformance of Reader UT in M2 parameter mismatch handling.       | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_NEG_M4_MISMATCH_PARAMETER      | Verify conformance of Reader UT in M4 parameter mismatch handling.       | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_ADVERTISEMENT_FORMAT           | Verify conformance of Reader UT in advertisement format.                 | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_NEG_SUSPEND_MISMATCH_PARAMETER | Verify conformance of Reader UT in suspend parameter mismatch handling.  | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_STEPUP_PHASE                   | Verify conformance of Reader UT in step-up phase.                        | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+
      | BLEUWB_RDR_EXPEDITED_FAST_PHASE           | Verify conformance of Reader UT in expedited fast phase.                 | PASS   |                             |
      +-------------------------------------------+--------------------------------------------------------------------------+--------+-----------------------------+

Running the test
****************

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

               ACCESS GRANTED

            When access is granted and lock is unlocked, the device signals this state by turning on a dedicated LED.
            For details, see the :ref:`lock simulator<lock_simulator>` section.
            When the provided Access Credential public key is incorrect the following output will be displayed:

            .. code-block:: console

               ACCESS DENIED

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


.. note::
   If the application is built with Matter support, you can control the door lock remotely.
   For details on how to enable this feature, see the |NCS| door lock sample documentation in the `Matter door lock remote access`_ section.

Execute the Aliro certification tests
*************************************

This guide assumes that you have uploaded the PICS file :file:`applications/doorlock/docs/certification_assets/Aliro PICS v0.9.3.r2.xml` to the Test Harness.
You can also select tests manually.

.. figure:: /images/pics_upload.png
   :scale: 70%
   :alt: PICS file upload.

   PICS file upload.

When PICS file is uploaded, all required tests are selected automatically for NFC and Bluetooth LE Reader suites:

.. figure:: /images/tests_selected_by_pics.png
   :scale: 70%
   :alt: BLE selected tests.

   Bluetooth LE selected tests.

The following table explains what should be configured before running a specific test. 
Some tests may also require post actions.

.. tabs::

   .. tab:: NFC Reader
      
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | Test case(s)                                               | Door Lock commands to execute                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
      +============================================================+===============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================+
      | ALL TESTS                                                  | Before test session:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa                                                                                                                                                                                                                                                                                                                                                                                                                             |
      |                                                            | * dl install group_id 37652039312061652031642033642065                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_FAST,                                              | Before test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
      | NFC_RDR_NEG_AUTH0_EXTRA_TAG,                               +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_NEG_AUTH0_WRONG_VALUE,                             | * dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa                                                                                                                                                                                                                                                                                                                                                                                                                             |
      | NFC_RDR_NEG_AUTH1_EXTRA_TAG,                               +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_NEG_AUTH1_WRONG_UD_SIGNATURE,                      | Optional:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
      | NFC_RDR_NEG_AUTH1_WRONG_VALUES,                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_NEG_SEL_RSP_NO_COMMON_EXPEDITED_PROTOCOL_VERSION,  | * dl install group_id 37652039312061652031642033642065                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
      | NFC_RDR_NEG_STEPUP_AD_DEVICE_KEY_INFO_MISMATCH,            | * dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                             |
      | NFC_RDR_NEG_STEPUP_AD_DOCTYPE_NOT_ALIROA,                  | * dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                            |
      | NFC_RDR_NEG_STEPUP_AD_INVALID_ACCESS_DATA_ELEMENT_VERSION, |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_INVALID_HASH_ISSUER_AUTH,            |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_INVALID_SIGNATURE_ISSUER_AUTH,       |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_ISSUER_CERT_INVALID_SIGNATURE,       |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_ISSUER_CERTIFICATE_TIME_MISMATCH,    |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_ISSUER_DOCTYPE_MISMATCH,             |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_NO_ACCESS_RULE_FOR_READ,             |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_NO_DATA_ELEMENTS,                    |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_NO_ISSUER_CERT_NO_KEY_ID,            |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_UNKNOWN_CRITICAL_ACCESS_EXTENSION,   |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      | NFC_RDR_NEG_STEPUP_AD_UNKNOWN_READER_RULE                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_NEG_STEPUP_AD_VALIDITY_ITERATION                   | Before test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa                                                                                                                                                                                                                                                                                                                                                                                                                             |
      |                                                            | * dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                             |
      |                                                            | * dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                            |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | Optional:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl install group_id 37652039312061652031642033642065                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | After test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl provisioning CI_key clear 0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_STANDARD_CERT_IN_LOAD_CERT_WITH_CHAINING           | Before test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl provisioning AC_key clear 0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
      |                                                            | * dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                             |
      |                                                            | * dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                            |
      |                                                            | * dl install group_id 000000000000000000113344667799ab                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
      |                                                            | * dl provisioning issuer_pk set 048608ad7d0bf1258ba6249aa3854b43acef9443cce31b617c32503adb959e5ad7d5915a1ff34ab461b46ff598c5fc72c6a6c8787c886741a4029bb3476eceff26                                                                                                                                                                                                                                                                                                                                                                                                                            |
      |                                                            | * dl provisioning reader_cert set 3082010a040200003082010280145555555555555555555555555555555555555555811e637573746f6d20697373756572206e616d652e2e2e2e2e2e2e2e2e2e2e2e820d3230303130323030303030305a830d3235303530353030303030305a841e637573746f6d207375626a656374206e616d652e2e2e2e2e2e2e2e2e2e2e85420004f27d92b78c0ba00c105dc56b9f5a669d67d44fea5139b85dc5e368c851167c9f40b8d9add7ce7bf846331f1f8fd06e1e15cfd540190f482ec294f52690349f188648003045022051bc8503cc09a80f9f19fa033edcf68fb4c341499d908950de62382e9650ee5b022100a954899fd291c40d4f3f9f749518958678ff5688e7bdaa8a3535faf72450e506|
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | After test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl provisioning CI_key clear 0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
      |                                                            | * dl install group_id 37652039312061652031642033642065                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        | 
      |                                                            | * dl provisioning issuer_pk clear                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
      |                                                            | * dl provisioning reader_cert clear                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_STANDARD_NO_CERT                                   | Before test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
      |                                                            +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      |                                                            | * dl install group_id 37652039312061652031642033642065                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
      |                                                            | * dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa                                                                                                                                                                                                                                                                                                                                                                                                                             |
      |                                                            |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_STEPUP_AD_ACCESS_RULE,                             | Before test:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
      | NFC_RDR_STEPUP_AD_ISSUER_CERT,                             +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | NFC_RDR_STEPUP_AD_ISSUER_CERT_KEY_ID,                      | * dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                             |
      | NFC_RDR_STEPUP_AD_KEY_ID,                                  | * dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137                                                                                                                                                                                                                                                                                                                                                                                                                            |
      | NFC_RDR_STEPUP_AD_UNKNOWN_NON_ACCESS_EXTENSION,            | * dl install group_id 37652039312061652031642033642065                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
      | NFC_RDR_STEPUP_AD_UNKNOWN_NON_CRITICAL_ACCESS_EXTENSION    |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
      +------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

   .. tab:: Bluetooth LE UWB Reader

      +---------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | Test case(s)                    | Door Lock commands to execute                                                                                                                                      |
      +=================================+====================================================================================================================================================================+
      | ALL TESTS                       | Before test:                                                                                                                                                       |
      |                                 |                                                                                                                                                                    |
      |                                 | * dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa  |
      |                                 | * dl install group_id 37652039312061652031642033642065                                                                                                             |
      +---------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | BLEUWB_RDR_EXPEDITED_FAST_PHASE | After test:                                                                                                                                                        |
      |                                 |                                                                                                                                                                    |
      |                                 | * dl kpersistent clear 0                                                                                                                                           |
      +---------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
      | BLEUWB_RDR_STEPUP_PHASE         | Before test:                                                                                                                                                       |
      |                                 |                                                                                                                                                                    |
      |                                 | * dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137  |
      |                                 | * dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137 |
      |                                 |                                                                                                                                                                    |
      |                                 | After test:                                                                                                                                                        |
      |                                 |                                                                                                                                                                    |
      |                                 | * dl install group_id 37652039312061652031642033642065                                                                                                             |
      |                                 |                                                                                                                                                                    |
      +---------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
