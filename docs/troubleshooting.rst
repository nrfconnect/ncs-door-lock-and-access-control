.. _troubleshooting:

Troubleshooting
###############

.. contents::
   :local:
   :depth: 2

The following page outlines common troubleshooting scenarios for device under test (DUT) and test harness setups.

Issues when connecting to RaspberryPi
*************************************

In cases where it is impossible to connect to a Raspberry Pi through WiFi or an Ethernet cable, you must configure the network manually:

1. Eject the SD card from the RaspberryPi and mount it on your laptop.
   You will see two partitions: ``writable`` and ``system-boot``.

#. On the ``writable`` partition, navigate to the :file:`writable/etc/netplan/99_config.yaml` file.
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

#. Identify the Ethernet device on your local machine and assign network configurations.
   Ensure both your local machine and RaspberryPi are on the same network:

    .. code-block:: console

        ip link set enx109819b3e9d4 up
        ip addr add 192.167.1.1/24 dev

Test harness returns error "Error returned: 0x60a" for BLE or UWB tests
***********************************************************************

This error is returned when the Murata LBUA0VG2BP-EVK-P (BLE/UWB interface) has incorrect firmware version.
To fix this error, you need to update the firmware of the Murata board to the latest version compatible with the test harness.


Test harness stalling
=====================

Test harness can experience stalling if it is used after extended periods of operation.
The common issues include:

* Test harness does not respond to commands sent from the Reader.

* The front-end web UI does not react to user inputs or actions.

To diagnose and address these issues, retrieve the back-end Docker logs and check for errors.
Run the following commands:

1. Access the test harness host or IP:

   .. code-block:: console

       ssh aliro@aliro-th.local

#. Retrieve the Docker logs:

   .. code-block:: console

       aliro@aliro-th: docker logs aliro-certification-tool-backend-1

Based on the results apply the following workarounds:

* If the error occurs from Daemon in stream:

  * Issue description: ``Error grabbing logs: invalid character '\x00' looking for the beginning of value.``
    This error indicates a log corruption issue.
  * Workaround: To resolve this issue, truncate the Docker logs using the following command:

    .. code-block:: console

       sudo truncate -s 0 $(docker inspect --format='{{.LogPath}}' aliro-certification-tool-backend-1)

    This command clears the existing logs, potentially resolving corruption issues:

* If the error is related to KeyError in test script:

  * Issue description: ``test_collections.aliro.support.aliro_test_case:wrapper:34 | Error occurred during script: KeyError: KeyError('"type"')``.
    This error indicates a missing or incorrect key in a dictionary.

  * Workaround: Reboot the test harness.
