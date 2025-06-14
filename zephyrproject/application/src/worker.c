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
    const char *worker_name = (const char *)p1;
    char formatted_message[50];
    while (1) {
        // Formatta il messaggio con snprintf
        if (formatted_message != NULL) {
            snprintf(formatted_message, sizeof(formatted_message), "Thread: %s, tick %u", worker_name, counter); // Formatta il messaggio

            // Prepara il messaggio
            msg.tid = k_current_get();
            msg.message = formatted_message;  // Assegna il messaggio formattato

            msg.cmd = GUI_CMD_WORKER_COUNT;  // Tipo di comando

            // Invia il messaggio al thread GUI
            k_msgq_put(&gui_msgq, &msg, K_NO_WAIT);
        }
        counter++;
        k_msleep(1000);  // Aspetta 1 secondo prima di inviare il prossimo aggiornamento
    }
}