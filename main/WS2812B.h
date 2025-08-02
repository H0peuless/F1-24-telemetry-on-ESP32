#include "env.h"
#include "driver/gptimer_types.h"
#include "driver/gptimer.h"
#include "esp_log.h"

#ifdef STRIP_SIZE

struct LED_Strip 
{
    int G [STRIP_SIZE];
    int R [STRIP_SIZE];
    int B [STRIP_SIZE];
};

static struct LED_Strip RPM;

void init_clock();
void LED_Strip_refresh(uint16_t RPM);

#endif