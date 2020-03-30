 /*
  * The library is not extensively tested and only
  * meant as a simple explanation and for inspiration.
  * NO WARRANTY of ANY KIND is provided.
  */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf_drv_spi.h"
#include "boards.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_log.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_dcnd_spi.h"

/* Pins to connect MPU. Pinout is different for nRF51 DK and nRF52 DK
 * and therefore I have added a conditional statement defining different pins
 * for each board. This is only for my own convenience. 
 */

#define E_PAPER_SPI_SS_PIN   SPI_SS_PIN
#define E_PAPER_SPI_MISO_PIN SPI_MISO_PIN
#define E_PAPER_SPI_MOSI_PIN SPI_MOSI_PIN
#define E_PAPER_SPI_SCK_PIN  SPI_SCK_PIN

#define DCND_SPI_TIMEOUT           1000000 //default value 10000

#define SPI_INSTANCE   0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
    //NRF_LOG_INFO("Transfer completed.");
    /*NRF_LOG_INFO("Transfer completed.");
    if (m_rx_buf[0] != 0)
    {
        NRF_LOG_INFO(" Received:");
        NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
    }*/
}


/**@brief Function to initiate TWI drivers
 *
 * @retval      uint32_t        Error code
 */
uint32_t nrf_drv_epaper_spi_init(void)
{
    uint32_t err_code;

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = E_PAPER_SPI_SS_PIN;
    spi_config.miso_pin = E_PAPER_SPI_MISO_PIN;
    spi_config.mosi_pin = E_PAPER_SPI_MOSI_PIN;
    spi_config.sck_pin  = E_PAPER_SPI_SCK_PIN;

    err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function to initiate TWI drivers
 *
 * @retval      uint32_t        Error code
 */
uint32_t nrf_drv_epaper_spi_transfer(uint8_t const * p_tx_buffer,
                                     uint8_t         tx_buffer_length)
{
    uint32_t err_code;
    uint32_t timeout = DCND_SPI_TIMEOUT;

    uint8_t p_rx_buffer;

    err_code = nrf_drv_spi_transfer(&spi, p_tx_buffer, tx_buffer_length, &p_rx_buffer, 0x00);
    APP_ERROR_CHECK(err_code);

    while((!spi_xfer_done) && --timeout);
    if(!timeout) return NRF_ERROR_TIMEOUT;
    spi_xfer_done = false;

    return err_code;
}