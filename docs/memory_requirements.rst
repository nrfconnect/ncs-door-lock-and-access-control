.. _hw_requirements_memory:

Memory requirements
###################

.. contents::
   :local:
   :depth: 2

This page details the memory requirements for the |REPO_NAME| :ref:`applications <doc_applications>`.

.. _hw_requirements_memory_ram_flash:

RAM and flash memory requirements
*********************************

RAM and flash memory requirement values differ depending on the DK and the programmed application.

The following bar charts and tables list memory requirement values for the |APPS_NAME| built in their default configuration, for both the ``debug`` and ``release`` build types.
Memory layout is taken from the DTS files used by each application, while memory usage is taken from the build output.
Values are provided in kilobytes (KB).

Table columns are grouped by internal NVM and RAM.
Application and RAM cells show used and free space separated by ``/``.
Other NVM columns list the reserved partition size for that region.

.. note::
    The provided values might differ between the |REPO_NAME| releases.
    The Application and RAM values depend on the build configuration in use and are different if non-default options are enabled.

.. note::
    The RAM values show statically allocated memory only, including thread stacks and statically allocated heaps.
    The free RAM is not necessarily available to the application, as it can be used at runtime, for example, by the C library ``malloc`` arena.

.. include:: /include/memory_values_commit.txt

|mem_values_caption|.

.. tabs::

   .. group-tab:: Charts

      .. tabs::

         .. group-tab:: nRF52840 DK

            Memory requirements for the applications running on the nRF52840 DK.

            .. memory-board::
               :board: nrf52840

         .. group-tab:: nRF5340 DK

            Memory requirements for the applications running on the nRF5340 DK.

            .. memory-board::
               :board: nrf5340

         .. group-tab:: nRF54L15 DK

            Memory requirements for the applications running on the nRF54L15 DK.

            .. memory-board::
               :board: nrf54l15

         .. group-tab:: nRF54LM20 DK

            Memory requirements for the applications running on the nRF54LM20 DK.

            .. memory-board::
               :board: nrf54lm20

   .. group-tab:: Tables

      .. tabs::

         .. group-tab:: nRF52840 DK

            The following table lists memory requirements for the applications running on the nRF52840 DK.

            .. memory-table::
               :board: nrf52840

         .. group-tab:: nRF5340 DK

            The following table lists memory requirements for the applications running on the nRF5340 DK.

            .. memory-table::
               :board: nrf5340

         .. group-tab:: nRF54L15 DK

            The following table lists memory requirements for the applications running on the nRF54L15 DK.

            .. memory-table::
               :board: nrf54l15

         .. group-tab:: nRF54LM20 DK

            The following table lists memory requirements for the applications running on the nRF54LM20 DK.

            .. memory-table::
               :board: nrf54lm20

Reference memory layouts
************************

The following bar charts and tables show the memory maps for each of the supported DKs.
All the |APPS_NAME| use the same memory layout on a given DK.
The partition sizes match the :ref:`RAM and flash memory requirements <hw_requirements_memory_ram_flash>` listed above and are defined in the :file:`dts/door_lock/common` devicetree files.

The external flash holds the secondary MCUboot slot, which stores the application update image during DFU.
On the nRF5340 DK, it also holds the network core update image.

.. tabs::

   .. group-tab:: Charts

      .. tabs::

         .. group-tab:: nRF52840 DK

            The following memory map is valid for the applications running on the nRF52840 DK.

            .. memory-layout-board::
               :board: nrf52840

         .. group-tab:: nRF5340 DK

            The following memory map is valid for the applications running on the nRF5340 DK.

            .. memory-layout-board::
               :board: nrf5340

         .. group-tab:: nRF54L15 DK

            The following memory map is valid for the applications running on the nRF54L15 DK.

            .. memory-layout-board::
               :board: nrf54l15

         .. group-tab:: nRF54LM20 DK

            The following memory map is valid for the applications running on the nRF54LM20 DK.

            .. memory-layout-board::
               :board: nrf54lm20

         .. group-tab:: nRF54LM20 DK with internal memory only

            The following memory map is valid for the applications running on the nRF54LM20 DK with internal memory only.

            Only the :ref:`Matter Door Lock <doc_matter_door_lock_application>` application supports this memory layout, using the ``internal`` file suffix.

            .. memory-layout-board::
               :board: nrf54lm20_internal

   .. group-tab:: Tables

      .. tabs::

         .. group-tab:: nRF52840 DK

            The following table lists memory partitions for the applications running on the nRF52840 DK.

            .. memory-layout-table::
               :board: nrf52840

         .. group-tab:: nRF5340 DK

            The following table lists memory partitions for the applications running on the nRF5340 DK.

            .. memory-layout-table::
               :board: nrf5340

         .. group-tab:: nRF54L15 DK

            The following table lists memory partitions for the applications running on the nRF54L15 DK.

            .. memory-layout-table::
               :board: nrf54l15

         .. group-tab:: nRF54LM20 DK

            The following table lists memory partitions for the applications running on the nRF54LM20 DK.

            .. memory-layout-table::
               :board: nrf54lm20

         .. group-tab:: nRF54LM20 DK with internal memory only

            The following table lists memory partitions for the applications running on the nRF54LM20 DK with internal memory only.

            Only the :ref:`Matter Door Lock <doc_matter_door_lock_application>` application supports this memory layout, using the ``internal`` file suffix.

            .. memory-layout-table::
               :board: nrf54lm20_internal
