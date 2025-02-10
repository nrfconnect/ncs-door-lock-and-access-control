.. _testing:

Testing and troubleshooting
###########################

.. _test_environment:

Test environment
****************

Test environment consists of two major parts:

1. Moonlight DK (nRF54L15-DK) acts as the Reader (door lock part) with NFC card reader expansion board.

   More detailed hardware information about the Reader:

   - :ref:`hw_requirements_development_kit`
   - :ref:`hw_requirements_nfc_reader`

#. Aliro Test Harness acts as a User Device (device which unlocks door lock). Check :ref:`hw_requirements_test_harness` section for more information.

.. _tests_environment_configuration:

Test environment configuration
****************

.. note::

   All examples related to `Aliro Certification Tool`_ are based on *dryrun/test_event4-2024-aliro_specification_v0.9.0-v1.0* tag.

The process of Test Harness configuration, tests selection and execution is described in `test harness usage instructions`_ section of `Aliro Certification Tool`_. Access to this repository may be restricted, follow :ref:`hw_requirements_test_harness` instructions to obtain the access.
Configuration of a Test Project requires to provide DUT Reader public key. The key can be obtained from serial console of a DUT. Instructions on how to build, flash and access serial console can be found in :ref:`building_and_running` section.

Once device is flashed with desired Aliro firmware welcome message should be displayed on serial console with provisioning key of **DUT Reader**  information:

   .. code-block:: console

    *** Booting My Application v0.1.0-f352de3b49e1 ***
    *** Using nRF Connect SDK v2.9.99-d4304c5292dc ***
    *** Using Zephyr OS v3.7.99-20a2ba291a8f ***
    I:                                                                         
                                                @@@   @@@@
          @@@@@@@@@@@@                          @@@
        @@@@@      @@@@@              @@@@   @@ @@@    @@      @@     @@@@
       @@@@          @@@@          @@@@  @@@@@@ @@@    @@   @@@@@  @@@   @@@@
      @@@@     @@      @@@        @@@       @@@ @@@    @@  @@@   @@@       @@@
      @@@     @@@@@    @@@       @@@        @@@ @@@    @@  @@    @@@        @@
      @@@   @@@@@@@@   @@@       @@@        @@@ @@@    @@  @@    @@@        @@
       @@@    @@@@    @@@@        @@@      @@@@ @@@    @@  @@     @@       @@@
        @@@   @@@@   @@@@           @@@@@@@@@@@  @@@@@ @@  @@      @@@@@@@@@
         @@@@ @@@@ @@@@
           @@ @@@@ @@                                                        

    I: Initialize Crypto PSA backend
    W: 
    ### WARNING: Tests keys are used (NOT allowed for production!) ###

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

Provide the key in the **Configuring a Test Project** step of `test harness usage instructions`_.

.. figure:: /images/dut_reader_public_key.png
   :scale: 50%
   :alt: Test harness project configuration.

   Test harness project configuration.

.. _tests_execution:

Tests execution
***************

Test execution process is described in `running test scripts`_. NFC Reader tests starts with *RD* prefix. Example of RD-NFC-CONTROLFLOW-1.0 tests results are shown below:

.. figure:: /images/tests_selection.png
   :scale: 50%
   :alt: Tests selection view.

   Tests selection view.

.. figure:: /images/test_results.png
   :scale: 50%
   :alt: Example test results view.

   Example test results view.

.. _troubleshooting:

Troubleshooting (Linux)
***********************

DUT (Reader)
------------

DUT (DK) provides serial connection. To identify correct serial device Linux command: *dmesg -hW*  (follow mode with human-readable output) can be used.
Once DK is connected it should report similar output:
    .. code-block:: console

        [  +3,077652] usb 3-1.3.1: new full-speed USB device number 20 using xhci_hcd
        [  +0,091171] usb 3-1.3.1: New USB device found, idVendor=1366, idProduct=1059, bcdDevice= 1.00
        [  +0,000014] usb 3-1.3.1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
        [  +0,000003] usb 3-1.3.1: Product: J-Link
        [  +0,000001] usb 3-1.3.1: Manufacturer: SEGGER
        [  +0,000002] usb 3-1.3.1: SerialNumber: 001057795280
        [  +0,030034] cdc_acm 3-1.3.1:1.0: ttyACM0: USB ACM device
        [  +0,000645] cdc_acm 3-1.3.1:1.2: ttyACM1: USB ACM device
        [  +0,000722] usb-storage 3-1.3.1:1.5: USB Mass Storage device detected
        [  +0,000710] scsi host0: usb-storage 3-1.3.1:1.5
        [  +0,001120] hid-generic 0003:1366:1059.001B: hiddev5,hidraw19: USB HID v1.00 Device [SEGGER J-Link] on usb-0000:00:14.0-1.3.1/input6
        [  +1,003913] scsi 0:0:0:0: Direct-Access     SEGGER   MSD Volume       1.00 PQ: 0 ANSI: 4

    ..

Serial number can be used to identify serial port(s):
    .. code-block:: console

        > ls -l /dev/serial/by-id/
        total 0
        lrwxrwxrwx 1 root root 13 lis  8 08:33 usb-SEGGER_J-Link_001057795280-if00 -> ../../ttyACM0
        lrwxrwxrwx 1 root root 13 lis  8 08:33 usb-SEGGER_J-Link_001057795280-if02 -> ../../ttyACM1
    
    ..

`Serial Terminal app`_ can be used to access serial connection.

Test Harness (User Device)
--------------------------

In some cases it may happen that it is impossible to connect to Raspberry Pi over WiFi or Ethernet cable.
In this case manual network configuration can be done. Eject SD Card from RaspberryPi and mount it on your Laptop. There should be two partitions  *writable*  and  *system-boot*.
Configure Test Harness *eth0* and local ethernet address in the same subnetwork. Example configuration can be done as follows:

On  *writable*  partition open  */media/$USER/writable/etc/netplan/99_config.yaml*  (assumes that  *writable*  partition is mounted under  */media*  mount point on your system) and configure *eth0* interface as follows:
    
    .. code-block:: yaml
        
        network:
            version: 2
            ethernets:
                eth0:
                addresses:
                    - 192.167.1.10/24
                gateway4: 192.167.1.1
                nameservers:
                    addresses:
                    - 8.8.8.8
                    - 8.8.4.4
    ..

Identify ethernet device on local machine and assign network configuration so both interfaces are configured in the same network (assuming local ethernet port is *enx109819b3e9d4*):

    .. code-block:: console
        
        ip link set enx109819b3e9d4 up
        ip addr add 192.167.1.1/24 dev 
    ..