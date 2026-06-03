.. _matter_door_lock_application:

Matter Door Lock Application
############################

.. contents::
   :local:
   :depth: 2

The Matter |matter_name| Application demonstrates the usage of the `Matter`_ application layer to build a door lock device with one basic bolt.
You can use it as a reference for creating your application.

This device works as a Matter accessory device, meaning it can be paired and controlled remotely over a Matter network built on top of a low-power 802.15.4 (Thread) protocol.
This device works as a Thread `Sleepy End Device`_.
See the `Adding clusters to Matter application`_ page for an overview of the process you need to follow.

Requirements
************

The application supports the following development kits:

.. list-table:: Supported hardware platforms
   :widths: auto
   :header-rows: 1

   * - Hardware platforms
     - PCA
     - Board name
     - Board target
   * - `nRF54LM20 DK`_
     - PCA10184
     - `nrf54lm20dk`_
     - | ``nrf54lm20dk/nrf54lm20b/cpuapp``
       | ``nrf54lm20dk/nrf54lm20a/cpuapp``
   * - `nRF54L15 DK`_
     - PCA10156
     - `nrf54l15dk`_
     - ``nrf54l15dk/nrf54l15/cpuapp``
   * - `nRF5340 DK`_
     - PCA10095
     - `nrf5340dk`_
     - ``nrf5340dk/nrf5340/cpuapp``
   * - `nRF52840 DK`_
     - PCA10056
     - `nrf52840dk`_
     - ``nrf52840dk/nrf52840``

If you want to commission the |matter_name| device and control it remotely through a Thread network, you need to set-up the `Thread Border Router`_ and control it with the `chip-tool <Matter chip-tool guide_>`_, or use a commercial ecosystem controller.
When this happens, you will also be able to control it through a Matter controller device `configured on PC or smartphone <Matter testing_>`_.
This requires additional hardware depending on the setup you choose.

.. note::

    Matter requires the GN tool. 
    If you are updating from the |NCS| version earlier than v1.5.0, see the `GN installation instructions <GN tool_>`_.

IPv6 network support
====================

Matter over Thread (IPv6 network) is supported for the |matter_dks_thread|.

Overview
********

The application uses buttons for changing the lock and device states, and LEDs to show the state of these changes.
You can test it in the following ways:

* Standalone, using a single DK that runs the |matter_name| application.
* Remotely over the Thread protocol, which in either case requires more devices, including a Matter controller that you can configure either on a PC or a mobile device.

.. _matter_door_lock_application_features:

Matter Door Lock Application features
=====================================

The |matter_name| application implements the following features:

* |matter_name| credentials - Support for |matter_name| credentials and users.
* Matter Bluetooth LE with Nordic UART Service - Support for Matter Bluetooth LE with Nordic UART Service.
* Scheduled timed access - Support for scheduled timed access.

Door Lock application credentials
---------------------------------

.. toggle::

   By default, the application supports only PIN code credentials, but it is possible to implement support for other |matter_name| credential types by using the ``MatterAccess`` module.
   The credentials can be used to control remote access to the bolt lock.
   The PIN code assigned by the Matter controller is stored persistently, which means that it can survive a device reboot.
   The secure storage implementation is enabled by default (``CONFIG_DOOR_LOCK_MATTER_ACCESS_STORAGE_PROTECTED_STORAGE``) to store credentials and other |matter_type|'s configuration data.

   The application supports multiple |matter_name| users and PIN code credentials.
   The following Kconfig options control the limits of the users and credentials that can be added to the |matter_name|:

   * ``CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_USERS`` - Maximum number of users supported by the |matter_name|.
   * ``CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER`` - Maximum number of credentials that can be assigned to one user.
   * ``CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_TYPE`` - Maximum number of credentials in total.
   * ``CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_CREDENTIAL_LENGTH`` - Maximum length of a single credential in bytes.

.. _matter_lock_app_ble_nus:

Matter Bluetooth LE with Nordic UART Service
--------------------------------------------

.. toggle::

   The |REPO_NAME| lets you extend the |matter_name| with an additional Bluetooth LE service (`Nordic UART Service (NUS)`_) and use it, even when the device is not connected to a Matter network.
   Using NUS, you can declare commands specific to a |matter_type| and use them to control the device remotely through Bluetooth LE.
   The connection between the device and the Bluetooth controller is secured and requires providing a passcode to pair the devices.

   In the |matter_full_name|, you can use the following commands with the Bluetooth LE with NUS:

   * ``Lock`` - To lock the door of the connected device.
   * ``Unlock`` - To unlock the door of the connected device.

   If the device is already connected to the Matter network, the notification about changing the lock state will be sent to the Bluetooth controller.

.. _matter_lock_scheduled_timed_access:

Scheduled timed access
----------------------

.. toggle::

   The scheduled timed access feature is an optional |matter_type| feature that can be applicable to all available lock users.
   You can use the scheduled timed access feature to allow guest users of the home to access the lock at the specific scheduled times.
   To use this feature, you need to create at least one user on the application device, and assign credentials to that user.
   For more information about setting user credentials, see the *Saving users and credentials* on |matter_name| devices section of the `Matter chip-tool guide`_ in the `Matter documentation`_ set, and the :ref:`matter_door_lock_application_testing` section.
   You can schedule the following types of timed access:

      * ``Week-day`` - Restricts access to a specified time window on certain days of the week for a specific user.
        This schedule grants repeated access each week.
        When the schedule is cleared, the user is granted unrestricted access.

      * ``Year-day`` - Restricts access to a specified time window on a specific date window.
        This schedule grants access only once, and does not repeat.
        When the schedule is cleared, the user is granted unrestricted access.

      * ``Holiday`` - Sets up a holiday operating mode in the lock device.
        You can choose one of the following operating modes:

         * ``Normal`` - The lock operates normally.
         * ``Vacation`` - Only remote operations are enabled.
         * ``Privacy`` - All external interactions with the lock are disabled.
           Can only be used if the lock is in the locked state.
           Manually unlocking the lock changes the mode to ``Normal``.
         * ``NoRemoteLockUnlock`` - All remote operations with the lock are disabled.
         * ``Passage`` - The lock can be operated without providing a PIN.
           This option can be used, for example, for employees during working hours.

   To use the scheduled timed access feature, :ref:`enable the Schedules support <matter_lock_app_configuration_advanced>`.

   See the :ref:`matter_door_lock_application_testing` section of this application for more information about testing the scheduled timed access feature.

.. _matter_lock_app_configuration:
.. _matter_lock_app_custom_configs:

Configuration
*************

This section describes the configuration options for the |matter_type|.

|config|

The application uses a :file:`prj.conf` configuration file located in the sample root directory for the default configuration.
It also provides additional files for different custom configurations.
When you build the sample, you can select one of these configurations using the ``FILE_SUFFIX`` variable.

See `Custom configurations`_ and `Providing CMake options <CMake options_>`_ for more information.

.. note::

    |matter_unique_discriminator_note|

The |matter_type| supports the following build configurations:

.. list-table:: |matter_name| build configurations
   :widths: auto
   :header-rows: 1

   * - Configuration
     - File name
     - :makevar:`FILE_SUFFIX`
     - Supported board
     - Description
   * - Debug (default)
     - :file:`prj.conf`
     - No suffix
     - All from `Requirements`_
     - Debug version of the application.

       Enables additional features for verifying the application behavior, such as logs.
   * - Release
     - :file:`prj_release.conf`
     - ``release``
     - All from `Requirements`_
     - Release version of the application.

       Enables only the necessary application functionalities to optimize its performance.

.. _matter_lock_app_configuration_advanced:

Advanced configuration options
==============================

This section describes advanced configuration options that you can apply in this |matter_type|.
Use the ``click to show`` toggle to expand the content.

Device firmware upgrade support
-------------------------------

.. toggle::

   The |matter_type| supports device firmware upgrade (DFU) over-the-air (OTA) using the following protocols:

   * `Matter OTA`_ update protocol that uses the Matter operational network for querying and downloading a new firmware image.
   * Simple Management Protocol (SMP) over Bluetooth LE.
     In this case, the DFU can be done either using a smartphone application or a PC command-line tool.
     This protocol is not part of the Matter specification.

   In both cases, the `MCUboot`_ secure bootloader is used to apply the new firmware image.

   The DFU over Matter is enabled by default.
   Additionally, you can enable the DFU over SMP by using the ``-DCONFIG_CHIP_DFU_OVER_BT_SMP=y`` build flag.

   See `CMake options`_ for instructions on how to add these options to your build.

   The following platforms require external flash memory to perform the DFU:

   * nRF52840 DK
   * nRF5340 DK
   * nRF54L15 DK

   You can run DFU without external flash memory on the nRF54LM20 DK using the `MCUboot image compression`_ feature.

   .. tabs::

      .. group-tab:: |nRFVSC|

         When building with |nRFVSC|, add your desired *dfu_build_flag* to :guilabel:`Extra CMake arguments`.
         For example add ``-DCONFIG_CHIP_DFU_OVER_BT_SMP=y`` to enable DFU over BT SMP.

      .. group-tab:: Command line

         When building on the command line, run the following command with *board_target* replaced with the board target name of the hardware platform you are using (see `Requirements`_), and *dfu_build_flag* replaced with the desired DFU build flag:

         .. parsed-literal::
            :class: highlight

            west build -b *board_target* -- *dfu_build_flag*

         For example:

         .. code-block:: console

            west build -b nrf54l15dk/nrf54l15/cpuapp -- -DCONFIG_CHIP_DFU_OVER_BT_SMP=y

Factory data support
--------------------

.. toggle::

   In this |matter_type|, factory data support specific to the |NCS| is enabled by default for all configurations.
   This means that a new factory data set will be automatically generated when building for the target board.

   To disable factory data support, set the following Kconfig options to ``n``:

   * ``CONFIG_CHIP_FACTORY_DATA``
   * ``SB_CONFIG_MATTER_FACTORY_DATA_GENERATE``

   To learn more about factory data, read the `Factory provisioning`_ user guide.

Custom board with Nordic SoC
----------------------------

.. toggle::

   To prepare the |matter_type| to work with a custom board, complete the following steps:

      1. Refer to the `Create your board directory`_ Zephyr guide and complete the instructions.
      #. Modify the contents of the :file:`board.yaml` file according to the `Write your board YAML`_ user guide.
      #. Update the `Write your devicetree`_ (all `.dts` and `.dtsi` files) to match your board's requirements.
      #. Write Kconfig files to enable all required Kconfig options for your board.
      #. If your device uses external flash, add its devicetree definition under the :file:`board/<board_name>_<soc_name>.overlay` file, and set ``nordic,pm-ext-flash`` in the devicetree's ``chosen`` configuration.
      #. Refer to the `Advanced Matter Kconfig options`_ user guide, create your list of advanced configurations for your board, and apply the selected Kconfig options in the :file:`prj.conf` file.
      #. See the `List of threads used in Matter application`_ and adjust stack sizes according to your board and project requirements.
      #. A custom board does not have support for LEDs and buttons by default.
         Therefore, you need to provide your own implementation of the :file:`nrf/samples/matter/common/src/board/board.cpp` board file.

   For more information, see the following guides:

      * `Board porting guide`_ and `Custom board definitions`_ to learn how to create a custom board directory.
      * `Optimizing memory usage in Matter applications`_ to learn how to optimize memory on your board.
      * `Advanced Matter Kconfig options`_ to learn about Matter configuration.
      * `Matter hardware and memory requirements`_ to learn about hardware requirements for Nordic Development Kits and to use as a reference when planning your custom board.

Internal memory only
--------------------

.. toggle::

    For the |matter_dks_internal|, you can configure the |matter_type| to use only the internal RRAM for storage.
    It applies to the DFU as well, which means that both the currently running firmware and the new firmware to be updated will be stored within the device's internal RRAM memory.

    The DFU image fits in the internal flash memory if you use `MCUboot image compression <Bootloader configuration in Matter_>`_.

    This configuration is disabled by default for the Matter |matter_name| |matter_type|.
    To enable it, set the ``FILE_SUFFIX`` CMake option to ``internal``.

    To build the |matter_type| for the nRF54LM20 DK with support for Matter OTA DFU and DFU over Bluetooth SMP, and using internal RRAM only:

    .. tabs::

       .. group-tab:: |nRFVSC|

          Add ``-DCONFIG_CHIP_DFU_OVER_BT_SMP=y -DFILE_SUFFIX=internal`` to :guilabel:`Extra CMake arguments` in your build configuration.

       .. group-tab:: Command line

          .. code-block:: console

             west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DCONFIG_CHIP_DFU_OVER_BT_SMP=y -DFILE_SUFFIX=internal

    To build the |matter_type| for the same purpose in the ``release`` configuration:

    .. tabs::

       .. group-tab:: |nRFVSC|

          Add ``-DCONFIG_CHIP_DFU_OVER_BT_SMP=y -DFILE_SUFFIX=internal -Dmatter-door-lock-app_EXTRA_CONF_FILE=prj_release.conf`` to :guilabel:`Extra CMake arguments` in your build configuration.

          .. code-block:: console

             west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DCONFIG_CHIP_DFU_OVER_BT_SMP=y -DFILE_SUFFIX=internal -Dmatter-door-lock-app_EXTRA_CONF_FILE=prj_release.conf

    In this case, the size of the MCUboot secondary partition used for storing the new application image is approximately 30-40% smaller than it would be when using a configuration with external flash memory support.

.. _matter_lock_app_configuration_nus:

Enabling Matter Bluetooth LE with Nordic UART Service
-----------------------------------------------------

.. toggle::

    To enable the `Nordic UART Service (NUS)`_ feature, build with the ``bt_nus`` snippet (for example, add ``-Dmatter-door-lock-app_SNIPPET=bt_nus`` to your CMake arguments).

    .. note::

      This |matter_type| supports one Bluetooth LE connection at a time.
      Matter commissioning, DFU, and NUS over Bluetooth LE must be run separately.

   The |matter_name|'s Bluetooth LE service extension with NUS requires a secure connection with a smartphone, which is established using a security PIN code.
   The pairing passkey is controlled by the ``CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY`` Kconfig option.
   Its default is ``123456`` unless you override it at build time (for example, ``-DCONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY=112233``).
   During the pairing procedure, the device will print the passkey to the log console in the following way:

   .. code-block:: console

      PROVIDE THE FOLLOWING CODE IN YOUR MOBILE APP: 123456

   See the :ref:`matter_door_lock_application_testing` section for more information about how to test this feature.

.. _matter_lock_app_schedules:

Scheduled timed access
----------------------

.. toggle::

   To enable the scheduled timed access feature:

   .. tabs::

      .. group-tab:: |nRFVSC|

         Add ``-DEXTRA_CONF_FILE=schedules.conf`` to :guilabel:`Extra CMake arguments` in your build configuration.

      .. group-tab:: Command line

         Run the following command with *board_target* replaced with the board target name:

         .. parsed-literal::
            :class: highlight

            west build -b *board_target* -p -- -DEXTRA_CONF_FILE=schedules.conf

User interface
**************

This section describes the user interface available on the development kit in this |matter_type|.

Development kit interface
=========================

This |matter_type| implements the following interface available on a development kit.

LED 1:
  Shows the overall state of the device and its connectivity.
  The following states are possible:

  * Short Flash On (50 ms on/950 ms off) - The device is in the unprovisioned (unpaired) state and is waiting for a commissioning application to connect.
  * Rapid Even Flashing (100 ms on/100 ms off) - The device is in the unprovisioned state and a commissioning application is connected over Bluetooth LE.
  * Solid On - The device is fully provisioned.

LED 2:
   Shows the state of the lock.
   The following states are possible:

   * Solid On - The bolt is extended and the door is locked.
   * Off - The bolt is retracted and the door is unlocked.
   * Rapid Even Flashing (50 ms on/50 ms off during 2 s) - The simulated bolt is in motion from one position to another.

     Additionally, the LED starts blinking evenly (500 ms on/500 ms off) when the Identify command of the Identify cluster is received on the endpoint ``1``.
     The command's argument can be used to specify the duration of the effect.

Button 1:
   Depending on how long you press the button:

   * If pressed for less than three seconds:

     * If the device is not provisioned to the Matter network, it initiates the Simple Management Protocol (SMP) server and Bluetooth LE advertising for Matter commissioning.
       After that, the Device Firmware Update (DFU) over Bluetooth Low Energy can be started.
       Bluetooth LE advertising makes the device discoverable over Bluetooth LE for the predefined period of time (1 hour by default).

     * If the device is already provisioned to the Matter network, it re-enables the SMP server.
       After that, the DFU over Bluetooth Low Energy can be started.

     * If pressed for more than three seconds, it initiates the factory reset of the device.
       Releasing the button within three seconds of the initiation cancels the factory reset procedure.

Button 2:
   Changes the lock state to the opposite one.

SEGGER J-Link USB Port:
   Used for getting logs from the device or for communicating with it through the command-line interface.

NFC port with antenna attached:
    Optionally used for obtaining the onboarding information from the Matter accessory device to start the commissioning the device procedure while using a commercial ecosystem.
    See the :ref:`Testing with commercial ecosystem <matter_lock_app_testing_commercial_ecosystem>` section.


Building and running
********************

.. include:: ../docs/include/matter_building_and_running_intro.txt

|matter_ble_advertising_manual|

Testing
*******

This section shows how to test the |matter_type|.
You can test it using your PC and the `CHIP Tool for Linux or macOS <Matter network concepts_>`_ or commercial ecosystem that supports Matter.

.. _matter_lock_app_testing_start:

Testing with CHIP Tool
======================

Complete the following steps to test the |matter_name| device using CHIP Tool:

.. |node_id| replace:: 1

.. rst-class:: numbered-step

Prepare Matter network
----------------------

To set up the Matter over Thread, complete the following steps:

1. Configure the Thread Border Router.
   See the Running OTBR using Docker section on the `Thread Border Router`_ page.

#. Download the prebuilt CHIP tool package from the `Matter nRF Connect releases`_ GitHub page.
   Make sure that the package is compatible with your |NCS| version.

.. rst-class:: numbered-step

Prepare your DK
---------------

To flash your DK with the |matter_type| and prepare it for testing, complete the following steps:

1. |connect_kit|
#. |connect_terminal_ANSI|
#. If the device was not erased during the programming, perform the factory reset procedure.

   |matter_factory_reset|

.. rst-class:: numbered-step

Commission to Matter network
----------------------------

To commission the device to the Matter network complete the following steps:

1. Obtain a Thread active dataset from OTBR:

   .. tabs::

      .. group-tab:: OTBR on Raspberry Pi

         a. Connect to the Raspberry Pi through USB or SSH.
         b. Run the following commands:

         .. code-block:: console

            sudo ot-ctl
            > dataset active -x

      .. group-tab:: OTBR on PC using Docker

         a. Run the following command:

         .. code-block:: console

            sudo docker exec -it otbr sh -c "ot-ctl dataset active -x"

   The output should look like:

   .. code-block:: console

      080000000000000000000300001735060004001fffe00208deadbeefcafe12340708fd123456789abc00000510112233445566778899aabbccddeeff00030a54657374576f726b3031010211220410aabbccddeeff00112233445566778899aa0c0402a0f7f8
      Done

#. Run the following command and fill the *<thread dataset>* argument obtain in the previous step:

   .. parsed-literal::
      :class: highlight

      chip-tool pairing ble-thread |node_id| hex:*<thread dataset>* 20202021 3840

.. rst-class:: numbered-step

Unlock the Door Lock Application
--------------------------------

Send the Unlock command to the |matter_name| through Matter controller.
Use the following command:

.. parsed-literal::
   :class: highlight

   chip-tool doorlock unlock-door |node_id| 1 --timedInteractionTimeoutMs 5000

Observe that the |matter_name| is unlocked.

.. rst-class:: numbered-step

Lock the Door Lock Application
------------------------------

Send the Lock command to the |matter_name| through Matter controller.
Use the following command:

.. parsed-literal::
   :class: highlight

   chip-tool doorlock lock-door |node_id| 1 --timedInteractionTimeoutMs 5000

Observe that the |matter_name| is locked.

.. _matter_lock_app_testing_commercial_ecosystem:

Testing with commercial ecosystem
=================================

Before starting testing, ensure that the ecosystem supports the device types enabled in this |matter_type|.
See the ecosystem manual page for instructions on how to use it.

When you start the commissioning procedure, the ecosystem controller must get the onboarding information from the Matter accessory device.
The onboarding information representation depends on your commissioner setup.

For this |matter_type|, you can use one of the following `Onboarding information formats`_ to provide the commissioner with the data payload that includes the device discriminator and the setup PIN code:

  .. list-table:: |matter_name| |matter_type| onboarding information
     :header-rows: 1

     * - QR Code
       - QR Code Payload
       - Manual pairing code
     * - Scan the following QR code with the app for your ecosystem:

         |matter_qr_code_image|

       - |matter_qr_code_payload|
       - |matter_pairing_code|

When the factory data support is enabled, the onboarding information will be stored in the build directory in the following files:

   * The :file:`factory_data.png` file includes the generated QR code.
   * The :file:`factory_data.txt` file includes the QR code payload and the manual pairing code.

|matter_cd_info_note_for_samples|


.. _matter_door_lock_application_testing:

Testing Door Lock Application features
======================================

Besides testing the basic functionality of the |matter_name|, you can test the following features.
Some of them requires a different command to build the application.
Expand the following toggles to see testing steps for each feature.

.. _matter_lock_app_remote_access_with_pin:

Testing remote access with PIN code credential
----------------------------------------------

.. toggle::

   .. note::
      You can test the PIN code credential support with any Matter compatible controller.
      The following steps use the CHIP Tool controller as an example.
      For more information about setting user credentials, see the *Saving users and credentials on Door Lock devices* section of the `Matter chip-tool guide`_ page.

   #. Prepare the development kit for testing.
   #. Make the |matter_name| require a PIN code for remote operations:

      .. code-block:: console

         chip-tool doorlock write require-pinfor-remote-operation 1 |node_id| 1 --timedInteractionTimeoutMs 5000

   #. Add the example ``Home`` |matter_name| user:

      .. code-block:: console

         chip-tool doorlock set-user 0 2 Home 123 1 0 0 |node_id| 1 --timedInteractionTimeoutMs 5000

      This command creates a ``Home`` user with a unique ID of ``123`` and an index of ``2``.
      The new user's status is set to ``1``, and both its type and credential rule to ``0``.
      The user is assigned to the |matter_name| cluster residing on endpoint ``1`` of the node with ID ``10``.

   #. Add the example ``12345678`` PIN code credential to the ``Home`` user:

      .. code-block:: console

         chip-tool doorlock set-credential 0 '{"credentialType": 1, "credentialIndex": 1}' 12345678 2 null null |node_id| 1 --timedInteractionTimeoutMs 5000

   #. Unlock the |matter_name| with the given PIN code:

      .. code-block:: console

         chip-tool doorlock unlock-door |node_id| 1 --PINCode 12345678 --timedInteractionTimeoutMs 5000

   #. Reboot the device.
   #. Wait until the device it rebooted and attached back to the Matter network.
   #. Unlock the |matter_name| with the PIN code provided before the reboot:

      .. code-block:: console

         chip-tool doorlock unlock-door |node_id| 1 --PINCode 12345678 --timedInteractionTimeoutMs 5000

   .. note::
      Accessing the |matter_name| remotely without a valid PIN code credential will fail.

.. _matter_lock_app_schedule_testing:

Testing scheduled timed access
------------------------------

.. note::
    With scheduled timed access, you can configure and store access times on the |matter_type|, but the application does not enforce those time constraints when the lock is accessed.
    You can add such logic in your application (``BoltLockManager`` class) to enforce the time constraints when the lock is accessed.

.. toggle::

   .. note::
      You can test :ref:`matter_door_lock_application_features` using any Matter compatible controller.
      The following steps use the CHIP Tool controller as an example.

      All scheduled timed access entries are saved to non-volatile memory and loaded automatically after device reboot.
      Adding a single schedule for a user contributes to the settings partition memory occupancy increase.

   #. Prepare the development kit for testing.
   #. Add the example ``Home`` |matter_name| user:

      .. code-block:: console

         chip-tool doorlock set-user 0 2 Home 123 1 0 0 |node_id| 1 --timedInteractionTimeoutMs 5000

      This command creates a ``Home`` user with a unique ID of ``123`` and an index of ``2``.
      The new user's status is set to ``1``, and both its type and credential rule to ``0``.
      The user is assigned to the |matter_name| cluster residing on endpoint ``1`` of the node with ID |node_id|.

   #. Set the example ``Week-day`` schedule using the following command:

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock set-week-day-schedule *weekday-index* *user-index* *days-mask* *start-hour* *start-minute* *end-hour* *end-minute* *destination-id* *endpoint-id*

      * *weekday-index* is the index of the new schedule, starting from ``1``.
        The maximum value is defined by the ``CONFIG_LOCK_MAX_WEEKDAY_SCHEDULES_PER_USER`` Kconfig option.

      * *user-index* is the user index defined for the user created in the previous step.
      * *days-mask* is a bitmap of numbers of the week days starting from ``0`` as a Sunday and finishing at ``6`` as a Saturday.
        For example, to assign this schedule to Tuesday, Thursday and Saturday you need to provide ``84`` because it is equivalent to the ``01010100`` bitmap.

      * *start-hour* is the starting hour for the week day schedule.
      * *start-minute* is the starting minute for the week day schedule.
      * *end-hour* is the ending hour for the week day schedule.
      * *end-minute* is the ending hour for the week day schedule.
      * *destination-id* is the device node ID.
      * *endpoint-id* is the Matter |matter_name| endpoint, in this application assigned to ``1``.

      For example, use the following command to set a ``Week-day`` schedule with index ``1`` for Tuesday, Thursday and Saturday to start at 7:30 AM and finish at 10:30 AM, dedicated for user with ID ``2``:

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock set-week-day-schedule 1 2 84 7 30 10 30 |node_id| 1


   #. Set the example ``Year-day`` schedule using the following command:

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock set-year-day-schedule *yearday-index* *user-index* *localtime-start* *localtime-end* *destination-id* *endpoint-id*


      * *yearday-index* is the index of the new schedule, starting from ``1``.
        The maximum value is defined by the ``CONFIG_LOCK_MAX_YEARDAY_SCHEDULES_PER_USER`` Kconfig option.

      * *user-index* is the user index defined for the user created in the previous step.
      * *localtime-start* is the starting time in Epoch Time.
      * *localtime-end* is the ending time in Epoch Time.
      * *destination-id* is the device node ID.
      * *endpoint-id* is the Matter |matter_name| endpoint, in this application assigned to ``1``.

      Both ``localtime-start`` and ``localtime-end`` are in seconds with the local time offset based on the local timezone and DST offset on the day represented by the value.

      For example, use the following command to set a ``Year-day`` schedule with index ``1`` to start on Monday, May 27, 2024, at 7:00:00 AM GMT+02:00 DST and finish on Thursday, May 30, 2024, at 7:00:00 AM GMT+02:00 DST dedicated for user with ID ``2``::

      .. code-block:: console

         chip-tool doorlock set-year-day-schedule 1 2 1716786000 1717045200 |node_id| 1

   #. Set the example ``Holiday`` schedule using the following command:

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock set-holiday-schedule *holiday-index* *localtime-start* *localtime-end* *operating-mode* *destination-id* *endpoint-id*

      * *holiday-index* is the index of the new schedule, starting from ``1``.
      * *localtime-start* is the starting time in Epoch Time.
      * *localtime-end* is the ending time in Epoch Time.
      * *operating-mode* is the operating mode described in the :ref:`matter_door_lock_application_features` section of this guide.
      * *destination-id* is the device node ID.
      * *endpoint-id* is the Matter |matter_name| endpoint, in this application assigned to ``1``.

      For example, use the following command to setup a ``Holiday`` schedule with the operating mode ``Vacation`` to start on Monday, May 27, 2024, at 7:00:00 AM GMT+02:00 DST and finish on Thursday, May 30, 2024, at 7:00:00 AM GMT+02:00 DST:

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock set-holiday-schedule 1 1716786000 1717045200 1 |node_id| 1

   #. Read saved schedules using the following commands and providing the same arguments you used in the earlier steps:

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock get-week-day-schedule *weekday-index* *user-index* *destination-id* *endpoint-id*

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock get-year-day-schedule *yearday-index* *user-index* *destination-id* *endpoint-id*

      .. parsed-literal::
         :class: highlight

         chip-tool doorlock get-holiday-schedule *holiday-index* *destination-id* *endpoint-id*

Testing Bluetooth LE with Nordic UART Service
---------------------------------------------

.. toggle::

   To test the `Nordic UART Service (NUS)`_ feature, complete the following steps:

   .. note::
      Some of the steps depend on which :ref:`configuration <matter_lock_app_configuration>` the application was built with.

   #. Install `nRF Toolbox`_ on your Android (Android 11 or newer) or iOS (iOS 16.1 or newer) smartphone.
   #. Build the |matter_type| application for Matter over Thread with the ``bt_nus`` snippet.

      .. tabs::

         .. group-tab:: |nRFVSC|

            Add ``-Dmatter-door-lock-app_SNIPPET=bt_nus`` to :guilabel:`Extra CMake arguments` in your build configuration.

         .. group-tab:: Command line

            For example, for the ``nrf52840dk/nrf52840``:

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 -- -Dmatter-door-lock-app_SNIPPET=bt_nus

   #. Prepare the development kit for testing.
   #. If you built the application with the debug configuration, connect the board to a UART console to see the log entries from the device.
   #. Open the nRF Toolbox application on your smartphone.
   #. Select :guilabel:`Universal Asynchronous Receiver/Transmitter UART` from the list in the nRF Toolbox application.
   #. Tap on :guilabel:`Connect`.
      The application connects to the devices connected through UART.

   #. Select :guilabel:`MatterLock` from the list of available devices.
      The Bluetooth Pairing Request with an input field for passkey appears as a notification (Android) or on the screen (iOS).

   #. Depending on the configuration you are using:

      * If you rely on ``CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY`` (default ``123456``): Enter that passkey on the phone.
      * If the device prints a passkey to the log, complete the following steps:

         a. Search the device's logs to find ``PROVIDE THE FOLLOWING CODE IN YOUR MOBILE APP:`` phrase.
         #. Read the passkey from the console logs.
         #. Enter the passcode on your smartphone.

   #. Wait for the Bluetooth LE connection to be established between the smartphone and the DK.
   #. In the nRF Toolbox application, add the following macros:

      * ``Lock`` as the Command value type ``Text`` and any image.
      * ``Unlock`` as the Command value type ``Text`` and any image.

   #. Tap on the generated macros and to change the lock state.

   The Bluetooth LE connection between a phone and the DK will be suspended when the commissioning to the Matter network is in progress or there is an active session of SMP DFU.

   To read the current |matter_type| state from the device, read the Bluetooth LE RX characteristic.
   The new lock state is updated after changing the state from any of the following sources: NUS, buttons, Matter stack.

Dependencies
************

This |matter_type| uses the Matter library that includes the |NCS| platform integration layer:

* `Matter`_

In addition, the |matter_type| uses the following |NCS| components:

* `DK Buttons and LEDs`_
* `URI messages and records`_
* `Type 2 Tag`_

The |matter_type| depends on the following Zephyr libraries:

* `Logging`_
* `Kernel Services`_
