#pragma once

#include <Arduino.h>
#include <FlashStorage.h>
#include "../Timer/Timer.h"
#include "../Task/Task.h"

#define LOGGER_NUMBER_PAGES 10                      // see Flash Computations File
#define LOGGER_TASK_CALL_INTERVAL_MS 10000          // Logger task called every 10s 
#define LOGGER_WRITE_INTERVAL_MS 86400000ULL        // How often we cyclically update the flash from the buffer (writing the buffer can additionally be forced: Once per day)
#define LOGGER_NO_RESET_HISTORY_ITEMS 10

struct ResetHistoryItem_t{
    uint8_t reason;
    uint64_t time_of_reset;
};

struct FlashData_t {
    uint32_t log_id;
    uint64_t time_of_storage;
    ResetHistoryItem_t reset_history[LOGGER_NO_RESET_HISTORY_ITEMS];
    uint16_t number_lora_joinig_errors;
};

class Logger: public Task
{
    static Logger* the_logger;                                            // Pointer to Singleton Logger Object
    static FlashStorageClass<FlashData_t>* storage[LOGGER_NUMBER_PAGES];  // Array of LOGGER_NUMBER_PAGES flash pages

    Timer* timer;

    FlashData_t ram_buffer;                 // The RAM-Buffer in which we write / from which we read
    uint64_t time_next_buffer_write_ms;     // Point of time in which the buffer shall be cyclically stored next

    uint8_t last_page_written = 0;          // The number of the last page that was used to write the buffer

public:
    Logger();

    static Logger* get_logger();    // Retrieves Singleton Logger Object
    void read_flash_into_buffer();  // Reads the last written Flash Page pack into the buffer
    void write_buffer_into_flash(); // Writes the buffer onto the next flash page, increases flash last_page_written
    void run();                     // inherited from task, cyclical activities
    void initialize();              // To be called once on startup to initialize the logger's RAM buffer from flash; this cannot be done in the constructor, since the singleton object is created to early, and initialization will result in a lockup
    void add_reset(uint8_t reason, uint64_t time);  // Overwrites the oldest reset in the RAM buffer with the provided reset information
    void report_lora_joining_error();
    void reset_lora_joining_errors();
    ResetHistoryItem_t* get_last_reset_history_item();
    ResetHistoryItem_t* get_first_reset_history_item();
    FlashData_t* get_ram_buffer();

    void add_to_lora_status_message(LoraMessage *msg);

private:
    void read_flash_page_into_buffer(int no_flashpage);
    void write_buffer_into_flash_page(int no_flashpage);     // physically writes buffer to flash
    uint16_t get_index_of_last_reset();
    uint16_t get_index_of_oldest_reset();
};