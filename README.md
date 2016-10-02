# PMC-BIOS-Reader
An attempt to implement the LPC protocol in order to read (and eventually write) from a PMC 49fl004t-33jce chip using an STM32VLDiscovery board. 

How to Use
=======

All connections use GPIO port A.

RST - Pin 0
LFRAME - Pin 1
LAD[3:0] - Pin 2 to 5

Known Issues
========
- Doesn't work