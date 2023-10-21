//
// Created by eddie on 10/21/23.
//

#include <btstack_defines.h>
#include <hci.h>
#include <ble/sm.h>
#include <ble/gatt-service/hids_device.h>
#include <btstack.h>
#include <pico/multicore.h>

#include "include/bluetooth.hpp"
#include "keys.h"


static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t protocol_mode = 1;

// clang-format off
static const uint8_t adv_data[] = {
        // Flags general discoverable, BR/EDR not supported
        0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
        // Name
        0x0d, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'D', 'e', 'r', 'p', 'b', 'o', 'a', 'r', 'd', ' ', ' ', ' ',
        // 16-bit Service UUIDs
        0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
        ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
        // Appearance HID - Keyboard (Category 15, Sub-Category 1)
        0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
};
// clang-format on
static const uint8_t adv_data_len = sizeof(adv_data);

static void packet_handler(uint8_t packet_type, [[maybe_unused]] uint16_t channel,
                           uint8_t *packet, [[maybe_unused]] uint16_t size) noexcept {

    uint8_t report[] = {0, 0, 4, 0, 0, 0, 0, 0};
    // Early return if we don't care about packet
    if (packet_type != HCI_EVENT_PACKET)
        return;

    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = HCI_CON_HANDLE_INVALID;
            break;
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;
        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            sm_numeric_comparison_confirm(
                    sm_event_passkey_input_number_get_handle(packet));
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            break;
        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    // hid_embedded_start_typing();
                    break;
                case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                    con_handle =
                            hids_subevent_boot_keyboard_input_report_enable_get_con_handle(
                                    packet);
                    break;
                case HIDS_SUBEVENT_PROTOCOL_MODE:
                    // protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW:
                    // typing_can_send_now();
                    hids_device_send_input_report(con_handle, report, sizeof(report));
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

Bluetooth::Bluetooth() {
    this->battery = 100;

    battery_service_server_init(this->battery);
    device_information_service_server_init();

    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION |
                                       SM_AUTHREQ_BONDING);

    att_server_init(profile_data, nullptr, nullptr);

    hids_device_init(0, hid_desc_boot, sizeof(hid_desc_boot));

    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr = {};
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0,
                                  null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t *) adv_data);
    gap_advertisements_enable(1);

    hids_device_register_packet_handler(packet_handler);
    btstack_packet_callback_registration_t bt_packet_cb;
    bt_packet_cb.callback = &packet_handler;
    hci_add_event_handler(&bt_packet_cb);

    btstack_packet_callback_registration_t sm_packet_cb;
    sm_packet_cb.callback = &packet_handler;
    sm_add_event_handler(&sm_packet_cb);

    hci_power_control(HCI_POWER_ON);

    multicore_launch_core1(btstack_run_loop_execute);
}
