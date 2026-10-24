/*

MIT License

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

#include <pico/stdlib.h>
#include <tusb.h>

#include <a2pico.h>

#include "board.h"
#include "MouseInterfaceCard.h"

void main(void) {    
    a2pico_init(board);

    stdio_uart_init_full(uart0, PICO_DEFAULT_UART_BAUD_RATE, a2pico_tx(), a2pico_rx());

    if (a2pico_led() >= 0) {
        gpio_init(a2pico_led());
        gpio_set_dir(a2pico_led(), GPIO_OUT);
    }

    tusb_init();

    mouseControllerInit();

    while (true) {
        tuh_task();

        mouseControllerRun();
    }
}
