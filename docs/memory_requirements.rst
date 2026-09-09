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

.. tabs::

   .. group-tab:: nRF52840 DK

      The following table lists memory requirements for the applications running on the nRF52840 DK.

      .. list-table::
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
           - 891
           - 4
           - 32
           - 955
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
           - 396
           - 4
           - 32
           - 460
           - 92

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 28
           - 299
           - 4
           - 32
           - 363
           - 80

   .. group-tab:: nRF5340 DK

      The following table lists memory requirements for the applications running on the nRF5340 DK.

      .. list-table::
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
           - 833
           - 4
           - 32
           - 901
           - 223

         * - :ref:`Matter and Aliro Door Lock <doc_aliro_matter_door_lock_application>` (Release)
           - 32
           - 757
           - 4
           - 32
           - 825
           - 204

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Debug)
           - 32
           - 367
           - 4
           - 32
           - 435
           - 89

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 32
           - 262
           - 4
           - 32
           - 330
           - 76

   .. group-tab:: nRF54L15 DK

      The following table lists memory requirements for the applications running on the nRF54L15 DK.

      .. list-table::
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
           - 899
           - 4
           - 40
           - 995
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
           - 408
           - 4
           - 40
           - 504
           - 93

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 52
           - 304
           - 4
           - 40
           - 400
           - 80

   .. group-tab:: nRF54LM20 DK

      The following table lists memory requirements for the applications running on the nRF54LM20 DK.

      .. list-table::
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
           - 899
           - 4
           - 48
           - 1003
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
           - 407
           - 4
           - 48
           - 511
           - 93

         * - :ref:`Aliro Access Control <doc_aliro_access_control_application>` (Release)
           - 52
           - 304
           - 4
           - 48
           - 408
           - 81
