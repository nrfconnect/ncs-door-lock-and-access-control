.. _hw_requirements:

Hardware requirements
#####################

To run and test the NCS door lock, you must have the required hardware.

.. _hw_requirements_development_kit:

Development kit
***************

The application supports the following development kit (DK):

.. list-table::
   :header-rows: 1

   * - Hardware platforms
     - PCA
     - Board name
     - Build target
   * - `nRF54L15 DK`_
     - PCA10156
     - `nrf54l15dk`_
     - ``nrf54l15dk/nrf54l15/cpuapp``

.. _hw_requirements_vddio_configuration:

Configuring VDDIO voltage
=========================

The nRF54L15 DK operates at default voltage level of 1.8V.
Some expansion boards, especially Arduino-style boards, require higher voltage to operate properly.
You can adjust the voltage for ports on the nRF54L15 DK up to 3.3V using the `Board Configurator app`_.
First, you must `install the Board Configurator app <Installing Board Configurator app_>`_ and once completed `Update your DK's configuration <Updating the board configuration_>`_.

See the recommended configuration for VDD (nPM VOUT1):

.. figure:: /images/VDDIO-configuration.png
   :scale: 70%
   :alt: VDD board configuration.

   VDD board configuration.

.. _hw_requirements_nfc_reader:

Near Field Communication (NFC) reader
*************************************

To start working with the application, you must have at least one NFC reader.
The application supports the following NFC readers and their corresponding development expansion boards:

+-------------------+---------------------------------+
| NFC reader        | NFC reader expansion board      |
+===================+=================================+
| `ST25R100`_       | `X-NUCLEO-NFC09A1`_             |
+-------------------+---------------------------------+
| `ST25R3916B`_     | `X-NUCLEO-NFC08A1`_             |
+-------------------+---------------------------------+
| `ST25R3911B`_     | `X-NUCLEO-NFC05A1`_             |
+-------------------+---------------------------------+

.. note::

   The `X-NUCLEO-NFC09A1`_ board requires a minimum of 2.7V to operate.
   Because of that, you must adjust the GPIO voltage for the `nRF54L15 DK`_ as outlined in the :ref:`hw_requirements_vddio_configuration` section.

The DK does not have Arduino-compatible header, therefore, you must connect your board using wires.
To connect the NFC reader expansion board to the `nRF54L15 DK`_, refer to the following pin mapping.
The pinout is applicable for each of the supported NFC reader expansion boards:

+-------------------+-----------------------+
| nRF54L15 DK       | X-NUCLEO board        |
+===================+=======================+
| P1.13             | SCK MCU (D13)         |
+-------------------+-----------------------+
| P1.12             | MISO MCU (D12)        |
+-------------------+-----------------------+
| P1.11             | MOSI MCU (D11)        |
+-------------------+-----------------------+
| P2.08             | /SS MCU (D10)         |
+-------------------+-----------------------+
| P0.04             | IRQ MCU (A0)          |
+-------------------+-----------------------+
| VBUS              | +5V                   |
+-------------------+-----------------------+
| VDDIO             | +3V3                  |
+-------------------+-----------------------+
| GND               | GND                   |
+-------------------+-----------------------+

.. figure:: /images/nRF54L_X_NUCLEO_connection.png
   :scale: 50%
   :alt: Expansion board connection to the nRF54L15 DK.

   X-NUCLEO expansion board connection to the nRF54L15 DK.

.. _hw_requirements_test_harness:

Test harness hardware
*********************

For testing purposes, use the official `Aliro Certification Tool`_ as a test harness.
To set it up, you must first meet the `test harness hardware requirements`_.

.. note::

   In case you do not have an access to this repository, send a request to the help@csa-iot.org providing your GitHub username.
   Be aware that you must first become a member of the `Connectivity Standards Alliance`_ (CSA).
