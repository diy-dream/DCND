 /* 
  * The library is not extensively tested and only 
  * meant as a simple explanation and for inspiration. 
  * NO WARRANTY of ANY KIND is provided. 
  */

#ifndef NRF_DRV_ICM20948__
#define NRF_DRV_ICM20948__


#include <stdbool.h>
#include <stdint.h>

 

/**@brief Function to initiate TWI drivers
 *
 * @retval      uint32_t        Error code
 */
uint32_t nrf_drv_epaper_spi_init(void);
	
/**@brief Function to initiate TWI drivers
 *
 * @retval      uint32_t        Error code
 */
uint32_t nrf_drv_epaper_spi_transfer(uint8_t const * p_tx_buffer,
                                     uint8_t         tx_buffer_length);
													
#endif /* NRF_DRV_ICM20948__ */

/**
  @}
*/

