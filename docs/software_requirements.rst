.. _sw_requirements:

Software requirements
#####################

.. contents::
   :local:
   :depth: 2

This page summarizes the requirements for setting up and working with the |app_name|.

.. _sdk_set_up:

|NCS|
*****

Before you start working with the application, you must install the |NCS| development environment and |NCS| toolchain.

Prepare the environment:

1. From the `Installing the nRF Connect SDK`_ page, complete the following steps:

   a. Updating operating system
   #. Installing prerequisites
   #. Installing the |NCS| toolchain

#. Once completed, open the repository and locate the :file:`ncs` directory.
   By default, this is one level up from the location where you installed the toolchain.
   This directory holds all |NCS| repositories.

#. Start the toolchain environment for your operating system by using the following command:

   .. code-block:: console

      nrfutil sdk-manager toolchain launch --ncs-version v3.3.0 --shell

#. Initialize west:

   .. code-block:: console

      west init -m https://github.com/nrfconnect/ncs-door-lock-and-access-control --mr main project-workspace
      cd project-workspace

   This command clones the `ncs-door-lock-and-access-control`_ manifest repository into :file:`project-workspace`.

#. Use the following command to clone the project repository and all of its submodules:

   .. code-block:: console

      west update

   This might take some time, depending on your connection.

#. Apply the patches to the workspace:

   .. code-block:: console

      west patch apply

#. Export a `Zephyr CMake package`_.
   This enables CMake to automatically load the boilerplate code required for building |NCS| applications:

   .. code-block:: console

      west zephyr-export

   With the default location to install the toolchain, your directory structure will look similar to the following:

   .. code-block:: none

      ncs
      ├─── toolchains
      │  └─── <toolchain-installation>
      └─── project-workspace
         ├─── .west
         ├─── bootloader
         ├─── modules
         ├─── ncs-door-lock-and-access-control
         ├─── nrf
         ├─── nrfxlib
         ├─── zephyr
         └─── ...

   In this simplified structure preview, *<toolchain-installation>* corresponds to the toolchain version.

#. Complete `setting up the command-line build environment`_.

   Once you have completed all the steps, the development environment will be correctly configured.

Aliro Certification Tool
************************

If you are developing a door lock with Aliro support, and you want to execute the Aliro certification tests, you must have the official :ref:`Aliro Certification Tool <hw_requirements_test_harness>`.

Matter development tools
*************************

If you are developing a door lock with Matter support, you need the `Matter tools`_.
