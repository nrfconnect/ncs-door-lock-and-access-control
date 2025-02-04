.. _sw_requirements:

Software requirements
#####################

.. _sdk_set_up:

nRF Connect SDK
---------------

To work with |APP_NAME|, you need to install the nRF Connect SDK development environment and the nRF Connect SDK toolchain.
Follow the `Installing the nRF Connect SDK`_ instruction with one exception.

In the `Get the nRF Connect SDK code`_ installaltion step for, the ``add-on`` skip step 4, and use following commands:

.. code-block:: bash

   west init -m https://github.com/nrfconnect/ncs-door-lock-app --mr main door-lock-workspace
   cd door-lock-workspace

This will clone the ``add-on`` manifest repository `ncs-door-lock-app`_ into ``door-lock-workspace``.

.. note::
   Before executing ``west update`` make sure that you have access to `ncs-aliro`_ private repository.

Aliro Certification Tool
------------------------

Install official `Aliro Certification Tool`_ to be able to execute test cases.
Follow :ref:`test harness hardware <test_harness_hardware>` to get the tool.
