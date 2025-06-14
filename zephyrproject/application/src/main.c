#include <zephyr/kernel.h>
#include "app.h"

// Dati e stack per i thread definiti globalmente
K_THREAD_STACK_DEFINE(gui_stack, CONFIG_MAIN_STACK_SIZE);      ///< Stack per il thread GUI
static struct k_thread gui_thread_data;                         ///< Dati del thread GUI

K_THREAD_STACK_DEFINE(worker_stack, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread Worker
static struct k_thread worker_thread_data;                     ///< Dati del thread Worker
//-------------------------------
K_THREAD_STACK_DEFINE(worker_stack2, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread Worker2
static struct k_thread worker_thread_data2;                     ///< Dati del thread Worker2

K_THREAD_STACK_DEFINE(worker_stack3, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread Worker3
static struct k_thread worker_thread_data3;                     ///< Dati del thread Worker3

K_THREAD_STACK_DEFINE(worker_stack4, CONFIG_MAIN_STACK_SIZE);   ///< Stack per il thread Worker3
static struct k_thread worker_thread_data4;                     ///< Dati del thread Worker3
// ------------------------------------
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
                                         NULL, NULL, NULL,
                                         WORKER_PRIORITY, 0, K_NO_WAIT);
    register_thread("Worker", worker_tid);  ///< Registra il thread Worker
    
    // ------------------------------- SOTTO THREAD DI TEST --------------------------------------
    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di creare il thread Worker
    
    // Crea il thread Worker
    k_tid_t worker_tid2 = k_thread_create(&worker_thread_data2, worker_stack2,
                                         K_THREAD_STACK_SIZEOF(worker_stack2),
                                         worker_thread,
                                         NULL, NULL, NULL,
                                         WORKER_PRIORITY, 0, K_NO_WAIT);
    register_thread("Worker2", worker_tid2);  ///< Registra il thread Worker

    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di creare il thread Worker

    // Crea il thread Worker
    k_tid_t worker_tid3 = k_thread_create(&worker_thread_data3, worker_stack3,
                                         K_THREAD_STACK_SIZEOF(worker_stack3),
                                         worker_thread,
                                         NULL, NULL, NULL,
                                         WORKER_PRIORITY, 0, K_NO_WAIT);
    register_thread("Worker3", worker_tid3);  ///< Registra il thread Worker

    k_sleep(K_SECONDS(2));  ///< Attende 2 secondi prima di creare il thread Worker

    // Crea il thread Worker
    k_tid_t worker_tid4 = k_thread_create(&worker_thread_data4, worker_stack4,
                                         K_THREAD_STACK_SIZEOF(worker_stack4),
                                         worker_thread,
                                         NULL, NULL, NULL,
                                         WORKER_PRIORITY, 0, K_NO_WAIT);
    register_thread("Worker4", worker_tid4);  ///< Registra il thread Worker
    //--------------------------------------------------------------------------------------------------
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