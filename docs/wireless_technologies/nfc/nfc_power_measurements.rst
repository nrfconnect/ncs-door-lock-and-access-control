.. _nfc_power_measurements:

NFC current measurements
########################

.. contents::
   :local:
   :depth: 2

This page describes how to measure NFC-related current consumption of an |APP_NAME| using `Power Profiler Kit II`_ (PPK).

.. note::
   The procedures, switch settings, and plots described on this page use an `nRF5340 DK`_, an `X-NUCLEO-NFC12A1`_ expansion board, and the Aliro standalone application in *release* configuration.
   Adapt the wiring and switch positions if you use different hardware.

Overview
********

The NFC current measurements are separated into two parts:

* :ref:`nfc_power_measurements_nrf_soc`
* :ref:`nfc_power_measurements_nfc_reader`

Choose the measurement based on what you want to analyze and optimize:

* To check only the SoC current consumption, follow :ref:`nfc_power_measurements_nrf_soc`.
* To check only the NFC reader, follow :ref:`nfc_power_measurements_nfc_reader`.
* To evaluate the combined impact of both components, run both procedures separately and add or compare the results.
  The PPK cannot measure SoC and NFC module current in a single wiring setup.

The analysis covers two functional modes of the NFC reader:

* IDLE — Average current over the full capture window with no NFC tag or User Device at the Reader (for example, a 60-second window).
* ACTIVE — Average current while an NFC tag or User Device is held at the Reader during the capture window.

Prerequisites
*************

Ensure you have the following hardware:

* `Power Profiler Kit II`_ (PPK)
* Development kit (for example, `nRF5340 DK`_)
* `X-NUCLEO-NFC12A1`_ NFC reader expansion board

Ensure you have installed the following software:

* `nRF Connect for Desktop`_ with the Power Profiler app

.. _nfc_power_measurements_build:

Build configuration
*******************

All measurements in this document were taken with the following build configuration for the Aliro standalone application:

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp -p -- -DFILE_SUFFIX=release

In the Aliro standalone application, the *release* configuration keeps the UART shell enabled (used for provisioning the Aliro Reader).
This adds background UART activity and can impact the measured current consumption.

.. note::

   To remove UART-related current consumption, disable the UART node used by the shell or console in your DTS overlay, for example:

   .. code-block:: dts

      &uart0 {
              status = "disabled";
      };

.. _nfc_power_measurements_nrf_soc:

SoC current measurement
***********************

In this setup, the development kit is powered directly by the PPK, while the NFC module remains powered from the USB port.

Hardware connection
===================

Connect the development kit and the PPK as shown in the following schematic:

.. figure:: /images/nfc_current_schematic_soc_nrf5340_ppk2.png
   :scale: 85%
   :alt: Schematic for measuring nRF SoC current.

Development kit switch settings
===============================

Configure the development kit as follows for measurement:

* **POWER** switch: **ON**
* **nRF power source**: **VDD**
* **VEXT->nRF**: **ON**
* **SW6 (nRF ONLY|DEFAULT)**: **nRF ONLY** (SEGGER, LEDs, and external flash powered off)

In the nRF ONLY mode, the external flash memory is powered down.
Since the |APP_NAME| uses external flash, it will not start correctly unless you power up the flash.

Whether the external flash current is included in the measurements depends on how it is powered:

* Powered from **VDD**: excluded from the measurements
* Powered from **VDD nRF**: included in the measurements

Choose one of the following approaches:

* Full development kit current measurement — Set **SW6** to **nRF DEFAULT**.
  This measures the full development kit current, including SEGGER, external flash, and other peripherals.

* nRF SoC-only current measurement — Keep **SW6** set to **nRF ONLY**, but power the external flash from **VDD**.
  This allows the application to run while minimizing additional current paths.
  This option requires a hardware modification of your development kit:

  * Reconfigure the external flash power source solder bridges so that the flash is powered from **VDD**:

    * **SB16 (VDD_PER)**: **OPEN** (remove the solder bridge)
    * **SB17 (VDD)**: **SHORT** (add a solder bridge)
    * **SB18 (VDD_nRF)**: **OPEN**

  * For details on external memory, see the datasheet of your device (for example, `nRF5340 DK external memory`_).

All measurements in this document were taken using the nRF SoC-only current measurement approach.

Power Profiler Kit II settings
==============================

Configure the Power Profiler Kit II as follows:

* Mode: Source Meter
* Supply voltage: ``3000`` mV
* Sampling rate: ``100 000`` samples per second
* Capture duration: ``60`` s

Procedure
=========

Complete the following steps:

#. Connect the PPK to your development kit as the power source.
#. Program the device under test (DUT):

   Use the option that matches your hardware setup.
   Switch positions differ during flashing and during measurement.

   .. tabs::

      .. tab:: nRF SoC-only current measurement

         #. Power up external flash using VDD.
         #. Set switches to **VEXT OFF** and **nRF DEFAULT**.
         #. Flash the firmware built in :ref:`Build configuration <nfc_power_measurements_build>`.
         #. Set switches to **VEXT ON** and **nRF ONLY** for measurement.

      .. tab:: Full development kit current measurement

         #. Set switches to **VEXT OFF** and **nRF DEFAULT**.
         #. Enable PPK power output.
         #. Flash the firmware built in :ref:`Build configuration <nfc_power_measurements_build>`.

#. Enable PPK power output.
#. Start the capture in Power Profiler.

   For IDLE mode, do not present a tag or User Device at the Reader.
   For ACTIVE mode, hold an NFC tag or User Device at the Reader for the capture window.

#. After 60 seconds, the measurement ends automatically.
   You can also stop it manually.

.. _nfc_power_measurements_nfc_reader:

NFC reader current measurement
******************************

In this setup, the NFC module is powered from the development kit, while the development kit itself is powered through USB.
The PPK measures only the NFC module current.

Hardware connection
===================

Connect the development kit and the PPK as shown in the following schematic:

.. figure:: /images/nfc_current_schematic_module_st25r300_ppk2.png
   :scale: 85%
   :alt: Schematic for measuring NFC module current.

Development kit switch settings
===============================

Configure the development kit as follows:

* **POWER** switch: **ON**
* **nRF power source**: **VDD**
* **VEXT->nRF**: **OFF**
* **SW6 (nRF ONLY|DEFAULT)**: **nRF DEFAULT**

Power Profiler Kit II settings
==============================

Configure the Power Profiler Kit II as follows:

* Mode: Ampere Meter
* Capture duration: ``60`` s

Procedure
=========

#. Power the development kit using USB.
#. Flash the firmware built in :ref:`Build configuration <nfc_power_measurements_build>`.
#. Connect the PPK in series between the development kit **VDD** and the NFC module power input, as shown in the schematic above.
#. Start the capture in the Power Profiler app.

   For IDLE mode, do not present a tag or User Device at the Reader.
   For ACTIVE mode, hold an NFC tag or User Device at the Reader for the capture window.

#. After 60 seconds, the measurement ends automatically.
   You can also stop it manually.

Analyzing results
*****************

The following figures show current consumption captured on an nRF5340 DK with an `X-NUCLEO-NFC12A1`_ expansion board using the build configuration described above.

IDLE mode
=========

.. figure:: /images/nfc_current_plot_soc_idle_nrf5340.png
   :scale: 70%
   :alt: nRF5340 SoC current consumption in IDLE mode.

   nRF5340 SoC current consumption in IDLE mode (SoC measurement setup).

.. figure:: /images/nfc_current_plot_module_idle_x_nucleo_nfc12a1.png
   :scale: 70%
   :alt: X-NUCLEO-NFC12A1 current consumption in IDLE mode.

   X-NUCLEO-NFC12A1 NFC module current consumption in IDLE mode (NFC reader measurement setup).

.. figure:: /images/nfc_current_plot_idle_wum_comparison.png
   :scale: 70%
   :alt: Average current in IDLE mode as a function of wake-up mode configuration.

   Average NFC module current in IDLE mode depending on wake-up mode configuration.

In IDLE mode, the X-NUCLEO-NFC12A1 module current consumption depends on the ``CONFIG_RFAL_WAKE_UP_MODE`` Kconfig choice.
This option adjusts RFAL driver settings that control the wake-up procedure.
Among other effects, it changes ``RFAL_WUM_PERIOD`` (wake-up timer period; how often wake-up measurements are performed) in the ``wakeupConfig`` structure, which affects average current consumption in IDLE mode.
See :ref:`nfc_integration` for the available wake-up mode options and :file:`drivers/nfc/stm/nfc_configs/Kconfig` for option definitions.

ACTIVE mode
===========

.. figure:: /images/nfc_current_plot_active_worker_interval_comparison.png
   :scale: 70%
   :alt: nRF5340 SoC average current in ACTIVE mode as a function of NFC worker interval.

   nRF5340 SoC average current in ACTIVE mode depending on ``CONFIG_RFAL_NFC_WORKER_INTERVAL_MS``.

In ACTIVE mode, development kit current consumption depends on ``CONFIG_RFAL_NFC_WORKER_INTERVAL_MS`` (the time between consecutive NFC worker executions).
Configure it in :file:`applications/*/src/aliro/platform/nfc/Kconfig`.
However, the largest source of total current in this mode is the NFC module itself.

RFO tuning
==========

The RF output (RFO) driver resistance has a significant impact on NFC module current consumption in ACTIVE mode.
Increasing the RFO output resistance multiplier reduces NFC module current in ACTIVE mode, but it also reduces sensitivity and read range.

.. figure:: /images/nfc_current_plot_active_rfo_comparison_st25r300.png
   :scale: 70%
   :alt: X-NUCLEO-NFC12A1 average current in ACTIVE mode as a function of RFO driver resistance multiplier.

   X-NUCLEO-NFC12A1 average current in ACTIVE mode depending on RFO driver resistance multiplier.

You can configure the RFO driver resistance using the RFAL API:

.. code-block:: c

   rfalChipSetRFO(uint8_t rfo);

The ``rfo`` parameter is a value from 0 (lowest output resistance) to 15 (highest output resistance).
For details, see the `ST25R300 datasheet`_.

Other parameters also affect NFC current consumption.
You can find them in :file:`applications/*/src/aliro/platform/nfc/Kconfig` and :file:`drivers/nfc/stm/nfc_configs/Kconfig`.
For example, ``RFAL_WAKEUP_NPOLLS`` controls how many polling cycles are performed before and after Wake-Up mode, which can impact average NFC module current in ACTIVE mode.
See the `RFAL documentation`_ for more details.

Related documentation
*********************

* :ref:`nfc_integration` — Reference RFAL architecture and Kconfig options referenced in the plots above.
* :ref:`nfc_custom_integration` — Performance factors to consider when porting a third-party NFC transceiver.
