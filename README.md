# MyASICap
Astronomy capture tool for ASI cameras

![Sample](sample.png)

- On Raspberry Pi 4 for the ZWO EFW to work had to add a line to /lib/udev/rules.d/99-asi.rules as follows:  
`KERNEL=="hidraw*", ATTRS{idVendor}=="03c3", ATTRS{idProduct}=="1f01", MODE="0666"`
