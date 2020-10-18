#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "lwip/sockets.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#define SSID "wifi ssid"
#define SSID_PASS "wifi password"

#define HTTP_SERVER_IP "93.184.216.34" // example.com
#define HTTP_SERVER_PORT 80

static const char *REQUEST_MESSAGE = "GET / HTTP/1.0\r\n"
                                     "Host: example.com\r\n"
                                     "User-Agent: ESP32 WiFi HTTP Client\r\n"
                                     "\r\n";

static const char *TAG = "scan";

void requestHTTP(void);

esp_err_t event_handler(void *ctx, system_event_t *event)
{

    if (event->event_id == SYSTEM_EVENT_STA_GOT_IP)
    {

        printf("IP recebido do AccessPoint: %d.%d.%d.%d \n", IP2STR(&event->event_info.got_ip.ip_info.ip));
        requestHTTP();
    }
    return ESP_OK;
}

void requestHTTP()
{

    int sock, ret;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = PP_HTONS(HTTP_SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(HTTP_SERVER_IP);

    sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("sock < 0 - FATAL!!!\n");
        esp_restart();
    }

    ret = lwip_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret != 0)
    {
        printf("connect != 0 - FATAL!!!\n");
        esp_restart();
    }

    printf("Enviando requisicao HTTP...\n");
    lwip_send(sock, REQUEST_MESSAGE, strlen(REQUEST_MESSAGE), 0);

    char recvbuf[1024] = {0};

    while ((ret = lwip_recv(sock, recvbuf, 1024, 0)) > 0)
    {
        printf("bytes recebidos: %d\n", ret);
        recvbuf[ret] = 0;
        printf("conteudo recebido: %s\n", recvbuf);
    }

    lwip_close(sock);
}

static void print_auth_mode(int authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher)
    {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher)
    {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}

static void init_nvs()
{
    /* Inicializa a partição nvs
     * Ela usada para armazenamento não-volátil, 
     * utilizada para armazenar calibração de dados. 
     * Ele também é usado para armazenar as configurações 
     * do WiFi, por exemplo e ela pode ser utilizada 
     * para outros dados de aplicação.
     */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{

    /* ESP_ERROR_CHECK
    macro serve a um propósito semelhante assert, 
    exceto que verifica o valor de esp_err_t, em 
    vez de uma condição booleana. Se o argumento 
    de ESP_ERROR_CHECK()não for igual ESP_OK, 
    uma mensagem de erro será impressa no console 
    e abort()será chamada
    */

    ESP_ERROR_CHECK(esp_netif_init()); //Inicializa a pilha TCP/IP e retorna ESP_OK ou ESP_FAIL
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    //configuração do Wifi com os valores padrão
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t scanlist = 3; // maximum number of scan list
    wifi_ap_record_t ap_info[scanlist];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    // configura wifi no modo estação e checa o retorno
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // inicializa wifi no modo configurado (sta) e checa retorno
    ESP_ERROR_CHECK(esp_wifi_start());

    // faz varredura dos APs disponíveis
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

    // obtém a lista de APs encontrados
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&scanlist, ap_info));

    // obtém o número de APs encontrados
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    ESP_LOGI(TAG, "Total APs scanned = %u\n\v", ap_count);
    for (int i = 0; (i < scanlist) && (i < ap_count); i++)
    {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        if (ap_info[i].authmode != WIFI_AUTH_WEP)
        {
            print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        }
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
    }
}

void app_main(void)
{
    printf("\n --- Hello Mosquitto --- \n\v");

    printf("\n --- init_nvs() --- \n\v");
    init_nvs();

    printf("\n --- wifi_scan() --- \n\v");
    wifi_scan();

    printf("\n --- wifi_connect() --- \n\v");
        printf("Inicializando cliente WiFi/HTTP...\n");

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
 
    wifi_config_t sta_config = {
        .sta = {
            .ssid = SSID,
            .password = SSID_PASS,
            .bssid_set = 0
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}
