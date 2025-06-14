#include "app.h"
#include <string.h>

// Array che tiene traccia dei thread monitorati
monitored_thread_t monitored_threads[MAX_TASKS];
int monitored_thread_count = 0; ///< Contatore dei thread monitorati

/**
 * @brief Registra un nuovo thread nel sistema di monitoraggio.
 *
 * Questa funzione aggiunge un thread alla lista dei thread monitorati
 * e invia un comando di aggiornamento alla GUI.
 *
 * @param name Nome del thread da registrare
 * @param tid ID del thread da registrare
 */
void register_thread(const char *name, k_tid_t tid) {
    if (monitored_thread_count < MAX_TASKS) {
        k_mutex_lock(&monitored_threads_mutex, K_FOREVER); // Blocca l'accesso al mutex

        // Aggiungi il thread all'array dei thread monitorati
        monitored_threads[monitored_thread_count].name = name;
        monitored_threads[monitored_thread_count].tid = tid;
        monitored_threads[monitored_thread_count].label = NULL;  // Inizializza la label a NULL
        monitored_thread_count++;  // Incrementa il contatore dei thread monitorati

        k_mutex_unlock(&monitored_threads_mutex);  // Rilascia il mutex dopo l'operazione

        // Crea un messaggio di refresh per aggiornare la GUI con il nuovo thread
        gui_command_t cmd = {
            .cmd = GUI_CMD_REFRESH,  // Comando di refresh
            .tid = tid,              // ID del thread
            .message = NULL          // Messaggio vuoto, la GUI aggiornerà la visualizzazione
        };

        // Invia il comando di aggiornamento alla coda dei messaggi della GUI
        k_msgq_put(&gui_msgq, &cmd, K_NO_WAIT);
    }
}

/**
 * @brief Deregistra un thread dal sistema di monitoraggio.
 *
 * Questa funzione rimuove un thread dalla lista dei thread monitorati
 * e invia un comando di aggiornamento alla GUI.
 *
 * @param tid ID del thread da rimuovere
 */
void unregister_thread(k_tid_t tid) {
    k_mutex_lock(&monitored_threads_mutex, K_FOREVER);  // Acquisisce il mutex per proteggere l'accesso
    for (int i = 0; i < monitored_thread_count; i++) {
        if (monitored_threads[i].tid == tid) {
            // Sposta i thread successivi per rimuovere l'elemento
            for (int j = i; j < monitored_thread_count - 1; j++) {
                monitored_threads[j] = monitored_threads[j + 1];
            }

            monitored_thread_count--;  // Decrementa il contatore dei thread monitorati

            k_mutex_unlock(&monitored_threads_mutex);  // Rilascia il mutex dopo l'operazione

            // Crea un messaggio di refresh per aggiornare la GUI con il thread rimosso
            gui_command_t cmd = {
                .cmd = GUI_CMD_REFRESH,  // Comando di refresh
                .tid = tid,              // ID del thread rimosso
                .message = NULL          // Messaggio vuoto per rimuovere il thread dalla GUI
            };

            // Invia il comando di aggiornamento alla coda dei messaggi della GUI
            k_msgq_put(&gui_msgq, &cmd, K_NO_WAIT);
            
            break;  // Esce dal ciclo dopo aver trovato e rimosso il thread
        }
    }
}