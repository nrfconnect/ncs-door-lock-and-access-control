.. _testing:

Testing
#######

.. contents::
   :local:
   :depth: 2

This page will guide you through the testing instructions for the |app_name|.

.. _testing_environment:

Test environment
****************

Test environment consists of two major components:

* The Nordic Semiconductor’s :ref:`development kit (DK) <hw_requirements>`, which serves as the Reader in the door lock component.
  It must be attached to an NFC card reader expansion board and optionally to a UWB module.
* The Aliro Test Harness, which acts as the User Device and simulates unlocking of the door lock.
* If you are using Matter, you must have the `Matter controller tools`_  and `Matter over Thread tools`_ for testing the provisioning of Aliro credentials from the Matter controller.

See the :ref:`hw_requirements` page for more details.

.. _testing_environment_configuration:

.. _setting_up_the_aliro_test_harness:

Setting up the Aliro Test Harness
*********************************

This section provides instructions on setting up the Test Harness and getting access credentials from it.
In case you do not have access to `Aliro Certification Tool`_ repository, see the :ref:`hw_requirements_test_harness` section for further guidance.

.. note::
   All examples related to the `Aliro Certification Tool`_ are based on the ``dryrun/test_event4-2024-aliro_specification_v0.9.0-v1.0`` tag.

#. Follow the `Test harness usage instructions`_ in the `Aliro Certification Tool`_ repository.

#. Open your Test Harness project's JSON configuration and locate the ``dut_reader_public_key``, ``th_access_credential_public_key``, ``dut_reader_group_identifier``, and ``dut_reader_group_sub_identifier`` fields.

   .. figure:: /images/th_config.png
      :scale: 70%
      :alt: Test harness project configuration.

      Test harness project configuration.


Aliro door lock provisioning with CLI
*************************************

This section provides instructions on setting up the Test Harness, selecting tests, and executing them.

#. Follow the :ref:`setting_up_the_aliro_test_harness` to locate necessary credentials in the Test Harness project configuration.

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

#. Set up the test harness by inputting the 65-byte long Reader public key into the ``dut_reader_public_key`` field.

#. Install the Reader identifier in the Reader device.
   The full 32-byte Reader identifier can be set using the following ``dl install`` shell command:

   .. code-block:: console

      uart:~$ dl install identifier <32-byte reader_identifier in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl install identifier 00113344667799AA00113344667799AA113344667799AA00113344667799AA00

   Alternatively, you can set the Reader group identifier and Reader group sub-identifier individually using the following ``dl install`` shell commands:

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

      uart:~$ dl provisioning AC_key set <key id> <65-byte public key in hex in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa

   .. note::
      You can also use the following command to clear the public key stored in the DUT:

      .. code-block:: console

         uart:~$ dl provisioning AC_key clear <key id>

      For example:

      .. code-block:: console

         uart:~$ dl provisioning AC_key clear 0

      To clear all public keys stored in the DUT, use the following command:

      .. code-block:: console

         uart:~$ dl provisioning AC_key clear all

      To list all public keys stored in the DUT, use the following command:

      .. code-block:: console

         uart:~$ dl provisioning AC_key list

      For example:

      .. code-block:: console

         uart:~$ dl provisioning AC_key list
         [0]: 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efb
         [1]: (null)
         [2]: (null)
         [3]: (null)
         [4]: (null)
         [5]: (null)
         [6]: (null)
         [7]: (null)
         [8]: (null)
         [9]: (null)

.. _testing_verification:

Aliro door lock provisioning with Matter
****************************************

If you are building an application with Matter support (see :ref:`building_and_running` and ``-DSNIPPET='matter'`` option), you can provision Aliro credentials from the Matter controller.
First, ensure that all the necessary software tools and hardware are configured (see :ref:`testing_environment`).
The following steps will guide you through the process of commissioning the Aliro door lock to Matter network and provisioning Aliro credentials.

.. note::
   This guide uses CHIP Tool as a Matter controller.
   For details, see the `Matter chip-tool guide`_.

#. Set up the `Thread Border Router`_ with the Nordic Thread coprocessor.

   a. Configure the `Thread radio co-processor`_.

   #. Run the OpenThread Border Router and `form a Thread network using Docker <Running OTBR using Docker_>`_.

   #. Obtain the Thread operational dataset for commissioning devices into created Thread network by running the following Docker command:

      .. code-block:: console

         sudo docker exec -it otbr sh -c "sudo ot-ctl dataset active -x"

      Save this operational dataset as it will be needed for commissioning the door lock device.

#. Commission the Aliro door lock to Matter over Thread network.

   a. Put the door lock device in commissioning mode by pressing the appropriate button on the development kit.
      Refer to the :ref:`matter_ui` section for more details.

   #. Use CHIP Tool to commission the device with Thread operational dataset obtained:

      .. code-block:: console

         ./chip-tool pairing ble-thread <node-id> hex:<thread-dataset> <pin-code> <discriminator>

      * ``<node-id>`` - A unique node ID for the device (for example, ``1``)
      * ``<thread-dataset>`` - The obtained Thread operational dataset
      * ``<pin-code>`` - Device pairing code (default: ``30033003``)
      * ``<discriminator>`` - Device discriminator (default: ``3003``)

      For example:

      .. code-block:: console

         ./chip-tool pairing ble-thread 1 hex:0e080000000000010000000300001335060004001fffe002084c70c4ba47c6a0780708fd12345678901234051000112233445566778899aabbccddeeff030e4f70656e5468726561642048423030010212340410445f2b5ca6f2a93a55ce570a70efeecb0c0402a0f7f8 30033003 3003

   #. Verify successful commissioning by reading basic device information:

      .. code-block:: console

         ./chip-tool basicinformation read vendor-name 1 0

      You should see the vendor information of the commissioned door lock device.

#. Configure Aliro reader based on the Test Harness configuration.

   a. Generate an ECC key pair for the Reader by running the following commands:

      .. code-block:: console

         KEY_OUTPUT=$(openssl ecparam -name prime256v1 -genkey | openssl ec -text -noout) && \
         echo "Private Key:" && \
         echo "$KEY_OUTPUT" | sed -n '/priv:/,/pub:/p' | grep -v 'priv:\|pub:' | tr -d ' \n:' && \
         echo -e "\nPublic Key:" && \
         echo "$KEY_OUTPUT" | sed -n '/pub:/,/ASN1/p' | grep -v 'pub:\|ASN1' | tr -d ' \n:'

      See the following example output:

      .. code-block:: console

         Private Key:
         9df123f58dd15f6bab71bb6635827faf25100b043cdf6b62c93ea3c244ad4403
         Public Key:
         043c05b91fc09a84ad2ab7940a1b84f09b8ddf5323f1aac0f6568e1c973f37275dc67500a9df08d1bd69ee04e8641d9cbc73a4c1be30eed64def414f8afdc44642

   #. Follow the :ref:`setting_up_the_aliro_test_harness` section to locate the necessary credentials in the Test Harness project configuration.

   #. Configure the Test Harness project:

      * Set ``dut_reader_public_key`` to the generated ``Public Key``.
      * Note down the values of ``th_access_credential_public_key`` and ``dut_reader_group_identifier``.
        You will need them for further configuration.

   #. Set the Aliro Reader configuration remotely with CHIP Tool using the following command:

      .. code-block:: console

         ./chip-tool doorlock set-aliro-reader-config hex:<signing_key> hex:<verification_key> hex:<group_identifier> <node-id> <endpoint-id> --GroupResolvingKey hex:<group_resolving_key>

      * ``<signing_key>`` - Signing key of the Reader device (generated ``Private Key``)
      * ``<verification_key>`` - Verification key of the Reader device (generated ``Public Key``)
      * ``<group_identifier>`` - Group identifier of the Reader device (corresponds to ``dut_reader_group_identifier``)
      * ``<node-id>`` - Unique node ID for the device (for example, ``1``)
      * ``<endpoint-id>`` - Endpoint ID of the Reader device (for example, ``1`` for the door lock cluster)
      * ``<group_resolving_key>`` - Group resolving key of the Reader device

      .. note::
         The ``<group_resolving_key>`` option corresponds to ``dut_reader_group_sub_identifier``, which can be set in the Test Harness project configuration.

      See the following example:

      .. code-block:: console

         ./chip-tool doorlock set-aliro-reader-config hex:9df123f58dd15f6bab71bb6635827faf25100b043cdf6b62c93ea3c244ad4403 hex:043c05b91fc09a84ad2ab7940a1b84f09b8ddf5323f1aac0f6568e1c973f37275dc67500a9df08d1bd69ee04e8641d9cbc73a4c1be30eed64def414f8afdc44642 hex:00113344667799AA00113344667799AA 1 1 --GroupResolvingKey hex:00000000000000000000000000000000 --timedInteractionTimeoutMs 5000

      .. note::
         The ``GroupResolvingKey`` can only be set if the Aliro door lock application is built with Bluetooth LE transport and Ultra-Wideband (UWB) support.
         Currently, the Aliro Test Harness expects the ``GroupResolvingKey`` to be set to all zeros.

#. Add door lock user and AliroEvictableEndpointKey credentials using CHIP Tool.

   a. Add the first user (Home user):

      .. code-block:: console

         ./chip-tool doorlock set-user 0 1 Home 0 1 0 0 1 1 --timedInteractionTimeoutMs 5000

      * Endpoint ID - ``0``
      * User index - ``1``
      * User name - ``Home``
      * User unique ID - ``0``
      * User status (1 = enabled) - ``1``
      * User type (0 = unrestricted) - ``0``
      * Credential rule (0 = single) - ``0``
      * Node ID of the door lock - ``1``
      * Fabric index - ``1``

   #. Set the first AliroEvictableEndpointKey credential for the Home user:

      .. code-block:: console

         ./chip-tool doorlock set-credential 0 '{"credentialType": 7, "credentialIndex": 1}' hex:<credential-data> 1 null null 1 1 --timedInteractionTimeoutMs 5000

      * Endpoint ID - ``0``
      * User index - ``1``
      * User name - ``Home``
      * User unique ID - ``0``
      * User status (1 = enabled) - ``1``
      * User type (0 = unrestricted) - ``0``
      * Credential rule (0 = single) - ``0``
      * Node ID of the door lock - ``1``
      * Fabric index - ``1``
      * Credential type - ``7``
      * Credential index - ``1``
      * ``<credential-data>`` - Is an octet string parameter with the secret credential data (corresponds to ``th_access_credential_public_key``)

       See the following example:

      .. code-block:: console

         ./chip-tool doorlock set-credential 0 '{"credentialType": 7, "credentialIndex": 1}' hex:04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa 1 null null 1 1 --timedInteractionTimeoutMs 5000

   #. Read the status of credential assigned to the user to verify that it was set correctly:

      .. code-block:: console

         ./chip-tool doorlock get-credential-status {"credentialType": 7, "credentialIndex": 1} 1 1

.. _matter_ui:

Matter door lock user interface
*******************************

This section describes the user interface of the |APP_NAME| when Matter is enabled.

.. tabs::

   .. group-tab:: nRF52, nRF53, nRF54LM20 DKs

      LED 1:
         .. include:: /include/matter_state_led.txt

      LED 2:
         Shows the state of the lock.
         The following states are possible:

         * Solid On - The bolt is extended and the door is locked.
         * Off - The bolt is retracted and the door is unlocked.
         * Rapid Even Flashing (50 ms on/50 ms off during 2 s) - The simulated bolt is in motion from one position to another.

         Additionally, the LED starts blinking evenly (500 ms on/500 ms off) when the Identify command of the Identify cluster is received on the endpoint ``1``.
         The command's argument can be used to specify the duration of the effect.

      Button 1:
         .. include:: /include/matter_button.txt

      Button 2:
         * Changes the lock state to the opposite one.


   .. group-tab:: nRF54L15 DK

      LED 0:
          .. include:: /include/matter_state_led.txt

      LED 1:
         Shows the state of the lock.
         The following states are possible:

         * Solid On - The bolt is extended and the door is locked.
         * Off - The bolt is retracted and the door is unlocked.
         * Rapid Even Flashing (50 ms on/50 ms off during 2 s) - The simulated bolt is in motion from one position to another.

         Additionally, the LED starts blinking evenly (500 ms on/500 ms off) when the Identify command of the Identify cluster is received on the endpoint ``1``.
         The command's argument can be used to specify the duration of the effect.

      Button 1:
         * Changes the lock state to the opposite one.

      Button 2:
         .. include:: /include/matter_button.txt

Aliro and Matter lock state synchronization
*******************************************

When the door lock is unlocked through Aliro, the lock state automatically synchronizes with the Matter door-lock server cluster.
This synchronization ensures that User Devices in the Matter network are notified of lock state changes in real time.

To verify the current lock state, run the following command:

.. code-block:: console

   ./chip-tool doorlock read lock-state 1 1

After executing the command, find the lock state in the response, for example:

.. code-block:: console

   [1757071349.943] [949263:949265] [TOO]   LockState: 1

The response indicates that the lock is locked.

Next, try to execute, for example, the ``BLEUWB_RDR_EXPEDITED_STANDARD_PHASE`` test with Bluetooth LE transport (see :ref:`testing_verification_th`) using the Test Harness.
If the test is successful, run again the ``./chip-tool doorlock read lock-state 1 1`` command.
You should see the following status in the response:

.. code-block:: console

   [1757072050.821] [951199:951201] [TOO]   LockState: 2

The response indicates that the lock is unlocked.

.. _testing_verification_th:

Verification and testing process
********************************

Once you have successfully provisioned Aliro credentials, either through CLI or with Matter, perform tests to ensure your device is functioning correctly.
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
            | BLEUWB_RDR_EXPEDITED_STANDARD_PHASE                       | Verify conformance of Reader UT in standard phase expedited transaction. |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_RANGING_SUSPEND                                | Verify conformance of Reader UT in ranging suspend functionality.        |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_NEG_FAILED_L2CAP                               | Verify conformance of Reader UT in L2CAP connection failure handling.    |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_NEG_FAILED_SPSM_L2CAP                          | Verify conformance of Reader UT in SPSM L2CAP failure handling.          |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_NEG_TIMEOUT_BEFORE_AUTH0                       | Verify conformance of Reader UT in timeout handling before AUTH0.        |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_TIMEOUT_EXTENSION                              | Verify conformance of Reader UT in timeout extension handling.           |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_NEG_M2_MISMATCH_PARAMETER                      | Verify conformance of Reader UT in M2 parameter mismatch handling.       |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_NEG_M4_MISMATCH_PARAMETER                      | Verify conformance of Reader UT in M4 parameter mismatch handling.       |
            +-----------------------------------------------------------+--------------------------------------------------------------------------+
            | BLEUWB_RDR_ADVERTISEMENT_FORMAT                           | Verify conformance of Reader UT in advertisement format.                 |
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

               ACCESS GRANTED

            .. note::
               When access is granted, the device also signals this event by turning on a dedicated LED. For details, see the :ref:`access decision indicator <access_decision_indicator>` section.

            When the provided access credential public key is incorrect the following output will be displayed:

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

Additional CLI commands
=======================

To check the revision of the Aliro library on your device, run the following command in the device shell:

.. code-block:: console

   uart:~$ dl info
   Aliro version: v0.2.0-22-g7da4b2e
   NFC reader: ST25R100
