CoAP door lock service
#####################

A minimal nRF7120 DK demo of the ``coap_service`` snippet, based on the
NCS v3.5.0-preview4 Zephyr ``samples/net/sockets/coap_server`` sample.
The door starts locked; ``true`` means unlocked

Build and flash
***************

From the repository root in the NCS workspace:

.. code-block:: console

   west build -b nrf7120dk/nrf7120/cpuapp -d build_coap samples/coap_service -S coap_service
   west flash -d build_coap

The snippet enables the existing CoAP subsystem, which supplies its resource
linker section. The sample enables C++ and zcbor.
No credentials are compiled in or saved.

Connect and exercise
********************

Open the DK's serial shell at 115200 baud. For a WPA2-Personal network:

.. code-block:: console

   wifi connect -s "SSID" -k 1 -p "PASSWORD"
   wifi status
   net ipv4

Wait for DHCP to assign an IPv4 address. From a host on the same network,
use libcoap's ``coap-client-openssl``.

Read the state:

.. code-block:: console

   coap-client-openssl -m get coap://<ip>:5683/door-lock
   locked

Unlock, then read the state again to see ``unlocked`` (CBOR boolean true):

.. code-block:: console

   coap-client-openssl -m put -t application/cbor -e "%f5" coap://<ip>:5683/door-lock
   coap-client-openssl -m get coap://<ip>:5683/door-lock

Lock, then read the state again to see ``locked`` (CBOR boolean false):

.. code-block:: console

   coap-client-openssl -m put -t application/cbor -e "%f4" coap://<ip>:5683/door-lock
   coap-client-openssl -m get coap://<ip>:5683/door-lock

Press the DK's button 0 (``sw0``, ``DK_BTN1_MSK``) to toggle the same state,
then GET again. State changes are printed on the shell and indicated by LED0.

The service listens on UDP port 5683 without authentication or encryption.
Use only on a trusted test network. There is no actuator, persistence, or
automatic relocking.
