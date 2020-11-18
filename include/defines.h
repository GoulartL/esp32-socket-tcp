#ifndef defines
#define defines

/* Definições e Constantes */
#define TRUE 1
#define FALSE 0
#define PORT 3333
#define DEBUG TRUE
/*#define LED_R GPIO_NUM_15
#define LED_G GPIO_NUM_12
#define LED_B GPIO_NUM_14*/ 
#define DHT_GPIO GPIO_NUM_16
#define PIN_SDA GPIO_NUM_5
#define PIN_SCL GPIO_NUM_4
#define GPIO_OUTPUT_PIN_SEL ((1ULL << LED_G) | (1ULL << LED_B))
#define GPIO_INPUT_PIN_SEL (1ULL << BUTTON)
#define EXAMPLE_ESP_WIFI_SSID "Seu SSID aqui"
#define EXAMPLE_ESP_WIFI_PASS "Sua senha aqui"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define TRIGGER_GPIO  14
#define ECHO_GPIO     12
#define MAX_DISTANCE_CM 500 // 5m max

void inicia_wifi(void);
void inicia_socket(void);
void inicia_sensor(void);
void inicia_oled(void);
void wifi_init_sta(void);
void task_ipe(void *pvParameter);
void task_dht(void *pvParamters);
void task_oLED(void *pvParameters);
void task_ultrasonic(void *pvParamters);
void envia_mensagem(char escolha, int temp, int umid, int distancia, int sock);

#endif