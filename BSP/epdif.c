/**
 *  @filename   :   epdif.c
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     July 7 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epdif.h"
#include "nrf_drv_dcnd_spi.h"
#include "nrf_delay.h"

#include "nrf.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
//#include "main.h"

//extern SPI_HandleTypeDef hspi1;

/*EPD_Pin epd_cs_pin = {
  SPI_CS_GPIO_Port,
  SPI_CS_Pin,
};

EPD_Pin epd_rst_pin = {
  RST_GPIO_Port,
  RST_Pin,
};

EPD_Pin epd_dc_pin = {
  DC_GPIO_Port,
  DC_Pin,
};

EPD_Pin epd_busy_pin = {
  BUSY_GPIO_Port,
  BUSY_Pin,
};*/

EPD_Pin pins[4];

void EpdDigitalWriteCallback(int pin_num, int value) {
  if (value == HIGH) {
    nrf_drv_gpiote_out_set(pin_num);
  } else {
    nrf_drv_gpiote_out_clear(pin_num);
  }
}

int EpdDigitalReadCallback(int pin_num) {
  if (nrf_drv_gpiote_in_is_set(pin_num)) {
    return HIGH;
  } else {
    return LOW;
  }
}

void EpdDelayMsCallback(unsigned int delaytime) {
    nrf_delay_ms(delaytime);
}

void EpdSpiTransferCallback(unsigned char data) {
  //HAL_GPIO_WritePin((GPIO_TypeDef*)pins[CS_PIN].port, pins[CS_PIN].pin, GPIO_PIN_RESET);
  //HAL_SPI_Transmit(&hspi1, &data, 1, 1000);
  //HAL_GPIO_WritePin((GPIO_TypeDef*)pins[CS_PIN].port, pins[CS_PIN].pin, GPIO_PIN_SET);
  nrf_drv_epaper_spi_transfer(&data, 1);
}

/*
 * TODO
 */
void gpio_out_config(int pin_num)
{
    ret_code_t err_code;

    // Change the pinMode to OUTPUT and pull the pin LOW
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(pin_num, &out_config);
    APP_ERROR_CHECK(err_code);
}

/*
 * TODO
 */
void gpio_in_config(int pin_num)
{
    ret_code_t err_code;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(pin_num, &in_config, NULL);
    APP_ERROR_CHECK(err_code);
}

int EpdInitCallback(void) {
  nrf_drv_epaper_spi_init();

  uint32_t err_code = nrf_drv_gpiote_init();
  APP_ERROR_CHECK(err_code);

  gpio_out_config(RST_PIN);
  gpio_out_config(DC_PIN);
  gpio_in_config(BUSY_PIN);

  /*pins[CS_PIN] = epd_cs_pin;
  pins[RST_PIN] = epd_rst_pin;
  pins[DC_PIN] = epd_dc_pin;
  pins[BUSY_PIN] = epd_busy_pin;*/
  
  return 0;
}

