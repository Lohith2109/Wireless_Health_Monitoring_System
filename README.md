# Wireless_Health_Monitoring_System

https://www.notion.so/Log-Book-FYP-316ba6ea69618020a2b4e11247bd43f1?source=copy_link

*Guidelines on PCB*
The PCB has two layers: F.Cu (front copper) and B.Cu (back copper). The vias in the PCB require a wire to be inserted through them and soldered on both sides to connect the two layers.

Soldering for the pin headers must be done only on the B.Cu side. The header pins for the Arduino HAT need to be pushed up fully so that the plastic spacer is positioned at the bottom, and they must be soldered on the B.Cu side. On the top side, only the plastic spacer of the Arduino HAT pins should be visible.

The remaining component pin headers should be placed in the normal orientation (the opposite direction of the Arduino HAT pins) and soldered on the B.Cu side.


*Guidelines on 3D enclosure*
The height of the boxes is not correct. After mounting the PCB on the Arduino and connecting the jumper wires, the overall height of the circuit was greater than I had predicted. This pushed the slit upwards, causing the lid to no longer function properly. I created a layer to hold the slit at the new height using hot glue and made another lid for the box.

The battery unit fits but can still move, making it difficult to switch the system on and off. I used hot glue to create a compartment to ensure the battery unit does not move. Additionally, I had to raise the height of a few components so they would be visible with the new lid. This was done using foam and cardboard, which were glued together.

Velcro has been used to assemble the box to avoid the use of screws. This applies to both units.
