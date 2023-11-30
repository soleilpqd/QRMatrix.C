# Vocabulary

**QR Code** is a method to encode human text into a 2D graphical image (symbology) - a square of black and white dots. That 2D image is called **QR Symbol**. The name of each dot is **QR Module**.

> Common QR Codes look alike a chess board, so in source code I call **QR Symbol** as **QR Board** and **QR Module** as **QR Cell**.

There are 2 types of **QR Module**, named after their colors in common QR Codes: **black** modules & **white** modules.
> But it does not mean that they must have those colors. These 2 types of **QR Module** should have different colors with high contrast (so they can have other colors than black and white, or **black** module can be white. Let's say the QR Decoder being color-blind and see the QR code in grayscale).

QR Standard uses **QR Version** to index the dimension or size of **QR Symbol**, eg. **QR Version 1** is **QR Symbol** 21x21 (21 **modules** on each side, and 441 **modules** on surface).
 - **QR Version** range is from 1 to 40. **MicroQR version** range is from 1 to 4.
 - The offset between 2 consecutive *QR versions* is 4 modules, eg. **Version 1** is 21x21, **Version 2** is 25x25.
 - The offset between 2 consecutive *MicroQR versions* is 2 modules, eg. **Version 1** is 11x11, **Version 2** is 13x13.

The **QR Symbol** should (not *must*) be surrounded by an empty area (commonly same color as **white module**), named as **Quite zone**.
 - For normal **QR**, **Quiet zone** has 4 **modules** width.
 - For **MicroQR**, **Quiet zone** has 2 **modules** width.
