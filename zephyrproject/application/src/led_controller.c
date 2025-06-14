#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "app.h"

// Definizioni per il controllo del LED
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Semafori per la sincronizzazione dei thread
static struct k_sem sem_on;
static struct k_sem sem_off;

K_THREAD_STACK_DEFINE(led_on_stack, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread ledon
static struct k_thread led_on_thread_data;                     ///< Dati del thread ledon

K_THREAD_STACK_DEFINE(led_off_stack, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread ledoff
static struct k_thread led_off_thread_data;                     ///< Dati del thread ledoff

// Thread per accendere il LED
void led_on_thread(void *p1, void *p2, void *p3) {
    while (1) {
        k_sem_take(&sem_on, K_FOREVER);  // Attende che venga dato il semaforo per accendere
        gpio_pin_set(led.port, led.pin, 1);  // Accende il LED
        k_msleep(1000);  // Attendi 1 secondo
        k_sem_give(&sem_off);  // Dai il semaforo per spegnere il LED
    }
}

// Thread per spegnere il LED
void led_off_thread(void *p1, void *p2, void *p3) {
    while (1) {
        k_sem_take(&sem_off, K_FOREVER);  // Attende che venga dato il semaforo per spegnere
        gpio_pin_set(led.port, led.pin, 0);  // Spegne il LED
        k_msleep(1000);  // Attendi 1 secondo
        k_sem_give(&sem_on);  // Dai il semaforo per accendere il LED
    }
}

// Thread di controllo che avvia il ciclo di accensione/spegnimento
void led_controller_thread(void *p1, void *p2, void *p3) {
    // Configura il pin come output
    int ret = gpio_pin_configure(led.port, led.pin, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        unregister_thread(k_current_get());
        return;
    }

    // Inizializza i semafori
    k_sem_init(&sem_on, 0, 1);   // Semaforo per accendere il LED
    k_sem_init(&sem_off, 0, 1);  // Semaforo per spegnere il LED

    // Crea i thread
    k_tid_t led_on_tid = k_thread_create(&led_on_thread_data, led_on_stack, K_THREAD_STACK_SIZEOF(led_on_stack),
                    led_on_thread, NULL, NULL, NULL, LED_ON_PRIORITY, 0, K_NO_WAIT);
    register_thread("LED_ON", led_on_tid);  ///< Registra il thread LED ON

    k_tid_t led_off_tid = k_thread_create(&led_off_thread_data, led_off_stack, K_THREAD_STACK_SIZEOF(led_off_stack),
                    led_off_thread, NULL, NULL, NULL, LED_OFF_PRIORITY, 0, K_NO_WAIT);
    register_thread("LED_OFF", led_off_tid);  ///< Registra il thread LED OFF

    // Avvia il ciclo di accensione
    k_sem_give(&sem_on);  // Dai il semaforo per accendere il LED 

    k_sleep(K_SECONDS(2));
    unregister_thread(k_current_get());
    k_thread_abort(k_current_get());  ///< Termina il thread corrente
}