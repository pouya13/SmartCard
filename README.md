
# Smart Card Reader
This repository contains codes to request smart cards for internal authentication. During this process, the smart card will use the master key to encrypt a random message and return the suitable answer. The final goal of this project is to use this information to record side-channel traces and deploy an attack on smart card in order to retrieve the master key.
This project uses the Smart Card Library (SCLib) to send APDU commands to the smart card and get the response.

# Requirements
- Arduino IDE
- Smart Card
- A printed circuit board (PCB)

# PCB
I have designed a custom Printed Circuit Board (PCB) using Altium Designer to deploy my desired attack. The PCB file is available at the PCB directory.