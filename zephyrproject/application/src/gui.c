#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <lvgl.h>
#include <lvgl_input_device.h>
#include "app.h"

struct k_mutex gui_mutex;
static const int32_t sleep_time_ms = 50; // tempo di sleep del loop principale
void refreshGUI(lv_obj_t *scr, lv_style_t *rect_style);
void create_buttons(lv_obj_t *button_container, lv_style_t *rect_style);
static void btn_event_handler(lv_event_t * e);

// Thread GUI: inizializza il display e costruisce la schermata grafica
void gui_thread(void *p1, void *p2, void *p3) {
    const struct device *display;
    // const struct device *touch_dev;

    lv_style_t rect_style;

    display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display)); // Ottiene il riferimento al dispositivo display dichiarato nel device tree.
    if (!device_is_ready(display)) { // Verifica che il dispositivo sia pronto
        printk("Error: display not ready\n");
        return;
    }
    /*touch_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_touch));
    if (!device_is_ready(touch_dev)) {
        printk("Error: Touchscreen not ready\n");
        return;
    }*/
    k_mutex_init(&gui_mutex); // Inizializza mutex per accesso thread-safe a LVGL

    // Sfondo della schermata attiva
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); // Sfondo nero
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0); // Imposta l’opacità totale (copertura completa dello sfondo).

    // contenitore per i thread (lo scrolling si attiva automaticamente se gli oggetti vanno fuori dallo schermo del contenitore)
    lv_obj_t *rect_container = lv_obj_create(scr);
    lv_obj_set_size(rect_container, LV_HOR_RES, LV_VER_RES - BUTTON_HEIGHT * 1.5 - RECT_SPACING * 2);
    lv_obj_align(rect_container, LV_ALIGN_TOP_MID, 0, 0);

    // contenitore per i pulsanti
    lv_obj_t *button_container = lv_obj_create(scr);
    lv_obj_set_size(button_container, LV_HOR_RES, BUTTON_HEIGHT * 1.5);
    lv_obj_align(button_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_move_background(button_container);  // Porta il contenitore dei pulsanti sopra gli altri oggetti

    // stile del rettangolo
    lv_style_init(&rect_style);
    lv_style_set_bg_color(&rect_style, lv_color_hex(0x3333AA)); // colore sfondo
    lv_style_set_bg_opa(&rect_style, LV_OPA_COVER);              // opacità piena
    lv_style_set_radius(&rect_style, 4);                         // angoli arrotondati
    
    display_blanking_off(display); // Disattiva il “blanking” (schermata nera) del display, rendendo visibile il contenuto disegnato.
    
    // Crea i rettangoli per i thread monitorati
    refreshGUI(rect_container, &rect_style);

    // Crea i pulsanti nella parte inferiore
    create_buttons(button_container, &rect_style);

    while (1) {
        gui_command_t cmd;
        if (k_msgq_get(&gui_msgq, &cmd, K_NO_WAIT) == 0) {
            k_mutex_lock(&gui_mutex, K_FOREVER);
            switch (cmd.cmd) {
                case GUI_CMD_REFRESH:
                    refreshGUI(rect_container, &rect_style);
                    break;

                case GUI_CMD_WORKER_COUNT:
                    // Cerca il thread corrispondente e aggiorna la sua etichetta
                    for (int i = 0; i < monitored_thread_count; i++) {
                        if (monitored_threads[i].tid == cmd.tid) {
                            // Aggiorna l'etichetta con il messaggio ricevuto
                            lv_label_set_text(monitored_threads[i].label, cmd.message);
                            break;
                        }
                    }
                    break;
            }

            k_mutex_unlock(&gui_mutex);
        }
        lv_timer_handler(); // Mantiene attiva la GUI (aggiorna eventi, animazioni)
        k_msleep(sleep_time_ms); // sospende il thread corrente per il tempo indicato (50 ms).
    }
}

void refreshGUI(lv_obj_t *scr, lv_style_t *rect_style){
    // Crea o aggiorna i rettangoli
    lv_obj_clean(scr);
    for (int i = 0; i < monitored_thread_count; i++) {
        lv_obj_t *rect = lv_obj_create(scr); // Crea rettangolo per il thread
        lv_obj_set_size(rect, LV_HOR_RES - 20, RECT_HEIGHT);  // Larghezza quasi piena, altezza definita
        lv_obj_add_style(rect, rect_style, 0); // Applica lo stile
        lv_obj_align(rect, LV_ALIGN_TOP_MID, 0, i * (RECT_HEIGHT + RECT_SPACING) + 10); // Posizione dei rettangoli

        // Crea l'etichetta con il nome del thread
        lv_obj_t *label = lv_label_create(rect);  // Crea etichetta
        lv_label_set_text_fmt(label, "Thread: %s", monitored_threads[i].name); // Imposta il testo
        lv_obj_set_style_text_color(label, lv_color_white(), 0); // Imposta il colore del testo
        lv_obj_center(label); // Centra l'etichetta

        // Salva il riferimento al rettangolo (label)
        monitored_threads[i].label = label;
    }
}

// Funzione per creare i pulsanti
void create_buttons(lv_obj_t *button_container, lv_style_t *rect_style) {
    lv_obj_t *btn1 = lv_btn_create(button_container);
    lv_obj_set_size(btn1, LV_HOR_RES / 2 - 20, BUTTON_HEIGHT);
    lv_obj_add_style(btn1, rect_style, 0); // Applica lo stile
    lv_obj_align(btn1, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *btn_label1 = lv_label_create(btn1);
    lv_label_set_text(btn_label1, "btn one");
    lv_obj_set_style_text_color(btn_label1, lv_color_white(), 0); // Imposta il colore del testo
    lv_obj_center(btn_label1);

    lv_obj_t *btn2 = lv_btn_create(button_container);
    lv_obj_set_size(btn2, LV_HOR_RES / 2 - 20, BUTTON_HEIGHT);
    lv_obj_add_style(btn2, rect_style, 0); // Applica lo stile
    lv_obj_align(btn2, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_t *btn_label2 = lv_label_create(btn2);
    lv_label_set_text(btn_label2, "btn two");
    lv_obj_set_style_text_color(btn_label2, lv_color_white(), 0); // Imposta il colore del testo
    lv_obj_center(btn_label2);

    // Associa l'evento di clic ai pulsanti
    lv_obj_add_event_cb(btn1, btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn2, btn_event_handler, LV_EVENT_CLICKED, NULL);
}

// Gestore degli eventi dei pulsanti
static void btn_event_handler(lv_event_t * e) {
    //lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);

    // Ottieni il colore attuale di sfondo
    lv_color_t current_bg_color = lv_obj_get_style_bg_color(obj, LV_STATE_DEFAULT);

    // Cambia il colore di sfondo in base al colore corrente
    if (current_bg_color.red == 0 && current_bg_color.green == 255 && current_bg_color.blue == 0) {
        // Se il colore è verde (0x00FF00), cambia a rosso (0xFF0000)
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), LV_STATE_DEFAULT);
    } else {
        // Se il colore non è verde, cambia a verde
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x00FF00), LV_STATE_DEFAULT);
    }
}