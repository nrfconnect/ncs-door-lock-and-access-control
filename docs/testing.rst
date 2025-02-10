.. _testing:

Testing and troubleshooting
###########################

This page will guide you through the testing instructions for the |app_name|.

.. _test_environment:

Test environment
****************

Test environment consists of two major parts:

* The `nRF54L15 DK`_, which acts as the Reader for the door lock component, with an NFC card reader expansion board.
* The Aliro Test Harness, which acts as the User Device (the device that unlocks the door lock).

For more details on these requirements, see the :ref:`hw_requirements` page.

.. _tests_environment_configuration:

Configuring test environment
****************************

.. note::
   All examples related to the `Aliro Certification Tool`_ are based on the ``dryrun/test_event4-2024-aliro_specification_v0.9.0-v1.0`` tag.

The process of configuring the Test Harness, selecting tests, and executing them is described in the `test harness usage instructions`_ of the `Aliro Certification Tool`_ repository.
In case you do not have an access to this repository, see the :ref:`hw_requirements_test_harness` section for further guidance.

#. Obtain the public key for the device under test (DUT) Reader.
   You can retrieve this information from the serial console of a DUT.
   For information on how to build, flash, and access the serial console, see the :ref:`building_and_running` documentation page.
   Once the device is flashed with the specified Aliro firmware, you will see a welcome message displaying the provisioned key and information for the DUT Reader:

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
   :scale: 70%

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

The following section outlines the most common troubleshooting scenarios.
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
==========================

In some cases, it may be impossible to connect to a Raspberry Pi through WiFi or an Ethernet cable.
In such cases, you must configure the network manually. 

1. First, you must eject the SD card from the RaspberryPi and mount it on your laptop. 
   You will see two partitions: ``writable`` and ``system-boot``.

#. Configure the Test Harness ``eth0`` and the local ethernet address to be in the same subnet. 
   On the ``writable`` partition, navigate to :file:`*/media/$USER/writable/etc/netplan/99_config.yaml*` (assuming the ``writable`` partition is mounted under ``/media`` mount point on your system).
   Apply the following configuration for the ``eth0`` interface:
    
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

Identify the Ethernet device on your local machine and assign network configurations so that both interfaces are on the same network (assuming local ethernet port is ``enx109819b3e9d4``):

    .. code-block:: console
        
        ip link set enx109819b3e9d4 up
        ip addr add 192.167.1.1/24 dev 
    ..