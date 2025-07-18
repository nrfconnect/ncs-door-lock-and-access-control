.. _troubleshooting:

Troubleshooting
###############

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
