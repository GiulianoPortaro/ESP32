#include "library.h"
#include "function.c"

// **************************************************************************************************
// Main
void app_main(void) {

	// Disable log message
	esp_log_level_set("*", ESP_LOG_NONE);

	// Boot flash memory
	nvs_flash_init();

	// Wi-Fi configuration
	wifi_setup();

	// SNTP configuration
	sntp_configuration();

	time_t now;
	time(&now);

	// Check timestamp
	while(now < 1538932409) {
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		time(&now);
	}

	// Callback on function when capture one packet
	printf("WIFI PROMISCUOUS SET CALLBACK: ");
	esp_check_error(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));

	// Sniffer mode on
	printf("ACTIVATE PROMISCUOUS MODE: ");
	esp_check_error(esp_wifi_set_promiscuous(true));

	// Loop
	while (true) {
		// Delay of TIMER milliseconds
		vTaskDelay(TIMER / portTICK_PERIOD_MS);

		printf("*********************************************************\n");

		printf("\n");

		// Send packet at server
		handle_send_packets();

		// Print actual time
		time_t now;
		struct tm timeinfo;
		time(&now);
		localtime_r(&now, &timeinfo);
		while(timeinfo.tm_year < (2018 - 1900)) {
			vTaskDelay(5000 / portTICK_PERIOD_MS);
			time(&now);
			localtime_r(&now, &timeinfo);
		}

		// Print the actual time with different formats
		char buffer[100];

		// Change the timezone to Italy
		setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
		tzset();

		// Print the actual time in Italy
		localtime_r(&now, &timeinfo);
		strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
		printf("*********************************************************\n");
		printf("%s\n", buffer);
		printf("*********************************************************\n");

		printf("\n");

    }
}
// **************************************************************************************************

// **************************************************************************************************
// Handler to send packet at server and to clean the list
void handle_send_packets() {

	printf("*********************************************************\n");

    unsigned int status = 1;

    int s = connection_initializer();

	packet *tmp = head;

	if(s != -1) {
		uint8_t esp_mac[6];

		// MAC ESP32
		esp_efuse_mac_get_default(esp_mac);

		printf("Sending MAC at the server.\n");
		if(write(s, esp_mac, sizeof(esp_mac)) != sizeof(esp_mac)) {
			printf("Error during sending ID...\n");
			status = 0;
		}

		printf("*********************************************************\n");
	}
	else {
		status = 0;
	}

	if(head != NULL) {
		while(tmp) {

			if(status == 1) {

				// Send MSG_OK
				if(write(s, MSG_OK, strlen(MSG_OK)) != strlen(MSG_OK) && status == 1) {
					printf("Error during sending MSG_OK...\n");
					status = 0;
				}

				// Send size of HASH
				int size_hash = strlen(head->hash);
				if(write(s, &size_hash, sizeof(size_hash)) != sizeof(size_hash) && status == 1) {
					printf("Error during sending size of HASH...\n");
					status = 0;
				}

				// Send HASH
				printf("Sending Hash: %s\n", head->hash);
				if(write(s, head->hash, size_hash) != size_hash && status == 1) {
					printf("Error during sending HASH...\n");
					status = 0;
				}

				// Send Packet Type
				printf("Sending Packet: %s\n", head->packet);
				if(write(s, head->packet, strlen(head->packet)) != strlen(head->packet) && status == 1) {
					printf("Error during sending PACKET...\n");
					status = 0;
				}

				// Send Channel
				printf("Sending Channel: %02d\n", head->channel);
				if(write(s, &head->channel, sizeof(head->channel)) != sizeof(head->channel) && status == 1) {
					printf("Error during sending CHANNEL...\n");
					status = 0;
				}

				// Send MAC
				printf("Sending MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", head->mac[0], head->mac[1], head->mac[2], head->mac[3], head->mac[4], head->mac[5]);
				if(write(s, head->mac, sizeof(head->mac)) != sizeof(head->mac) && status == 1) {
					printf("Error during sending MAC...\n");
					status = 0;
				}

				// Send RSSI
				printf("Sending RSSI: %02d\n", head->rssi);
				if(write(s, &head->rssi, sizeof(head->rssi)) != sizeof(head->rssi) && status == 1) {
					printf("Error during sending RSSI...\n");
					status = 0;
				}

				// Send Number of Sequence
				printf("Sending Number of Sequence %u\n", (*(uint8_t *)head->sequence_control));
				if(write(s, &head->sequence_control, sizeof(head->sequence_control)) != sizeof(head->sequence_control) && status == 1) {
					printf("Error during sending SEQUENCE_CONTROL...\n");
					status = 0;
				}

				// Send Timestamp
				printf("Sending Timestamp: %u\n", (unsigned int)head->timestamp);
				if(write(s, &head->timestamp, sizeof(head->timestamp)) != sizeof(head->timestamp) && status == 1) {
					printf("Error during sending TIMESTAMP...\n");
					status = 0;
				}

				// Send size of SSID
				if(write(s, &head->size_ssid, sizeof(head->size_ssid)) != sizeof(head->size_ssid) && status == 1) {
					printf("Error during sending SSID...\n");
					status = 0;
				}

				// Send SSID
				printf("Sending SSID: %s\n", head->ssid);
				if(write(s, head->ssid, strlen(head->ssid)) != strlen(head->ssid) && status == 1) {
					printf("Error during sending SSID...\n");
					status = 0;
				}

				// Send MSG_ERR if status == 0
				if(status == 0) {
					write(s, MSG_ERR, strlen(MSG_ERR));
				}

				printf("\n");

			}

			tmp = head->next;
		    free(head);
		    head = tmp;
		}

		// Send MSG_END to close connection
		if(status == 1 && write(s, MSG_END, strlen(MSG_END)) != strlen(MSG_END)) {
			printf("Error during sending MSG_END...\n");
		}

		printf("Cleaning list completed!");

		if(s != -1) {

			close(s);

			printf("\n*********************************************************\n\n");

		}
	}
	// No packets available
	else {

		// Send MSG_END to close connection
		if(status == 1 && write(s, MSG_END, strlen(MSG_END)) != strlen(MSG_END)) {
			printf("Error during sending MSG_END...\n");
		}

		if(s != 1) close(s);

		printf("No packets available...");
	}

	printf("\n*********************************************************\n\n");
}
// **************************************************************************************************

// **************************************************************************************************
// Handler packet; insert the information about packets into a list
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {

	// Use to obtain the sequence_control
	wlan_mac_header_t *header = (void *)(((wifi_promiscuous_pkt_t *)buff)->payload);

	// Check if the packet is of type Management (BEACON, PROBE)
	if (type != WIFI_PKT_MGMT) return;

	// Check if the packet is PROBE REQUEST
	if(header->control.subtype != 4) return;

	// Packet
	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;

	// Header
	const wlan_mac_header_t *hdr = &((wifi_ieee80211_packet_t *)ppkt->payload)->hdr;

	// Tmp buffer for extract information form packet
	packet *tmp = (packet *)malloc(sizeof(packet));

	// If the malloc done...
	if(tmp != NULL) {

		// SSID lenght
		char lenght[3];

		// Read the lenght of SSID (0 if not present)
		sprintf(lenght, "%02x%02x", ppkt->payload[24], ppkt->payload[25]);
		int number = (int)strtol(lenght, NULL, 16);

		tmp->size_ssid = number;

		// Read the SSID if it is present
		for(int i = 26; i < number + 26; i++) {
			tmp->ssid[i-26] = (char)ppkt->payload[i];
		}

		tmp->ssid[number] = '\0';

		// MAC
		tmp->mac[0] = hdr->address_2[0];
		tmp->mac[1] = hdr->address_2[1];
		tmp->mac[2] = hdr->address_2[2];
		tmp->mac[3] = hdr->address_2[3];
		tmp->mac[4] = hdr->address_2[4];
		tmp->mac[5] = hdr->address_2[5];

		// RSSI
		tmp->rssi = ppkt->rx_ctrl.rssi;

		// CHANNEL
		tmp->channel = ppkt->rx_ctrl.channel;

		// SEQUENCE
		tmp->sequence_control[0] = hdr->sequence_control[0];
		tmp->sequence_control[1] = hdr->sequence_control[1];

		// TIMESTAMP
		time(&tmp->timestamp);

		// TYPE
		strcpy(tmp->packet, "PROBE");

		// HASH
		mbedtls_md_context_t ctx;

		unsigned char hashtmp[64];

		mbedtls_md_init(&ctx);
		mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), 0);
		mbedtls_md_starts(&ctx);
		mbedtls_md_update(&ctx, ppkt->payload, ppkt->rx_ctrl.sig_len);
		mbedtls_md_finish(&ctx, hashtmp);
		mbedtls_md_free(&ctx);

		for(int i = 0; i < sizeof(hashtmp); i++) {
			char str[3];
			sprintf(str, "%02x", (int)hashtmp[i]);
			if(i == 0) strcpy(tmp->hash, str);
			else strcat(tmp->hash, str);
		}

		strcat(tmp->hash, "\0");
		// -------------------------------------------------------------------

		tmp->next = NULL;

		// Insert of packet into list
		if (head == NULL) head = tmp;
		else tails->next = tmp;

		tails = tmp;

		printf("HASH=%s, PACKET=PROBE, CHAN=%02d, RSSI=%02d, "
			"MAC=%02x:%02x:%02x:%02x:%02x:%02x, "
			"SEQ=%u, "
			"TIMESTAMP=%u, "
			"SSID=%s\n\n",
			tmp->hash,
			tmp->channel,
			tmp->rssi,
			tmp->mac[0],tmp->mac[1],tmp->mac[2],tmp->mac[3],tmp->mac[4],tmp->mac[5],
			(*(uint8_t *)tmp->sequence_control),
			(unsigned int)tmp->timestamp,
			tmp->ssid
		);
	}
}
// **************************************************************************************************
