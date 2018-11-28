#include "library.h"

// Event group for inter-task communication
static EventGroupHandle_t event_group;

// **************************************************************************************************
// Check error in esp32 configuration function
void esp_check_error(int x) {
	switch(x) {
		case ESP_OK:
			printf("+OK\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_WIFI_NOT_INIT:
			printf("- WiFi is not initialized by esp_wifi_init...\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_WIFI_IF:
			printf("- Invalid interface...\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_INVALID_ARG:
			printf("- Invalid Argument...\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_NO_MEM:
			printf("- Out of memory\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_WIFI_MODE:
			printf("- Invalid mode\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_WIFI_PASSWORD:
			printf("- Invalid password\n");
			printf("*********************************************************\n");
			break;
		case ESP_ERR_WIFI_NVS:
			printf("- WiFi internal NVS error\n");
			printf("*********************************************************\n");
			break;
		default:
			printf("- Error not found...\n");
			printf("*********************************************************\n");
			break;
	}
}
// **************************************************************************************************

// **************************************************************************************************
// Handler event wi-fi
esp_err_t event_handler(void *ctx, system_event_t *event) {
    switch(event->event_id) {

    	// Start wifi event
		case SYSTEM_EVENT_STA_START:
		  esp_wifi_connect();
		  break;

		// Esp32 lost ip event: waiting the end of sniffing phase end restart the connection with AP
		case SYSTEM_EVENT_STA_LOST_IP:
			esp_wifi_disconnect();
			esp_wifi_connect();
			break;

	    // Esp obtain ip event: Set the bit to can start sending data phase with the server
		case SYSTEM_EVENT_STA_GOT_IP:
		  xEventGroupSetBits(event_group, CONNECTED_BIT);
		  break;

		// Disconnection event: Clear the bit to close sending data phase with the server
		case SYSTEM_EVENT_STA_DISCONNECTED:
		  xEventGroupClearBits(event_group, CONNECTED_BIT);
	      esp_wifi_connect();
		  break;

    	default:
    		break;
    }

	return ESP_OK;
}
// **************************************************************************************************

// **************************************************************************************************
// Connection with SNTP service
void sntp_configuration() {

	xEventGroupWaitBits(event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

	// Initialize the SNTP service
	sntp_setoperatingmode(SNTP_OPMODE_POLL);  // Unicast comunication
	sntp_setservername(0, "ntp1.inrim.it");   // Server sntp
	sntp_init();
}
// **************************************************************************************************

// **************************************************************************************************
// Setup and start the wifi connection
void wifi_setup() {

	// Initialize the event group wifi heandler
	event_group = xEventGroupCreate();

	// Init of LWIP: to use protocol TCP/IP
	tcpip_adapter_init();

	// Callback for handler all wifi event
	esp_event_loop_init(event_handler, NULL);

	wifi_country_t wifi_country = {.cc = "CN", .schan = 1, .nchan = 13, .policy = WIFI_COUNTRY_POLICY_AUTO};
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

	printf("WIFI INIT: ");
	esp_check_error(esp_wifi_init(&wifi_init_config));

	printf("WIFI SET COUNTRY");
	esp_check_error(esp_wifi_set_country(&wifi_country));

	printf("WIFI SET STORAGE RAM");
	esp_check_error(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	printf("WIFI SET MODE STATION: ");
	esp_check_error(esp_wifi_set_mode(WIFI_MODE_STA));

	printf("WIFI CONFIG");
	esp_check_error(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

	printf("WIFI START");
	esp_check_error(esp_wifi_start());
}
// **************************************************************************************************

// Connection initializer
int connection_initializer() {

	int s;
	struct sockaddr_in serverAddress;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s == -1) {
		printf("Error init socket...\n");
		return -1;
	}

	serverAddress.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &serverAddress.sin_addr.s_addr);
	serverAddress.sin_port = htons(PORT);

	if(connect(s, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) == -1) {
		printf("Error during connection...\n");
		return -1;
	}

	return s;
}
// **************************************************************************************************
