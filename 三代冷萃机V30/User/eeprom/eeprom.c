#include "eeprom.h"
#include "iic.h"
#include <stdio.h>
#include <stddef.h>
#include "systick.h"
#include "string.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "EEPROM"

#define EEPROM_BLOCK0_ADDRESS    0xA0
#define BUFFER_SIZE              256
uint16_t eeprom_address;

config_Union config;
record_Union record;
deviceInfo_Union deviceInfo;

/*!
    \brief      I2C read and write functions
    \param[in]  none
    \param[out] none
    \retval     I2C_OK or I2C_FAIL
*/
uint8_t i2c_24c02_test(void)
{
    uint16_t i;
    uint8_t i2c_buffer_write[BUFFER_SIZE];
    uint8_t i2c_buffer_read[BUFFER_SIZE];

    log_v("\r\nAT24C02 writing...\r\n");

    /* initialize i2c_buffer_write */
    for(i = 0; i < BUFFER_SIZE; i++) {
        i2c_buffer_write[i] = i;
        log_v("0x%02X ", i2c_buffer_write[i]);
        if(15 == i % 16) {
            log_v("\r\n");
        }
    }
    /* EEPROM data write */
    eeprom_buffer_write_timeout(i2c_buffer_write, EEP_FIRST_PAGE, BUFFER_SIZE);
    log_v("AT24C02 reading...\r\n");
    /* EEPROM data read */
    eeprom_buffer_read_timeout(i2c_buffer_read, EEP_FIRST_PAGE, BUFFER_SIZE);
    /* compare the read buffer and write buffer */
    for(i = 0; i < BUFFER_SIZE; i++) {
        if(i2c_buffer_read[i] != i2c_buffer_write[i]) {
            log_e("0x%02X ", i2c_buffer_read[i]);
            log_e("Err:data read and write aren't matching.\n\r");
            return I2C_FAIL;
        }
        log_v("%d ", i2c_buffer_read[i]);
        if(15 == i % 16) {
            log_v("\r\n");
        }
    }
    log_v("I2C-AT24C02 test passed!\n\r");
    return I2C_OK;
}

/*!
    \brief      initialize peripherals used by the I2C EEPROM driver
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_eeprom_init(void)
{
    eeprom_address = EEPROM_BLOCK0_ADDRESS;
}

/*!
    \brief      write one byte to the EEPROM and use timeout function
    \param[in]  p_buffer: pointer to the buffer containing the data to be written to the EEPROM
    \param[in]  write_address: EEPROM's internal address to write to
    \param[out] none
    \retval     none
*/
uint8_t eeprom_byte_write_timeout(uint8_t *p_buffer, uint16_t write_address)
{
    uint8_t   state = I2C_START;
    uint16_t  timeout = 0;
    uint8_t   i2c_timeout_flag = 0;

    /* enable acknowledge */
    i2c_ack_config(I2CX, I2C_ACK_ENABLE);
    while(!(i2c_timeout_flag)) {
        switch(state) {
        case I2C_START:
            /* i2c master sends start signal only when the bus is idle */
            while(i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_start_on_bus(I2CX);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            } else {
                timeout = 0;
                state   = I2C_START;
                log_e("i2c bus is busy in WRITE BYTE!\n");
            }
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while((! i2c_flag_get(I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_master_addressing(I2CX, eeprom_address, I2C_TRANSMITTER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends start signal timeout in WRITE BYTE!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while((! i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_flag_clear(I2CX, I2C_FLAG_ADDSEND);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master clears address flag timeout in WRITE BYTE!\n");
            }
            break;
        case I2C_TRANSMIT_DATA:
            /* wait until the transmit data buffer is empty */
            while((! i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2CX, write_address>>8);
                timeout = 0;
                state = I2C_TRANSMIT_DATA2;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends data timeout in WRITE BYTE!\n");
            }
        case I2C_TRANSMIT_DATA2:
            /* wait until the transmit data buffer is empty */
            while((! i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2CX, write_address%256);
                timeout = 0;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends data timeout in WRITE BYTE!\n");
            }


            /* wait until BTC bit is set */
            while((!i2c_flag_get(I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2CX, *p_buffer);
                timeout = 0;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends data timeout in WRITE BYTE!\n");
            }
            /* wait until BTC bit is set */
            while((!i2c_flag_get(I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                state = I2C_STOP;
                timeout = 0;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends data timeout in WRITE BYTE!\n");
            }
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2CX);
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL0(I2CX) & I2C_CTL0_STOP) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends stop signal timeout in WRITE BYTE!\n");
            }
            break;
        default:
            state = I2C_START;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            log_e("i2c master sends start signal in WRITE BYTE.\n");
            break;
        }
    }
    return I2C_END;
}
/*!
    \brief      write more than one byte to the EEPROM with a single write cycle
    \param[in]  p_buffer: pointer to the buffer containing the data to be written to the EEPROM
    \param[in]  write_address: EEPROM's internal address to write to
    \param[in]  number_of_byte: number of bytes to write to the EEPROM
    \param[out] none
    \retval     none
*/
uint8_t eeprom_page_write_timeout(uint8_t *p_buffer, uint16_t write_address, uint8_t number_of_byte)
{
    uint8_t   state = I2C_START;
    uint16_t  timeout = 0;
    uint8_t   i2c_timeout_flag = 0;

    /* enable acknowledge */
    i2c_ack_config(I2CX, I2C_ACK_ENABLE);
    while(!(i2c_timeout_flag)) {
        switch(state) {
        case I2C_START:
            /* i2c master sends start signal only when the bus is idle */
            while(i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_start_on_bus(I2CX);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            } else {
                i2c0_bus_reset();
                timeout = 0;
                state = I2C_START;
                log_e("i2c bus is busy in WRITE!\n");
            }
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while((! i2c_flag_get(I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_master_addressing(I2CX, eeprom_address, I2C_TRANSMITTER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends start signal timeout in WRITE!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while((! i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_flag_clear(I2CX, I2C_FLAG_ADDSEND);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master clears address flag timeout in WRITE!\n");
            }
            break;
        case I2C_TRANSMIT_DATA:
            /* wait until the transmit data buffer is empty */
            while((! i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2CX, write_address>>8);
                timeout = 0;
                state = I2C_TRANSMIT_DATA2;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends EEPROM's internal address timeout in WRITE!\n");
            }
        case I2C_TRANSMIT_DATA2:
            /* wait until the transmit data buffer is empty */
            while((! i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                /* send the EEPROM's internal address to write to : only one byte address */
                i2c_data_transmit(I2CX, write_address%256);
                timeout = 0;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends EEPROM's internal address timeout in WRITE!\n");
            }


            /* wait until BTC bit is set */
            while((!i2c_flag_get(I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                timeout = 0;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends data timeout in WRITE!\n");
            }
            while(number_of_byte--) {
                i2c_data_transmit(I2CX, *p_buffer);
                /* point to the next byte to be written */
                p_buffer++;
                /* wait until BTC bit is set */
                while((!i2c_flag_get(I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    timeout = 0;
                } else {
                    timeout = 0;
                    state = I2C_START;
                    log_e("i2c master sends data timeout in WRITE!\n");
                }
            }
            timeout = 0;
            state = I2C_STOP;
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2CX);
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL0(I2CX) & I2C_CTL0_STOP) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends stop signal timeout in WRITE!\n");
            }
            break;
        default:
            state = I2C_START;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            log_e("i2c master sends start signal in WRITE.\n");
            break;
        }
    }
    return I2C_END;
}

/*!
    \brief      write buffer of data to the EEPROM use timeout function
    \param[in]  p_buffer: pointer to the buffer  containing the data to be written to the EEPROM
    \param[in]  write_address: EEPROM's internal address to write to
    \param[in]  number_of_byte: number of bytes to write to the EEPROM
    \param[out] none
    \retval     none
*/
void eeprom_buffer_write_timeout(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte)
{
    uint8_t number_of_page = 0, number_of_single = 0, count = 0;
    uint16_t address = 0;
    address = write_address % I2C_PAGE_SIZE;
    count = I2C_PAGE_SIZE - address;
    number_of_page = number_of_byte / I2C_PAGE_SIZE;
    number_of_single = number_of_byte % I2C_PAGE_SIZE;

    /* if write_address is I2C_PAGE_SIZE aligned  */
    if(0 == address) {
        while(number_of_page--) {
            eeprom_page_write_timeout(p_buffer, write_address, I2C_PAGE_SIZE);
            eeprom_wait_standby_state_timeout();
            write_address += I2C_PAGE_SIZE;
            p_buffer += I2C_PAGE_SIZE;
        }
        if(0 != number_of_single) {
            eeprom_page_write_timeout(p_buffer, write_address, number_of_single);
            eeprom_wait_standby_state_timeout();
        }
    } else {
        /* if write_address is not I2C_PAGE_SIZE aligned */
        if(number_of_byte < count) {
            eeprom_page_write_timeout(p_buffer, write_address, number_of_byte);
            eeprom_wait_standby_state_timeout();
        } else {
            number_of_byte -= count;
            number_of_page = number_of_byte / I2C_PAGE_SIZE;
            number_of_single = number_of_byte % I2C_PAGE_SIZE;

            if(0 != count) {
                eeprom_page_write_timeout(p_buffer, write_address, count);
                eeprom_wait_standby_state_timeout();
                write_address += count;
                p_buffer += count;
            }
            /* write page */
            while(number_of_page--) {
                eeprom_page_write_timeout(p_buffer, write_address, I2C_PAGE_SIZE);
                eeprom_wait_standby_state_timeout();
                write_address += I2C_PAGE_SIZE;
                p_buffer += I2C_PAGE_SIZE;
            }
            /* write single */
            if(0 != number_of_single) {
                eeprom_page_write_timeout(p_buffer, write_address, number_of_single);
                eeprom_wait_standby_state_timeout();
            }
        }
    }
}

/*!
    \brief      read data from the EEPROM
    \param[in]  p_buffer: pointer to the buffer that receives the data read from the EEPROM
    \param[in]  read_address: EEPROM's internal address to start reading from
    \param[in]  number_of_byte: number of bytes to reads from the EEPROM
    \param[out] none
    \retval     none
*/
uint8_t eeprom_buffer_read_timeout(uint8_t *p_buffer, uint16_t read_address, uint16_t number_of_byte)
{
    uint8_t   state = I2C_START;
    uint8_t   read_cycle = 0;
    uint16_t  timeout = 0;
    uint8_t   i2c_timeout_flag = 0;

    /* enable acknowledge */
    i2c_ack_config(I2CX, I2C_ACK_ENABLE);
    while(!(i2c_timeout_flag)) {
        switch(state) {
        case I2C_START:
            if(RESET == read_cycle) {
                /* i2c master sends start signal only when the bus is idle */
                while(i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    /* whether to send ACK or not for the next byte */
                    if(2 == number_of_byte) {
                        i2c_ackpos_config(I2CX, I2C_ACKPOS_NEXT);
                    }
                } else {
                    i2c0_bus_reset();
                    timeout = 0;
                    state = I2C_START;
                    log_e("i2c bus is busy in READ!\n");
                }
            }
            /* send the start signal */
            i2c_start_on_bus(I2CX);
            timeout = 0;
            state = I2C_SEND_ADDRESS;
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while((! i2c_flag_get(I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                if(RESET == read_cycle) {
                    i2c_master_addressing(I2CX, eeprom_address, I2C_TRANSMITTER);
                    state = I2C_CLEAR_ADDRESS_FLAG;
                } else {
                    i2c_master_addressing(I2CX, eeprom_address, I2C_RECEIVER);
                    if(number_of_byte < 3) {
                        /* disable acknowledge */
                        i2c_ack_config(I2CX, I2C_ACK_DISABLE);
                    }
                    state = I2C_CLEAR_ADDRESS_FLAG;
                }
                timeout = 0;
            } else {
                timeout = 0;
                state = I2C_START;
                read_cycle = 0;
                log_e("i2c master sends start signal timeout in READ!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            /* address flag set means i2c slave sends ACK */
            while((!i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_flag_clear(I2CX, I2C_FLAG_ADDSEND);
                if((SET == read_cycle) && (1 == number_of_byte)) {
                    /* send a stop condition to I2C bus */
                    i2c_stop_on_bus(I2CX);
                }
                timeout = 0;
                state   = I2C_TRANSMIT_DATA;
            } else {
                timeout = 0;
                state   = I2C_START;
                read_cycle = 0;
                log_e("i2c master clears address flag timeout in READ!\n");
            }
            break;
        case I2C_TRANSMIT_DATA:
            if(RESET == read_cycle) {
                /* wait until the transmit data buffer is empty */
                while((! i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    /* send the EEPROM's internal address to write to : only one byte address */
                    i2c_data_transmit(I2CX, read_address>>8);
                    timeout = 0;
                } else {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle = 0;
                    log_e("i2c master wait data buffer is empty timeout in READ!\n");
                }
                while((! i2c_flag_get(I2CX, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    /* send the EEPROM's internal address to write to : only one byte address */
                    i2c_data_transmit(I2CX, read_address%256);
                    timeout = 0;
                } else {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle = 0;
                    log_e("i2c master wait data buffer is empty timeout in READ!\n");
                }

                /* wait until BTC bit is set */
                while((!i2c_flag_get(I2CX, I2C_FLAG_BTC)) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle++;
                } else {
                    timeout = 0;
                    state = I2C_START;
                    read_cycle = 0;
                    log_e("i2c master sends EEPROM's internal address timeout in READ!\n");
                }
            } else {
                while(number_of_byte) {
                    timeout++;
                    if(3 == number_of_byte) {
                        /* wait until BTC bit is set */
                        while(!i2c_flag_get(I2CX, I2C_FLAG_BTC));
                        /* disable acknowledge */
                        i2c_ack_config(I2CX, I2C_ACK_DISABLE);
                    }
                    if(2 == number_of_byte) {
                        /* wait until BTC bit is set */
                        while(!i2c_flag_get(I2CX, I2C_FLAG_BTC));
                        /* send a stop condition to I2C bus */
                        i2c_stop_on_bus(I2CX);
                    }
                    /* wait until RBNE bit is set */
                    if(i2c_flag_get(I2CX, I2C_FLAG_RBNE)) {
                        /* read a byte from the EEPROM */
                        *p_buffer = i2c_data_receive(I2CX);

                        /* point to the next location where the byte read will be saved */
                        p_buffer++;

                        /* decrement the read bytes counter */
                        number_of_byte--;
                        timeout = 0;
                    }
                    if(timeout > I2C_TIME_OUT) {
                        timeout = 0;
                        state = I2C_START;
                        read_cycle = 0;
                        log_e("i2c master sends data timeout in READ!\n");
                    }
                }
                timeout = 0;
                state = I2C_STOP;
            }
            break;
        case I2C_STOP:
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL0(I2CX) & I2C_CTL0_STOP) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                timeout = 0;
                state = I2C_END;
                i2c_timeout_flag = I2C_OK;
            } else {
                timeout = 0;
                state = I2C_START;
                read_cycle = 0;
                log_e("i2c master sends stop signal timeout in READ!\n");
            }
            break;
        default:
            state = I2C_START;
            read_cycle = 0;
            i2c_timeout_flag = I2C_OK;
            timeout = 0;
            log_e("i2c master sends start signal in READ.\n");
            break;
        }
    }
    return I2C_END;
}

/*!
    \brief      wait for EEPROM standby state use timeout function
    \param[in]  none
    \param[out] none
    \retval     none
*/
uint8_t eeprom_wait_standby_state_timeout()
{
    uint8_t   state = I2C_START;
    uint16_t  timeout = 0;
    uint8_t   i2c_timeout_flag = 0;

    while(!(i2c_timeout_flag)) {
        switch(state) {
        case I2C_START:
            /* i2c master sends start signal only when the bus is idle */
            while(i2c_flag_get(I2CX, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_start_on_bus(I2CX);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            } else {
                i2c0_bus_reset();
                timeout = 0;
                state = I2C_START;
                log_e("i2c bus is busy in EEPROM standby!\n");
            }
            break;
        case I2C_SEND_ADDRESS:
            /* i2c master sends START signal successfully */
            while((! i2c_flag_get(I2CX, I2C_FLAG_SBSEND)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_master_addressing(I2CX, eeprom_address, I2C_TRANSMITTER);
                timeout = 0;
                state = I2C_CLEAR_ADDRESS_FLAG;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends start signal timeout in EEPROM standby!\n");
            }
            break;
        case I2C_CLEAR_ADDRESS_FLAG:
            while((!((i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) || (i2c_flag_get(I2CX, I2C_FLAG_AERR)))) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                if(i2c_flag_get(I2CX, I2C_FLAG_ADDSEND)) {
                    i2c_flag_clear(I2CX, I2C_FLAG_ADDSEND);
                    timeout = 0;
                    /* send a stop condition to I2C bus */
                    i2c_stop_on_bus(I2CX);
                    i2c_timeout_flag = I2C_OK;
                    /* exit the function */
                    return I2C_END;
                } else {
                    /* clear the bit of AE */
                    i2c_flag_clear(I2CX, I2C_FLAG_AERR);
                    timeout = 0;
                    state = I2C_STOP;
                }
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master clears address flag timeout in EEPROM standby!\n");
            }
            break;
        case I2C_STOP:
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2CX);
            /* i2c master sends STOP signal successfully */
            while((I2C_CTL0(I2CX) & I2C_CTL0_STOP) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                timeout = 0;
                state = I2C_START;
            } else {
                timeout = 0;
                state = I2C_START;
                log_e("i2c master sends stop signal timeout in EEPROM standby!\n");
            }
            break;
        default:
            state = I2C_START;
            timeout = 0;
            log_e("i2c master sends start signal end in EEPROM standby!.\n");
            break;
        } 
    }
    return I2C_END;

}


/**
 * ************************************************************************
 * @brief 读取保存数据
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void readSaveData(void)
{
    eeprom_buffer_read_timeout(config.bytes, CONFIG_ADDRESS, sizeof(config_TypeDef));   //读取配置信息
    eeprom_buffer_read_timeout(record.bytes, RECORD_ADDRESS, sizeof(record_TypeDef));   //读取记录信息
    eeprom_buffer_read_timeout(deviceInfo.bytes, DEVICE_INFO_ADDRESS, sizeof(deviceInfo_TypeDef));  //读取设备信息
    if(config.data.version != VERSION)
    {
        config.data.version = VERSION;
        config.data.isLock = IS_LOCK;
        config.data.zeroLevel = ZERO_LEVEL;         //零位水位
        config.data.coffeeCompensateLevel = COFFEE_COMPENSATE_LEVEL;
        config.data.teaCompensateLevel = TEA_COMPENSATE_LEVEL;
        config.data.inletOverTime = INLET_OVER_TIME;
        config.data.drainOverTime = DRAIN_OVER_TIME;
        config.data.circulationValveSwicthTime  = CIRCULATION_VALVE_SWICTH_TIME;
        config.data.drainValveSwicthTime = DRAIN_VALVE_SWICTH_TIME;
        config.data.drainDelayTime = DRAIN_DELAY_TIME;
        config.data.washDrainRepeat = WASH_DRAIN_REPEAT;
        config.data.washSingleTime = WASH_SINGLE_TIME;
        config.data.washPauseTime = WASH_PAUSE_TIME;
        config.data.washLoopTimes = WASH_LOOP_TIMES;
        config.data.washFirstVolume = WASH_FIRST_VOLUME;
        config.data.washSecondVolume = WASH_SECOND_VOLUME;
        config.data.washTime = WASH_TIME;
        config.data.waterVolumeLow = WATER_VOLUME_LOW;
        config.data.waterVolumeHigh = WATER_VOLUME_HIGH;
        config.data.waterChangeVal = WATER_CHANGE_VAL;
        config.data.makeChangeTime = TIME_CHANGE_VAL;
        config.data.weightMin = WEIGHT_MIN;
        config.data.weightMax = WEIGHT_MAX;
        config.data.weightChangeVal = WEIGHT_CHANGE_VAL;
        config.data.drainTime = DRAIN_TIME;
        for(uint8_t i = 0;i<NUMBER_OF_PARAMETERS;i++)
        {
            config.data.coffeeMake.vol[i] = COFFEE_MAKE_VOLUME;
            config.data.coffeeMake.time[i] = COFFEE_MAKE_TIME;
            config.data.coffeeMake.weight[i] = COFFEE_MAKE_WEIGHT;
            config.data.coffeeMake.autoDrainangeFlag[i] = COFFEE_MAKE_AUTO_DRAINANGE_FLAG;
            config.data.teaMake.vol[i] = TEA_MAKE_VOLUME;
            config.data.teaMake.time[i] = TEA_MAKE_TIME;
            config.data.teaMake.weight[i] = TEA_MAKE_WEIGHT;
            config.data.teaMake.autoDrainangeFlag[i] = TEA_MAKE_AUTO_DRAINANGE_FLAG;
        }
        for(uint8_t i = 0; i < NUMBER_OF_COLLECTIONS; i++)
        {
            config.data.coffeeMake.collectFlag[i] = COFFEE_MAKE_COLLECT_FLAG;
            config.data.teaMake.collectFlag[i] = TEA_MAKE_COLLECT_FLAG;
        }
        config.data.coffeeMake.currentNumber = COFFEE_MAKE_CURRENT_NUMBER;
        config.data.teaMake.currentNumber = TEA_MAKE_CURRENT_NUMBER;

        config.data.waterTap.time[0] = 5;
        config.data.waterTap.time[1] = 10;
        config.data.waterTap.time[2] = 15;
        config.data.waterTap.pumpRatio = 5.0f;
        writeConfigData();
    
        if(record.data.coffeeMakeCnt == 0xFFFFFFFF)
        {
            record.data.coffeeMakeCnt = 0;
            record.data.teaMakeCnt = 0;
            record.data.washCnt = 0;
            record.data.sanitCnt = 0;
            record.data.circulationValveCnt = 0;
            record.data.drainValveCnt = 0;
            record.data.circulationPumpRunTime = 0;
            record.data.waterInletValveCnt = 0;
            record.data.washValveCnt = 0;
            record.data.waterTapwashCnt = 0;
            record.data.DCPumpRunTime = 0;
            writeRecordData();
        }
        deviceInfo.data.MQTT.isRegistered = 0;
        deviceInfo.data.HardwareVersion = HARDWARE_VERSION;
        deviceInfo.data.FirmwareVersion = SOFTWARE_VERSION;
        strncpy(deviceInfo.data.MQTT.ESN,deviceInfo.data.DeviceID, sizeof(deviceInfo.data.DeviceID));
        strncpy(deviceInfo.data.MQTT.ETID,ETID_,sizeof(ETID_));
        initMQTTTopics();
        strncpy(deviceInfo.data.MQTT.serverAddress, MQTT_ADDR, sizeof(MQTT_ADDR));
        strncpy(deviceInfo.data.MQTT.registerAddress, REGISTER_ADDR, sizeof(REGISTER_ADDR));
        write_device_info_data(DEVICE_INFO_MQTT);
    }
}

/**
 * ************************************************************************
 * @brief 写入配置参数
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void writeConfigData(void)
{
    eeprom_buffer_write_timeout(config.bytes, CONFIG_ADDRESS, sizeof(config_TypeDef));
}

/**
 * ************************************************************************
 * @brief 写入记录数据
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void writeRecordData(void)
{
    eeprom_buffer_write_timeout(record.bytes, RECORD_ADDRESS, sizeof(record_TypeDef));
}






/**
 * ************************************************************************
 * @brief 配置数据写入函数
 * 
 * @param[in] part  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void write_config_data(config_DataPart part)
{
    switch (part) {
        case CONFIG_VERSION:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.version, offsetof(config_TypeDef, version), sizeof(config.data.version));
            break;
        case CONFIG_IS_LOCK:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.isLock, offsetof(config_TypeDef, isLock), sizeof(config.data.isLock));
            break;
        case CONFIG_ZERO_LEVEL:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.zeroLevel, offsetof(config_TypeDef, zeroLevel), sizeof(config.data.zeroLevel));
            break;
        case CONFIG_COFFEE_COMPENSATE_LEVEL:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.coffeeCompensateLevel, offsetof(config_TypeDef, coffeeCompensateLevel), sizeof(config.data.coffeeCompensateLevel));
            break;
        case CONFIG_TEA_COMPENSATE_LEVEL:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.teaCompensateLevel, offsetof(config_TypeDef, teaCompensateLevel), sizeof(config.data.teaCompensateLevel));
            break;
        case CONFIG_INLET_OVER_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.inletOverTime, offsetof(config_TypeDef, inletOverTime), sizeof(config.data.inletOverTime));
            break;
        case CONFIG_DRAIN_OVER_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.drainOverTime, offsetof(config_TypeDef, drainOverTime), sizeof(config.data.drainOverTime));
            break;
        case CONFIG_CIRCULATION_VALVE_SWITCH_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.circulationValveSwicthTime, offsetof(config_TypeDef, circulationValveSwicthTime), sizeof(config.data.circulationValveSwicthTime));
            break;
        case CONFIG_DRAIN_VALVE_SWITCH_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.drainValveSwicthTime, offsetof(config_TypeDef, drainValveSwicthTime), sizeof(config.data.drainValveSwicthTime));
            break;
        case CONFIG_DRAIN_DELAY_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.drainDelayTime, offsetof(config_TypeDef, drainDelayTime), sizeof(config.data.drainDelayTime));
            break;
        case CONFIG_WASH_DRAIN_REPEAT:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washDrainRepeat, offsetof(config_TypeDef, washDrainRepeat), sizeof(config.data.washDrainRepeat));
            break;
        case CONFIG_WASH_SINGLE_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washSingleTime, offsetof(config_TypeDef, washSingleTime), sizeof(config.data.washSingleTime));
            break;
        case CONFIG_WASH_PAUSE_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washPauseTime, offsetof(config_TypeDef, washPauseTime), sizeof(config.data.washPauseTime));
            break;
        case CONFIG_WASH_LOOP_TIMES:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washLoopTimes, offsetof(config_TypeDef, washLoopTimes), sizeof(config.data.washLoopTimes));
            break;
        case CONFIG_WASH_FIRST_VOLUME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washFirstVolume, offsetof(config_TypeDef, washFirstVolume), sizeof(config.data.washFirstVolume));
            break;
        case CONFIG_WASH_SECOND_VOLUME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washSecondVolume, offsetof(config_TypeDef, washSecondVolume), sizeof(config.data.washSecondVolume));
            break;
        case CONFIG_WASH_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.washTime, offsetof(config_TypeDef, washTime), sizeof(config.data.washTime));
            break;
        case CONFIG_COFFEE_MAKE:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.coffeeMake, offsetof(config_TypeDef, coffeeMake), sizeof(config.data.coffeeMake));
            break;
        case CONFIG_TEA_MAKE:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.teaMake, offsetof(config_TypeDef, teaMake), sizeof(config.data.teaMake));
            break;
        case CONFIG_WATER_TAP:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.waterTap, offsetof(config_TypeDef, waterTap), sizeof(config.data.waterTap));
            break;
        // 新添加的参数
        case CONFIG_WATER_VOLUME_LOW:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.waterVolumeLow, offsetof(config_TypeDef, waterVolumeLow), sizeof(config.data.waterVolumeLow));
            break;
        case CONFIG_WATER_VOLUME_HIGH:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.waterVolumeHigh, offsetof(config_TypeDef, waterVolumeHigh), sizeof(config.data.waterVolumeHigh));
            break;
        case CONFIG_WATER_CHANGE_VAL:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.waterChangeVal, offsetof(config_TypeDef, waterChangeVal), sizeof(config.data.waterChangeVal));
            break;
        case CONFIG_WEIGHT_MIN:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.weightMin, offsetof(config_TypeDef, weightMin), sizeof(config.data.weightMin));
            break;
        case CONFIG_WEIGHT_MAX:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.weightMax, offsetof(config_TypeDef, weightMax), sizeof(config.data.weightMax));
            break;
        case CONFIG_WEIGHT_CHANGE_VAL:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.weightChangeVal, offsetof(config_TypeDef, weightChangeVal), sizeof(config.data.weightChangeVal));
            break;
        case CONFIG_MAKE_CHANGE_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&config.data.makeChangeTime, offsetof(config_TypeDef, makeChangeTime), sizeof(config.data.makeChangeTime));
            break;
        case CONFIG_ALL:
            eeprom_buffer_write_timeout(config.bytes, 0, sizeof(config_TypeDef));
            break;
        default:
            // 处理无效的数据部分
            break;
    }
}


/**
 * ************************************************************************
 * @brief 记录数据写入函数
 * 
 * @param[in] part  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void write_record_data(record_DataPart part)
{
    
    size_t record_offset = RECORD_ADDRESS;
    
    switch (part) {
        case RECORD_COFFEE_MAKE_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.coffeeMakeCnt, 
                                       record_offset + offsetof(record_TypeDef, coffeeMakeCnt), 
                                       sizeof(record.data.coffeeMakeCnt));
            break;
        case RECORD_TEA_MAKE_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.teaMakeCnt, 
                                       record_offset + offsetof(record_TypeDef, teaMakeCnt), 
                                       sizeof(record.data.teaMakeCnt));
            break;
        case RECORD_WASH_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.washCnt, 
                                       record_offset + offsetof(record_TypeDef, washCnt), 
                                       sizeof(record.data.washCnt));
            break;
        case RECORD_SANIT_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.sanitCnt, 
                                       record_offset + offsetof(record_TypeDef, sanitCnt), 
                                       sizeof(record.data.sanitCnt));
            break;
        case RECORD_CIRCULATION_VALVE_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.circulationValveCnt, 
                                       record_offset + offsetof(record_TypeDef, circulationValveCnt), 
                                       sizeof(record.data.circulationValveCnt));
            break;
        case RECORD_DRAIN_VALVE_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.drainValveCnt, 
                                       record_offset + offsetof(record_TypeDef, drainValveCnt), 
                                       sizeof(record.data.drainValveCnt));
            break;
        case RECORD_CIRCULATION_PUMP_RUN_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.circulationPumpRunTime, 
                                       record_offset + offsetof(record_TypeDef, circulationPumpRunTime), 
                                       sizeof(record.data.circulationPumpRunTime));
            break;
        case RECORD_WATER_INLET_VALVE_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.waterInletValveCnt, 
                                       record_offset + offsetof(record_TypeDef, waterInletValveCnt), 
                                       sizeof(record.data.waterInletValveCnt));
            break;
        case RECORD_WASH_VALVE_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.washValveCnt, 
                                       record_offset + offsetof(record_TypeDef, washValveCnt), 
                                       sizeof(record.data.washValveCnt));
            break;
        case RECORD_WATER_TAP_WASH_CNT:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.waterTapwashCnt, 
                                       record_offset + offsetof(record_TypeDef, waterTapwashCnt), 
                                       sizeof(record.data.waterTapwashCnt));
            break;
        case RECORD_DC_PUMP_RUN_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.DCPumpRunTime, 
                                       record_offset + offsetof(record_TypeDef, DCPumpRunTime), 
                                       sizeof(record.data.DCPumpRunTime));
            break;
        case RECORD_COOLING_RUN_TIME:
            eeprom_buffer_write_timeout((uint8_t*)&record.data.coolingRunTime, 
                                       record_offset + offsetof(record_TypeDef, coolingRunTime), 
                                       sizeof(record.data.coolingRunTime));
            break;
        case RECORD_ALL:
            eeprom_buffer_write_timeout(record.bytes, record_offset, sizeof(record_TypeDef));
            break;
        default:
            // 处理无效的数据部分
            break;
    }
}

/**
 * ************************************************************************
 * @brief 设备信息数据写入函数
 *
 * @param[in] part  设备信息数据部分标识符
 *
 * @note 该函数用于将设备信息数据写入EEPROM中。
 *       根据传入的part参数，选择性地写入设备信息的不同部分。
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void write_device_info_data(deviceInfo_DataPart part)
{
    size_t device_info_offset = DEVICE_INFO_ADDRESS;
    switch (part) {
        case DEVICE_INFO_HARDWARE_VERSION:
            eeprom_buffer_write_timeout((uint8_t*)&deviceInfo.data.HardwareVersion, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, HardwareVersion), 
                                       sizeof(deviceInfo.data.HardwareVersion));
            break;
        case DEVICE_INFO_FIRMWARE_VERSION:
            eeprom_buffer_write_timeout((uint8_t*)&deviceInfo.data.FirmwareVersion, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, FirmwareVersion), 
                                       sizeof(deviceInfo.data.FirmwareVersion));
            break;
        case DEVICE_INFO_DEVICE_ID:
            eeprom_buffer_write_timeout((uint8_t*)&deviceInfo.data.DeviceID, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, DeviceID), 
                                       sizeof(deviceInfo.data.DeviceID));
            break;
        case DEVICE_INFO_DATE_OF_MANUFACTURE:
            eeprom_buffer_write_timeout((uint8_t*)&deviceInfo.data.DateOfManufacture, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, DateOfManufacture), 
                                       sizeof(deviceInfo.data.DateOfManufacture));
            break;
        case DEVICE_INFO_MQTT:
            eeprom_buffer_write_timeout((uint8_t*)&deviceInfo.data.MQTT, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, MQTT), 
                                       sizeof(deviceInfo.data.MQTT));
            break;
        case DEVICE_INFO_ALL:
            eeprom_buffer_write_timeout(deviceInfo.bytes, device_info_offset, sizeof(deviceInfo_TypeDef));
            break;
        default:
            // 处理无效的数据部分
            break;
    }
}


/**
 * ************************************************************************
 * @brief 设备信息数据读取函数
 *
 * @param[in] part  设备信息数据部分标识符
 *
 * @note 该函数用于从EEPROM中读取设备信息数据。
 *       根据传入的part参数，选择性地读取设备信息的不同部分。
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void read_device_info_data(deviceInfo_DataPart part)
{
    size_t device_info_offset = DEVICE_INFO_ADDRESS;
    switch (part) {
        case DEVICE_INFO_HARDWARE_VERSION:
            eeprom_buffer_read_timeout((uint8_t*)&deviceInfo.data.HardwareVersion, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, HardwareVersion), 
                                       sizeof(deviceInfo.data.HardwareVersion));
            break;
        case DEVICE_INFO_FIRMWARE_VERSION:
            eeprom_buffer_read_timeout((uint8_t*)&deviceInfo.data.FirmwareVersion, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, FirmwareVersion), 
                                       sizeof(deviceInfo.data.FirmwareVersion));
            break;
        case DEVICE_INFO_DEVICE_ID:
            eeprom_buffer_read_timeout((uint8_t*)&deviceInfo.data.DeviceID, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, DeviceID), 
                                       sizeof(deviceInfo.data.DeviceID));
            break;
        case DEVICE_INFO_DATE_OF_MANUFACTURE:
            eeprom_buffer_read_timeout((uint8_t*)&deviceInfo.data.DateOfManufacture, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, DateOfManufacture), 
                                       sizeof(deviceInfo.data.DateOfManufacture));
            break;
        case DEVICE_INFO_MQTT:
            eeprom_buffer_read_timeout((uint8_t*)&deviceInfo.data.MQTT, 
                                       device_info_offset + offsetof(deviceInfo_TypeDef, MQTT), 
                                       sizeof(deviceInfo.data.MQTT));
            break;
        case DEVICE_INFO_ALL:
            eeprom_buffer_read_timeout(deviceInfo.bytes, device_info_offset, sizeof(deviceInfo_TypeDef));
            break;
        default:
            // 处理无效的数据部分
            break;
    }
}

