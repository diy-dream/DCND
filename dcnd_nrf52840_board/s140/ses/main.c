/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 * @defgroup uart_example_main main.c
 * @{
 * @ingroup uart_example
 * @brief UART Example Application main file.
 *
 * This file contains the source code for a sample application using UART.
 *
 */

#include <stdint.h>
#include <string.h>

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "ble_bas.h"
#include "ble_nus.h"
//#include "app_uart.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_saadc.h"
#include "nrf_power.h"
#include "nrf_bootloader_info.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "epd4in2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include "DCND.h"

#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#define PIN_CHG         12 //P0.12
#define PIN_PG          7  //P0.07
#define PIN_ISET2       8  //P0.08
#define PIN_CLR_SCREEN  30 //P0.30 

#define COLORED     0
#define UNCOLORED   1

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "DCND"                                      /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define BATTERY_LEVEL_MEAS_INTERVAL     APP_TIMER_TICKS(2000)                       /**< Battery level measurement interval (ticks). */
#define MIN_BATTERY_LEVEL               81                                          /**< Minimum simulated battery level. */
#define MAX_BATTERY_LEVEL               100                                         /**< Maximum simulated 7battery level. */
#define BATTERY_LEVEL_INCREMENT         1                                           /**< Increment between each simulated battery level measurement. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   600                                         /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION    6                                           /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS  270                                         /**< Typical forward voltage drop of the diode . */
#define ADC_RES_10BIT                   1024                                        /**< Maximum digital value for 10-bit ADC conversion. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 *
 * @retval     Result converted to millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)


BLE_BAS_DEF(m_bas);                                                                 /**< Structure used to identify the battery service. */
BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */
APP_TIMER_DEF(m_battery_timer_id);
APP_TIMER_DEF(m_push_btn_timer_id);

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

static nrf_saadc_value_t adc_buf[2];

static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE},
    {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE}
};

static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt);

/** EPaper variable**/
//unsigned char* frame_buffer = (unsigned char*)malloc(EPD_WIDTH * EPD_HEIGHT / 8);
static unsigned char frame_buffer[15000];
EPD epd;
Paint paint;
static volatile bool isDisplay = true;
static uint8_t line_counter = 0;
static uint8_t index_character = 0;
static char message[250];
static dcnd_t dcnd;

/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}

//lint -esym(528, m_app_shutdown_handler)
/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);


static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
        // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //Go to system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};


static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));

    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}


static void disconnect(uint16_t conn_handle, void * p_context)
{
    UNUSED_PARAMETER(p_context);

    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
    }
    else
    {
        NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
    }
}



// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
        {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");

            // Prevent device from advertising on disconnect.
            ble_adv_modes_config_t config;
            advertising_config_get(&config);
            config.ble_adv_on_disconnect_disabled = true;
            ble_advertising_modes_config_set(&m_advertising, &config);

            // Disconnect all other bonded devices that currently are connected.
            // This is required to receive a service changed indication
            // on bootup after a successful (or aborted) Device Firmware Update.
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling the ADC interrupt.
 *
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        nrf_saadc_value_t adc_result;
        uint16_t          batt_lvl_in_milli_volts;
        uint8_t           percentage_batt_lvl;
        uint32_t          err_code;

        adc_result = p_event->data.done.p_buffer[0];

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);
        APP_ERROR_CHECK(err_code);

        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) +
                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;
        percentage_batt_lvl = battery_level_in_percent(batt_lvl_in_milli_volts);

        err_code = ble_bas_battery_level_update(&m_bas, percentage_batt_lvl, BLE_CONN_HANDLE_ALL);
        if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_BUSY) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
           )
        {
            APP_ERROR_HANDLER(err_code);
        }
    }
}


/**@brief Function for configuring ADC to do battery level conversion.
 */
static void adc_configure(void)
{
    ret_code_t err_code = nrf_drv_saadc_init(NULL, saadc_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_saadc_channel_config_t config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
    err_code = nrf_drv_saadc_channel_init(0, &config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&adc_buf[0], 1);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&adc_buf[1], 1);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *          This function will start the ADC.
 *
 * @param[in] p_context   Pointer used for passing some arbitrary information (context) from the
 *                        app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    ret_code_t err_code;
    err_code = nrf_drv_saadc_sample();
    APP_ERROR_CHECK(err_code);
}


static bool btn_timer_is_set = false;

/**@brief Function for handling the push button timer timeout, long or short.
 *
 * @details TODO.
 *
 * @param[in] p_context   Pointer used for passing some arbitrary information (context) from the
 *                        app_start_timer() call to the timeout handler.
 */
static void button_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    if(!nrf_drv_gpiote_in_is_set(PIN_CLR_SCREEN)){
        NVIC_SystemReset();
    }else{
        NRF_LOG_INFO("Clear the screen");
        EPD_DisplayFrame(&epd, IMG_PAS_DE_NOUVEAU_MSG);
    }

    btn_timer_is_set = false;
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);

    // Create clear button timer.
    err_code = app_timer_create(&m_push_btn_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                button_timeout_handler);

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void print_character_on_screen(uint8_t * p_data)
{
    int i = 0;

    //Clear the frame_buffer
    Paint_Clear(&paint, COLORED);

    p_data[strlen(p_data) - 1] = '\0';

    uint8_t max = strlen(p_data)/MAX_LINE_CHARACTER;
    char line[MAX_LINE_CHARACTER + 1];
    memset(line, '\0', sizeof(line));

    //Function to remove/replace all bad the characters
    for(int counter = 0; counter < strlen(p_data); counter++){
        if(p_data[counter] == '\n' || p_data[counter] == '\r' || p_data[counter] == '\t'){
            p_data[counter] = 0x20; //0x20 = SPACE
        }else if(p_data[counter] == 0xC3){
            //Special character
            if(p_data[counter+1] == 0xA9 || p_data[counter+1] == 0xA8 || p_data[counter+1] == 0xAB || p_data[counter+1] == 0xAA){ // 0xA9 = �, 0xA8 = �, 0xAB = �, 0xAA = �
                p_data[counter] = 0x65; //0x65 = e
            }else{
                //Unknow character
                p_data[counter] = 0x3F; //0x3F = ?
            }
            for(int cnt_move = counter+1; cnt_move < strlen(p_data); cnt_move++){
                p_data[cnt_move] = p_data[cnt_move+1];
            }
        }
    }

    // ******* //

    if(max > MAX_LINE)
    {
        //Print lines except the last one
        for(i = 0; i < MAX_LINE; i++){
            strncpy(line, p_data + (MAX_LINE_CHARACTER * i), MAX_LINE_CHARACTER);
            Paint_DrawStringAt(&paint, 0, Y_INIT_POSITION + (Y_SPACE * line_counter), line, &Font24, UNCOLORED);
            line_counter++;
        }
    }
    else
    {
        //Print lines except the last one
        for(i = 0; i < max; i++){
            strncpy(line, p_data + (MAX_LINE_CHARACTER * i), MAX_LINE_CHARACTER);
            Paint_DrawStringAt(&paint, 0, Y_INIT_POSITION + (Y_SPACE * line_counter), line, &Font24, UNCOLORED);
            line_counter++;
        }
    
        //Print the last line
        char end_line[MAX_LINE_CHARACTER + 1];
        memset(end_line, '\0', sizeof(end_line));
        strncpy(end_line, p_data + (MAX_LINE_CHARACTER * i), strlen(p_data) - (MAX_LINE_CHARACTER * i));
        Paint_DrawStringAt(&paint, 0, Y_INIT_POSITION + (Y_SPACE * line_counter), end_line, &Font24, UNCOLORED);
        line_counter++;
    }
    
    EPD_DisplayFrame(&epd, frame_buffer);
}

void dcnd_protocol_acknowledge(uint8_t * p_data)
{
    uint32_t err_code;
    //TODO
#if 0
    do
    {
        uint16_t length = (uint16_t)strlen(p_data);
        err_code = ble_nus_data_send(&m_nus, p_data, &length, m_conn_handle);
        if ((err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_NOT_FOUND))
        {
            APP_ERROR_CHECK(err_code);
        }
    } while (err_code == NRF_ERROR_RESOURCES);
#endif
}


static char imageMsg[15000];
//static uint8_t * imageMsg;
static uint16_t indexImg = 0;
static uint16_t indexError = 0;


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code = NRF_SUCCESS;

        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }

        if(dcnd.state == IDLE_STATE)
        {
            dcnd_protocol_decode(&dcnd, p_evt->params.rx_data.p_data);
            if(dcnd.message_types == START_TRANSFER_MSG )
            {
                NRF_LOG_INFO("RECEIVED_DATA_MSG\n");
                dcnd.state = RECEIVED_DATA_MSG;
                memset (dcnd.message, '\0', MAX_LINE_CHARACTER * MAX_LINE);
                line_counter = 0;

                dcnd_protocol_acknowledge("RECEIVED_DATA_MSG ACKNOWLEDGE\n");
            }
            else if(dcnd.message_types == START_TRANSFER_IMG)
            {
                NRF_LOG_INFO("RECEIVED_DATA_IMG\n");
                dcnd.state = RECEIVED_DATA_IMG;
                //memset (imageMsg, 0x00, 15000);
                //indexImg = 0;
            }
        }
        else if(dcnd.state == RECEIVED_DATA_MSG)
        {
            dcnd_protocol_decode(&dcnd, p_evt->params.rx_data.p_data);
            if(dcnd.message_types == STOP_TRANSFER_MSG)
            {
                NRF_LOG_INFO("IDLE_STATE\n");
                NRF_LOG_INFO("MSG_RECEIVED\n");
                dcnd.state = IDLE_STATE;

                NRF_LOG_HEXDUMP_DEBUG(dcnd.message, strlen(dcnd.message));
                print_character_on_screen(dcnd.message);

                dcnd_protocol_acknowledge("MSG_RECEIVED ACKNOWLEDGE\n");
            }else{
                NRF_LOG_INFO("MSG_RECEIVED_IN_PROGRESS\n");

                char str[MAX_LINE_CHARACTER * MAX_LINE];
                memset(str, '\0', sizeof(str));
                
                //strcpy(str, p_evt->params.rx_data.p_data);
                strncpy(str, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
                
                strcat(dcnd.message, str);
                NRF_LOG_INFO("dcnd.message = %s", dcnd.message);
                NRF_LOG_INFO("str = %s", str);
                dcnd_protocol_acknowledge("MSG_RECEIVED_IN_PROGRESS ACKNOWLEDGE\n");
            } 
        }
        else if(dcnd.state == RECEIVED_DATA_IMG)
        {
            dcnd_protocol_decode(&dcnd, p_evt->params.rx_data.p_data);
            if(dcnd.message_types == STOP_TRANSFER_IMG)
            {
                NRF_LOG_INFO("IDLE_STATE\n");
                NRF_LOG_INFO("IMG_RECEIVED\n");
                dcnd.state = IDLE_STATE;
                indexImg = 0;
                NRF_LOG_INFO("Image msg = %s", imageMsg);
                EPD_DisplayFrame(&epd, imageMsg);
            }else{
                NRF_LOG_INFO("IMG_RECEIVED_IN_PROGRESS\n");

                /*char strTemp[100];
                memset(strTemp, '\0', sizeof(strTemp));

                strncpy(strTemp, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
                strcat(imageMsg, strTemp);*/

                char strTmp[2];
                //memset(strTmp, '\0', sizeof(strTmp));

                uint8_t number;

                NRF_LOG_INFO("indexImg =  %d", indexImg);

                for(int i = 0; i < (p_evt->params.rx_data.length - 1); i++){
                    strTmp[0] = p_evt->params.rx_data.p_data[i];
                    strTmp[1] = p_evt->params.rx_data.p_data[++i];

                    number = (uint8_t)strtol((const char *)&strTmp, NULL, 16);

                    imageMsg[indexImg] = number;

                    /*if(IMG_TEST[indexImg] != imageMsg[indexImg]){
                        NRF_LOG_INFO("Error IMG_TEST[%d] != imageMsg[%d]", indexImg, indexImg);
                        NRF_LOG_INFO("Error         0x%x != 0x%x", IMG_TEST[indexImg], imageMsg[indexImg]);
                        indexError++;
                        if(indexError > 64){
                            APP_ERROR_CHECK(NRF_ERROR_INVALID_DATA);
                        }
                    }*/
        
                    indexImg++;
                }

                NRF_LOG_INFO("Data length = %d", p_evt->params.rx_data.length);
                NRF_LOG_INFO("Data raw = %s", p_evt->params.rx_data.p_data);
                NRF_LOG_INFO("Image strTemp = %s", imageMsg);
            }  
        }
    }
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_bas_init_t     bas_init;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};
    ble_dfu_buttonless_init_t dfus_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize DFU.
    dfus_init.evt_handler = ble_dfu_evt_handler;

    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    bas_init.evt_handler          = on_bas_evt;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec        = SEC_OPEN;
    bas_init.bl_cccd_wr_sec   = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    uint32_t err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            break;
        case BLE_ADV_EVT_IDLE:
            //sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling the Battery Service events.
 *
 * @details This function will be called for all Battery Service events which are passed to the
 |          application.
 *
 * @param[in] p_bas  Battery Service structure.
 * @param[in] p_evt  Event received from the Battery Service.
 */
static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            // Start battery timer
            err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_BAS_EVT_NOTIFICATION_ENABLED

        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            err_code = app_timer_stop(m_battery_timer_id);
            APP_ERROR_CHECK(err_code);
            break; // BLE_BAS_EVT_NOTIFICATION_DISABLED

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}


/*
 * TODO
 */
void output_voltage_setup(void)
{
    // Configure UICR_REGOUT0 register only if it is set to default value.
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
        (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V3 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
    }
}


static bool pin_chg_is_set = false;
static bool pin_pg_is_set = false;
/**@brief TODO
 */
void in_pin_chg_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(nrf_drv_gpiote_in_is_set(pin))
    {
        pin_chg_is_set = true;
        NRF_LOG_INFO("PIN CHG 1 TTL");

        if(!pin_pg_is_set)
        {
            EPD_DisplayFrame(&epd, IMG_BATTERY_FULLY_CHARGE);
            NRF_LOG_INFO("Battery charge finish");
        }
    }
    else
    {
        pin_chg_is_set = false;
        NRF_LOG_INFO("PIN CHG 0 TTL");

        if(!pin_pg_is_set)
        {
            EPD_DisplayFrame(&epd, IMG_BATTERY_CHARGE);
            NRF_LOG_INFO("Battery charge start");
        }
    }
}


/**@brief TODO
 */
void in_pin_pg_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(nrf_drv_gpiote_in_is_set(pin))
    {
        pin_pg_is_set = true;
        NRF_LOG_INFO("PIN PG 1 TTL");
    }
    else
    {
        pin_pg_is_set = false;
        NRF_LOG_INFO("PIN PG 0 TTL");
    }
}


/**@brief TODO
 */
void bq24090_init(void)
{
    ret_code_t err_code;

    // Configure PIN_CHG in input of BQ24090.
    nrf_drv_gpiote_in_config_t pin_chg_in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    pin_chg_in_config.pull = NRF_GPIO_PIN_NOPULL;
    
    err_code = nrf_drv_gpiote_in_init(PIN_CHG, &pin_chg_in_config, in_pin_chg_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_CHG, true);

    // Configure PIN_PG in input of BQ24090.
    nrf_drv_gpiote_in_config_t pin_pg_in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    pin_pg_in_config.pull = NRF_GPIO_PIN_NOPULL;
    
    err_code = nrf_drv_gpiote_in_init(PIN_PG, &pin_pg_in_config, in_pin_pg_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_PG, true);
  
    // Configure PIN_ISET2 in output of BQ24090.
    nrf_drv_gpiote_out_config_t pin_iset2_out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(PIN_ISET2, &pin_iset2_out_config);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_set(PIN_ISET2);
}


/**@brief TODO
 */
void in_pin_clr_screen_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    ret_code_t err_code;

    if(!btn_timer_is_set){
        err_code = app_timer_start(m_push_btn_timer_id, 5000, NULL); //2s 
        APP_ERROR_CHECK(err_code);

        btn_timer_is_set = true;
    }
}


/**@brief TODO
 */
void clear_button_init(void)
{
    ret_code_t err_code;

    // Configure PIN_CLR_SCREEN in input.
    nrf_drv_gpiote_in_config_t pin_clr_screen_in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    pin_clr_screen_in_config.pull = NRF_GPIO_PIN_PULLUP;
    
    err_code = nrf_drv_gpiote_in_init(PIN_CLR_SCREEN, &pin_clr_screen_in_config, in_pin_clr_screen_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_CLR_SCREEN, true);
}


/**@brief Application main function.
 */
int main(void)
{
    uint32_t err_code;
    bool erase_bonds;

    if (NRF_POWER->MAINREGSTATUS &
       (POWER_MAINREGSTATUS_MAINREGSTATUS_High << POWER_MAINREGSTATUS_MAINREGSTATUS_Pos))
    {
        output_voltage_setup();
    }

    // Initialize.
    log_init();

    // Initialize the async SVCI interface to bootloader before any interrupts are enabled.
    err_code = ble_dfu_buttonless_async_svci_init();
    APP_ERROR_CHECK(err_code);

    timers_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    adc_configure();
    services_init();
    advertising_init();
    conn_params_init();

    // Start execution.
    NRF_LOG_INFO("DCND started");
    NRF_LOG_FLUSH();

    advertising_start();

    dcnd_init(&dcnd);

    if (EPD_Init(&epd) != 0) {
      NRF_LOG_INFO("e-Paper init failed");      
      return -1;
    }else{
      NRF_LOG_INFO("e-Paper init sucess");
    }  

    /* Battery Lipo Charger */
    bq24090_init();

    /*TODO*/
    clear_button_init();

    Paint_Init(&paint, frame_buffer, epd.width, epd.height);
    Paint_Clear(&paint, UNCOLORED);

    EPD_DisplayFrame(&epd, IMG_PAS_DE_NOUVEAU_MSG);

    NRF_LOG_INFO("DCND enter in idle state");

    //TEST 1 (Text too big for the screen)
    //print_character_on_screen("Un texte est un ensemble coherent d'enonces qui forme une unite de sens et qui a une intention communicative (le but est de transmettre un message). L'adjectif litteraire, a son tour, est associe a la litterature, soit l'ensemble des expertises visant � lire et � ecrire correctement.");

    //TEST2 (Text 4 lines, 3 full lines and 1 last line with 3 character)
    //print_character_on_screen("Un texte est un ensemble coherent d'enonces qui forme une unite de sens ");

    //TEST3 (Text 4 lines full lines)
    //print_character_on_screen("Un texte est un ensemble coherent d'enonces qui forme une unite de sens et qui a une intenti");

    //TEST (Text 4 full screen message)
    //print_character_on_screen("Un texte est un ensemble coherent d'enonces qui forme une unite de sens et qui a une intention communicative (le but est de transmettre un message). L'adjectif litteraire, a son tour, est associe a la litter");


    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}

/** @} */