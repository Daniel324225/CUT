#include <stdio.h>
#include <stdlib.h>

#include "printer.h"

int Printer_run(void* printer_v) {
    Printer* printer = printer_v;

    while(!*(printer->done)) {
        Watchdog_notify_active(printer->wdi);
        while (!Lock_for_read(printer->input_lock, 100)) {
           Watchdog_notify_active(printer->wdi);
        }

        const size_t core_count = printer->input->core_count;
        system("clear");
        for (size_t core_idx = 0; core_idx != core_count; ++core_idx){
            printf("Core %3zu usage: %6.2f%%\n", core_idx, (double)printer->input->core_usage[core_idx] * 100.0);
        }
        
        Lock_unlock(printer->input_lock);
        
        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
    }
    
    Watchdog_notify_finished(printer->wdi);
    return 0;
}
