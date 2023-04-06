/*************************************************
File:       			BMH23M001.cpp
Author:           BESTMODULES
Description:      I2C communication with the BMH23M001 and obtain the corresponding value 
History：			  
V1.0.1	 -- initial version；2023-01-30；Arduino IDE :v1.8.13

**************************************************/
#include "BMH23M001.h"
/*************************************************
Description:  Constructor
Parameters:   theWire : Wire object if your board has more than one I2C interface 
              address : Module IIC address  
Return:                           
Others:   
*************************************************/
BMH23M001::BMH23M001(TwoWire *theWire)
{
  _wire = theWire;
  
}
/*************************************************
Description:   Module Initial
Parameters:
Return:                    
Others:   
*************************************************/
void BMH23M001::begin(uint8_t i2c_addr)
{
     _i2caddr = i2c_addr;
     _wire->begin();
     HIRCCstate();//Wait for the crystal to stabilize
     setDebounce(DEBOUNCE_NO);  
     setTimeOut(2);
     setADConverter( RATE_10HZ,REFERENCE_VREFP_VREFN);
     setDecset(DECSET_0);
     setGain(GAIN_32,VREF_GAIN_0_5);

     writeReg(PWRC, 0xA8);//VCM enable
     writeReg(ADCR1, 0x00);//
     writeReg(ADCS, 0x1F);
     writeReg(ADCTE, 0xE7);
}
/*************************************************
Description:  Start ADC conversion
Parameters: 
Return:            
Others:           
*************************************************/
void BMH23M001::beginADC()
{   
    uint8_t temp[1];
    readReg(temp,ADCR0);//
    writeReg(ADCR0, temp[0] & 0x1F); // ADRST = 0 ADSLP = 0  ADOFF=0 (ADC power on and enter normal mode)
    readReg(temp,ADCR0);
    writeReg(ADCR0, temp[0] | 0x80); // ADRST = 1
    writeReg(ADCR0, temp[0] & 0x7F); // ADRST = 0
}
/*************************************************
Description:  read ADC convert data
Parameters:            
Return:       N:data is updated 
              0：No data updated
Others:    
*************************************************/
long BMH23M001::readADCData()
{
  long adc24Data = 0;
  uint16_t count = 1000;
  uint8_t RegData[1],dataL[1],dataM[1],dataH[1];
  for (uint8_t i = 0; i < count; i++)
  {
      readReg(RegData,ADCR1);
      if ((RegData[0] & 0x02) == 0x02)
      {
      _ADCflag = 1;//The conversion is complete
      break;
      }
      if (i == count - 1)
      {
      _ADCflag = 0;//Converting
      }
  }
  if (_ADCflag == 1)
  {
    writeReg(ADCR1, RegData[0] | 0x04);           // ADCDL = 1 Lock Data for read
    readReg(dataL,ADRL);
    readReg(dataM,ADRM);
    readReg(dataH,ADRH);
    int32_t adcdata = (dataH[0] << 24) | (dataM[0] << 16) | (dataL[0] << 8) ;
    adcdata /= 256;
    adc24Data = adcdata;
    writeReg(ADCR1, RegData[0] & 0xFB);           // ADCDL = 0 Unlock Data for ADC Convert
    writeReg(ADCR1, RegData[0] & 0xFd);           // eoc = 0,A/D is in transition
    return adc24Data;
  } 
  else
  {
    return 0;
  }
}
/*************************************************
Description:  Disable ADC function
Parameters:            
Return:            
Others:     Bandgap on,PGA on,SINC on,ADC off,VCM off,VRN/VRP off
*************************************************/
void BMH23M001::endADC()
{
  uint8_t temp[1]={0};
  readReg(temp,ADCR0);
  writeReg( ADCR0, temp[0] | 0x40);
}
/*************************************************
Description:  Disable ADC function and HIRCC
Parameters:           
Return:            
Others:    ADC off,SINC off,VCM off,Bandgap off,PGA off,VRN/VRP off
*************************************************/
void BMH23M001::powerDown()
{
  uint8_t temp[]={0};
  readReg(temp,PWRC);
  writeReg(PWRC, temp[0] | 0x7F);//
  readReg(temp,ADCR0);
  writeReg( ADCR0, temp[0] | 0x20);//
  writeReg( HIRCC, 0x00);
  writeReg( HXTC, 0x00);
}
/*************************************************
Description:  Wake up the module
Parameters:           
Return:            
Others:     
*************************************************/
void BMH23M001::wakeup()
{
  uint8_t temp[1]={0};
  readReg(temp,PWRC);
  writeReg(PWRC, temp[0] | 0x80);
  HIRCCstate();
}
/*************************************************
Description:  write register
Parameters:   regAdd:Register address 
              regData:Data written to registers      
Return:            
Others:       
*************************************************/
void BMH23M001::writeReg(uint8_t regAdd, uint8_t regData)
{
    while (_wire->available() > 0)
    {
      _wire->read();
    }
   _wire->beginTransmission(_i2caddr);
   _wire->write(regAdd);
   _wire->write(regData);
   _wire->endTransmission();
   delay(1);
}
/*************************************************
Description:  Read register value
Parameters:   regAdd:Register address        
Return:       Register value     
Others:  
*************************************************/
void BMH23M001::readReg(uint8_t rbuf[],uint8_t regAdd)
{
   _wire->beginTransmission(_i2caddr);
   _wire->write(regAdd);
   _wire->endTransmission();
   _wire->requestFrom((int)_i2caddr,1);
   //delay(10);
   while (_wire->available())
   {
     rbuf[0] = _wire->read();
   }
   delay(1);
}
/*************************************************
Description:   set AD converter
Parameters:    sampleRate:RATE_5HZ  0x00  
                          RATE_10HZ  0x01 
                          RATE_20HZ  0x02
                          RATE_40HZ  0x03
                          RATE_80HZ  0x04
                          RATE_160HZ 0x05
                          RATE_320HZ 0x06
                          RATE_640HZ 0x07   
                reference:REFERENCE_VCM_AVSS 0x00       //Internal reference voltage pair
                          REFERENCE_VREFP_VREFN 0x01    //External reference voltage pair                  
Return:     1：Set success 0：Set fail
Others:   
*************************************************/
bool BMH23M001::setADConverter (uint8_t sampleRate ,uint8_t reference)
{
    if(_HIRCCflag)
    {
        writeReg(ADCR0, (1 << 5) + (sampleRate << 1) + (reference));
        return 1;
    }
    return 0;     
}
/*************************************************
Description:  set decset
Parameters:   decset:Differential input signal PGAOP/PGAON offset selection 
                     DECSET_0V  0x00        //!< DCSET=+0V
                     DECSET_UP_0_25 0x01   //!< DCSET= +0.25 × ΔVR_I
                     DECSET_UP_0_5 0x02    //!< DCSET= +0.5 × ΔVR_I
                     DECSET_UP_0_75 0x03   //!< DCSET= +0.75 × ΔVR_I
                     DECSET_0 0x04        //!< DCSET=+0V,
                     DECSET_DOWN_0_25 0x05 //!< DCSET= -0.25 × ΔVR_I
                     DECSET_DOWN_0_5 0x06  //!< DCSET= -0.5 × ΔVR_I
                     DECSET_DOWN_0_75 0x07 //!< DCSET= -0.75 × ΔVR_I             
Return:       1：Set success 0：Set fail
Others:   
*************************************************/
bool BMH23M001::setDecset(uint8_t decset)
{
    if(_HIRCCflag)
    {
        writeReg(PGAC1, decset << 1);
        return 1;
    }
    return 0;    
  
}
/*************************************************
Description:  set GAIN
Parameters:   gain: GAIN_1  0x00
                    GAIN_2  0x01
                    GAIN_4  0x02
                    GAIN_8  0x03
                    GAIN_16  0x04
                    GAIN_32  0x05
                    GAIN_64  0x0D
                    GAIN_128  0x0E 
          vrefGain: VREF_GAIN_1 0x00
                    VREF_GAIN_0_5 0x01
                    VREF_GAIN_0_25 0x02                  
Return:   1：Set success 0：Set fail
Others:   
*************************************************/
bool BMH23M001::setGain (uint8_t gain ,uint8_t vrefGain)
{
    if(_HIRCCflag)
    {
        writeReg(PGAC0, (vrefGain << 5) + gain);
        return 1;
    }
    return 0;    
}
/*************************************************
Description:  set channel
Parameters:   channel:Adc channel negative config
                      CHSP_AN0_N_AN1  0x00
                      CHSP_AN2_N_AN3  0x09
                      CHSP_VDACO_N_VDACO  0x24
                      CHSP_VCM_N_VCM  0x36
                      CHSP_VTSP_N_VTSN  0x3F                
Return:       1：Set success 0：Set fail
Others:   
*************************************************/
bool BMH23M001::setChannel(uint8_t channel)
{
    if(_HIRCCflag)
    {
        writeReg(PGACS, channel);
        return 1;
    }
    return 0;    
}
/*************************************************
Description:  set Debounce
Parameters:   I2CDebounce:I2C debounce   
Return:       1：Set success 0：Set fail
Others: 
*************************************************/
bool BMH23M001::setDebounce(uint8_t I2CDebounce)
{
    if(_HIRCCflag)
    {
        writeReg(SIMC0, (I2CDebounce << 2));
        return 1;
    }
    return 0;
}
/*************************************************
Description:  set timeOut
Parameters:   I2CTimeOut:I2C timeout config                
Return:       1：Set success 0：Set fail
Others:   
*************************************************/
bool BMH23M001::setTimeOut(uint8_t I2CTimeOut)
{
    if(_HIRCCflag)
    {
        writeReg(SIMTOC, (1 << 7) + I2CTimeOut);
        return 1;
    }
    return 0;
}
/*************************************************
Description:  HIRCC Enable
Parameters:            
Return:       
Others:    
*************************************************/
void BMH23M001::HIRCCstate()
{
    writeReg(HIRCC, 0x01);
    const char count = 100;
    uint8_t temp[1]={0};
    for (size_t i = 0; i < count; i++)
    {
        readReg(temp,HIRCC);
        if (temp[0] == 0x03)
        {
        _HIRCCflag = 1;//success
        break;
        }
        if (i == count - 1)
        {
        _HIRCCflag = 0;//fail
        }
    }
}
