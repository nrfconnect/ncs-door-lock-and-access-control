.. _nfc_power_measurements:

NFC current measurements
########################

.. contents::
   :local:
   :depth: 2

This page describes a method for measuring NFC current consumption of an |APP_NAME| using `Power Profiler Kit II`_ (PPK).

Overview
********

The NFC current measurements are separated into two parts:

* :ref:`nfc_power_measurements_nrf_soc`
* :ref:`nfc_power_measurements_nfc_reader`

Choose the measurements based on what you want to analyze and optimize:

* If you wish to check only the SoC current consumption, focus on the :ref:`development kit SoC current measurements <nfc_power_measurements_nrf_soc>`. 
* If your focus is the NFC reader, use the :ref:`NFC reader current measurements instead <nfc_power_measurements_nfc_reader>`. 
* To evaluate the combined impact of both components, measure the overall power consumption of the system, including both the SoC and the NFC reader.

The analysis covers two functional modes of the NFC reader:

* IDLE - Average current over the entire duration of the measurement time window with no NFC tag present (e.g., a 60-second measurement window).
* ACTIVE - Average current when an NFC tag is present in a given time window.

Prerequisites
*************

Ensure you have the following hardware:

* `Power Profiler Kit II`_
* Development kit (for example, `nRF5340 DK`_ which was used for the measurements in this document)
* NFC reader expansion board (`X-NUCLEO-NFC12A1`_)

Ensure you have installed the following software:

* `nRF Connect for Desktop`_ with the Power Profiler app

Build configuration
*******************

All measurements in this document were taken with the following build configuration:

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp -p -- -DFILE_SUFFIX=release

In the Aliro standalone application, the *release* configuration keeps the UART shell enabled (used for provisioning the Aliro Reader).
This adds background UART activity and can impact the measured current consumption.

To remove this additional current consumption, disable the UART node used by the shell/console in your DTS overlay, for example:

.. code-block:: dts

   &uart0 {
           status = "disabled";
   };

Hardware setup
****************

The following subsections describe how the development kit and PPK are connected and configured in the given measurement scenario.

.. _nfc_power_measurements_nrf_soc:

nRF SoC current measurement
===========================

Connect the development kit and the PPK as shown in the following schematic:

.. figure:: /images/nfc_current_schematic_soc_nrf5340_ppk2.png
   :scale: 85%
   :alt: Schematic for measuring nRF SoC current.

In this setup, the DK is powered directly by the PPK, while the NFC module remains powered from the USB port.

Development kit configuration
=============================

Configure the DK as follows:

- **POWER** switch: **ON**
- **nRF power source**: **VDD**
- **VEXT->nRF**: **ON**
- **SW6 (nRF ONLY|DEFAULT)**: **nRF ONLY** (SEGGER, LEDs and external flash powered off)

In **nRF ONLY** mode, the external flash memory is powered down.
Since the |APP_NAME| uses external flash, it will not start correctly unless you power up the flash.

Whether the external flash current is included in the measurements depends on how it is powered:

* Powered from **VDD**: excluded from the measurements
* Powered from **VDD nRF**: included in the measurements

Choose one of the following approaches:

* Full development kit current measurement - Set **SW6** to **nRF DEFAULT**.
  This measures the full DK current, including SEGGER, external flash, and other peripherals.

* nRF SoC-only current measurement - Keep **SW6** set to **nRF ONLY**, but power the external flash from **VDD**.
  This allows the application to run while minimizing additional current paths.
  This option requires a hardware modification of your development kit:

  * Reconfigure the external flash power source solder bridges so that the flash is powered from **VDD**:

    * **SB16 (VDD_PER)**: **OPEN** (remove the solder bridge)
    * **SB17 (VDD)**: **SHORT** (add a solder bridge)
    * **SB18 (VDD_nRF)**: **OPEN**

  * For details, see `nRF5340 DK external memory`_.

All measurements in this document were taken using the nRF SoC-only current measurement approach.

Power Profiler Kit II configuration
===================================

Configure the Power Profiler Kit II as follows:

* Mode: Source Meter
* Supply voltage: ``3000`` mV
* Sampling rate: ``100 000`` samples per second
* Capture duration: ``60`` s

Measuring current
*****************

This section describes the step-by-step procedure to capture the current measurements.

#. Connect PPK to your development kit as the power source.
#. Program the DUT:

   Use the option that matches your hardware setup:

   .. tabs::

      .. tab:: nRF SoC-only current measurement

         #. Power up external flash using VDD.
         #. Set switches to VEXT OFF and nRF DEFAULT.
         #. Flash the firmware.
         #. Set switches to VEXT ON and nRF ONLY.

      .. tab:: Development kit's entire current measurement

         #. Set switches to VEXT OFF and nRF DEFAULT.
         #. Enable PPK power output.
         #. Flash the firmware.

#. Enable power output.
#. Start the capture in Power Profiler.
#. After 60 seconds, the measurement ends automatically.
   You can also stop it manually.

.. _nfc_power_measurements_nfc_reader:

NFC module current
******************

Connect the development kit and the PPK as shown in the following schematic:

.. figure:: /images/nfc_current_schematic_module_st25r300_ppk2.png
   :scale: 85%
   :alt: Schematic for measuring NFC module current.

In this configuration, the NFC module is powered from the development kit, while the development kit itself is powered through USB.

Development Kit configuration
=============================

- **POWER** switch: **ON**
- **nRF power source**: **VDD**
- **VEXT->nRF**: **OFF**
- **SW6 (nRF ONLY|DEFAULT)**: **nRF DEFAULT**

Power Profiler Kit II configuration
===================================

- Mode: Ampere Meter
- Capture duration: ``60`` s

Measuring current
=================

This section describes the step-by-step procedure used to capture the current measurements.

#. Power the Development Kit setup using USB.
#. Connect PPK in series between the development kit's VDD and the NFC module's power input as shown in the :ref:`schematic <nfc_power_measurements_nfc_reader>`.
#. Start the capture in Power Profiler app.
#. After 60 seconds, the measurement ends automatically.
   You can also stop it manually.

Analyzing results
*****************

The following figures show current consumption for an nRF5340 DK and an `X-NUCLEO-NFC12A1`_ expansion board.

.. figure:: /images/nfc_current_plot_soc_idle_nrf5340.png
   :scale: 70%
   :alt: nRF5340 SoC current consumption in IDLE mode.

   nRF5340 SoC current consumption in IDLE mode.

.. figure:: /images/nfc_current_plot_module_idle_x_nucleo_nfc12a1.png
   :scale: 70%
   :alt: X-NUCLEO-NFC12A1 current consumption in IDLE mode.

   X-NUCLEO-NFC12A1 NFC module current consumption in IDLE mode.

.. figure:: /images/nfc_current_plot_idle_wum_comparison.png
   :scale: 70%
   :alt: Average current in IDLE mode as a function of wake-up mode configuration.

   Average current consumption depending on the NFC sensitivity settings in IDLE mode.

In IDLE mode, the X-NUCLEO-NFC12A1 module current consumption depends on the ``CONFIG_RFAL_WAKE_UP_MODE`` Kconfig choice.
This option adjusts RFAL driver settings that control the wake-up procedure (among other things, it changes ``RFAL_WUM_PERIOD`` (wake-up timer period; how often wake-up measurements are performed) in the ``wakeupConfig`` structure),
which affects the average current consumption of the NFC module in IDLE mode.
You can find available configurations in the :file:`drivers/nfc/stm/nfc_configs/Kconfig` file.

.. figure:: /images/nfc_current_plot_active_worker_interval_comparison.png
   :scale: 70%
   :alt: Average current in ACTIVE mode as a function of wake-up mode configuration.

In ACTIVE mode, the Development Kit current consumption depends on the ``RFAL_NFC_WORKER_INTERVAL_MS`` parameter (the time between consecutive NFC worker executions).
However, the largest source of the total current in this mode is the NFC module itself.

RFO tuning
***********

This section explains how NFC RF output resistance configuration affects ACTIVE‑mode current consumption and how to adjust it while balancing power, sensitivity, and read range.

The RF output (RFO) driver resistance has a significant impact on the NFC module current consumption in ACTIVE mode.
Increasing the RFO output resistance multiplier reduces the NFC module current consumption in ACTIVE mode, but it also reduces sensitivity and read range.

.. figure:: /images/nfc_current_plot_active_rfo_comparison_st25r300.png
   :scale: 70%
   :alt: X-NUCLEO-NFC12A1 average current in ACTIVE mode as a function of RFO driver resistance multiplier.

You can configure the RFO driver resistance using the RFAL API:

.. code-block:: c

   rfalChipSetRFO(uint8_t rfo);

The ``rfo`` parameter is a value from 0 (lowest output resistance) to 15 (highest output resistance).
For details, see the `ST25R300 datasheet`_.

Other parameters also affect the NFC current consumption.

You can find them in the :file:`app/src/aliro/platform/nfc/Kconfig` and :file:`drivers/nfc/stm/nfc_configs/Kconfig` files.
For example, ``RFAL_WAKEUP_NPOLLS`` controls how many polling cycles are performed before and after Wake-Up mode, which can impact average current consumption of the NFC module in ACTIVE mode.

You can find more details about the RFAL driver configuration in the `RFAL documentation`_.
