#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

/* =========================================
 * CONFIGURAÇÕES DO LED (Thread 2)
 * ========================================= */
#define SLEEP_TIME_ON_MS   5000
#define SLEEP_TIME_OFF_MS  25000

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Função da Thread do LED */
void led_thread_function(void)
{
    if (!gpio_is_ready_dt(&led)) {
        return;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    while (1) {
        gpio_pin_set_dt(&led, 1);
        k_msleep(SLEEP_TIME_ON_MS);

        gpio_pin_set_dt(&led, 0);
        k_msleep(SLEEP_TIME_OFF_MS);
    }
}

/* Criação da Thread do LED (Pilha: 512 bytes, Prioridade: 7) */
K_THREAD_DEFINE(led_thread_id, 512, led_thread_function, NULL, NULL, NULL, 7, 0, 0);


/* =========================================
 * CONFIGURAÇÕES DO ADC (Thread Principal)
 * ========================================= */
/* Obtém as especificações do ADC a partir do app.overlay */
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "Configuração do ADC ausente no Devicetree (app.overlay)!"
#endif

static const struct adc_dt_spec pot = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

int main(void)
{
    int err;
    uint16_t adc_buffer; /* Variável que armazenará a leitura bruta */

    /* Estrutura que diz ao driver onde salvar o dado lido */
    struct adc_sequence sequence = {
        .buffer = &adc_buffer,
        .buffer_size = sizeof(adc_buffer),
    };

    printk("Iniciando Sistema de Leitura e Pisca-LED!\n");

    /* Verifica se o dispositivo de ADC está pronto */
    if (!device_is_ready(pot.dev)) {
        printk("Erro: ADC não está pronto.\n");
        return 0;
    }

    /* Configura o canal do ADC com as definições do app.overlay */
    err = adc_channel_setup_dt(&pot);
    if (err < 0) {
        printk("Erro ao configurar canal do ADC: %d\n", err);
        return 0;
    }

    /* Prepara a sequência de leitura baseada no devicetree */
    err = adc_sequence_init_dt(&pot, &sequence);
    if (err < 0) {
        printk("Erro ao iniciar a sequência do ADC: %d\n", err);
        return 0;
    }

    /* Loop da Thread Principal (Lê o potenciômetro a cada 1 segundo) */
    while (1) {
        err = adc_read(pot.dev, &sequence);
        
        if (err < 0) {
            printk("Falha na leitura do ADC (erro %d)\n", err);
        } else {
            /* Como configuramos 12 bits, o valor impresso será de 0 a 4095 */
            printk("Valor do Potenciômetro (Bruto): %d\n", adc_buffer);
        }

        /* Dorme por 1 segundo antes de ler de novo */
        k_msleep(1000);
    }

    return 0;
}