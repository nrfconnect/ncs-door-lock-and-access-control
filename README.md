# ncs-door-lock-app
NCS manifest repo for the door lock reference application

## Building local documentation

To build the documentation locally, you must complete a minimal setup.

Navigate to the ``docs`` folder, and install Sphinx and other documentation dependencies by running the following command:

```shell
pip install -r requirements.txt
```

Once successfully installed, you can build the Sphinx documentation from the ``docs`` folder:

```shell
make html
```

The output is stored in the ``build`` folder.

