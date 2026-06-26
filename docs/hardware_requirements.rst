.. _hw_requirements:

Hardware requirements
#####################

.. contents::
   :local:
   :depth: 2

To run and test the |APPS_NAME|, you must have the required hardware.

.. _hw_requirements_development_kit:

Development kit
***************

Depending on the transport technology you want to use, the |APPS_NAME| support the following development kits (DK):

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
       ``nrf54lm20dk/nrf54lm20b/cpuapp``

.. _hw_requirements_vddio_configuration:

Configuring VDDIO voltage for nRF54L Series devices
===================================================

The nRF54L15 DK and the nRF54LM20 DK operate at a default voltage level of 1.8V.
Some expansion boards required for |APPS_NAME| with Aliro support require a higher voltage than the default.
The recommended VDD setting depends on the development kit.

.. tabs::

   .. group-tab:: nRF54LM20 DK

      You can adjust the voltage for ports on the nRF54LM20 DK using the `Board Configurator app`_.

      #. `Install the Board Configurator app <Installing Board Configurator app_>`_.
      #. `Update your DK's configuration <Updating the board configuration_>`_.

      Set VDD (nPM VOUT1) and VDDIO (nPM VOUT2) to 2.7V.

      .. note::
         Do not set the voltages above 2.7V on the nRF54LM20 DK.
         While 2.7V is sufficient for NFC reader expansion boards, higher voltages can interfere with the onboard LEDs.

      See the recommended configuration for VDD (nPM VOUT1):

      .. figure:: /images/VDDIO-configuration-nrf54lm20dk.png
         :scale: 70%
         :alt: VDD board configuration.

         VDD board configuration.

   .. group-tab:: nRF54L15 DK

      You can adjust the voltage for ports on the nRF54L15 DK up to 3.3V using the `Board Configurator app`_.
      
      #. `Install the Board Configurator app <Installing Board Configurator app_>`_.
      #. `Update your DK's configuration <Updating the board configuration_>`_.

      See the recommended configuration for VDD (nPM VOUT1):

      .. figure:: /images/VDDIO-configuration-nrf54l15dk.png
         :scale: 70%
         :alt: VDD board configuration.

         VDD board configuration.

.. _hw_requirements_nfc_reader:

Near Field Communication reader
*******************************

To start working with the |APPS_NAME| with Aliro support, you must have at least one Near Field Communication (NFC) reader.
The |REPO_NAME| supports the following NFC readers and their corresponding development expansion boards:

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

   .. group-tab:: nRF54LM20 DK

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

      .. figure:: /images/nRF54LM20_X_NUCLEO_connection.png
         :scale: 75%
         :alt: Expansion board connection to the nRF54LM20 DK.

         X-NUCLEO expansion board connection to the nRF54LM20 DK.

      .. note::
         To make the hardware setup easier, you can request the PCB design files of the Arduino-compatible adapters for the `nRF54L15 DK`_  and the `nRF54LM20 DK`_ via `Nordic DevZone`_.

   .. group-tab:: nRF54L15 DK

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

      .. note::
         To make the hardware setup easier, you can request the PCB design files of the Arduino-compatible adapters for the `nRF54L15 DK`_  and the `nRF54LM20 DK`_ via `Nordic DevZone`_.

   .. group-tab:: nRF5340 DK

      The `nRF5340 DK <nRF5340 DK connector interface_>`_ has **P5** and **P20** connectors located between their Arduino headers.
      These connectors might cause electrical shorts with the NFC reader expansion board, which can lead to NFC driver initialization failures and application crashes.
      To prevent shorts, ensure proper connection of the NFC reader expansion board and maintain adequate clearance between the connectors and the expansion board.

   .. group-tab:: nRF52840 DK

      The `nRF52840 DK <nRF52840 DK connector interface_>`_ has **P5** and **P20** connectors located between their Arduino headers.
      These connectors might cause electrical shorts with the NFC reader expansion board, which can lead to NFC driver initialization failures and application crashes.
      To prevent shorts, ensure proper connection of the NFC reader expansion board and maintain adequate clearance between the connectors and the expansion board.

.. _hw_requirements_uwb_module:

Ultra-wideband (UWB) module
***************************

The |REPO_NAME| supports the following UWB module that you can use with the |APPS_NAME| when using Aliro over UWB transport:

+-------------------+-------------------------------------------------------------+
| UWB module        | UWB module expansion board                                  |
+===================+=============================================================+
| `QM35825`_        | Reworked `QM35825DK-05`_                                    |
+-------------------+-------------------------------------------------------------+

.. note::

   |QM35_EXPERIMENTAL_NOTE|

The Qorvo Arduino Interface Board is one of the boards that make up the Qorvo QM35825DK.
It requires a custom rework to become compatible with Nordic Development Kits.
The following table shows the pin mapping for the reworked board:

.. tabs::

   .. group-tab:: nRF54LM20 DK

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

   .. group-tab:: nRF5340 DK

      The `nRF5340 DK`_ has Arduino-compatible headers, so you can connect the Qorvo Arduino Interface Board directly to the development kit.

.. note::

   Contact your local Nordic Regional Sales Manager to obtain a bundle consisting of the `nRF54LM20 DK`_, the Qorvo `QM35825DK-05`_ with the required custom rework, and the Arduino-compatible adapter, which enables easy connection of the Nordic and Qorvo DKs.

.. note::

   By default, the applications in the |REPO_NAME| use the shared SPI configuration, which allows for concurrent use of the SPI bus by both the NFC Reader and the UWB module.
   Therefore, the NFC Reader and the UWB module can share the same Arduino header.

.. _hw_requirements_test_harness:

Test harness hardware
*********************

If you are developing a door lock with Aliro support, for testing purposes, use the official `Aliro Certification Tool`_ as a test harness.
To set it up, you must first meet the `test harness hardware requirements`_.

.. note::

   In case you do not have access to this repository, send a request to the help@csa-iot.org providing your GitHub username.
   Be aware that you must first become a member of the `Connectivity Standards Alliance`_ (CSA).
