#include <zephyr/kernel.h>
#include <lvgl.h>
#include <stdint.h>
#include <stdio.h>
#include "app.h"

#include <zephyr/drivers/gpio.h>

// Thread Worker: simula attività aggiornando le etichette dei task
void worker_thread(void *p1, void *p2, void *p3) {
    uint32_t counter = 0;
    gui_command_t msg;

    while (1) {
        // Cerca il thread corrente nella lista dei thread monitorati
        for (int i = 0; i < monitored_thread_count; i++) {
            if (monitored_threads[i].tid == k_current_get()) {
                // Formatta il messaggio con snprintf
                char *formatted_message = k_malloc(50);  // Alloca memoria per la stringa (50 caratteri sufficienti)
                if (formatted_message != NULL) {
                    snprintf(formatted_message, 50, "Thread: %s, tick %u", monitored_threads[i].name, counter); // Formatta il messaggio

                    // Prepara il messaggio
                    msg.tid = monitored_threads[i].tid;
                    msg.message = formatted_message;  // Assegna il messaggio formattato

                    msg.cmd = GUI_CMD_WORKER_COUNT;  // Tipo di comando

                    // Invia il messaggio al thread GUI
                    k_msgq_put(&gui_msgq, &msg, K_NO_WAIT);

                    // Ricordati di liberare la memoria allocata
                    k_free(formatted_message);
                }
            }
        }
        counter++;
        k_msleep(1000);  // Aspetta 1 secondo prima di inviare il prossimo aggiornamento
    }
}