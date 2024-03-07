# Linux-Payload (Autolycus)
Note: This payload is operational but development is ongoing. Do not test this payload on sensitive information.

This is an example of a payload that can be used in conjunction with the Nekrosis application to establish persistence on a Linux target. By virtue of being a deployable payload it is written in pure C and thus assumes no dependencies on the target system.

Currently the payload can be deployed with the COPY directive. The payload will work with the C2 server to copy all non-executable readable files to the remote controller. The C2 server setup has a few sensitivities that are specified in the opening comments of the main.c file. Essentially the controller has to both serve files (via a simple web server) and accept files (via a VSFTDP server). Exact configuration details can be found in main.c.
