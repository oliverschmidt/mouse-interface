/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

// From https://github.com/raspberrypi/pico-examples/blob/master/usb/host/host_cdc_msc_hid/hid_app.c

#include <pico/stdlib.h>
#include <tusb.h>

#include "MouseInterfaceCard.h"

// Each HID instance can have multiple reports
#define MAX_REPORT 4

static struct
{
    uint8_t report_count;
    tuh_hid_report_info_t report_info[MAX_REPORT];
} hid_info[CFG_TUH_HID];

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
    printf("HID device address = %d, instance = %d is mounted\n", dev_addr, instance);

    // Interface protocol (hid_interface_protocol_enum_t)
    const char *protocol_str[] = {"None", "Keyboard", "Mouse"};
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    printf("HID Interface Protocol = %s\n", protocol_str[itf_protocol]);

    // By default host stack will use activate boot protocol on supported interface.
    // Therefore for this simple example, we only need to parse generic report descriptor (with built-in parser)
    if (itf_protocol == HID_ITF_PROTOCOL_NONE)
    {
        hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len);
        printf("HID has %u reports\n", hid_info[instance].report_count);
    }

    // request to receive report
    // tuh_hid_report_received_cb() will be invoked when report is available
    if (!tuh_hid_receive_report(dev_addr, instance))
    {
        printf("Error: cannot request to receive report\n");
    }

#ifdef PICO_DEFAULT_LED_PIN
    gpio_put(PICO_DEFAULT_LED_PIN, true);
#endif
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    printf("HID device address = %d, instance = %d is unmounted\n", dev_addr, instance);

#ifdef PICO_DEFAULT_LED_PIN
    gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
}

static void process_mouse_report(hid_mouse_report_t const *report)
{
    static hid_mouse_report_t prev_report = {0};

    uint8_t button_changed_mask = report->buttons ^ prev_report.buttons;
    prev_report = *report;

//    if (button_changed_mask)
//    {
//        printf(" %c%c%c ",
//               report->buttons & MOUSE_BUTTON_LEFT ? 'L' : '-',
//               report->buttons & MOUSE_BUTTON_MIDDLE ? 'M' : '-',
//               report->buttons & MOUSE_BUTTON_RIGHT ? 'R' : '-');
//    }
//    printf("(%d %d %d)\n", report->x, report->y, report->wheel);

    if (button_changed_mask & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT))
    {
        bool pressed = (report->buttons & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT)) != 0;
        mouseControllerUpdateButton(0, pressed);
    }
    if (button_changed_mask & MOUSE_BUTTON_MIDDLE)
    {
        bool pressed = (report->buttons & MOUSE_BUTTON_MIDDLE) != 0;
        mouseControllerUpdateButton(1, pressed);
    }
    if (report->x || report->y)
    {
        mouseControllerMoveXY(report->x / 2, report->y / 2);
    }
}

static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    (void)dev_addr;

    uint8_t const rpt_count = hid_info[instance].report_count;
    tuh_hid_report_info_t *rpt_info_arr = hid_info[instance].report_info;
    tuh_hid_report_info_t *rpt_info = NULL;

    if (rpt_count == 1 && rpt_info_arr[0].report_id == 0)
    {
        // Simple report without report ID as 1st byte
        rpt_info = &rpt_info_arr[0];
    }
    else
    {
        // Composite report, 1st byte is report ID, data starts from 2nd byte
        uint8_t const rpt_id = report[0];

        // Find report id in the array
        for (uint8_t i = 0; i < rpt_count; i++)
        {
            if (rpt_id == rpt_info_arr[i].report_id)
            {
                rpt_info = &rpt_info_arr[i];
                break;
            }
        }

        report++;
        len--;
    }

    if (!rpt_info)
    {
        printf("Couldn't find report info !\n");
        return;
    }

    // For complete list of Usage Page & Usage checkout src/class/hid/hid.h. For examples:
    // - Keyboard                     : Desktop, Keyboard
    // - Mouse                        : Desktop, Mouse
    // - Gamepad                      : Desktop, Gamepad
    // - Consumer Control (Media Key) : Consumer, Consumer Control
    // - System Control (Power key)   : Desktop, System Control
    // - Generic (vendor)             : 0xFFxx, xx
    if (rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP)
    {
        switch (rpt_info->usage)
        {
        case HID_USAGE_DESKTOP_KEYBOARD:
            printf("HID receive keyboard report\n");
            break;

        case HID_USAGE_DESKTOP_MOUSE:
//            printf("HID receive mouse report\n");
            // Assume mouse follow boot report layout
            process_mouse_report((hid_mouse_report_t const *)report);
            break;

        default:
            break;
        }
    }
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    switch (itf_protocol)
    {
    case HID_ITF_PROTOCOL_KEYBOARD:
        printf("HID receive boot keyboard report\n");
        break;

    case HID_ITF_PROTOCOL_MOUSE:
//        printf("HID receive boot mouse report\n");
        process_mouse_report((hid_mouse_report_t const *)report);
        break;

    default:
        // Generic report requires matching ReportID and contents with previous parsed report info
        process_generic_report(dev_addr, instance, report, len);
        break;
    }

    // continue to request to receive report
    if (!tuh_hid_receive_report(dev_addr, instance))
    {
        printf("Error: cannot request to receive report\n");
    }
}
