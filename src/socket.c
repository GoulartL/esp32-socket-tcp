
#include "includes.h"
#include "defines.h"

static void tcp_server_task(void *pvParameters);
static const char *TAG = "Socket";

QueueHandle_t bufferTemperatura;
QueueHandle_t bufferUmidade;
QueueHandle_t bufferDistancia;

void inicia_socket()
{
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET, 5, NULL);
}

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];
    uint16_t temp;
    uint16_t umid;
    uint32_t distancia;

    do
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0)
        {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        }
        else if (len == 0)
        {
            ESP_LOGW(TAG, "Connection closed");
        }
        else
        {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            xQueueReceive(bufferTemperatura, &temp, pdMS_TO_TICKS(2000));
            xQueueReceive(bufferUmidade, &umid, pdMS_TO_TICKS(2000));
            xQueueReceive(bufferDistancia, &distancia, pdMS_TO_TICKS(2000));

            envia_mensagem(rx_buffer[0], temp, umid, distancia, sock);

        }
    } while (len > 0);
}

void envia_mensagem(char escolha, int temp, int umid, int distancia, int sock)
{

    char message[170];
    char str_sensor[20];

    if (escolha == '1')
    {

        if (temp < 18)
        {
            strcpy(message, "Está esfriando meu querido, melhor pegar um agasalho!\n");
        }
        else if (temp >= 18 && temp <= 25)
        {
            strcpy(message, "Tá uma temperatura agradável, bem do jeitinho que eu gosto\n");
        }
        else if (temp > 25 && temp < 30)
        {
            strcpy(message, "Rapaz, ta meio quente viu, mas ta de boa\n");
        }
        else if (temp > 30)
        {
            strcpy(message, "Ta pegando fogo bixo\n");
        }
        sprintf(str_sensor, "Temperatura: %d°C", temp);
        send(sock, strcat(message, str_sensor), strlen(message), 0);
    }
    else if (escolha == '2')
    {

        if (umid < 50)
        {
            strcpy(message, "Umidade é algo que não existe aqui mano\n");
        }
        else if (umid >= 50 && umid <= 75)
        {
            strcpy(message, "Tá no ar condicionad né? Tá tranquilasso\n");
        }
        else if (umid > 75 && umid < 90)
        {
            strcpy(message, "É, ta desse jeitinho aí, tá tranquilo até\n");
        }
        else if (umid > 90)
        {
            strcpy(message, "Dá pra tomar água pelo ar\n");
        }
        sprintf(str_sensor, "Umidade: %d%%", umid);
        send(sock, strcat(message, str_sensor), strlen(message), 0);
    }
    else if (escolha == '3')
    {

        if (distancia < 50)
        {
            strcpy(message, "Há um indivíduo usando teu PC!\n");
        }
        else
        {
            strcpy(message, "Não há ninguém no seu PC!\n");
        }
        sprintf(str_sensor, "Distância: %d", distancia);
        send(sock, strcat(message, str_sensor), strlen(message), 0);
    }
    else
    {
        strcpy(message, "Olá, eu sou o Socketinho, estou aqui para te ajudar :D\nEm que posso ser útil?\n1 -> Temperatura do seu quarto\n2 -> Umidade do seu quarto\n3 -> Se há alguém no seu pc");
    }
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    char message[170];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
    else if (addr_family == AF_INET6)
    {
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0)
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1)
    {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        }
        else if (source_addr.sin6_family == PF_INET6)
        {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        strcpy(message, "Olá, eu sou o Socketinho, estou aqui para te ajudar :D\nEm que posso ser útil?\n1 -> Temperatura do seu quarto\n2 -> Umidade do seu quarto\n3 -> Se há alguém no seu pc");
        send(sock, message, strlen(message), 0);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}