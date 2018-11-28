#ifndef MAIN_LIBRARY_H_
#define MAIN_LIBRARY_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"

#include "freertos/event_groups.h"

#include <lwip/sockets.h>
#include "apps/sntp/sntp.h"

#include "sdkconfig.h"

#include "nvs_flash.h"

#include "mbedtls/md.h"

// **************************************************************************************************
// Prototype
esp_err_t event_handler(void *ctx, system_event_t *event);
void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);
void sntp_configuration(void);
void handle_send_packets(void);
int connection_initializer(void);
void wifi_setup(void);
void esp_check_error(int x);
// **************************************************************************************************

// **************************************************************************************************
// Define
#define IP                "192.168.1.1"  // IPv4 server
#define PORT              1500           // Port server

#define MSG_ERR           "-ER\r\n"      // End of sequence
#define MSG_OK            "+OK\r\n"      // Start of sequence
#define MSG_END           "+EN\r\n"      // Delimiter packet

#define MAX_APs           120            // Max number of AP
#define TIMER             60000          // Stop sniffing mode after 1 minute

#define CONNECTED_BIT     BIT0           // Connection bit - Waiting the ip assigned by AP
// **************************************************************************************************

// **************************************************************************************************
// Data for connection with the AP
wifi_config_t wifi_config = {
	.sta = {
		.ssid = "NameOfAP",
		.password = "superSecretPasswordOfAP"
	}
};
// **************************************************************************************************

// **************************************************************************************************
// Packet to send
typedef struct list_packet {
	char               hash[129];
	char               packet[9];
	int                channel;
	uint8_t            mac[6];
	int                rssi;
	uint8_t            sequence_control[2];
	time_t             timestamp;
	int                size_ssid;
	char               ssid[32];
	struct list_packet *next;
} packet;

packet *tails = NULL;
packet *head = NULL;
// **************************************************************************************************

// **************************************************************************************************
// Packet info
typedef struct {
    uint16_t protocol : 2;
    uint16_t type : 2;
    uint16_t subtype : 4;
    uint16_t to_ds : 1;
    uint16_t from_ds : 1;
    uint16_t more_frag : 1;
    uint16_t retry : 1;
    uint16_t power_mgt : 1;
    uint16_t more_data : 1;
    uint16_t wep : 1;
    uint16_t order : 1;
} wlan_frame_control_t;

// **************************************************************************************************
// Packet header
typedef struct {
    wlan_frame_control_t control;
    uint8_t              duration[2];
    uint8_t              address_1[6];
    uint8_t              address_2[6];
    uint8_t              address_3[6];
    uint8_t              sequence_control[2];
    uint8_t              address_4[6];
    uint8_t              ssid[33];
} wlan_mac_header_t;
// **************************************************************************************************

// **************************************************************************************************
// Packet payload
typedef struct {
	wlan_mac_header_t hdr;
	uint8_t payload[0];     // ( network data ended with 4 bytes csum (CRC32)
} wifi_ieee80211_packet_t;
// **************************************************************************************************

#endif /* MAIN_LIBRARY_H_ */
