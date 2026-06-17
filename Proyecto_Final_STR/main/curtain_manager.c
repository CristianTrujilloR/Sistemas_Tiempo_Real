#include "curtain_manager.h"
#include "servo_motor.h"

#include <stdio.h>

static uint8_t curtain_mode = 0;
/*
 * 0 = AUTO
 * 1 = MANUAL
 */

static uint8_t curtain_percent = 0;

/* ===================================================== */
/* Init                                                   */
/* ===================================================== */

void curtain_init(void)
{
    servo_init();

    curtain_mode = 0;
    curtain_percent = 0;

    servo_set_percent(0);

    printf("Curtain Manager Initialized\n");
}

/* ===================================================== */
/* Mode                                                   */
/* ===================================================== */

void curtain_set_mode(uint8_t mode)
{
    curtain_mode = mode;

    printf(
        "Curtain Mode = %s\n",
        curtain_mode ? "MANUAL" : "AUTO"
    );
}

/* ===================================================== */
/* Manual Position                                        */
/* ===================================================== */

void curtain_set_manual_percent(uint8_t percent)
{
    if(percent > 100)
    {
        percent = 100;
    }

    curtain_percent = percent;

    if(curtain_mode == 1)
    {
        servo_set_percent(percent);

        printf(
            "Curtain Manual Position = %d%%\n",
            percent
        );
    }
}

/* ===================================================== */
/* Automatic Position                                     */
/* ===================================================== */

void curtain_set_auto_percent(uint8_t percent)
{
    if(percent > 100)
    {
        percent = 100;
    }

    curtain_percent = percent;

    if(curtain_mode == 0)
    {
        servo_set_percent(percent);

        printf(
            "Curtain Auto Position = %d%%\n",
            percent
        );
    }
}

/* ===================================================== */
/* Getters                                                */
/* ===================================================== */

uint8_t curtain_get_mode(void)
{
    return curtain_mode;
}

uint8_t curtain_get_percent(void)
{
    return curtain_percent;
}