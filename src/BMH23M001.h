/*************************************************
File:       			BMH23M001.h
Author:           BESTMODULES
Description:      Define classes and required variables
History：
V1.0.1	 -- initial version；2023-01-30；Arduino IDE : v1.8.13

**************************************************/

#ifndef _BMH23M001_H__
#define _BMH23M001_H__

#include <Arduino.h>
#include <Wire.h>

class BMH23M001
{
  public:
    BMH23M001(TwoWire *theWire = &Wire);
    void begin(uint8_t i2c_addr = 0x68);
    void beginADC();
    long readADCData();
    void endADC();
    void powerDown();
    void wakeup();
    void writeReg(uint8_t regAdd, uint8_t regData);
    void readReg(uint8_t rbuf[],uint8_t regAdd);
    bool setADConverter ( uint8_t sampleRate ,uint8_t reference);
    bool setDecset(uint8_t decset);
    bool setGain(uint8_t gain,uint8_t vrefGain);
    bool setChannel(uint8_t channel);
    bool setDebounce(uint8_t I2CDebounce);  

  private:
    TwoWire *_wire;
    uint8_t _i2caddr;
    uint8_t _HIRCCflag;
    uint8_t _ADCflag;
    void HIRCCstate();
    bool setTimeOut(uint8_t I2CTimeOut) ;
};
  /*! I2C debounce config */
#define    DEBOUNCE_NO 0x00
#define    DEBOUNCE_2_CLOCK 0x01
#define    DEBOUNCE_4_CLOCK 0x11
  /*! I2C address config */
#define    ADDRESS_50  0x50
#define    ADDRESS_58  0x58
#define    ADDRESS_60  0x60
#define    ADDRESS_68  0x68
  /*! Adc reference voltage config */
#define    REFERENCE_VCM_AVSS 0x00       //Internal reference voltage pair
#define    REFERENCE_VREFP_VREFN 0x01    //External reference voltage pair
  /*! Sampling Rate config */
#define    RATE_5HZ  0x00  
#define    RATE_10HZ  0x01 
#define    RATE_20HZ  0x02
#define    RATE_40HZ  0x03
#define    RATE_80HZ  0x04
#define    RATE_160HZ 0x05
#define    RATE_320HZ 0x06
#define    RATE_640HZ 0x07
  /*! Adc channel negative config */
#define    CHSP_AN0_N_AN1  0x00
#define    CHSP_AN2_N_AN3  0x09
#define    CHSP_VDACO_N_VDACO  0x24
#define    CHSP_VCM_N_VCM  0x36
#define    CHSP_VTSP_N_VTSN  0x3F
  /*! Adc VREF GAIN config*/
#define    VREF_GAIN_1 0x00
#define    VREF_GAIN_0_5 0x01
#define    VREF_GAIN_0_25 0x02
  /*! Adc  GAIN config*/
#define    GAIN_1  0x00
#define    GAIN_2  0x01
#define    GAIN_4  0x02
#define    GAIN_8  0x03
#define    GAIN_16  0x04
#define    GAIN_32  0x05
#define    GAIN_64  0x0D
#define    GAIN_128  0x0E
  /*! Differential input signal PGAOP/PGAON offset selection*/
#define    DECSET_0  0x00        //!< DCSET=+0V
#define    DECSET_UP_0_25 0x01   //!< DCSET= +0.25 × ΔVR_I
#define    DECSET_UP_0_5 0x02    //!< DCSET= +0.5 × ΔVR_I
#define    DECSET_UP_0_75 0x03   //!< DCSET= +0.75 × ΔVR_I
#define    DECSET_DOWN_0_25 0x05 //!< DCSET= -0.25 × ΔVR_I
#define    DECSET_DOWN_0_5 0x06  //!< DCSET= -0.5 × ΔVR_I
#define    DECSET_DOWN_0_75 0x07 //!< DCSET= -0.75 × ΔVR_I
/*Register address */    
#define PWRC    (0x00)
#define PGAC0   (0x01)
#define PGAC1   (0x02)
#define PGACS   (0x03)
#define ADRL    (0x04)
#define ADRM    (0x05)
#define ADRH    (0x06)
#define ADCR0   (0x07)
#define ADCR1   (0x08)
#define ADCS    (0x09)
#define ADCTE   (0x0A)
#define DAH     (0x0B)
#define DAL     (0x0C)
#define DACC    (0x0D)
#define SIMC0   (0x0E)
#define SIMTOC  (0x10)
#define HIRCC   (0x11)
#define HXTC    (0x12)

#endif
