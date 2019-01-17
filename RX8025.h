/*
 * RX8025.h
 *
 *  Created on: Feb 2018
 *      Author: raulMrello
 *
 *	RX8025 es el driver de control del chip RTC RX8025, que utiliza un canal I2C. Implementa el interfaz RealTimeClock
 *	para leer y escribir la hora del chip RTC.
 */
 
#ifndef RX8025_H
#define RX8025_H
 
#include "mbed.h"
#include "RealTimeClock.h"
 

class RX8025 : public RealTimeClock {
public:
 
    /** Configure data pin (with other devices on I2C line)
      * @param data SDA and SCL pins
      */
    RX8025(PinName p_sda, PinName p_scl, bool defdbg=false);
 
//    /** Configure data pin (with other devices on I2C line)
//      * @param I2C previous definition
//      */
//    RX8025(I2C& p_i2c);

    /** Configure data pin (with other devices on I2C line)
      * @param I2C previous definition
      */
    RX8025(I2C* p_i2c, bool defdbg=false);


    /** Set I2C clock frequency
      * @param freq.
      * @return none
      */
    void frequency(int hz){
        _i2c.frequency(hz);
    }


	/** (RealTimeClock) Lee la hora actual con formato standard
     *  @param t Recibe la hora en formato 'struct tm'
     *  @return Código de error o 0.
     */
    virtual int getTime(tm* t);


	/** (RealTimeClock) Actualiza la hora del RTC
     *  @param t Hora para actualizar el RTC
     *  @return Código de error o 0.
     */
    virtual int setTime(tm &t);


    /** Read one byte from specific register
      * @param register address
      * @param pdata Dato leído del registro
      * @return Código de error <0
      */
    int8_t read_reg_byte(uint8_t reg, char *pdata);


    /** Write one byte into specific register
      * @param register address, data
      * @return Código de error <0
      */
    int8_t write_reg_byte(uint8_t reg, char data);

 
private:
    I2C *_i2c_p;
    I2C &_i2c;
    uint8_t RX8025_addr;

    struct rtc_time {    // BCD format
        uint8_t rtc_seconds;
        uint8_t rtc_minutes;
        uint8_t rtc_hours;
        uint8_t rtc_weekday;
        uint8_t rtc_date;
        uint8_t rtc_month;
        uint8_t rtc_year_raw;
        uint16_t rtc_year;
    };

    /** Inicializa el chip RTC
     *
     * @return Código de error
     */
    int8_t init(void);


    /** Convierte un valor a BCD
     *
     * @param data Valor a convertir
     * @return Valor BCD
     */
    uint8_t bin2bcd(uint8_t data);


    /** Convierte un valor a binario
     *
     * @param bcd Valor BCD a convertir
     * @return Valor convertido a binario
     */
    uint8_t bcd2bin(uint8_t bcd);


    /** Escribe la alarma tipo D
     *
     * @param time Hora de la alarma
     * @return Código de error (-1) o 0
     */
    int8_t set_alarmD_reg (uint16_t time);


    /** Set Alarm-D / INTA time
      * @param next time (unit: minutes) from now on minimum = 2 minuteï½“!!
      * @return error code or 0
      */
    int8_t set_next_alarmD_INTA(uint16_t time);


    /** Clear Alarm-D / INTA interrupt
      * @param none
      * @return error code or 0
      */
    int8_t clear_alarmD_INTA(void);


    /** Lee el calendario en formato bcd
     *
     * @param tm Resultado obtenido
     * @return Código de error
     */
    int8_t read_rtc_direct (rtc_time *tm);

    /** Escribe el calendario en formato bcd
     *
     * @param tm Calendario a escribir
     * @return Código de error
     */
    int8_t write_rtc_direct (rtc_time *tm);
};
 
#endif      // RX8025_H
