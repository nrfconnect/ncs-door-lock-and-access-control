.. _sw_requirements:

Software requirements
#####################

This page summarizes the requirements for setting up and working with the |app_name|.

.. _sdk_set_up:

|NCS|
*****

Before you start working with the application, you must have the following:

* Installed the |NCS| development environment and |NCS| toolchain
* Access to the `ncs-aliro`_ private repository

Complete `Installing the nRF Connect SDK`_.
In the `Get the nRF Connect SDK code`_ subsection, step 4, run the following command instead:

.. code-block:: bash

   west init -m https://github.com/nrfconnect/ncs-door-lock-app --mr main door-lock-workspace
   cd door-lock-workspace

.. note::
   |VSC| does not support private add-ons.
   You must use command line instructions to clone the repository.

This command will clone the `ncs-door-lock-app`_ add-on manifest repository into :file:`door-lock-workspace`.
Once executed, complete the remaining steps.

Aliro Certification Tool
************************

You must have the official :ref:`Aliro Certification Tool to be able to execute test cases <hw_requirements_test_harness>`.
