#include <zephyr/kernel.h>
#include "app.h"

// Dati e stack per i thread definiti globalmente
K_THREAD_STACK_DEFINE(gui_stack, CONFIG_MAIN_STACK_SIZE);      ///< Stack per il thread GUI
static struct k_thread gui_thread_data;                         ///< Dati del thread GUI

K_THREAD_STACK_DEFINE(worker_stack, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread Worker
static struct k_thread worker_thread_data;                     ///< Dati del thread Worker

K_THREAD_STACK_DEFINE(worker_stack2, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread Worker 2
static struct k_thread worker_thread_data2;                     ///< Dati del thread Worker 2

K_THREAD_STACK_DEFINE(led_controller_stack, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread LED CONTROLLER
static struct k_thread led_controller_data;                     ///< Dati del thread LED CONTROLLER

// Coda per la comunicazione tra thread (max 10 messaggi, 4 byte per messaggio)
K_MSGQ_DEFINE(gui_msgq, sizeof(gui_command_t), 10, 4);          ///< Coda dei messaggi per la GUI

extern const k_tid_t init_tid;  ///< ID del thread di inizializzazione
struct k_mutex monitored_threads_mutex;  ///< Mutex per proteggere monitored_threads

/**
 * @brief Funzione del thread di inizializzazione.
 *
 * Questa funzione crea e registra i thread GUI e Worker, quindi termina il thread di inizializzazione.
 * Gestisce anche i messaggi di registrazione dei thread.
 */
void init_thread(void *arg1, void *arg2, void *arg3) {
    register_thread("INIT", init_tid);  ///< Registra il thread di inizializzazione
    k_mutex_init(&monitored_threads_mutex);  ///< Inizializza il mutex per monitoraggio threads

    // Crea il thread GUI
    k_tid_t gui_tid = k_thread_create(&gui_thread_data, gui_stack,
                                      K_THREAD_STACK_SIZEOF(gui_stack),
                                      gui_thread,
                                      NULL, NULL, NULL,
                                      GUI_PRIORITY, 0, K_NO_WAIT);
    register_thread("GUI", gui_tid);  ///< Registra il thread GUI

    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di creare il thread Worker

    // Crea il thread Worker
    k_tid_t worker_tid = k_thread_create(&worker_thread_data, worker_stack,
                                         K_THREAD_STACK_SIZEOF(worker_stack),
                                         worker_thread,
                                         "Work_1", NULL, NULL,
                                         WORKER_PRIORITY, 0, K_NO_WAIT);
    register_thread("Worker", worker_tid);  ///< Registra il thread Worker
    
    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di creare il thread Worker 2
    
    // Crea il thread Worker 2
    k_tid_t worker_tid2 = k_thread_create(&worker_thread_data2, worker_stack2,
                                         K_THREAD_STACK_SIZEOF(worker_stack2),
                                         worker_thread,
                                         "Work_2", NULL, NULL,
                                         WORKER_PRIORITY, 0, K_NO_WAIT);
    register_thread("Worker2", worker_tid2);  ///< Registra il thread Worker 2

    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di creare il thread LED CONTROLLER

    // Crea il thread led controller
    k_tid_t led_controller_tid = k_thread_create(&led_controller_data, led_controller_stack,
                                         K_THREAD_STACK_SIZEOF(led_controller_stack),
                                         led_controller_thread,
                                         NULL, NULL, NULL,
                                         LED_CTRL_PRIORITY, 0, K_NO_WAIT);
    register_thread("Led_ctrl", led_controller_tid);  ///< Registra il thread led controller

    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di terminare il thread di inizializzazione
    unregister_thread(init_tid);  ///< Deregistra il thread di inizializzazione

    // Termina il thread di inizializzazione
    k_thread_abort(k_current_get());  ///< Termina il thread corrente (init_thread)
}

// Creazione del thread di inizializzazione
K_THREAD_DEFINE(init_tid,
                CONFIG_MAIN_STACK_SIZE,
                init_thread,
                NULL, NULL, NULL,
                0, 0, 0);  ///< Definizione del thread di inizializzazione