.. _addon_architecture_matter:
.. _wireless_technologies_matter:

Matter
######

The |APPS_NAME| integrate with the :doc:`Matter <ncs-matter:matter/index>` protocol stack provided by the |NCS| to enable seamless smart home integration and provisioning of Aliro-specific credentials through smart home ecosystems.

Applications are built on top of the :doc:`Matter door lock <ncs-matter:samples/lock/README>` sample available in the |NCS|, and reuse most of its functionality, including the Matter door lock cluster, user and credential management, and scheduled timed access.

For detailed information about Matter support in the |NCS|, including architecture, commissioning, security, and development guidelines, see the following pages:

* :doc:`Matter overview <ncs-matter:matter/overview/index>` — Introduction to Matter, covering its architecture, firmware upgrade process, and details on network commissioning and security.
* :doc:`Matter getting started <ncs-matter:matter/getting_started/index>` — Guide on developing Matter applications in the |NCS|, including the necessary tools and how to set up the testing environment.
* :doc:`Matter end product <ncs-matter:matter/end_product/index>` — Information on developing a Matter end product, including supported security features, recommended configurations, and the Matter certification process.

.. note::
   Currently, the |APP_NAME| supports Matter only with Thread technology.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   matter_power_measurements.rst
