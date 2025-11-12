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

Before you start working with the application, you must have installed the |NCS| development environment and |NCS| toolchain.

Prepare the environment:

1. From the `Installing the nRF Connect SDK`_ page, complete the following steps: updating operating system, installing prerequisities, and installing the |NCS| toolchain.

   .. note::

      |VSC| does not support private add-ons.
      You must use command-line to clone the repository.

#. Once completed, open the repository and locate the :file:`ncs` directory.
   By default, this is one level up from the location where you installed the toolchain.
   This directory will hold all |NCS| repositories.

#. Start the toolchain environment for your operating system using the following command:

   .. code-block:: console

      nrfutil sdk-manager toolchain launch --ncs-version v3.2.0-preview2 --shell

#. Initialize west:

   .. code-block:: console

      west init -m https://github.com/nrfconnect/ncs-door-lock-app --mr main door-lock-workspace
      cd door-lock-workspace

   This command will clone the `ncs-door-lock-app`_ add-on manifest repository into :file:`door-lock-workspace`.

#. Enter the following command to clone the project repository:

   .. code-block:: console

      west update

   Depending on your connection, this might take some time.

#. Export a `Zephyr CMake package`_.
   This allows CMake to automatically load the boilerplate code required for building |NCS| applications:

   .. code-block:: console

      west zephyr-export

   With the default location to install the toolchain, your directory structure now looks similar to this:

   .. code-block:: none

      ncs
      ├─── toolchains
      │  └─── <toolchain-installation>
      └─── <door-lock-workspace>
         ├─── .west
         ├─── bootloader
         ├─── modules
         ├─── ncs-door-lock-app
         ├─── nrf
         ├─── nrfxlib
         ├─── zephyr
         └─── ...

   In this simplified structure preview, *<toolchain-installation>* corresponds to the toolchain version and *<west-workspace>* corresponds to the SDK version name.

#. Complete `setting up the command-line build environment`_.

Once you have completed all the steps, the development environment should be correctly configured.

Aliro Certification Tool
************************

You must have the official :ref:`Aliro Certification Tool to be able to execute test cases <hw_requirements_test_harness>`.
