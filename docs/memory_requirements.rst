.. _hw_requirements_memory:

Memory requirements
###################

.. contents::
   :local:
   :depth: 2

This page details the memory requirements for the |REPO_NAME| :ref:`applications <doc_applications>`.

RAM and ROM memory requirements
*******************************

RAM and ROM memory consumption differs depending on the DK and the programmed application.

The following tables list memory requirement values for the |APPS_NAME| built in their default configuration, for both the ``debug`` and ``release`` build types.
Values are provided in kilobytes (KB).
The MCUboot ROM, Factory data, and Settings values correspond to the fixed partition sizes defined for each SoC, while the Application ROM, Total ROM, and Total RAM values are derived from the compiled images.

.. note::
    The provided values might differ between the |REPO_NAME| releases.
    The Application ROM, Total ROM, and Total RAM depend on the build configuration in use and are different if non-default options are enabled.

.. include:: /include/memory_values_commit.txt

.. tabs::

   .. group-tab:: nRF52840 DK

      The following table lists memory requirements for the applications running on the nRF52840 DK.

      .. list-table:: |mem_values_caption|
         :header-rows: 1

         * - Application
           - MCUboot ROM
           - Application ROM
           - Factory data
           - Settings
           - Total ROM
           - Total RAM (incl. static heap)

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Debug)
           - 28
           - 776
           - 4
           - 32
           - 840
           - 160

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 28
           - 689
           - 4
           - 32
           - 753
           - 156

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 28
           - 892
           - 4
           - 32
           - 956
           - 207

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 28
           - 820
           - 4
           - 32
           - 884
           - 188

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 28
           - 400
           - 4
           - 32
           - 464
           - 92

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 28
           - 301
           - 4
           - 32
           - 365
           - 80

   .. group-tab:: nRF5340 DK

      The following table lists memory requirements for the applications running on the nRF5340 DK.

      .. list-table:: |mem_values_caption|
         :header-rows: 1

         * - Application
           - MCUboot ROM
           - Application ROM
           - Factory data
           - Settings
           - Total ROM
           - Total RAM (incl. static heap)

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Debug)
           - 32
           - 711
           - 4
           - 32
           - 779
           - 172

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 32
           - 622
           - 4
           - 32
           - 690
           - 167

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 32
           - 834
           - 4
           - 32
           - 902
           - 223

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 32
           - 758
           - 4
           - 32
           - 826
           - 204

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 32
           - 372
           - 4
           - 32
           - 440
           - 89

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 32
           - 265
           - 4
           - 32
           - 333
           - 77

   .. group-tab:: nRF54L15 DK

      The following table lists memory requirements for the applications running on the nRF54L15 DK.

      .. list-table:: |mem_values_caption|
         :header-rows: 1

         * - Application
           - MCUboot ROM
           - Application ROM
           - Factory data
           - Settings
           - Total ROM
           - Total RAM (incl. static heap)

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Debug)
           - 52
           - 780
           - 4
           - 40
           - 876
           - 167

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 52
           - 689
           - 4
           - 40
           - 785
           - 162

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 52
           - 901
           - 4
           - 40
           - 997
           - 216

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 52
           - 824
           - 4
           - 40
           - 920
           - 197

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 52
           - 413
           - 4
           - 40
           - 509
           - 93

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 52
           - 307
           - 4
           - 40
           - 403
           - 81

   .. group-tab:: nRF54LM20 DK

      The following table lists memory requirements for the applications running on the nRF54LM20 DK.

      .. list-table:: |mem_values_caption|
         :header-rows: 1

         * - Application
           - MCUboot ROM
           - Application ROM
           - Factory data
           - Settings
           - Total ROM
           - Total RAM (incl. static heap)

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Debug)
           - 52
           - 783
           - 4
           - 48
           - 887
           - 168

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 52
           - 691
           - 4
           - 48
           - 795
           - 162

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 52
           - 901
           - 4
           - 48
           - 1005
           - 216

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 52
           - 825
           - 4
           - 48
           - 929
           - 197

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 52
           - 412
           - 4
           - 48
           - 516
           - 93

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 52
           - 307
           - 4
           - 48
           - 411
           - 81
