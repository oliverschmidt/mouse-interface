/*

MIT License

Copyright (c) 2024 Thorsten Brehm
Copyright (c) 2025 Oliver Schmidt (https://a2retro.de/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <pico/multicore.h>
#include <a2pico.h>

#include "MouseInterfaceCard.h"

#include "board.h"

extern const __attribute__((aligned(4))) uint8_t firmware[];

// offset to the currently seleced page in the firmware
static uint32_t offset;

static void __time_critical_func(reset)(bool asserted) {
    if (asserted) {
        offset = 0;
        
        mouseControllerReset();
    }
}

void __time_critical_func(board)(void) {

    a2pico_init(pio0);

    a2pico_resethandler(&reset);

    while (true) {
        uint32_t pico = a2pico_getaddr(pio0);
        uint32_t addr = pico & 0x0FFF;
        uint32_t io   = pico & 0x0F00;      // IOSTRB or IOSEL
        uint32_t strb = pico & 0x0800;      // IOSTRB
        uint32_t read = pico & 0x1000;      // R/W

        if (read) {
            if (!io) {  // DEVSEL
                a2pico_putdata(pio0, PIA6520_read(addr));
            } else if (!strb) {  // IOSEL
                a2pico_putdata(pio0, firmware[addr & 0xFF | offset]);
            }

        } else {
            uint32_t data = a2pico_getdata(pio0);

            if (!io) {  // DEVSEL
                // PIA registers are being written
                // PIA6520_fastwrite(address,value);
                // code just inlined here - so we can add debug hooks...
                switch (addr & 0x3)
                {
                    case 0x0:
                        if (Pia.CRA & 0x04)
                            Pia.ORA = data;
                        else
                            Pia.DDRA = data;
                        break;
                    case 0x1:
                        Pia.CRA = data & 0x3F;
                        break;
                    case 0x2:
                        if (Pia.CRB & 0x04)
                            Pia.ORB = data;
                        else
                            Pia.DDRB = data;
                        // prepare offset - so we don't need to do that in a tight read-cycle
                        offset = ((Pia.ORB & Pia.DDRB & 0x0E) << 7);
                        break;
                    case 0x3:
                        Pia.CRB = data & 0x3F;
                        break;
                }
            }
        }
    }
}
