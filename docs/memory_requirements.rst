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
On the nRF7120 DK, the applications do not use MCUboot, and the Wi-Fi patch ROM value corresponds to the Wi-Fi ROM patches merged into the application image.

.. note::
    The provided values might differ between the |REPO_NAME| releases.
    The Application ROM, Total ROM, and Total RAM depend on the build configuration in use and are different if non-default options are enabled.

.. include:: /include/memory_values_commit.txt

.. tabs::

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
           - 716
           - 4
           - 32
           - 784
           - 172

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 32
           - 612
           - 4
           - 32
           - 680
           - 167

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 32
           - 844
           - 4
           - 32
           - 912
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
           - 387
           - 4
           - 32
           - 455
           - 91

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 32
           - 269
           - 4
           - 32
           - 337
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
           - 786
           - 4
           - 40
           - 882
           - 167

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 52
           - 678
           - 4
           - 40
           - 774
           - 162

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 52
           - 911
           - 4
           - 40
           - 1007
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
           - 430
           - 4
           - 40
           - 526
           - 95

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 52
           - 310
           - 4
           - 40
           - 406
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
           - 789
           - 4
           - 48
           - 893
           - 168

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 52
           - 680
           - 4
           - 48
           - 784
           - 162

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 52
           - 911
           - 4
           - 48
           - 1015
           - 216

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 52
           - 824
           - 4
           - 48
           - 928
           - 197

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 52
           - 429
           - 4
           - 48
           - 533
           - 95

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 52
           - 310
           - 4
           - 48
           - 414
           - 81

   .. group-tab:: nRF7120 DK

      The following table lists memory requirements for the applications running on the nRF7120 DK.

      .. list-table:: |mem_values_caption|
         :header-rows: 1

         * - Application
           - MCUboot ROM
           - Application ROM
           - Wi-Fi patch ROM
           - Factory data
           - Settings
           - Total ROM
           - Total RAM (incl. static heap)

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Debug)
           - 0
           - 946
           - 48
           - 4
           - 48
           - 1046
           - 340

         * - :ref:`Matter Door Lock <doc_matter_door_lock_application>` (Release)
           - 0
           - 813
           - 48
           - 4
           - 48
           - 913
           - 333

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Debug)
           - 0
           - 964
           - 48
           - 4
           - 48
           - 1064
           - 384

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 0
           - 887
           - 48
           - 4
           - 48
           - 987
           - 364

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 0
           - 797
           - 48
           - 4
           - 48
           - 897
           - 325

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 0
           - 717
           - 48
           - 4
           - 48
           - 817
           - 314
