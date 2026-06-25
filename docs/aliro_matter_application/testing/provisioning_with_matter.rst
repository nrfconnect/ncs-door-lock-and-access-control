.. _testing_door_lock_provisioning_with_matter:
.. _testing_verification:

Aliro door lock provisioning with Matter
########################################

.. contents::
   :local:
   :depth: 2

This page covers the following methods for testing the Aliro door lock with Matter support:

* :ref:`testing_with_chip_tool` - Using the CHIP Tool command-line interface for commissioning and credential provisioning (intended for development and testing).
* :ref:`testing_with_apple_ecosystem` - Using Apple Home app and Apple Wallet for real-world user experience testing with iOS devices.
* :ref:`testing_with_samsung_ecosystem` - Using Samsung SmartThings and Samsung Wallet for real-world user experience testing with Android devices.

The application allows you to provision Aliro credentials from a Matter controller.
First, ensure that all the necessary software tools and hardware are configured (see :ref:`testing_environment`).
For development kit LEDs, buttons, and commissioning controls, see :ref:`matter_ui` in the :ref:`aliro_matter_access_control_application`.

.. _testing_with_chip_tool:

Testing with Matter CHIP Tool
*****************************

This guide uses CHIP Tool as a Matter controller to commission the door lock and provision Aliro credentials through the command-line interface.
For details, see the `Matter chip-tool guide`_.

.. note::
   This approach is recommended for development, testing, and debugging purposes.

#. Set up the `Thread Border Router`_ with the Nordic Thread coprocessor.

   a. Configure the `Thread radio co-processor`_.

   #. Run the OpenThread Border Router and `form a Thread network using Docker <Running OTBR using Docker_>`_.

   #. Obtain the Thread operational dataset for commissioning devices into the created Thread network by running the following Docker command:

      .. code-block:: console

         sudo docker exec -it otbr sh -c "sudo ot-ctl dataset active -x"

      Save this operational dataset as it will be needed for commissioning the door lock device.

#. Commission the Aliro door lock to the Matter over Thread network.

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

#. Configure the Aliro Reader based on the Test Harness configuration.

   a. Generate an ECC key pair for the Reader by running the following commands:

      .. code-block:: console

         KEY_OUTPUT=$(openssl ecparam -name prime256v1 -genkey | openssl ec -text -noout) && \
         echo "Private Key:" && \
         echo "$KEY_OUTPUT" | sed -n '/priv:/,/pub:/p' | grep -v 'priv:\|pub:' | tr -d ' \n:' && \
         echo -e "\nPublic Key:" && \
         echo "$KEY_OUTPUT" | sed -n '/pub:/,/ASN1/p' | grep -v 'pub:\|ASN1' | tr -d ' \n:'

      See an example output:

      .. code-block:: console

         Private Key:
         9df123f58dd15f6bab71bb6635827faf25100b043cdf6b62c93ea3c244ad4403
         Public Key:
         043c05b91fc09a84ad2ab7940a1b84f09b8ddf5323f1aac0f6568e1c973f37275dc67500a9df08d1bd69ee04e8641d9cbc73a4c1be30eed64def414f8afdc44642

   #. Follow the :ref:`Setting up the Aliro Test Harness <setting_up_the_aliro_test_harness>` section to locate the necessary credentials in the Test Harness project configuration.

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
      * ``<group_resolving_key>`` - Group resolving key of the Reader device. 
        This option corresponds to ``dut_reader_group_resolving_key``, which can be set in the Test Harness project configuration.

      See the following example:

      .. code-block:: console

         ./chip-tool doorlock set-aliro-reader-config hex:9df123f58dd15f6bab71bb6635827faf25100b043cdf6b62c93ea3c244ad4403 hex:043c05b91fc09a84ad2ab7940a1b84f09b8ddf5323f1aac0f6568e1c973f37275dc67500a9df08d1bd69ee04e8641d9cbc73a4c1be30eed64def414f8afdc44642 hex:00113344667799AA00113344667799AA 1 1 --GroupResolvingKey hex:00000000000000000000000000000000 --timedInteractionTimeoutMs 5000

      .. note::
         The ``GroupResolvingKey`` can only be set if the Aliro door lock application is built with Bluetooth LE transport and Ultra-Wideband (UWB) support.
         Currently, the Aliro Test Harness expects the ``GroupResolvingKey`` to be set to all zeros.

#. Add door lock user and ``AliroNonEvictableEndpointKey`` credentials using CHIP Tool.

   a. Add the first user (Home user):

      .. code-block:: console

         ./chip-tool doorlock set-user 0 1 Home 0 1 0 0 1 1 --timedInteractionTimeoutMs 5000

      * OperationType (0 = Add) - ``0``
      * UserIndex - ``1``
      * UserName - ``Home``
      * UserUniqueID - ``0``
      * UserStatus (1 = enabled) - ``1``
      * UserType (0 = unrestricted) - ``0``
      * CredentialRule (0 = single) - ``0``
      * Node ID of the door lock - ``1``
      * Endpoint ID - ``1``
      
   #. Set the first ``AliroNonEvictableEndpointKey`` credential for the Home user:

      .. code-block:: console

         ./chip-tool doorlock set-credential 0 '{"credentialType": 8, "credentialIndex": 1}' hex:<credential-data> 1 null null 1 1 --timedInteractionTimeoutMs 5000

      * OperationType (0 = Add) - ``0``
      * credentialType (8 = AliroNonEvictableEndpointKey) - ``8``
      * credentialIndex - ``1``
      * ``<credential-data>`` - An octet string parameter with the Access Credential public key data.
        It corresponds to ``th_access_credential_public_key``.
      * UserIndex - ``1``
      * UserStatus - ``null``
      * UserType - ``null``
      * Node ID of the door lock - ``1``
      * Endpoint ID - ``1``

      See the following example:

      .. code-block:: console

         ./chip-tool doorlock set-credential 0 '{"credentialType": 8, "credentialIndex": 1}' hex:04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa 1 null null 1 1 --timedInteractionTimeoutMs 5000

   #. Read the status of credential assigned to the user to verify that it was set correctly:

      .. code-block:: console

         ./chip-tool doorlock get-credential-status {"credentialType": 8, "credentialIndex": 1} 1 1

#. Add Aliro Credential Issuer key using CHIP Tool.
   Once you have a user created on the door lock, you can add the Credential Issuer public key to the Reader using the following command:

   .. code-block:: console

      ./chip-tool doorlock set-credential 0 '{"credentialType": 6, "credentialIndex": 1}' hex:<credential-data> 1 null null 1 1 --timedInteractionTimeoutMs 5000

   * OperationType (0 = Add) - ``0``
   * credentialType (6 = AliroCredentialIssuerKey) - ``6``
   * credentialIndex - ``1``
   * ``<credential-data>`` - An octet string parameter with the Credential Issuer public key data.
     It corresponds to ``dut_credential_issuer_public_key``.
   * UserIndex - ``1``
   * UserStatus - ``null``
   * UserType - ``null``
   * Node ID of the door lock - ``1``
   * Endpoint ID - ``1``

   See the following example:

   .. code-block:: console

      ./chip-tool doorlock set-credential 0 '{"credentialType": 6, "credentialIndex": 1}' hex:047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137 1 null null 1 1 --timedInteractionTimeoutMs 5000

.. _testing_with_apple_ecosystem:

Testing with Apple ecosystem
****************************

.. note::
   Support for testing with Apple ecosystem is `experimental <Software maturity_>`_ and is not optimized with respect to performance and power consumption.
   This feature is intended for development and testing purposes only.

This guide demonstrates how to test the Aliro door lock with Matter support using Apple's Home App and Wallet on iPhone.
For additional information about Matter testing with various ecosystems, see the `Matter testing with Apple, Google, and Samsung ecosystems`_ guide.

When testing |APP_NAME| with Apple ecosystem, you can use either:

* NFC - Tap to unlock using NFC technology
* NFC + Bluetooth LE + UWB - Unlock on approach using Bluetooth LE + UWB ranging (in addition to NFC)

Both configurations are supported and can be set up during the commissioning process.

.. note::
   Testing with Apple ecosystem requires both Step-up phase and Expedited-fast phase support to be enabled on the device.
   Both ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` and ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig options are enabled by default when building with Matter support.

Prerequisites
=============

Before you begin testing with Apple ecosystem, ensure you have the following:

* iPhone with iOS 26 or later - It is required for Aliro-related features in the Apple Home and Wallet apps. 
* Apple Home Hub - A HomePod Mini (recommended) or an Apple TV 4K, which acts as both a Matter controller and Thread Border Router.
  You can test the application with iPhone only, however, having an Apple Home Hub is recommended for best experience.
* Apple Home App - Pre-installed on iOS devices.
* Apple Wallet - Pre-installed on iOS devices.
* A development kit from Nordic Semiconductor equipped with |MATTER_ALIRO_APP_NAME| firmware :ref:`that supports Matter <aliro_matter_access_control_application>`.

Building the firmware
=====================

Build the firmware with the appropriate configuration for your testing scenario, for example:

* For NFC testing, run:

  .. code-block:: console

     west build -b nrf54lm20dk/nrf54lm20b/cpuapp applications/matter-aliro-door-lock-app

* For NFC + Bluetooth LE + UWB testing, run:

  .. code-block:: console

     west build -b nrf54lm20dk/nrf54lm20b/cpuapp applications/matter-aliro-door-lock-app -- -Dmatter-aliro-door-lock-app_SNIPPET='uwb_qm35'

.. note::
   Before commissioning the door lock, ensure that the Apple Home Hub is set up and added to your Apple Home.
   It will act as the Thread Border Router and Matter controller for your home network.

Commissioning the door lock to Apple Home
=========================================

The commissioning process allows you to add the door lock accessory into your Apple Home ecosystem using Matter over Thread.
Complete the following steps to commission the device:

1. Prepare the door lock device:

   .. tabs::

      .. tab:: NFC

         a. Flash the Aliro door lock firmware to your Nordic development kit (see :ref:`aliro_matter_access_control_application`).
         #. Connect the required hardware (:ref:`NFC reader expansion board<hw_requirements_nfc_reader>`).
         #. Power on the device and connect to the serial console to monitor the commissioning process.
         #. Start the commissioning mode on the device.
            Refer to the :ref:`matter_ui` section for detailed button assignments and LED indicators.

      .. tab:: NFC + Bluetooth LE + UWB

         a. Flash the Aliro door lock firmware with UWB support enabled to your Nordic development kit (see :ref:`aliro_matter_access_control_application` with the ``-Dmatter-aliro-door-lock-app_SNIPPET='uwb_qm35'`` option).
         #. Connect the required hardware:

            * :ref:`NFC reader expansion board<hw_requirements_nfc_reader>` (required for NFC unlock)
            * :ref:`UWB module<hw_requirements_uwb_module>` (required for UWB ranging)

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

   .. tabs::

      .. tab:: NFC

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

      .. tab:: NFC + Bluetooth LE + UWB

         a. Wait for the :guilabel:`Express Mode` setup screen with options for unlock methods.

            .. figure:: /images/apple_express_mode_setup.png
               :alt: Express Mode setup screen in Apple Home app
               :scale: 30%

               Express Mode configuration screen

         #. Tap :guilabel:`Turn On Express Mode` to enable both NFC-based tap to unlock and Bluetooth LE + UWB-based unlock on approach using your iPhone.
         #. Accept the prompt to enable proximity-based unlocking during commissioning.
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
=============================

Once the lock is successfully commissioned to Apple Home, a virtual home key containing Aliro credentials is automatically provisioned and added to your Apple Wallet.

Accessing the home key in Apple Wallet
--------------------------------------

To use your digital home key from Apple Wallet, complete the following:

#. Open the Wallet app on your iPhone.
#. Locate the new home card displaying a house icon.
   This card represents your digital home key and contains the Aliro cryptographic credentials required for secure authentication with the door lock.

Unlocking the door
------------------

.. tabs::

   .. tab:: NFC

      To unlock the door using your iPhone with NFC, complete the following steps:

      #. Approach the door lock with your iPhone, ensuring the Apple Wallet app is open and the Home Key card is active.
         Depending on the security settings you chose during the initial setup, you may be prompted for biometric authentication (Face ID or Touch ID) or passcode entry to make the card active.

      #. Hold your device near the NFC reader antenna on the door lock.
         This is a part of the STM Nucleo NFC reader expansion board attached to the development kit.

         Authentication behavior depends on your chosen security setting:

         * Face ID or Passcode required - Your device will ask for biometric authentication (Face ID or Touch ID) or passcode entry before unlocking.

      #. Wait for confirmation:

         * The Apple Wallet will display the following message:

           .. figure:: /images/apple_home_key_success.png
              :alt: Home key successfully added to Apple Wallet
              :scale: 30%

              Apple Wallet displaying confirmation after successful unlock with Home key

         * The device serial console outputs an ``ACCESS GRANTED`` log message.
         * The lock status in the Home app updates to :guilabel:`Unlocked`.

   .. tab:: NFC + Bluetooth LE + UWB

      To unlock the door using your iPhone with NFC + Bluetooth LE + UWB, you can use either method:

      Using NFC (tap to unlock):

      #. Approach the door lock with your iPhone, ensuring the Apple Wallet app is open and the Home Key card is active.
         Depending on the security settings you chose during the initial setup, you may be prompted for biometric authentication (Face ID or Touch ID) or passcode entry to make the card active.

      #. Hold your device near the NFC reader antenna on the door lock.
         This is a part of the STM Nucleo NFC reader expansion board attached to the development kit.

      #. Wait for confirmation - The Apple Wallet will display "Done" message, the device serial console outputs an ``ACCESS GRANTED`` log message, and the lock status in the Home app updates to :guilabel:`Unlocked`.

      Using Bluetooth LE + UWB (unlock on approach):

      #. Ensure your iPhone has Bluetooth enabled and is within range of the door lock.
         The door will automatically unlock when your iPhone is within the configured distance threshold.

      #. Approach the door lock with your iPhone.
         The device will automatically detect your proximity using UWB ranging.

      #. Wait for automatic unlock:

         * The door will unlock when your iPhone is within ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` (default: 150 cm).
         * The device serial console outputs an ``ACCESS GRANTED`` log message.
         * The lock status in the Home app updates to :guilabel:`Unlocked`.

      #. To lock the door again:

         * Move your iPhone beyond ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM + CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_EXIT_MARGIN_CM`` (default: 150 cm + 30 cm = 180 cm).
         * The door will automatically lock when all active UWB sessions are out of range.
         * The device serial console outputs a lock confirmation message.
         * The lock status in the Home app updates to :guilabel:`Locked`.

      .. note::
         The exit margin prevents rapid toggling when the distance fluctuates around the threshold.
         This ensures stable lock and unlock behavior.

         The behavior of the door lock can be customized and depends on various factors, including the behavior after unlocking.
         In the example configuration with ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_DISABLED``, the UWB session is terminated by the User Device (iPhone) after a certain period of time.
         The User Device resumes the session either when the Door Lock changes state from Non-secured to Secured (Unlocked => Locked), which is detected by the iPhone over Bluetooth LE, or through motion detection on the User Device side.

Troubleshooting
===============

If you encounter issues during testing with the Apple ecosystem, refer to the following troubleshooting guidance.

Commissioning issues
--------------------

If you are facing difficulties during the commissioning process, consider the following steps to diagnose and resolve the issue:

* Device is not discovered by the Home app:

  * Verify that the device is in active commissioning mode (LED indicators should show commissioning state as described in the :ref:`matter_ui`).
  * Ensure Bluetooth is enabled on your iPhone, and that the Home app has permission to access location services.
  * Confirm your iPhone is running iOS 26 or later.
  * Ensure that the HomePod mini is powered on, connected to Wi-Fi, and showing as online in the Home app.
  * Attempt to power cycle both the door lock device and the HomePod mini.

* Receiving an ``Uncertified Accessory`` warning:

  * This is expected behavior for development devices that are not yet certified by the Connectivity Standards Alliance (CSA).
  * Tap :guilabel:`Add Anyway` to proceed with commissioning.
  * Once the product is certified, this warning will no longer appear.

* Commissioning fails or times out:

  * Ensure the HomePod mini is on the same network as your iPhone.
  * Check the device serial console for error messages indicating commissioning failures.
  * Ensure the firmware was built with the correct application (``applications/matter-aliro-door-lock-app``).
  * Ensure the Step-up phase support is enabled (the ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` Kconfig option).
  * Try performing a factory reset of the device and restarting the commissioning process.
  * Ensure the Thread network is functioning correctly on the HomePod mini.

Home Key and Wallet issues
--------------------------

Problems with the Home Key appearing in Apple Wallet or other related issues can often be resolved by checking a few key settings and configurations:

* Home key does not appear in the Apple Wallet:

  .. tabs::

     .. tab:: NFC

        * Ensure you have completed the :guilabel:`Tap to Unlock` setup during the initial commissioning process
        * If you skipped this step, you can enable it later by opening the Home app, tapping the lock, going to the settings icon, and clicking :guilabel:`Tap to Unlock`.
        * Ensure your iPhone has Apple Wallet enabled and is signed in with your Apple ID.
        * Check if iCloud Keychain is enabled by going to iPhone :guilabel:`Settings` → :guilabel:`[Your Name]` → :guilabel:`iCloud` → :guilabel:`Passwords and Keychain`.

     .. tab:: NFC + Bluetooth LE + UWB

        * Ensure you have completed the :guilabel:`Express Mode` setup during the initial commissioning process
        * If you skipped this step, you can enable it later by opening the Home app, tapping the lock, going to the settings icon, and clicking :guilabel:`Express Mode`.
        * Ensure your iPhone has Apple Wallet enabled and is signed in with your Apple ID.
        * Check if iCloud Keychain is enabled by going to iPhone :guilabel:`Settings` → :guilabel:`[Your Name]` → :guilabel:`iCloud` → :guilabel:`Passwords and Keychain`.

* The *Hold Near Lock* message appears but nothing happens (NFC unlock):

  * Verify that the NFC reader hardware is properly connected to the development kit (check physical connections).
  * Ensure the firmware includes the NFC transport support by checking the build configuration.
  * Adjust the position of your iPhone relative to the NFC reader antenna to improve connectivity.
  * Check the device serial console for NFC reader initialization messages and any error logs.

* The door does not unlock automatically when approaching (Bluetooth LE + UWB):

  * Verify that the UWB module is properly connected to the development kit (check physical connections).
  * Ensure the firmware includes UWB support by checking the build configuration (``-Dmatter-aliro-door-lock-app_SNIPPET='uwb_qm35'``).
  * Ensure your iPhone is within ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` (default: 150 cm) of the door lock.
  * Check the device serial console for UWB ranging messages and distance measurements.
  * Verify that Bluetooth is enabled on your iPhone and that the Home app has permission to access location services.

* Face ID or passcode prompt does not appear when expected:

  * Review the home key security settings by going to Wallet app → :guilabel:`Home card` → :guilabel:`ⓘ` (info) → :guilabel:`Express Mode settings`.
  * If you wish to use the :guilabel:`Require Face ID or Passcode` option, ensure it is enabled in the settings.

For additional troubleshooting, guidance, and tips for various ecosystems, see the `Matter testing with Apple, Google, and Samsung ecosystems`_ guide.

.. _testing_with_samsung_ecosystem:

Testing with Samsung SmartThings ecosystem
******************************************

SmartThings and Samsung Wallet support Matter and Aliro.
The related software has been deployed to supported devices.

To receive support for this feature, your door lock product must pass the Works With SmartThings certification and Samsung Wallet QA testing.
Samsung operates a partner program to assist with these preparations.

To test Aliro with the SmartThings ecosystem, you must have a special Matter lock driver running on the SmartThings hub.

.. note::
   This guide explains how to test the application with the Samsung ecosystem using Aliro over NFC only.
   UWB is out of scope.
   Enabling Aliro over Bluetooth LE and UWB on a Samsung phone requires an additional software package on the phone.
   Contact Samsung for details.

For additional information about Matter testing with various ecosystems, see the `Matter testing with Apple, Google, and Samsung ecosystems`_ guide.

Prerequisites
=============

See the `Samsung Wallet device compatibility list`_ for supported Samsung phone models and their NFC and UWB capabilities.

The SmartThings hub can be any model that supports SmartThings and Matter.

For reference, the following setup was used during internal testing:

* Samsung Galaxy S24+ phone running Android 16 and One UI 8.5
* Aeotec SmartThings hub (model IM6001-V3P22, hub name ``V3_HUB``) with:

  * Matter SDK version: 1.5-0
  * Firmware version: 000.060.00012
  * Hub app version: 1.0.49-13

* A special version of the Samsung Wallet app (contact Samsung representatives to obtain the proper build)
* The |MATTER_ALIRO_APP_NAME| build with NFC support only (see :ref:`aliro_matter_access_control_application`)

Installing the Matter lock driver on the hub
============================================

Before commissioning the door lock, install a Matter lock driver on the SmartThings hub.
The driver must support your device's Vendor ID (VID) and Product ID (PID).

Complete the following steps to create and enroll the driver:

#. Fork the `SmartThingsEdgeDrivers`_ repository.

#. Add your device's VID, PID, brand, and model name to the `fingerprints.lua file`_.

   .. figure:: /images/testing_with_samsung/driver_lua_file.png
      :alt: fingerprints.lua file with device VID and PID
      :scale: 70%

      Adding the device fingerprint to the Matter lock driver

#. Create a pull request to the main branch.

   You can start testing before the pull request is merged:

   a. After the pull request is created, an invitation link is generated automatically.
   #. Use the invitation link to enroll the channel on your hub.
   #. You can begin testing immediately without waiting for the pull request to be merged.

   .. figure:: /images/testing_with_samsung/enrol_driver.png
      :alt: Enrolling the generated driver channel on the SmartThings hub
      :scale: 70%

      Enrolling the driver channel on the hub

Building the firmware
=====================

Build the |MATTER_ALIRO_APP_NAME| with NFC support only, using the same Matter VID and PID as in the hub lock driver.

For example, to keep the default Product ID and change only the Vendor ID, run the following command:

.. code-block:: console

   west build -b nrf54lm20dk/nrf54lm20a/cpuapp applications/matter-aliro-door-lock-app -p -- \
     -DCONFIG_CHIP_DEVICE_VENDOR_ID=4735 \
     -DCONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED=y \
     -DCONFIG_CHIP_FACTORY_DATA_GENERATE_CD=y

.. note::
   When you change the default VID or PID, factory data must be regenerated.
   Enable ``CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED`` and ``CONFIG_CHIP_FACTORY_DATA_GENERATE_CD`` Kconfig options in the build command or in :file:`prj.conf`.
   You must also have the ``chip-cert`` tool available on your ``PATH`` when building with generated certificates.

Commissioning the door lock to Samsung SmartThings
==================================================

The commissioning process allows you to add the door lock accessory into your SmartThings ecosystem using Matter over Thread.
Complete the following steps to commission the device:

#. Prepare the door lock device:

   a. Flash the Aliro door lock firmware with NFC support to your Nordic development kit (see :ref:`aliro_matter_access_control_application`).
   #. Connect the :ref:`NFC reader expansion board <hw_requirements_nfc_reader>`.
   #. Power on the device and connect to the serial console to monitor the commissioning process.

#. Open a terminal on your computer and locate the Matter QR code link printed in the device serial console after boot.

#. Open the SmartThings app on your Samsung phone.

   .. figure:: /images/testing_with_samsung/1_smart_things_app_main.jpg
      :alt: SmartThings app main screen
      :scale: 30%

      SmartThings app main screen

#. Tap the :guilabel:`+` button in the upper-right corner.

   .. figure:: /images/testing_with_samsung/2_adding_device.jpg
      :alt: Add device screen in SmartThings app
      :scale: 30%

      Adding a new device in SmartThings

#. Select :guilabel:`Scan QR code`.

   .. figure:: /images/testing_with_samsung/3_scan_qr_code.jpg
      :alt: Scan QR code option in SmartThings app
      :scale: 30%

      Scan QR code option

#. Scan the Matter QR code from the link displayed in the serial console.

   .. figure:: /images/testing_with_samsung/4_prepare_device.jpg
      :alt: QR code scanning screen in SmartThings app
      :scale: 30%

      Scanning the Matter QR code

#. Start Bluetooth LE advertising on the device by pressing **Button 1**.
   Refer to the :ref:`matter_ui` section for detailed button assignments and LED indicators.

#. When testing a non-certified Matter device, a notification pop-up appears.
   Tap :guilabel:`Continue` to proceed.

   .. note::
      This warning is expected for development devices that are not yet certified by the Connectivity Standards Alliance (CSA).
      Once the product is certified, this warning will no longer appear.

   .. figure:: /images/testing_with_samsung/5_not_certified_device.jpg
      :alt: Non-certified Matter device warning in SmartThings app
      :scale: 30%

      Non-certified device warning during commissioning

#. Wait while SmartThings adds the device to your home.

   .. figure:: /images/testing_with_samsung/6_checking_network.jpg
      :alt: SmartThings checking network during device setup
      :scale: 30%
      :align: center

      Checking network connectivity during setup

   .. figure:: /images/testing_with_samsung/7_almost_done.jpg
      :alt: SmartThings almost done adding the device
      :scale: 30%
      :align: center

      Device setup in progress

#. When prompted, set a PIN code for opening the lock, or skip this step.

   .. figure:: /images/testing_with_samsung/8_set_pincode.jpg
      :alt: Set PIN code screen in SmartThings app
      :scale: 30%
      :align: center

      Setting a PIN code for the lock

#. Confirm that the device has been added successfully.

   .. figure:: /images/testing_with_samsung/9_success.jpg
      :alt: Device added successfully in SmartThings app
      :scale: 30%
      :align: center

      Device added successfully

#. Tap :guilabel:`Done`.
   SmartThings downloads the plugin used to control your device.

   .. figure:: /images/testing_with_samsung/10_downloading_plugin.jpg
      :alt: Downloading device control plugin in SmartThings app
      :scale: 30%
      :align: center

      Downloading the device control plugin

#. Wait until the device is ready in SmartThings.

   .. figure:: /images/testing_with_samsung/11_ready_device.jpg
      :alt: Door lock ready in SmartThings app
      :scale: 30%
      :align: center

      Door lock ready in SmartThings

Using the digital key from Samsung Wallet
=========================================

After successful commissioning, add an Aliro digital key to Samsung Wallet and use it to unlock the door over NFC.

Adding the digital key to Samsung Wallet
----------------------------------------

Complete the following steps:

#. In the SmartThings device view, tap the :guilabel:`Wallet` icon at the bottom of the screen.

   .. figure:: /images/testing_with_samsung/12_add_key_start.jpg
      :alt: Wallet icon in SmartThings device view
      :scale: 30%
      :align: center

      Starting digital key setup from SmartThings

   .. figure:: /images/testing_with_samsung/13_adding_key.jpg
      :alt: Add digital key to Samsung Wallet screen
      :scale: 30%
      :align: center

      Adding the digital key to Samsung Wallet


#. Tap :guilabel:`Start` to add the key to Samsung Wallet.

#. When the digital key has been added successfully, tap :guilabel:`Done`.

   .. figure:: /images/testing_with_samsung/14_added_key.jpg
      :alt: Digital key added to Samsung Wallet confirmation
      :scale: 30%
      :align: center

      Digital key added to Samsung Wallet

   .. figure:: /images/testing_with_samsung/15_ready_wallet.jpg
      :alt: Digital key ready in Samsung Wallet
      :scale: 30%
      :align: center

      Digital key ready in Samsung Wallet

Unlocking the door with NFC
---------------------------

To unlock the door using your Samsung phone with NFC, complete the following steps:

#. Open Samsung Wallet and select the door lock digital key card.

#. Tap the NFC reader of the door lock with your phone.
   The NFC reader is part of the STM Nucleo NFC reader expansion board attached to the development kit.

#. Confirm that the door unlocks:

   .. figure:: /images/testing_with_samsung/16_unlock_confirm.jpg
      :alt: Unlock confirmation in Samsung Wallet
      :width: 300px
      :align: center

      Unlock confirmation in Samsung Wallet

   .. figure:: /images/testing_with_samsung/17_door_unlocked.jpg
      :alt: Door unlocked confirmation in SmartThings app
      :scale:  30%
      :align: center

      Door unlocked in SmartThings

   * The device serial console outputs an ``ACCESS GRANTED`` log message.
   * The lock status in the SmartThings app updates to :guilabel:`Unlocked`.

Aliro and Matter lock state synchronization
*******************************************

When the door lock is unlocked through Aliro, the lock state automatically synchronizes with the Matter door-lock server cluster.
This synchronization ensures that User Devices in the Matter network are notified of lock state changes in real time.

1. To verify the current lock state, run the following command:

   .. code-block:: console

      ./chip-tool doorlock read lock-state 1 1

#. After executing the command, find the lock state in the response, for example:

   .. code-block:: console

      [1757071349.943] [949263:949265] [TOO]   LockState: 1

   The response indicates that the lock is locked.

#. Execute a test, for example, ``BLEUWB_RDR_EXPEDITED_STANDARD_PHASE`` with Bluetooth LE + UWB transport (see :ref:`Verification and testing procedures <testing_verification_and_testing>`) using the Test Harness.
   If the test is successful, rerun the ``./chip-tool doorlock read lock-state 1 1`` command.

   You should see the following status in the response:

   .. code-block:: console

      [1757072050.821] [951199:951201] [TOO]   LockState: 2

   The response indicates that the lock is unlocked.
