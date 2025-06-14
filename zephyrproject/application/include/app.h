#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

// Costanti di configurazione per l'interfaccia grafica
#define RECT_HEIGHT     50  ///< Altezza del rettangolo nella GUI
#define RECT_SPACING    10  ///< Spaziatura tra i rettangoli nella GUI
#define GUI_PRIORITY    5   ///< Priorità del thread GUI
#define WORKER_PRIORITY 7   ///< Priorità del thread Worker
#define MAX_TASKS       6   ///< Numero massimo di task monitorabili
#define GUI_QUEUE_LEN   10  ///< Lunghezza della coda per i messaggi GUI
#define BUTTON_HEIGHT 40
#define BUTTON_SPACING 10

/** 
 * @brief Struttura per i thread monitorati.
 * Contiene nome, ID e label associata al thread.
 */
typedef struct {
    const char *name;      ///< Nome del thread
    k_tid_t tid;           ///< ID del thread
    lv_obj_t *label;       ///< Oggetto label nella GUI
} monitored_thread_t;

/**
 * @brief Tipi di comandi per la comunicazione con la GUI.
 */
typedef enum {
    GUI_CMD_REFRESH,       ///< Aggiorna la GUI
    GUI_CMD_WORKER_COUNT   ///< Aggiorna il contatore del worker
} gui_command_type_t;

/**
 * @brief Struttura del messaggio per i comandi della GUI.
 */
typedef struct {
    gui_command_type_t cmd;   ///< Tipo di comando
    k_tid_t tid;              ///< ID del thread da aggiornare
    const char *message;      ///< Messaggio da visualizzare nella label
} gui_command_t;

// Variabili esterne per la gestione dei thread e della comunicazione
extern struct k_msgq gui_msgq;           ///< Coda dei messaggi per il thread GUI
extern monitored_thread_t monitored_threads[MAX_TASKS]; ///< Array dei thread monitorati
extern int monitored_thread_count;       ///< Numero di thread monitorati
extern struct k_mutex gui_mutex;         ///< Mutex per la sincronizzazione della GUI
extern struct k_mutex monitored_threads_mutex;  ///< Mutex per proteggere monitored_threads

/** 
 * @brief Funzione del thread GUI.
 * Gestisce gli aggiornamenti della GUI in base ai messaggi ricevuti.
 */
void gui_thread(void *p1, void *p2, void *p3);

/** 
 * @brief Funzione del thread Worker.
 * Monitora i thread e aggiorna la GUI.
 */
void worker_thread(void *p1, void *p2, void *p3);

/**
 * @brief Registra un thread nel sistema di monitoraggio.
 * Aggiunge un thread alla lista monitorata e invia un comando di aggiornamento.
 */
void register_thread(const char *name, k_tid_t tid);

/**
 * @brief Deregistra un thread dal sistema di monitoraggio.
 * Rimuove un thread dalla lista monitorata e invia un comando di aggiornamento.
 */
void unregister_thread(k_tid_t tid);