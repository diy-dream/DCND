#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "DCND.h"

void dcnd_init(dcnd_t* dcnd)
{
    dcnd->state = IDLE_STATE;
    memset (dcnd->message, '\0', MAX_LINE_CHARACTER * MAX_LINE);
}

void dcnd_protocol_decode(dcnd_t* dcnd, uint8_t const * p_data)
{
    uint8_t len = strlen(p_data);

    //dcnd->message[strlen(dcnd->message) - 1] = '\0';

    char str[MAX_LINE_CHARACTER * MAX_LINE];
    memset(str, '\0', sizeof(str));

    for(int i = 0; i < len; i++)
    {
        if(p_data[i] == '\n')
        {
            strncpy(str, p_data,  i);
            break;
        }
    }

    if(!strcmp(START_TRANSFER_MSG_ID, str))
    {
        NRF_LOG_INFO("START_TRANSFER_MSG\n");
        dcnd->message_types = START_TRANSFER_MSG;
    }
    else if(!strcmp(STOP_TRANSFER_MSG_ID, str))
    {
        NRF_LOG_INFO("STOP_TRANSFER_MSG\n");
        dcnd->message_types = STOP_TRANSFER_MSG;
    }
    else if(!strcmp(START_TRANSFER_IMG_ID, str))
    {
        NRF_LOG_INFO("START_TRANSFER_IMG\n");
        dcnd->message_types = START_TRANSFER_IMG;
    }
    else if(!strcmp(STOP_TRANSFER_IMG_ID, str))
    {
        NRF_LOG_INFO("STOP_TRANSFER_IMG\n");
        dcnd->message_types = STOP_TRANSFER_IMG;
    }
    else
    {
        NRF_LOG_INFO("MSG_ID_UNKNOWN\n");
        dcnd->message_types = MSG_ID_UNKNOWN;
    }
}