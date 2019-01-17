/*
 * RX8025.cpp
 *
 *  Created on: Feb 2018
 *      Author: raulMrello
 *
 */
 
#include "RX8025.h"
//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

// RTC EPSON RX8025
//  7bit address = 0b0110010(No other choice)
#define RX8025ADDR  			(0x32 << 1)

#define RTC_Wk_Sunday          ((uint8_t)0x00)
#define RTC_Wk_Monday          ((uint8_t)0x01)
#define RTC_Wk_Tuesday         ((uint8_t)0x02)
#define RTC_Wk_Wednesday       ((uint8_t)0x03)
#define RTC_Wk_Thursday        ((uint8_t)0x04)
#define RTC_Wk_Friday          ((uint8_t)0x05)
#define RTC_Wk_Saturday        ((uint8_t)0x06)

// Register definition
#define RX8025_REG_SEC         0x0
#define RX8025_REG_MIN         0x1
#define RX8025_REG_HOUR        0x2
#define RX8025_REG_WDAY        0x3
#define RX8025_REG_DAY         0x4
#define RX8025_REG_MON         0x5
#define RX8025_REG_YEAR        0x6
#define RX8025_REG_OFFSET      0x7
#define RX8025_REG_ALARMW_MIN  0x8
#define RX8025_REG_ALARMW_HOUR 0x9
#define RX8025_REG_ALARMW_WDAY 0xa
#define RX8025_REG_ALARMD_MIN  0xb
#define RX8025_REG_ALARMD_HOUR 0xc
#define RX8025_REG_RESERVED    0xd
#define RX8025_REG_CONTL1      0xe
#define RX8025_REG_CONTL2      0xf


/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */
static const char* _MODULE_ = "[RX8025]........";
#define _EXPR_	(_defdbg && !IS_ISR())


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
RX8025::RX8025 (PinName p_sda, PinName p_scl, bool defdbg) : RealTimeClock(defdbg), _i2c_p(new I2C(p_sda, p_scl)), _i2c(*_i2c_p) {
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando driver RX8025");
    RX8025_addr = RX8025ADDR;
    _ready = (init() != -1)? true : false;
}


//------------------------------------------------------------------------------------
RX8025::RX8025 (I2C* p_i2c, bool defdbg) : RealTimeClock(defdbg), _i2c_p(p_i2c), _i2c(*_i2c_p){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando driver RX8025");
    RX8025_addr = RX8025ADDR;
    _ready = (init() != -1)? true : false;
}


//------------------------------------------------------------------------------------
int8_t RX8025::init(){
    rtc_time time;
    int8_t dt;
    char data = 0;

    if((dt = read_reg_byte(RX8025_REG_CONTL2, &data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL2");
    	return dt;
    }
    if (data & 0x10){   // Power on reset
    	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Detectado flag PON, borrandolo...");
        if((dt = write_reg_byte(RX8025_REG_CONTL2, 0)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WRT registro RX8025_REG_CONTL2");
        	return dt;
        }
        // Set 24H
        if((dt = read_reg_byte(RX8025_REG_CONTL1, &data)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL1");
        	return dt;
        }
        data |= 0x20;
        if((dt = write_reg_byte(RX8025_REG_CONTL1, data)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WRT registro RX8025_REG_CONTL1");
        	return dt;
        }

        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Ajusta modo 24h");
        // set January 1st,2018 0am as a default
        time.rtc_hours = 0;
		time.rtc_minutes = 0;
		time.rtc_seconds = 0;
		time.rtc_date = 1;
		time.rtc_month = 1;
		time.rtc_year_raw = 18;
		time.rtc_weekday = RTC_Wk_Monday;
        if((dt = write_rtc_direct(&time)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WRT write_rtc_direct");
        	return dt;
        }
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Ajustado calendario a 1-Ene-2018, 00:00:00");
        return 0;
    }

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "No se detecta flag PON");
	// Set 24H
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Ajusta modo 24h");
	if((dt = read_reg_byte(RX8025_REG_CONTL1, &data)) < 0){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL1");
		return dt;
	}
	data |= 0x20;
	if((dt = write_reg_byte(RX8025_REG_CONTL1, data)) < 0){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL1");
		return dt;
	}
	return 0;
}


//------------------------------------------------------------------------------------
int RX8025::getTime(tm* t){
    rtc_time time;
    int8_t dt;
    if((dt = read_rtc_direct(&time)) < 0){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "ERR_RD read_rtc_direct");
		return dt;
    }

    DEBUG_TRACE_D(_EXPR_, _MODULE_, "Leyendo calendario: %d:%d:%d, %d-%d-%d diasem=%d",
    					time.rtc_hours,
						time.rtc_minutes,
						time.rtc_seconds,
						time.rtc_date,
						time.rtc_month,
						time.rtc_year_raw,
						time.rtc_weekday);
    // convierte a stuct tm
    t->tm_sec  = time.rtc_seconds;
    t->tm_min  = time.rtc_minutes;
    t->tm_hour = time.rtc_hours;
    t->tm_mday = time.rtc_date;
    t->tm_wday = time.rtc_weekday;
    if ( time.rtc_weekday == RTC_Wk_Sunday) {
        t->tm_wday = 0; // Sun is not 7 but 0
    }
    t->tm_mon  = time.rtc_month - 1;
    t->tm_year = time.rtc_year_raw + 100;
    t->tm_isdst= 0;
    return 0;
}


//------------------------------------------------------------------------------------
int RX8025::setTime(tm &t){
    rtc_time time;
    int8_t dt;
 
    // convierte a struct rtc_time
    time.rtc_seconds  = t.tm_sec;
    time.rtc_minutes  = t.tm_min;
    time.rtc_hours    = t.tm_hour;
    time.rtc_date     = t.tm_mday;
    time.rtc_weekday = t.tm_wday;
    if ( t.tm_wday == 0) {
        time.rtc_weekday = RTC_Wk_Sunday;
    }
    time.rtc_month    = t.tm_mon + 1;
    time.rtc_year_raw = t.tm_year - 100;
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "Escribiendo calendario: %d:%d:%d, %d-%d-%d diasem=%d",
    					time.rtc_hours,
						time.rtc_minutes,
						time.rtc_seconds,
						time.rtc_date,
						time.rtc_month,
						time.rtc_year_raw,
						time.rtc_weekday);
    if((dt = write_rtc_direct(&time)) < 0){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WR write_rtc_direct");
		return dt;
    }
    return 0;
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
int8_t RX8025::read_reg_byte(uint8_t reg, char* pdata){
	char data = 0;
	const char creg = ((reg<<4) & 0xf0);
    if(_i2c.write(RX8025_addr, &creg, 1, true) != 0){
    	return -1;
    }
    if(_i2c.read(RX8025_addr, &data, 1, false) != 0){
    	return -1;
    }
    *pdata = data;
    return 0;
}


//------------------------------------------------------------------------------------
int8_t RX8025::write_reg_byte(uint8_t reg, char data){
	char buf[] = {(char)((uint8_t)(reg<<4) & 0xf0), data};
    if(_i2c.write(RX8025_addr, buf, 2, false) != 0){
    	return -1;
    }
    return read_reg_byte(((reg<<4) & 0xf0), &buf[1]);
}


//------------------------------------------------------------------------------------
int8_t RX8025::set_alarmD_reg (uint16_t time){
    rtc_time t;
    int8_t dt;
    char data = 0;
    uint8_t m, h;
    uint16_t set, real;
 
    if((dt = read_reg_byte(RX8025_REG_CONTL1, &data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL1");
    	return dt;
    }

    data &= ~0x40;        // DALE = 0
    if((dt = write_reg_byte(RX8025_REG_CONTL1, data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WR registro RX8025_REG_CONTL1");
    	return dt;

    }

    if((dt = read_rtc_direct(&t)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD read_rtc_direct");
    	return dt;
    }

    real = t.rtc_hours * 60 + t.rtc_minutes;
    set = real + time;
    m = t.rtc_minutes + (uint8_t)(time % 60);
    h = t.rtc_hours;
    if (m >= 60){
        m -= 60;
        h += 1;
    }
    h += (uint8_t)(time / 60);
    if (h >= 24){
        h -= 24;
    }
    char buf[] = {((RX8025_REG_ALARMD_MIN<<4) & 0xf0), bin2bcd(m), bin2bcd(h)};
    if(_i2c.write(RX8025_addr, buf, 3, false) != 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_I2C_WR");
    	return -1;
    }

    if((dt = read_reg_byte(RX8025_REG_CONTL1, &data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL1");
    	return dt;
    }

    data |= 0x40;         // DALE = 1
    if((dt = write_reg_byte(RX8025_REG_CONTL1, data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WR registro RX8025_REG_CONTL1");
    	return dt;
    }

    if((dt = read_reg_byte(RX8025_REG_ALARMD_MIN, &data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_ALARMD_MIN");
    	return dt;

    }
    real = bcd2bin(data);

    if((dt = read_reg_byte(RX8025_REG_ALARMD_HOUR, &data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_ALARMD_HOUR");
    	return dt;

    }
    real += (bcd2bin(data) * 60);
    if (set == real) {
        if((dt = read_rtc_direct(&t)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD calendario struct tm");
        	return dt;
        }
        real = t.rtc_hours * 60 + t.rtc_minutes;
        if (set > real){
            return 1;
        }
        return 0;
    }
    return 0;
}


//------------------------------------------------------------------------------------
int8_t RX8025::set_next_alarmD_INTA (uint16_t time){
    uint8_t ret;
    uint16_t t;
 
    if (time < 2){
        // Alarm does not check seconds digit.
        // If 59 to 0 is occured during setting here, 1 minute will have a trouble.
        t = 2;
    }
    else if (time > 1440){   // Less than 24 hours
        t = 1440;
    }
    else {
        t = time;
    }
    do{
        ret = set_alarmD_reg(t);
    } while(ret == 0);
    return 0;
}


//------------------------------------------------------------------------------------
int8_t RX8025::clear_alarmD_INTA (){
    int8_t dt;
    char data = 0;
    if((dt = read_reg_byte(RX8025_REG_CONTL2, &data)) < 0){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL2");
    	return dt;
    }
    do { // make sure to set Hi-imp state
        data = data & 0xfe;
        if((dt = write_reg_byte(RX8025_REG_CONTL1, data)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_WR registro RX8025_REG_CONTL1");
			return dt;
        }
        if((dt = read_reg_byte(RX8025_REG_CONTL2, &data)) < 0){
        	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_RD registro RX8025_REG_CONTL2");
        	return dt;
        }
    } while (data & 0x01);
    return 0;
}

//------------------------------------------------------------------------------------
uint8_t RX8025::bin2bcd (uint8_t dt){
    uint8_t bcdhigh = 0;
 
    while (dt >= 10) {
        bcdhigh++;
        dt -= 10;
    }
    return  ((uint8_t)(bcdhigh << 4) | dt);
}


//------------------------------------------------------------------------------------
uint8_t RX8025::bcd2bin (uint8_t dt){
    uint8_t tmp = 0;
 
    tmp = ((uint8_t)(dt & (uint8_t)0xf0) >> (uint8_t)0x4) * 10;
    return (tmp + (dt & (uint8_t)0x0f));
}


//------------------------------------------------------------------------------------
int8_t RX8025::read_rtc_direct (rtc_time *tm){
    char buf[8];
    if(_i2c.read(RX8025_addr, buf, 8) == -1){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_I2C_RD registros 0 al 6");
    	return -1;
    }
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "Leído buffer [");
    for(int i=0;i<8;i++){
    	DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d", buf[i]);
    }
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "]");

    tm->rtc_seconds = bcd2bin(buf[RX8025_REG_SEC+1]  & 0x7f);
    tm->rtc_minutes = bcd2bin(buf[RX8025_REG_MIN+1]  & 0x7f);
    tm->rtc_hours   = bcd2bin(buf[RX8025_REG_HOUR+1] & 0x3f);
    tm->rtc_weekday = buf[RX8025_REG_WDAY+1] & 0x07;
    tm->rtc_date    = bcd2bin(buf[RX8025_REG_DAY+1]  & 0x3f);
    tm->rtc_month   = bcd2bin(buf[RX8025_REG_MON+1]  & 0x1f);
    tm->rtc_year_raw= bcd2bin(buf[RX8025_REG_YEAR+1]);
    tm->rtc_year = tm->rtc_year_raw + 100 + 1900;
    return 0;
}


//------------------------------------------------------------------------------------
int8_t RX8025::write_rtc_direct (rtc_time *tm){
	char buf[8];
    buf[RX8025_REG_YEAR + 1] = bin2bcd(tm->rtc_year_raw);
    buf[RX8025_REG_MON  + 1] = bin2bcd(tm->rtc_month);
    buf[RX8025_REG_DAY  + 1] = bin2bcd(tm->rtc_date);
    buf[RX8025_REG_WDAY + 1] = (tm->rtc_weekday & 0x07);
    buf[RX8025_REG_HOUR + 1] = bin2bcd(tm->rtc_hours);
    buf[RX8025_REG_MIN  + 1] = bin2bcd(tm->rtc_minutes);
    buf[RX8025_REG_SEC  + 1] = bin2bcd(tm->rtc_seconds);
    buf[0] = ((RX8025_REG_SEC<<4) & 0xf0);
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "Escribiendo buffer [");
    for(int i=0;i<8;i++){
    	DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d", buf[i]);
    }
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "]");
    return _i2c.write(RX8025_addr, buf, 8, false);
}
 
