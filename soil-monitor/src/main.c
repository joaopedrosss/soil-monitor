#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Define os tempos de transição em milissegundos */
#define SLEEP_TIME_ON_MS   5000   /* 5 segundos aceso */
#define SLEEP_TIME_OFF_MS  25000  /* 25 segundos apagado (30s - 5s) */

/* Obtém a especificação do pino a partir do alias "led0" no Devicetree */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;

    /* Verifica se o dispositivo GPIO está pronto para uso */
    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    /* Configura o pino do LED como saída, inicializando-o no estado inativo */
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return 0;
    }

    /* Loop infinito da thread principal */
    while (1) {
        /* Liga o LED */
        gpio_pin_set_dt(&led, 1);
        k_msleep(SLEEP_TIME_ON_MS);

        /* Desliga o LED */
        gpio_pin_set_dt(&led, 0);
        k_msleep(SLEEP_TIME_OFF_MS);
    }

    return 0;
}