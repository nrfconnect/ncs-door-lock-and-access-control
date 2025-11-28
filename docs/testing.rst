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

The test environment consists of two major components:

* The Nordic Semiconductor’s :ref:`development kit (DK) <hw_requirements>`, which serves as the Reader in the door lock component.
  It must be attached to an NFC card reader expansion board and optionally to a UWB module.
* The Aliro Test Harness, which acts as the User Device and simulates unlocking of the door lock.
* If you are using Matter, you must have the `Matter controller tools`_  and `Matter over Thread tools`_ for testing the provisioning of Aliro credentials from the Matter controller.

See the :ref:`hw_requirements` page for more details.

.. _testing_environment_configuration:

.. _setting_up_the_aliro_test_harness:

Setting up the Aliro Test Harness
*********************************

This section provides instructions on setting up the Test Harness and getting Access Credentials from it.
In case you do not have access to `Aliro Certification Tool`_ repository, see the :ref:`hw_requirements_test_harness` section for further guidance.

.. note::
   All examples related to the `Aliro Certification Tool`_ are based on the `aliro-sve-v1.0`_ tag.

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

      uart:~$ dl provisioning AC_key set <key id> <65-byte public key in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa

   .. note::
      You can also use the following command to clear the public key stored in the DUT by its ID:

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

#. Optionally, set the ``dut_credential_issuer_public_key`` in the DUT using the following ``dl provisioning`` shell command:

   .. code-block:: console

      uart:~$ dl provisioning CI_key set <key id> <65-byte public key in hex without 0x>


   For example:

   .. code-block:: console

      uart:~$ dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

   .. note::

      You can also use the following commands:

      * ``uart:~$ dl provisioning CI_key clear <key id>`` - This command clears the public key stored in the DUT by its ID (for example, ``uart:~$ dl provisioning CI_key clear 0``).
      * ``uart:~$ dl provisioning CI_key clear all`` - This command clears all public keys stored in the DUT.
      * ``uart:~$ dl provisioning CI_key list`` - This command lists all public keys stored in the DUT.

      For example:

      .. code-block:: console

         uart:~$ dl provisioning CI_key list
         [0]: 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

#. Optionally, set the Credential Issuer Certificate Authority (CA) public key in the DUT using the following ``dl provisioning`` shell command:

   .. code-block:: console

      uart:~$ dl provisioning CI_CA_key set <65-byte public key in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

   .. note::

      You can also use the following commands:

      * ``uart:~$ dl provisioning CI_CA_key get`` - This command retrieves the Credential Issuer CA public key stored in the DUT.
      * ``uart:~$ dl provisioning CI_CA_key clear`` - This command clears the Credential Issuer CA public key stored in the DUT.

      For example:

      .. code-block:: console

         uart:~$ dl provisioning CI_CA_key get
         047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

.. _testing_reader_certificates:

Aliro Reader Certificate Testing
*********************************

The Aliro Reader can optionally use X.509 certificates for authentication during the expedited standard phase.
When a certificate is provisioned, the Reader sends a ``LOAD_CERT`` command after the ``AUTH0`` exchange, allowing the User Device to verify the Reader's identity.

This feature is controlled by the ``CONFIG_DOOR_LOCK_READER_CERTIFICATE`` Kconfig option (enabled by default for non-Matter builds).

Certificate generation and provisioning
========================================

Generating certificates
------------------------

#. Generate an Issuer key pair for signing certificates:

   .. code-block:: console

      cd scripts
      python3 generate_keypair.py --verbose

   Save the output:

   * ``ISSUER_PRIV`` - Issuer Private Key (32 bytes hex)
   * ``ISSUER_PUB`` - Issuer Public Key (65 bytes hex)

#. Generate a Reader certificate.

   You can choose between two certificate sizes:

   * **Short certificate** (~152 bytes) - Does not require APDU chaining
   * **Long certificate** (~270 bytes) - Requires APDU chaining (for testing chaining mechanism)

   **For short certificate:**

   .. code-block:: console

      python3 generate_reader_cert.py \
        --subject-pubkey <DUT_READER_PUBLIC_KEY> \
        --issuer-privkey <ISSUER_PRIV> \
        --output hex | xargs python3 compress_reader_cert.py

   **For long certificate (tests APDU chaining):**

   .. code-block:: console

      ./generate_max_size_cert.sh <DUT_READER_PUBLIC_KEY> <ISSUER_PRIV>

   Save the compressed certificate hex output for provisioning.

Configuring Test Harness
-------------------------

Update your Test Harness project configuration with the following fields:

* ``dut_reader_issuer_public_key`` - Set to ``ISSUER_PUB`` from step 1
* ``dut_reader_issuer_group_identifier`` - Must be different from ``dut_reader_group_identifier``

.. code-block:: json

   {
     "config": {
       "test_parameters": {
         "dut_reader_public_key": "<DUT_READER_PUBLIC_KEY>",
         "dut_reader_group_identifier": "00113344667799AA00113344667799AB",
         "dut_reader_issuer_group_identifier": "FFEEDDCCBBAA998877665544332211FF",
         "dut_reader_issuer_public_key": "<ISSUER_PUB>"
       }
     }
   }

.. important::
   The ``dut_reader_issuer_group_identifier`` must differ from ``dut_reader_group_identifier`` to properly test certificate-based authentication.

Provisioning DUT
----------------

Connect to the DUT via serial console and provision both the certificate and issuer public key:

#. Install the proper Reader group identifier (``dut_reader_issuer_group_identifier`` value):

   .. code-block:: console

      uart:~$ dl install group_id <dut_reader_issuer_group_identifier>

#. Provision the Issuer Public Key:

   .. code-block:: console

      uart:~$ dl provisioning issuer_pk set <ISSUER_PUB>

#. Provision the Reader Certificate (compressed):

   .. code-block:: console

      uart:~$ dl provisioning reader_cert set <COMPRESSED_CERT>

#. Verify provisioning:

   .. code-block:: console

      uart:~$ dl provisioning issuer_pk list
      Issuer public key (65 bytes): <ISSUER_PUB>

      uart:~$ dl provisioning reader_cert list
      Reader certificate (XXX bytes): <COMPRESSED_CERT>

.. note::
   The certificate size is limited by ``CONFIG_DOOR_LOCK_READER_CERTIFICATE_MAX_SIZE`` (default: 512 bytes).
   If you need to provision larger certificates, increase this value in your project configuration.

Running tests with certificates
================================

Once the certificate is provisioned, execute Test Harness test cases that include the ``LOAD_CERT`` command:

* ``NFC_RDR_STANDARD_CERT_IN_LOAD_CERT`` - Standard certificate test
* ``NFC_RDR_STANDARD_CERT_IN_LOAD_CERT_WITH_CHAINING`` - Certificate with APDU chaining

Expected behavior:

* DUT sends ``LOAD_CERT`` command with the provisioned certificate
* If using a long certificate: APDU chaining is used (multiple chunks)
* Test Harness verifies the certificate signature using ``ISSUER_PUB``
* Transaction proceeds normally after certificate validation

Clearing certificates
---------------------

To run standard tests without certificates, clear the provisioned data:

.. code-block:: console

   uart:~$ dl provisioning reader_cert clear
   uart:~$ dl provisioning issuer_pk clear

After clearing, the DUT will skip the ``LOAD_CERT`` state and proceed directly from ``AUTH0`` response to ``AUTH1`` (expedited standard phase).

.. _testing_verification:

Aliro door lock provisioning with Matter
****************************************

If you are building an application with Matter support (see :ref:`building_and_running` and ``-DSNIPPET='matter'`` option), you can provision Aliro credentials from a Matter controller.
First, ensure that all the necessary software tools and hardware are configured (see :ref:`testing_environment`).

The following section covers two different approaches for testing the Aliro door lock with Matter:

* Testing with Matter CHIP Tool - Using the CHIP Tool command-line interface for commissioning and credential provisioning (intended for development and testing).
* Testing with Apple Ecosystem - Using Apple Home app and Apple Wallet for real-world user experience testing with iOS devices.

.. _testing_with_chip_tool:

Testing with Matter CHIP Tool
==============================

This guide uses CHIP Tool as a Matter controller to commission the door lock and provision Aliro credentials through command-line interface.
For details, see the `Matter chip-tool guide`_.

.. note::
   This approach is recommended for development, testing, and debugging purposes.

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
      * ``<credential-data>`` - An octet string parameter with the secret credential data (corresponds to ``th_access_credential_public_key``)

      See the following example:

      .. code-block:: console

         ./chip-tool doorlock set-credential 0 '{"credentialType": 7, "credentialIndex": 1}' hex:04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa 1 null null 1 1 --timedInteractionTimeoutMs 5000

   #. Read the status of credential assigned to the user to verify that it was set correctly:

      .. code-block:: console

         ./chip-tool doorlock get-credential-status {"credentialType": 7, "credentialIndex": 1} 1 1

#. Add Aliro Credential Issuer key using CHIP Tool.
   Once you have a user created on the door lock, you can add the Credential Issuer public key to the Reader using the following command:

   .. code-block:: console

      ./chip-tool doorlock set-credential 0 '{"credentialType": 6, "credentialIndex": 1}' hex:<credential-data> 1 null null 1 1 --timedInteractionTimeoutMs 5000

   * Operation-type - ``0``.
     This operation adds a new credential to a ``User index``.
   * Credential type - ``6``.
     The value represents the Aliro Credential Issuer public key.
   * Credential index - ``1``
   * ``<credential-data>`` - Is an octet string parameter with the Credential Issuer public key data.
     It corresponds to ``th_access_credential_public_key``.
   * User index - ``1``.
   * User status - ``null``.
   * User type - ``null``.
   * Destination ID - ``1``.
   * Endpoint ID - ``1``.

       See the following example:

      .. code-block:: console

         ./chip-tool doorlock set-credential 0 '{"credentialType": 6, "credentialIndex": 1}' hex:047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137 1 null null 1 1 --timedInteractionTimeoutMs 5000

.. _testing_with_apple_ecosystem:

Testing with Apple ecosystem
=============================

This guide demonstrates how to test the Aliro door lock with Matter support using Apple's Home App and Wallet on iPhone.

For additional information about Matter testing with various ecosystems, see the `Matter testing with Apple, Google, and Samsung ecosystems`_ guide.

.. note::
   The Apple's Express Mode feature, which uses UWB ranging, is not currently supported by |APP_NAME|.
   While |APP_NAME|'s UWB ranging implementation can run with the Aliro Test Harness, it is not compliant with the Apple User Device.
   Currently, you can only use the NFC transport when testing |APP_NAME| with Apple ecosystem.

.. note::
   Testing with Apple ecosystem requires the Step-up phase support to be enabled on the device.
   This feature is enabled by default.
   Ensure you are building firmware with the ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` Kconfig option enabled.

Prerequisites
-------------

Before you begin testing with Apple ecosystem, ensure you have the following:

* iPhone with iOS 26 or later - Required for Aliro-related features in the Apple Home and Wallet apps.
* Apple Home Hub - A HomePod Mini (recommended) or an Apple TV 4K, which acts as both a Matter controller and Thread Border Router.
* Apple Home App - Pre-installed on iOS devices.
* Apple Wallet - Pre-installed on iOS devices.
* nRF Door Lock App build with Matter support - A development kit from Nordic Semiconductor equipped with Aliro door lock firmware that supports Matter.
  For details, see the :ref:`building_and_running` documentation page.

.. note::
   Before commissioning the door lock, ensure that the Apple Home Hub is set up and added to your Apple Home.
   It will act as the Thread Border Router and Matter controller for your home network.

Commissioning the door lock to Apple Home
------------------------------------------

The commissioning process allows you to add the door lock accessory into your Apple Home ecosystem using Matter over Thread.
Complete the following steps to commission the device:

1. Prepare the door lock device:

   a. Flash the Aliro door lock firmware with Matter support enabled to your Nordic development kit (see :ref:`building_and_running` with the ``-DSNIPPET='matter'`` option).
   #. Connect the required hardware (:ref:`NFC reader expansion board<hw_requirements_nfc_reader>`).
   #. Power on the device and connect to the serial console to monitor the commissioning process.
   #. Start the commissioning mode on the device.
      Refer to the :ref:`matter_ui` section for detailed button assignments and LED indicators.

#. Initiate Matter pairing in the Apple Home app:

   a. Open the Home App on your iPhone.
   #. Ensure your iPhone is connected to the same Wi-Fi network as your Apple Home Hub.
   #. Tap the :guilabel:`+` button in the top-right corner of the Home app.
   #. Select :guilabel:`Add Accessory` or :guilabel:`Add Device`.

#. Scan the Matter QR code displayed under the link that appears in the device's serial console after it boots up.
   The Home app will activate the camera for QR code scanning.

   .. note::
      During testing, you might see an Uncertified Accessory warning, indicating the device has not yet been certified by the Connectivity Standards Alliance for Matter compatibility.
      This behavior is expected for development devices.
      If this occurs, tap :guilabel:`Add Anyway` to proceed with commissioning.

   .. figure:: /images/apple_uncertified_accessory.png
      :alt: Uncertified Accessory warning in Apple Home app
      :scale: 30%

      Uncertified Accessory warning during commissioning

#. Complete the accessory setup:

   a. Wait while the Home App establishes a connection and commissions the device.
      The status message will show ``Setting Up...``.
   #. When prompted, assign a location or room for the lock (for example, "Entrance", "Front Door", or "Garage").

      .. figure:: /images/apple_lock_location_setup.png
         :alt: Lock location selection in Apple Home app
         :scale: 30%

         Selecting the lock location during setup

   #. Provide a name for the lock (for example, "Aliro lock" or "Front Door Lock").
   #. Review the suggested settings and tap :guilabel:`Continue` or :guilabel:`Done`.

#. Enable Home Key functionality.
   After successful commissioning, the Home app will automatically prompt you to set up the Home Key feature for Apple Wallet:

   a. Wait for the :guilabel:`Tap to Unlock` setup screen with options for unlock methods.

      .. figure:: /images/apple_tap_to_unlock_setup.png
         :alt: Tap to Unlock setup screen in Apple Home app
         :scale: 30%

         Tap to Unlock configuration screen

   #. Tap :guilabel:`Turn On Tap to Unlock` to enable NFC-based unlocking using your iPhone.
   #. Choose your preferred authentication methods:

      * :guilabel:`Require Face ID or Passcode` - Adds an additional security layer requiring biometric or PIN authentication before unlocking.
      * :guilabel:`Access Codes` - Adds an additional security layer requiring a PIN code to unlock the door lock remotely (through the Matter network).

   #. Tap :guilabel:`Done` to complete the Home Key provisioning.

#. Verify successful commissioning:

   a. Wait for the :guilabel:`Lock Added to Home` confirmation pop-up with a message indicating that you and all home members can now unlock the door.

      .. figure:: /images/apple_lock_added_confirmation.png
         :alt: Lock Added to Home confirmation
         :scale: 30%

         Confirmation that the lock has been successfully added

      The lock should now appear in your Home app within the assigned room, displaying its current state (:guilabel:`Locked` or :guilabel:`Unlocked`).

      .. figure:: /images/apple_home_app_lock_view.png
         :alt: Aliro lock in Apple Home app
         :scale: 30%

         Aliro lock displayed in the Home app

   #. Ensure that a new home card appears in your Apple Wallet app containing the digital key for the lock.

      .. figure:: /images/apple_wallet_hold_near_lock.png
         :alt: Apple Wallet Hold Near Lock instruction
         :scale: 30%

         Apple Wallet containing the newly added Home key

Using the home key for access
------------------------------

Once the lock is successfully commissioned to Apple Home, a virtual home key containing Aliro credentials is automatically provisioned and added to your Apple Wallet.
The home key enables secure, contactless access to the door lock using NFC technology.

Accessing the home key in Apple Wallet
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use your digital home key from Apple Wallet, complete the following:

#. Open the Wallet app on your iPhone.
#. Locate the new home card displaying a house icon.
   This card represents your digital home key and contains the Aliro cryptographic credentials required for secure authentication with the door lock.

Unlocking the door with NFC
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To unlock the door using your iPhone, complete the following steps:

#. Approach the door lock with your iPhone, ensuring the Apple Wallet app is open and the Home Key card is active.
   Depending on the security settings you chose during the initial setup, you may be prompted for biometric authentication (Face ID or Touch ID) or passcode entry to make the card active.

#. Hold your device near the NFC reader antenna on the door lock.
   This is a part of the STM Nucleo NFC reader expansion board attached to the development kit.

   Authentication behavior depends on your chosen security setting:

   * Face ID or Passcode required - Your device will ask for biometric authentication (Face ID or Touch ID) or passcode entry before unlocking.

#. Wait for confirmation:

   * The Apple Wallet will display "Done" message.

     .. figure:: /images/apple_home_key_success.png
        :alt: Home key successfully added to Apple Wallet
        :scale: 30%

        Apple Wallet displaying confirmation after successful unlock with Home key

   * The device serial console outputs an ``ACCESS GRANTED`` log message.
   * The lock status in the Home app updates to :guilabel:`Unlocked`.

Troubleshooting
---------------

If you encounter issues during testing with the Apple ecosystem, refer to the following troubleshooting guidance.

Commissioning issues
~~~~~~~~~~~~~~~~~~~~

If you are facing difficulties during the commissioning process, consider the following steps to diagnose and resolve the issue:

* Device is not discovered by the Home app:

  * Verify if the device is in active commissioning mode (LED indicators should show commissioning state as described in the :ref:`matter_ui`).
  * Ensure Bluetooth is enabled on your iPhone, and that Home app has permission to access location services.
  * Confirm your iPhone is running iOS 26 or later.
  * Ensure that the HomePod mini is powered on, connected to Wi-Fi, and showing as online in the Home app.
  * Attempt to power cycle both the door lock device and the HomePod mini.

* Receiving an "Uncertified Accessory" warning:

  * This is expected behavior for development devices that are not yet certified by the Connectivity Standards Alliance (CSA).
  * Tap :guilabel:`Add Anyway` to proceed with commissioning.
  * Once the product is certified, this warning will no longer appear.

* Commissioning fails or times out:

  * Ensure the HomePod mini is on the same network as your iPhone.
  * Check the device serial console for error messages indicating commissioning failures.
  * Ensure the firmware was built with Matter support enabled (the ``-DSNIPPET='matter'`` option).
  * Ensure the Step-up phase support is enabled (the ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` Kconfig option).
  * Try performing a factory reset of the device and restarting the commissioning process.
  * Ensure the Thread network is functioning correctly on the HomePod mini.

Home Key and Wallet issues
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Problems with the Home Key appearing in Apple Wallet or other related issues can often be resolved by checking a few key settings and configurations:

* Home key does not appear in the Apple Wallet:

  * Ensure you have completed the :guilabel:`Tap to Unlock` setup during the initial commissioning process
  * If you skipped this step, you can enable it later by opening the Home app, tapping the lock, going to the settings icon, and clicking :guilabel:`Tap to Unlock`.
  * Ensure your iPhone has Apple Wallet enabled and is signed in with your Apple ID.
  * Check if iCloud Keychain is enabled by going to iPhone :guilabel:`Settings` → :guilabel:`[Your Name]` → :guilabel:`iCloud` → :guilabel:`Passwords and Keychain`.

* The "Hold Near Lock" message appears but nothing happens:

  * Verify if the NFC reader hardware is properly connected to the development kit (check physical connections).
  * Ensure the firmware includes the NFC transport support by checking the build configuration.
  * Adjust the position of your iPhone relative to the NFC reader antenna to improve connectivity.
  * Check the device serial console for NFC reader initialization messages and any error logs.

* Face ID or passcode prompt does not appear when expected:

  * Review the home key security settings by going to Wallet app → :guilabel:`Home card` → :guilabel:`ⓘ` (info) → :guilabel:`Express Mode settings`.
  * If you wish to use the :guilabel:`Require Face ID or Passcode` option, ensure it is enabled in the settings.

For additional troubleshooting, guidance, and tips for various ecosystems, see the `Matter testing with Apple, Google, and Samsung ecosystems`_ guide.

.. _matter_ui:

Matter door lock user interface
*******************************

This section describes the user interface of the |APP_NAME| when Matter is enabled.

   LED 1:
      .. include:: /include/matter_state_led.txt

   LED 2:
      .. include:: /include/matter_signalling_led.txt

   Button 1:
      .. include:: /include/matter_button.txt

   Button 2:
      * Changes the lock state to the opposite one.


.. note::
   The button and LED numbering differs between development kits.
   On the nRF52840 and nRF5340 boards, the numbering starts from 1.
   On the nRF54L15 and nRF54LM20 boards, it starts from 0.

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

.. note::
   You can upload the :file:`applications/doorlock/docs/certification_assets/Aliro PICS v0.9.3.r2.xml` file to the Test Harness to automatically select tests for execution.
   However, **there is a known bug in this Test Harness revision** that causes the PICS file to select some incorrect tests and skip some needed to execute.

   **Tests that should be executed** (but may be skipped):

   * ``NFC_RDR_NEG_STEPUP_AD_SCHEDULE_IN_ACCESS_RULE_AND_READER``
   * ``NFC_RDR_NEG_STEPUP_AD_SCHEDULE_TIME_VERIFY_REQUIRED``
   * ``NFC_RDR_NEG_STEPUP_AD_TIME_VERIFICATION_REQUIRED``
   * ``BLEUWB_RDR_ADVERTISEMENT_FORMAT``

   **Tests that should not be executed** (but may be selected):

   * ``BLEUWB_RDR_CONTROL_FLOW_RDR_DESCRIPTOR_TAG``

   Always verify that the correct tests are chosen before executing them.
   If you don't upload the PICS file, you can choose the tests manually.

.. figure:: /images/th_test_cases.png
   :scale: 70%
   :alt: Test suites list.

   Test suites list.

Thanks to this, the Test Harness device will be operating as the Aliro User Device and communicate with the Reader device.

For verification, execute the following tests, based on the `aliro-sve-v1.0`_ tag.

The test results shown below were acquired on an nRF5340 DK build using the following command:

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp -- -Dapp_SNIPPET=nfc_uwb_coex_single_spi -DCONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE=y -DCONFIG_DOOR_LOCK_STEP_UP_PHASE=y -DCONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA=y

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

   .. tab:: Bluetooth LE

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

Additional CLI commands
=======================

* To check the revision of the Aliro library on your device, run the following command in the device shell:

.. code-block:: console

   uart:~$ dl info
   Aliro version: v0.2.0-22-g7da4b2e
   NFC reader: ST25R100

* To check the QM35825 SoC firmware version, run the following command in the device shell:

.. code-block:: console

   uart:~$ uwb qm35_fw_version
   13.1.0rc5_lu

.. note::
   This command is available only if you build the application with the ``uwb_qm35`` snippet.

* To list all Kpersistent keys stored in the device, run the following command in the device shell:

.. code-block:: console

   uart:~$ dl kpersistent list
   Number of Kpersistent keys: 1
   Index   ID          Public Key
   --------------------------------
   0       0x00041000  04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa

* To clear a specific Kpersistent key by its index, run the following command in the device shell:

.. code-block:: console

   uart:~$ dl kpersistent clear <index>

   For example:

.. code-block:: console

   uart:~$ dl kpersistent clear 0

* To clear all Kpersistent keys stored in the device, run the following command in the device shell:

.. code-block:: console

   uart:~$ dl kpersistent clear all

.. note::
   These commands are available only if you build the application with the Expedited-fast phase support enabled (the ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig option).

Bluetooth LE Nordic UART Service (NUS)
***************************************

The Nordic UART Service (NUS) is a Bluetooth LE service that allows for remote control of the door lock using predefined commands.
This feature demonstrates the use of an out-of-band access control mechanism, unrelated to the Aliro and Matter protocols.

Enabling the NUS feature
========================

.. tabs::

   .. group-tab:: With Matter disabled (Aliro only)

      Build the application with the ``bt_nus`` snippet.
      For example, if you are using the nRF5340 DK, run the following command:

      .. code-block:: console

         west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=bt_nus

   .. group-tab:: With Matter enabled (Aliro and Matter)

      Build the application with the ``matter`` snippet and enable the ``CONFIG_CHIP_NUS`` Kconfig option.
      For example, if you are using the nRF5340 DK, run the following command:

      .. code-block:: console

         west build -p -b nrf5340dk/nrf5340/cpuapp app -- -DSNIPPET=matter -DCONFIG_CHIP_NUS=y

Using the Bluetooth LE NUS service
==================================

.. tabs::

   .. group-tab:: With Matter disabled (Aliro only)

      The application registers an ``Unlock`` command that can be sent from the `nRF Toolbox mobile application`_.
      You can register additional custom commands for the NUS service by using the ``RegisterCommand`` method from the ``NUSService`` class in application code.

      1. **Pairing**: Connect to the device advertised as `AliroDL` using passkey: ``123456``.
      2. **Send Command**: Send "Unlock" command using NUS RX characteristic.
      3. **Result**: You will see the following output in the device serial console:

         .. code-block:: console

            <inf> door_lock_app: Unlock command received

      Additionally, if you enable the ``CONFIG_ACCESS_DECISION_INDICATOR`` Kconfig option, the access decision indicator (green **LED 1**) lights up for a time specified by the ``CONFIG_RESET_ACCESS_DECISION_INDICATOR_STATE_DELAY_MS`` Kconfig option.

   .. group-tab:: With Matter enabled (Aliro and Matter)

      Follow the detailed instructions on how to use NUS service with Matter provided in the `Matter door lock NUS`_ page.
