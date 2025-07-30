#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "lib/sd_card.h"
#include "lib/ssd1306.h"
#include "lib/mpu6050.h"
#include "lib/icons.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hw_config.h"

// Definição dos pinos I2C para o display OLED
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define ENDERECO_DISP 0x3C            // Endereço I2C do display

#define LED_BLUE_PIN 12                 // GPIO12 - LED azul
#define LED_GREEN_PIN 11                // GPIO11 - LED verde
#define LED_RED_PIN 13                  // GPIO13 - LED vermelho
#define BTN_A 5                         // GPIO5 - Botão A
#define BTN_B 6                         // GPIO6 - Botão B
#define BTN_J 22                        // GPIO22 - Botão Joystick
#define JOYSTICK_X 26                   // Pino do Joystick X
#define BUZZER_A_PIN 10                 // GPIO10 - Buzzer A
#define BUZZER_B_PIN 21                 // GPIO21 - Buzzer B

#define NUM_PIXELS 25   // Número de pixels da matriz de LEDs
#define MATRIX_PIN 7    // Pino da matriz de LEDs

uint32_t last_time = 0; // Variável para armazenar o tempo do último evento
static bool is_sd_card_mounted = false; // Variável para verificar se o cartão SD está montado
static bool is_logging_enabled = false; // Variável para verificar se o log está ativo
bool temp_flag = false; // Variável para exibir mensagem temporária no display
bool is_reading = false; // Variável para verificar se está lendo dados do cartão SD

char str_temp[30];
char str_status[30];

ssd1306_t ssd;

void gpio_irq_handler(uint gpio, uint32_t events);
void gpio_bitdog(void);
void set_system_status(const char *status);
void pwm_setup_gpio(uint gpio, uint freq);
void capture_imu_data_and_save();
void update_display(ssd1306_t ssd);

int main()
{
    stdio_init_all();
    set_system_status("Inicializando...");
    // Inicializa a I2C do Display OLED em 400kHz
    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);


    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO_DISP, I2C_PORT_DISP);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicialização da I2C do MPU6050
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Declara os pinos como I2C na Binary Info
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    mpu6050_reset();

    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_J, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    gpio_bitdog(); // Inicializa os pinos GPIO da BitDogLab

    int16_t aceleracao[3], gyro[3], temp;

    bool cor = true; 

    time_init();

    set_system_status("SD desmontado");

    while (true) {
        mpu6050_read_raw(aceleracao, gyro, &temp);

        // --- Cálculos para Giroscópio ---
        float gx = (float)gyro[0];
        float gy = (float)gyro[1];
        float gz = (float)gyro[2];

        // --- Cálculos para Acelerômetro (Pitch/Roll) ---
        float ax = aceleracao[0] / 16384.0f; // Escala para 'g'
        float ay = aceleracao[1] / 16384.0f;
        float az = aceleracao[2] / 16384.0f;

        if (temp_flag && !is_logging_enabled) {
            if (is_sd_card_mounted) {
                run_unmount();
                printf("Cartão SD desmontado!\n");
                snprintf(str_temp, sizeof(str_temp), "SD desmontado");
                set_system_status("SD desmontado");
                is_sd_card_mounted = false; // Atualiza o estado do cartão SD
            } else {
                set_system_status("Montando SD...");
                run_mount();
                printf("Cartão SD montado!\n");
                snprintf(str_temp, sizeof(str_temp), "SD montado");
                set_system_status("Aguardando");
                is_sd_card_mounted = true; // Atualiza o estado do cartão SD
            }
            temp_flag = false; // Garante que executa só uma vez por clique
        }

        if (is_reading) {
            set_system_status("Acessando SD");
            read_file("imu_data.csv"); // Lê o arquivo de dados do MPU6050
            snprintf(str_temp, sizeof(str_temp), "Leitura feita");
            is_reading = false; // Reseta a flag de leitura
            set_system_status("Aguardando");
        }

        if (is_logging_enabled) {
            if (is_sd_card_mounted) {
                set_system_status("Acessando SD");
                pwm_setup_gpio(BUZZER_A_PIN, 1000); // Ativa o buzzer   
                sleep_ms(100);
                pwm_setup_gpio(BUZZER_A_PIN, 0); // Desativa o buzzer
                capture_imu_data_and_save(); // Captura dados do MPU6050 e salva no cartão SD
                is_logging_enabled = false; // Desativa o log após a captura
                snprintf(str_temp, sizeof(str_temp), "Captura OFF");
                pwm_setup_gpio(BUZZER_A_PIN, 1000); // Ativa o buzzer   
                sleep_ms(100);
                pwm_setup_gpio(BUZZER_A_PIN, 0); // Desativa o buzzer
                sleep_ms(100);
                pwm_setup_gpio(BUZZER_A_PIN, 1000); // Ativa o buzzer   
                sleep_ms(100);
                pwm_setup_gpio(BUZZER_A_PIN, 0); // Desativa o buzzer
                set_system_status("Aguardando");
            } else {
                printf("Erro: Cartão SD não montado!\n");
                snprintf(str_temp, sizeof(str_temp), "SD desmontado");
                set_system_status("Erro: SD");
            }
        }

        update_display(ssd);

        sleep_ms(1000);
    }
}

// Função de callback para tratamento de interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events) { 
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Pega o tempo atual em ms
    if (current_time - last_time > 250000) { // Debouncing de 250ms
        last_time = current_time;
        if (gpio == BTN_A) { // Verifica se o botão A foi pressionado
            printf("Botão A pressionado!\n");
            temp_flag = true; // Armazena o estado atual do cartão SD
        } else if (gpio == BTN_B) { // Verifica se o botão B foi pressionado
            printf("Botão B pressionado!\n");

            if (!is_sd_card_mounted) {
                printf("Falha: SD não montado.\n");
                snprintf(str_temp, sizeof(str_temp), "SD desmontado");
                set_system_status("Erro: captura");
                return;
            }

            is_logging_enabled = !is_logging_enabled;

            if (is_logging_enabled) {
                printf("Captura ativada!\n");
                snprintf(str_temp, sizeof(str_temp), "Captura ON");
                set_system_status("Gravando...");
            } else {
                printf("Captura desativada!\n");
                snprintf(str_temp, sizeof(str_temp), "Captura OFF");
                pwm_setup_gpio(BUZZER_A_PIN, 0); 
                set_system_status("Aguardando");
            }

        } else if (gpio == BTN_J) {  // Verifica se o botão do joystick foi pressionado
            printf("Botão do joystick pressionado!\n");
            if (is_sd_card_mounted) {
                is_reading = true;
            } else {
                reset_usb_boot(0, 0);
            }
        }
    }
}

// Inicializar os Pinos GPIO da BitDogLab
void gpio_bitdog(void) {
    // Configuração dos LEDs como saída
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false);
    
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);
    
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, false);

    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    gpio_init(BTN_J);
    gpio_set_dir(BTN_J, GPIO_IN);
    gpio_pull_up(BTN_J);

    gpio_init(BUZZER_A_PIN);  
    gpio_set_dir(BUZZER_A_PIN, GPIO_OUT);
    gpio_init(BUZZER_B_PIN);  
    gpio_set_dir(BUZZER_B_PIN, GPIO_OUT);
}

void set_system_status(const char *status) {
    snprintf(str_status, sizeof(str_status), "%s", status);
    
    gpio_put(LED_RED_PIN, false); gpio_put(LED_GREEN_PIN, false); gpio_put(LED_BLUE_PIN, false); // Desliga todos os LEDs

    // Define cor do LED RGB com base no status
    if (strcmp(status, "Inicializando") == 0 || strcmp(status, "Montando SD") == 0 || strcmp(status, "SD desmontado") == 0) {
        gpio_put(LED_RED_PIN, true);    // Liga LED vermelho
        gpio_put(LED_GREEN_PIN, true); // Liga LED verde
    } else if (strcmp(status, "Aguardando") == 0) {
        gpio_put(LED_GREEN_PIN, true); // Liga LED verde
    } else if (strcmp(status, "Gravando...") == 0) {
        gpio_put(LED_RED_PIN, true);    // Liga LED vermelho
    } else if (strcmp(status, "Acessando SD") == 0) {
        gpio_put(LED_BLUE_PIN, true); // Liga LED azul
    } else if (strcmp(status, "Erro: SD") == 0 || strcmp(status, "Erro: captura") == 0) {
        gpio_put(LED_RED_PIN, true);    // Liga LED vermelho
        gpio_put(LED_BLUE_PIN, true); // Liga LED azul
    } else {
        gpio_put(LED_RED_PIN, false); gpio_put(LED_GREEN_PIN, false); gpio_put(LED_BLUE_PIN, false); // Desliga todos os LEDs
    }
}

// Função para configurar o PWM
void pwm_setup_gpio(uint gpio, uint freq) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);  // Define o pino como saída PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio);  // Obtém o slice do PWM

    if (freq == 0) {
        pwm_set_enabled(slice_num, false);  // Desabilita o PWM
        gpio_put(gpio, 0);  // Desliga o pino
    } else {
        uint32_t clock_div = 4; // Define o divisor do clock
        uint32_t wrap = (clock_get_hz(clk_sys) / (clock_div * freq)) - 1; // Calcula o valor de wrap

        // Configurações do PWM (clock_div, wrap e duty cycle) e habilita o PWM
        pwm_set_clkdiv(slice_num, clock_div); 
        pwm_set_wrap(slice_num, wrap);  
        pwm_set_gpio_level(gpio, wrap / 5);
        pwm_set_enabled(slice_num, true);  
    }
}

void capture_imu_data_and_save() {
    printf("\nCapturando dados do MPU6050 continuamente. Pressione BTN_B para parar.\n");
    update_display(ssd);

    FIL file;
    const char *filename = "imu_data.csv";

    FRESULT res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("\n[ERRO] Não foi possível abrir o arquivo para escrita. Monte o Cartão.\n");
        set_system_status("Erro: SD");
        return;
    }

    // Escreve o cabeçalho CSV
    const char *header = "Index,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z\n";
    UINT bw;
    res = f_write(&file, header, strlen(header), &bw);
    if (res != FR_OK) {
        printf("[ERRO] Não foi possível escrever o cabeçalho.\n");
        f_close(&file);
        set_system_status("Erro: SD");
        return;
    }

    int16_t accel[3], gyro[3], temp;
    int i = 0;

    while (is_logging_enabled) {
        mpu6050_read_raw(accel, gyro, &temp);

        float ax = accel[0] / 16384.0f;
        float ay = accel[1] / 16384.0f;
        float az = accel[2] / 16384.0f;
        float gx = gyro[0] / 131.0f;
        float gy = gyro[1] / 131.0f;
        float gz = gyro[2] / 131.0f;

        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                 i + 1, ax, ay, az, gx, gy, gz);

        res = f_write(&file, buffer, strlen(buffer), &bw);
        if (res != FR_OK) {
            printf("[ERRO] Falha ao escrever os dados.\n");
            f_close(&file);
            set_system_status("Erro: SD");
            return;
        }

        i++;
        sleep_ms(100);
    }

    f_close(&file);
    printf("Captura finalizada. Dados salvos em '%s'.\n", filename);
    set_system_status("Aguardando");
}


void update_display(ssd1306_t ssd) {
    ssd1306_fill(&ssd, !true);                           // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, true, !true);       // Desenha um retângulo
    ssd1306_line(&ssd, 3, 25, 123, 25, true);            // Desenha uma linha horizontal
    ssd1306_line(&ssd, 3, 50, 123, 50, true);            // Desenha outra linha horizontal
    ssd1306_draw_string(&ssd, "MOVEMENT", 32, 6);       // Escreve texto no display
    ssd1306_draw_string(&ssd, "DATALOGGER", 23, 16);    // Escreve texto no display
    ssd1306_draw_string(&ssd, "Status:", 10, 28);       // Escreve texto no display
    ssd1306_draw_string(&ssd, str_status, 10, 41);      // Exibe mensagem temporária
    ssd1306_draw_string(&ssd, str_temp, 10, 53);        // Exibe mensagem temporária
    ssd1306_send_data(&ssd);
}

