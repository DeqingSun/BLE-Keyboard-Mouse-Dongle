# BLE Keyboard & Mouse Dongle with Web Bluetooth control

![dongle working with RPi](https://raw.githubusercontent.com/DeqingSun/BLE-Keyboard-Mouse-Dongle/master/images/control.gif)

This is a small Keyboard/Mouse USB dongle with BLE control. It is designed for headless systems such as Raspberry Pi, HTPC, etc; to provide handy control with your smartphone.

Due to latency of Bluetooth low energy, this project is not a replacement of mouse or keyboard for daily usage. However, it will be quite convenient to add a small dongle to a Raspberry Pi to setup account and SSH server before you have set up network.

![dongle on RPi](https://raw.githubusercontent.com/DeqingSun/BLE-Keyboard-Mouse-Dongle/master/images/dongleOnPi.jpg)

## Link to controller's webpage


<https://deqingsun.github.io/BLE-Keyboard-Mouse-Dongle/kbd_access.html>

## How it works


I chose CC2540 for this project. It has full speed USB port and Bluetooth low energy capability. There are already a lot of CC2540 USB dongle on market for Packet Sniffer or Btool with TI's reference design, so I don't need to build hardware, I can buy the small form-factor dongles and reprogram them. 

![USB info](https://raw.githubusercontent.com/DeqingSun/BLE-Keyboard-Mouse-Dongle/master/images/usbInfo.png)

On USB side, there are 3 interfaces, one keyboard, one mouse and one consumer control. This is a pretty standard configuration.

![BLE info](https://raw.githubusercontent.com/DeqingSun/BLE-Keyboard-Mouse-Dongle/master/images/ble_service.jpg)

On BLE side, I created a service with 6 characters. Keyboard can be triggered in 3 different characters, one character sends back keyboard led status, one character is used for mouse report, and last one for consumer report. 

I also put Eddystone-URL formatted data in advertisement packages. So smartphone can pop-up URL directly with Physical Web. I can just pull down notification bar and click instead of typing URL in browser.

![Web info](https://raw.githubusercontent.com/DeqingSun/BLE-Keyboard-Mouse-Dongle/master/images/webInterface.png)

On web there is one page hosted on Github. It connects to BLE dongle with Web Bluetooth API. You can drag cursor with joystick or type with virtual keyboard.

## How to make one

This firmware can be programmed to any CC2540 board with USB support as it doesn’t require any external peripheral. However if your development board don't have USB already, make sure there is a 1.5K pull-up resistor on D+ line.

![dongles](https://raw.githubusercontent.com/DeqingSun/BLE-Keyboard-Mouse-Dongle/master/images/stickAll.jpg)

I used a development board to write firmware, then I bought a few small-factor dongles and all of them have programming pads. 

In order to program them, I had to solder 3 wires, DC, DD and RST. Since there was already a USB port, I supplied power with a USB connector breakout board directly to save 2 wires.

Finally I used CC Debugger to re-flash all chips. I tried to use an Arduino as programmer but I didn't get setup correct. But it is worth trying if you don't have CC Debugger. Here are links:

<https://github.com/RedBearLab/CCLoader>

<https://github.com/wavesoft/CCLib>

<http://magictale.com/2884/flashing-ble112-with-arduino-based-cc-debugger/>

Once those dongle is re-flashed with new firmware, plug them to your Raspberry Pi and enjoy ~.

