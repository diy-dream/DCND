 /* 
  * The library is not extensively tested and only 
  * meant as a simple explanation and for inspiration. 
  * NO WARRANTY of ANY KIND is provided. 
  */

#ifndef DCND_H
#define DCND_H


#include <stdbool.h>
#include <stdint.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define START_TRANSFER_MSG_ID           "START_TRANSFER_MSG"
#define STOP_TRANSFER_MSG_ID            "STOP_TRANSFER_MSG"
#define START_TRANSFER_IMG_ID           "START_TRANSFER_IMG"
#define STOP_TRANSFER_IMG_ID            "STOP_TRANSFER_IMG"

#define Y_INIT_POSITION                 20
#define Y_SPACE                         30
#define MAX_LINE_CHARACTER              23
#define MAX_LINE                        9

typedef enum state_types_t {
    IDLE_STATE, RECEIVED_DATA_MSG, RECEIVED_DATA_IMG
} state_types; 

typedef enum message_types_t {
    START_TRANSFER_MSG, STOP_TRANSFER_MSG, START_TRANSFER_IMG, STOP_TRANSFER_IMG, MSG_ID_UNKNOWN
} message_types; 

typedef struct {
  char message[MAX_LINE_CHARACTER*MAX_LINE];
  message_types message_types;
  state_types state;
} dcnd_t; 

/**@brief Function to initiate TWI drivers
 *
 * @retval      void
 */
void dcnd_protocol_decode(dcnd_t* dcnd, uint8_t const * p_data);

/**@brief Function to initiate TWI drivers
 *
 * @retval      void
 */
void dcnd_init(dcnd_t* dcnd);
													
#endif /* DCND_H */

/**
  @}
*/

