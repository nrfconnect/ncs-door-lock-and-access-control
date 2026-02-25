.. _hw_requirements:

Hardware requirements
#####################

.. contents::
   :local:
   :depth: 2

To run and test the |APP_NAME|, you must have the required hardware.

.. _hw_requirements_development_kit:

Development kit
***************

The application supports the following development kits (DK):

.. list-table::
   :header-rows: 1

   * - Hardware platforms
     - PCA
     - Board name
     - Build target
   * - `nRF52840 DK`_
     - PCA10056
     - `nrf52840dk`_
     - ``nrf52840dk/nrf52840``
   * - `nRF5340 DK`_
     - PCA10095
     - `nrf5340dk`_
     - ``nrf5340dk/nrf5340/cpuapp``
   * - `nRF54L15 DK`_
     - PCA10156
     - `nrf54l15dk`_
     - ``nrf54l15dk/nrf54l15/cpuapp``
   * - `nRF54LM20 DK`_
     - PCA10184
     - `nrf54lm20dk`_
     - ``nrf54lm20dk/nrf54lm20a/cpuapp``

.. _hw_requirements_vddio_configuration:

Configuring VDDIO voltage for nRF54L Series devices
===================================================

The nRF54L15 DK and the nRF54LM20 DK operate at default voltage level of 1.8V.
Some expansion boards, especially Arduino-style boards, require higher voltage to operate properly.

.. tabs::

   .. tab:: nRF54L15 DK

      You can adjust the voltage for ports on the nRF54L15 DK up to 3.3V using the `Board Configurator app`_.
      First, you must `install the Board Configurator app <Installing Board Configurator app_>`_ and once completed `Update your DK's configuration <Updating the board configuration_>`_.

      See the recommended configuration for VDD (nPM VOUT1):

      .. figure:: /images/VDDIO-configuration.png
         :scale: 70%
         :alt: VDD board configuration.

         VDD board configuration.

   .. tab:: nRF54LM20 DK

      You can adjust the voltage for ports on the nRF54LM20 DK using the following steps:

      #. Create a json file with this content:

         .. code-block:: json

            {
               "operations": [
                  {
                        "operation":{
                           "type": "smp",
                           "operation":2,
                           "group_id":64,
                           "command_id":0,
                           "sequence_number":0,
                           "data": [[6, true, 20, true, 22, true, 23, false, 42, true, 45, true, 47, true],[1,2700,2,2700]]
                        },
                        "operationId":"1"
                  }
               ]
            }

      #. Run the following command:

         .. code-block:: console

            nrfutil device x-execute-batch --traits boardController --batch-path <json_file_path>

      #. Turn the DK off and on using the power switch.

.. _hw_requirements_nfc_reader:

Near Field Communication reader
*******************************

To start working with the application, you must have at least one Near Field Communication (NFC) reader.
The application supports the following NFC readers and their corresponding development expansion boards:

+-------------------+---------------------------------+
| NFC reader        | NFC reader expansion board      |
+===================+=================================+
| `ST25R100`_       | `X-NUCLEO-NFC09A1`_             |
+-------------------+---------------------------------+
| `ST25R300`_       | `X-NUCLEO-NFC12A1`_             |
+-------------------+---------------------------------+

.. note::

   The `X-NUCLEO-NFC09A1`_ board requires a minimum of 2.7V to operate.
   Because of that, you must adjust the GPIO voltage for the `nRF54L15 DK`_ and the `nRF54LM20 DK`_ as outlined in the :ref:`hw_requirements_vddio_configuration` section.

Connecting the NFC reader
=========================

Follow the guidelines to connect the NFC reader expansion board based on your development kit:

.. tabs::

   .. tab:: nRF54LM20 DK

      The `nRF54LM20 DK`_ does not have Arduino-compatible header, therefore, you must connect your board using wires.
      To connect the NFC reader expansion board to the DK, refer to the following pin mapping.
      The pinout is applicable for each of the supported NFC reader expansion boards:

      +-------------------+-----------------------+
      | nRF54LM20 DK      | X-NUCLEO board        |
      +===================+=======================+
      | P1.13             | SCK MCU (D13)         |
      +-------------------+-----------------------+
      | P1.12             | MISO MCU (D12)        |
      +-------------------+-----------------------+
      | P1.11             | MOSI MCU (D11)        |
      +-------------------+-----------------------+
      | P1.10             | /SS MCU (D10)         |
      +-------------------+-----------------------+
      | P1.14             | IRQ MCU (A0)          |
      +-------------------+-----------------------+
      | VBUS              | +5V                   |
      +-------------------+-----------------------+
      | VDDIO             | +3V3                  |
      +-------------------+-----------------------+
      | GND               | GND                   |
      +-------------------+-----------------------+

   .. tab:: nRF54L15 DK

      The `nRF54L15 DK`_ does not have Arduino-compatible header, therefore, you must connect your board using wires.
      To connect the NFC reader expansion board to the DK, refer to the following pin mapping.
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

   .. tab:: nRF5340 DK
      
      The `nRF5340 DK <nRF5340 DK connector interface_>`_ has **P5** and **P20** connectors located between their Arduino headers.
      These connectors might cause electrical shorts with the NFC reader expansion board, which can lead to NFC driver initialization failures and application crashes.
      To prevent shorts, ensure proper connection of the NFC reader expansion board and maintain adequate clearance between the connectors and the expansion board.

   .. tab:: nRF52840 DK

      The `nRF52840 DK <nRF52840 DK connector interface_>`_ has **P5** and **P20** connectors located between their Arduino headers.
      These connectors might cause electrical shorts with the NFC reader expansion board, which can lead to NFC driver initialization failures and application crashes.
      To prevent shorts, ensure proper connection of the NFC reader expansion board and maintain adequate clearance between the connectors and the expansion board.

.. note::
   To make the hardware setup easier, you can request the PCB design files of the Arduino-compatible adapters for the `nRF54L15 DK`_  and the `nRF54LM20 DK`_ via `Nordic DevZone`_.

.. _hw_requirements_uwb_module:

Ultra wideband (UWB) module
***************************

The application supports the following UWB module:

+-------------------+-------------------------------------------------------------+
| UWB module        | UWB module expansion board                                  |
+===================+=============================================================+
| `QM35825`_        | Qorvo Arduino Interface Board (with custom rework by Qorvo) |
+-------------------+-------------------------------------------------------------+

The following table shows the pin mapping for the Qorvo Arduino Interface Board with custom rework by Qorvo:

.. tabs::

   .. tab:: nRF54LM20 DK

      The `nRF54LM20 DK`_ does not have Arduino-compatible header, therefore, you must connect your board using wires.
      To connect the Qorvo Arduino Interface Board to the DK, refer to the following pin mapping.

      +-------------------+---------------------------+
      | nRF54LM20 DK      | Qorvo Arduino Interface   |
      |                   | Board                     |
      +===================+===========================+
      | P1.13             | SPI0_CLK (D13)            |
      +-------------------+---------------------------+
      | P1.12             | SPI0_MISO (D12)           |
      +-------------------+---------------------------+
      | P1.11             | SPI0_MOSI (D11)           |
      +-------------------+---------------------------+
      | P3.11             | SPI0_CS (D2)              |
      +-------------------+---------------------------+
      | P0.0              | SS_IRQ (A5)               |
      +-------------------+---------------------------+
      | P3.7              | RST_HOST (D7)             |
      +-------------------+---------------------------+
      | P3.12             | EXTON (D5)                |
      +-------------------+---------------------------+
      | VBUS              | 5V_HOST                   |
      +-------------------+---------------------------+
      | VDDIO             | 3V3_HOST                  |
      +-------------------+---------------------------+
      | GND               | GND                       |
      +-------------------+---------------------------+

      .. note::
         To make the hardware setup easier, you can request the PCB design files of the Arduino-compatible adapter for the nRF54LM20 DK and Qorvo Arduino Interface Board through `Nordic DevZone`_.

   .. tab:: nRF5340 DK

      The `nRF5340 DK`_ has Arduino-compatible headers, so you can connect the Qorvo Arduino Interface Board directly to development kit.

.. note::
   Contact your local Qorvo Regional Sales Manager or Nordic Regional Sales Manager to obtain the Qorvo Arduino Interface Board with special rework done on the board.
   You need it to run the application with the QM35825 UWB module.

.. _hw_requirements_test_harness:

Test harness hardware
*********************

For testing purposes, use the official `Aliro Certification Tool`_ as a test harness.
To set it up, you must first meet the `test harness hardware requirements`_.

.. note::

   In case you do not have an access to this repository, send a request to the help@csa-iot.org providing your GitHub username.
   Be aware that you must first become a member of the `Connectivity Standards Alliance`_ (CSA).
