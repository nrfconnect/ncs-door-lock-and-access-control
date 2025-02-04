.. _hw_requirements:

Hardware requirements
#####################

Before running and testing the NCS door lock, make sure you have required hardware.

.. _development_kit:

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


.. _vddio_configuration:

Port VDDIO voltage configuration on the development kit
=======================================================

The nRF54L15 DK operates on a lower voltage level (1.8V) by default. Some expansion boards, especially Arduino-style boards, are not compatible
with this voltage, and they are required higher voltage to operate. The ports on the nRF54L15 can be adjusted up to 3.3V.

The voltage on the VDD rail of the nRF54L15 can be configured through the `nRF Connect Board Configurator`_ application.
See `Installing Board Configurator app`_ to install the application and `Updating the board configuration`_ in order to change the DK configuration.

See the picture below for VDD (nPM VOUT1) suggested configuration.

.. figure:: /images/VDDIO-configuration.png
   :scale: 50%
   :alt: Suggested VDD board configuration.

   Suggested VDD board configuration.

.. _nfc_reader:

Near Field Communication (NFC) reader
*************************************

In addition to the DK you need at least NFC reader which is mandatory.
The application supports the following NFC readers and development expansion boards based on them:

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
   The DK does not have Arduino-comatible header, thus the board must be connected via wires.
   The pinout is described below and works for each of mentioned boards.

.. warning::
   The ``X-NUCLEO-NFC09A1`` board requires at least ``2.7V`` to operate and for the `nRF54L15 DK`_ GPIO voltage should be adjusted.
   For more information, see :ref:`vddio_configuration`.

To connect the NFC reader expansion board to the `nRF54L15 DK`_ refer to the following pin mapping:

+-------------------+-----------------------+
| nRF54L15 DK board | X-NUCLEO-NFC09A1      |
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
   :alt: Expansion board connection.

   X-NUCLEO expansion board connection to nRF54L15 DK.

.. _test_harness_hardware:

Test harness hardware
*********************

For testing purposes, you can use the official `Aliro Certification Tool`_ (aka test harness).
To set up the test harness, you must meet the `test harness hardware requirements`_.

.. note::
   In order to gain access to this repository, you must be a member of the `Connectivity Standards Alliance`_ (CSA)
   and send a request to the e-mail address: help@csa-iot.org with your Github username.
