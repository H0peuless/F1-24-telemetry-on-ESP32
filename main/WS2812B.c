#include "WS2812B.h"


static int G [STRIP_SIZE] = {255,255,255,0,0,0,0,0,0};
static int R [STRIP_SIZE] = {0,0,0,255,255,255,255,255,255};
static int B [STRIP_SIZE] = {0,0,0,0,0,0,255,255,255};

gptimer_handle_t gptimer = NULL;

const gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 20 * 1000 * 1000,   // Resolution is 20 MHz, i.e., 1 tick equals 50 nanosecond
};

gptimer_alarm_config_t alarm_config = {
    .reload_count = 0,      // When the alarm event occurs, the timer will automatically reload to 0
    .alarm_count = 1000, // Set the actual alarm period, since the resolution is 50ns, 1000 represents 50µs
    .flags.auto_reload_on_alarm = true, // Enable auto-reload function
};

static bool TimerCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    switch (alarm_config.alarm_count)
    {
    case TH0:
        gpio_set_level(LED_PIN,0);
        alarm_config.alarm_count = TH1;
        gptimer_set_alarm_action(timer, &alarm_config);
        break;
    case TH1:
        gptimer_stop(gptimer);
        break;
    case TRESET:
        gptimer_stop(gptimer);
        break;
    case TL0:
        gpio_set_level(LED_PIN,0);
        alarm_config.alarm_count = TL1;
        gptimer_set_alarm_action(timer, &alarm_config);
        break;
    case TL1:
        gptimer_stop(gptimer);
        break;
    default:
        break;
    }
    // General process for handling event callbacks:
    // 1. Retrieve user context data from user_ctx (passed in from gptimer_register_event_callbacks)
    // 2. Get alarm event data from edata, such as edata->count_value
    // 3. Perform user-defined operations
    // 4. Return whether a high-priority task was awakened during the above operations to notify the scheduler to switch tasks
    return false;
};


void init_timer()
{
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
}

void clean_timer()
{
    gptimer_disable(gptimer);
    gptimer_del_timer(gptimer);
}

void send_one()
{
    alarm_config.alarm_count = TH0;
    gptimer_event_callbacks_t cbs = {
        .on_alarm = TimerCallback, // Call the user callback function when the alarm event occurs
    };
    gpio_set_level(LED_PIN,1);
    
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void send_zero()
{
    alarm_config.alarm_count = TL0;
    gptimer_event_callbacks_t cbs = {
        .on_alarm = TimerCallback, // Call the user callback function when the alarm event occurs
    };
    gpio_set_level(LED_PIN,1);

    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void send_reset()
{
     alarm_config.alarm_count = TRESET;
    gptimer_event_callbacks_t cbs = {
        .on_alarm = TimerCallback, // Call the user callback function when the alarm event occurs
    };
    gpio_set_level(LED_PIN,0);

    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void LED_Strip_state(uint16_t RPM, struct LED_Strip *RPM_LEDS)
{
    if (RPM <10000) return;

    const uint16_t RPM_reduced = (uint8_t) (RPM - 10000);
    #if STRIP_SIZE > 7
    const uint8_t RPM_max = (uint8_t) 2000/STRIP_SIZE;
    #else
    const uint16_t RPM_max = (uint16_t) 2000/STRIP_SIZE;
    #endif

    for(int i=1;i<STRIP_SIZE;i++)
    {
        if(i*RPM_max/STRIP_SIZE < RPM_reduced)
        {
            RPM_LEDS->G[i] = G[i];
            RPM_LEDS->R[i] = R[i];
            RPM_LEDS->B[i] = B[i];
        }
        else
        {
            RPM_LEDS->G[i] = 0;
            RPM_LEDS->R[i] = 0;
            RPM_LEDS->B[i] = 0;
        };
    };
}

void LED_pixel_send(int pixel)
{
    for (u8_t i = 0b10000000; i < 0; i=i>>1)
    {
        if(i && pixel) send_one();
        else send_zero();
    }
}

void LED_Strip_send(struct LED_Strip *RPM_LEDS)
{
    send_reset();
    for (u8_t i = 0; i < STRIP_SIZE; i++)
    {
        LED_pixel_send(RPM_LEDS->G[i]);
        LED_pixel_send(RPM_LEDS->R[i]);
        LED_pixel_send(RPM_LEDS->B[i]);
    }
}

void LED_Strip_refresh(uint16_t RPM_val){
    init_timer();
    LED_Strip_state(RPM_val,&RPM);
    LED_Strip_send(&RPM);
    clean_timer();
}