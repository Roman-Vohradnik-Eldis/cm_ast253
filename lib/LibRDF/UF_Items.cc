#include <iostream>

#include "UF_Items.hh"
#include "UF_Subfields.hh"

void uf_items :: Set_UF(uint8_t Value)
{
    UF = Value;
}

uint8_t uf_items ::  Get_UF() const
{
        return (UF);
}


void uf_items :: Set_AP(uint8_t *data)
{
    for(size_t i=0; i<3; i++)
        AP[i] = data[i];
}

void uf_items :: Set_AP(uint32_t Value)
{
    for(size_t i=0; i<3; i++)
        AP[i] = (Value>>((2-i)*8)) & 0xFF;
}

void uf_items :: Get_AP(uint8_t *data) const
{
    for(size_t i=0; i<3; i++)
        data[i] = AP[i];
}

uint32_t uf_items :: Get_AP() const
{
  uint32_t RetVal = 0;
  for(size_t i=0; i<3; i++) RetVal |= ((uint32_t)AP[i]) << ((2-i)*8);
  return RetVal;
}


bool uf_items :: Set_RL(uint8_t Value)
{
    if (UF==0)
    {
        Format.UF0.RL = Value;
        return true;
    }
    if (UF==16)
    {
        Format.UF16.RL = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_RL() const
{
    if (UF==0)
        return (Format.UF0.RL);
    if (UF==16)
        return (Format.UF16.RL);
    return 0xFF;
}


bool uf_items :: Set_AQ(uint8_t Value)
{
    if (UF==0)
    {
        Format.UF0.AQ = Value;
        return true;
    }
    if (UF==16)
    {
        Format.UF16.AQ = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_AQ() const
{
    if (UF==0)
        return (Format.UF0.AQ);
    if (UF==16)
        return (Format.UF16.AQ);
    return 0xFF;
}

bool uf_items :: Set_MU(uint8_t *data)
{
    if (UF==16)
    {
        for(size_t i=0; i<7; i++)
        {
            Format.UF16.MU[i] = data[i];
        }
        return true;
    }
    return false;
}

bool uf_items :: Get_MU(uint8_t *data) const
{
    if (UF==16)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = Format.UF16.MU[i];
        }
        return true;
    }
    return false;
}

bool uf_items :: Set_PC(uint8_t Value)
{
    if (UF==4)
    {
        Format.UF4.PC = Value;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.PC = Value;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.PC = Value;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.PC = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_PC() const
{
    if (UF==4)
        return (Format.UF4.PC);
    if (UF==5)
        return (Format.UF5.PC);
    if (UF==20)
        return (Format.UF20.PC);
    if (UF==21)
        return (Format.UF21.PC);
    return 0xFF;
}

bool uf_items :: Set_RR(uint8_t Value)
{
    if (UF==4)
    {
        Format.UF4.RR = Value;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.RR = Value;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.RR = Value;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.RR = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_RR() const
{
    if (UF==4)
        return (Format.UF4.RR);
    if (UF==5)
        return (Format.UF5.RR);
    if (UF==20)
        return (Format.UF20.RR);
    if (UF==21)
        return (Format.UF21.RR);
    return 0xFF;
}

bool uf_items :: Set_DI(uint8_t Value)
{
    if (UF==4)
    {
        Format.UF4.DI = Value;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.DI = Value;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.DI = Value;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.DI = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_DI() const
{
    if (UF==4)
        return (Format.UF4.DI);
    if (UF==5)
        return (Format.UF5.DI);
    if (UF==20)
        return (Format.UF20.DI);
    if (UF==21)
        return (Format.UF21.DI);
    return 0xFF;
}

bool uf_items :: Set_SD(uint16_t Value)
{
    if (UF==4)
    {
        Format.UF4.SD.Data = Value;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.SD.Data = Value;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.SD.Data = Value;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.SD.Data = Value;
        return true;
    }
    return false;
}

bool uf_items :: Set_SD(sd_di0 SD)
{
    if (UF==4)
    {
        Format.UF4.SD.SD_DI0 = SD;
        Format.UF4.DI = 0;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.SD.SD_DI0 = SD;
        Format.UF5.DI = 0;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.SD.SD_DI0 = SD;
        Format.UF20.DI = 0;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.SD.SD_DI0 = SD;
        Format.UF21.DI = 0;
        return true;
    }
    return false;
}

bool uf_items :: Set_SD(sd_di1 SD)
{
    if (UF==4)
    {
        Format.UF4.SD.SD_DI1 = SD;
        Format.UF4.DI = 1;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.SD.SD_DI1 = SD;
        Format.UF5.DI = 1;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.SD.SD_DI1 = SD;
        Format.UF20.DI = 1;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.SD.SD_DI1 = SD;
        Format.UF21.DI = 1;
        return true;
    }
    return false;
}

bool uf_items :: Set_SD(sd_di2 SD)
{
    if (UF==4)
    {
        Format.UF4.SD.SD_DI2 = SD;
        Format.UF4.DI = 2;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.SD.SD_DI2 = SD;
        Format.UF5.DI = 2;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.SD.SD_DI2 = SD;
        Format.UF20.DI = 2;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.SD.SD_DI2 = SD;
        Format.UF21.DI = 2;
        return true;
    }
    return false;
}

bool uf_items :: Set_SD(sd_di3 SD)
{
    if (UF==4)
    {
        Format.UF4.SD.SD_DI3 = SD;
        Format.UF4.DI = 3;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.SD.SD_DI3 = SD;
        Format.UF5.DI = 3;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.SD.SD_DI3 = SD;
        Format.UF20.DI = 3;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.SD.SD_DI3 = SD;
        Format.UF21.DI = 3;
        return true;
    }
    return false;
}

bool uf_items :: Set_SD(sd_di7 SD)
{
    if (UF==4)
    {
        Format.UF4.SD.SD_DI7 = SD;
        Format.UF4.DI = 7;
        return true;
    }
    if (UF==5)
    {
        Format.UF5.SD.SD_DI7 = SD;
        Format.UF5.DI = 7;
        return true;
    }
    if (UF==20)
    {
        Format.UF20.SD.SD_DI7 = SD;
        Format.UF20.DI = 7;
        return true;
    }
    if (UF==21)
    {
        Format.UF21.SD.SD_DI7 = SD;
        Format.UF21.DI = 7;
        return true;
    }
    return false;
}

uint16_t uf_items ::  Get_SD() const
{
    if (UF==4)
        return (Format.UF4.SD.Data);
    if (UF==5)
        return (Format.UF5.SD.Data);
    if (UF==20)
        return (Format.UF20.SD.Data);
    if (UF==21)
        return (Format.UF21.SD.Data);
    return 0xFF;
}

sd_di0 uf_items ::  Get_SD_DI0() const
{
    if (UF==4)
        return (Format.UF4.SD.SD_DI0);
    if (UF==5)
        return (Format.UF5.SD.SD_DI0);
    if (UF==20)
        return (Format.UF20.SD.SD_DI0);
    if (UF==21)
        return (Format.UF21.SD.SD_DI0);
    return (Format.UF4.SD.SD_DI0);
}

sd_di1 uf_items ::  Get_SD_DI1() const
{
    if (UF==4)
        return (Format.UF4.SD.SD_DI1);
    if (UF==5)
        return (Format.UF5.SD.SD_DI1);
    if (UF==20)
        return (Format.UF20.SD.SD_DI1);
    if (UF==21)
        return (Format.UF21.SD.SD_DI1);
    return (Format.UF4.SD.SD_DI1);
}

sd_di2 uf_items ::  Get_SD_DI2() const
{
    if (UF==4)
        return (Format.UF4.SD.SD_DI2);
    if (UF==5)
        return (Format.UF5.SD.SD_DI2);
    if (UF==20)
        return (Format.UF20.SD.SD_DI2);
    if (UF==21)
        return (Format.UF21.SD.SD_DI2);
    return (Format.UF4.SD.SD_DI2);
}

sd_di3 uf_items ::  Get_SD_DI3() const
{
    if (UF==4)
        return (Format.UF4.SD.SD_DI3);
    if (UF==5)
        return (Format.UF5.SD.SD_DI3);
    if (UF==20)
        return (Format.UF20.SD.SD_DI3);
    if (UF==21)
        return (Format.UF21.SD.SD_DI3);
    return (Format.UF4.SD.SD_DI3);
}

sd_di7 uf_items ::  Get_SD_DI7() const
{
    if (UF==4)
        return (Format.UF4.SD.SD_DI7);
    if (UF==5)
        return (Format.UF5.SD.SD_DI7);
    if (UF==20)
        return (Format.UF20.SD.SD_DI7);
    if (UF==21)
        return (Format.UF21.SD.SD_DI7);
    return (Format.UF4.SD.SD_DI7);
}

bool uf_items :: Set_MA(uint8_t *data)
{
    if (UF==20)
    {
        for(size_t i=0; i<7; i++)
        {
            Format.UF20.MA[i] = data[i];
        }
        return true;
    }
    if (UF==21)
    {
        for(size_t i=0; i<7; i++)
        {
            Format.UF21.MA[i] = data[i];
        }
        return true;
    }
    return false;
}

bool uf_items :: Get_MA(uint8_t *data) const
{
    if (UF==20)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = Format.UF20.MA[i];
        }
        return true;
    }
    if (UF==21)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = Format.UF21.MA[i];
        }
        return true;
    }
    return false;
}

bool uf_items :: Set_PR(uint8_t Value)
{
    if (UF==11)
    {
        Format.UF11.PR = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_PR() const
{
    if (UF==11)
        return (Format.UF11.PR);

    return 0xFF;
}

bool uf_items :: Set_IC(uint8_t Value)
{
    if (UF==11)
    {
        Format.UF11.IC = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_IC() const
{
    if (UF==11)
        return (Format.UF11.IC);
    if (UF==4)
    {
        if (Format.UF4.DI==0) return (Format.UF4.SD.SD_DI0.IIS);
        if (Format.UF4.DI==1) return (Format.UF4.SD.SD_DI1.IIS);
        if (Format.UF4.DI==3) return (Format.UF4.SD.SD_DI3.SIS & 0xF);
        if (Format.UF4.DI==7) return (Format.UF4.SD.SD_DI7.IIS);
        return 0xFF;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==0) return (Format.UF5.SD.SD_DI0.IIS);
        if (Format.UF5.DI==1) return (Format.UF5.SD.SD_DI1.IIS);
        if (Format.UF5.DI==3) return (Format.UF5.SD.SD_DI3.SIS & 0xF);
        if (Format.UF5.DI==7) return (Format.UF5.SD.SD_DI7.IIS);
        return 0xFF;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==0) return (Format.UF20.SD.SD_DI0.IIS);
        if (Format.UF20.DI==1) return (Format.UF20.SD.SD_DI1.IIS);
        if (Format.UF20.DI==3) return (Format.UF20.SD.SD_DI3.SIS & 0xF);
        if (Format.UF20.DI==7) return (Format.UF20.SD.SD_DI7.IIS);
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==0) return (Format.UF21.SD.SD_DI0.IIS);
        if (Format.UF21.DI==1) return (Format.UF21.SD.SD_DI1.IIS);
        if (Format.UF21.DI==3) return (Format.UF21.SD.SD_DI3.SIS & 0xF);
        if (Format.UF21.DI==7) return (Format.UF21.SD.SD_DI7.IIS);
        return 0xFF;
    }
    return 0xFF;
}

bool uf_items :: Set_CL(uint8_t Value)
{
    if (UF==11)
    {
        Format.UF11.CL = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_CL() const
{
    if (UF==11)
        return (Format.UF11.CL);
    if (UF==4)
    {
        if (Format.UF4.DI==0) return 0;
        if (Format.UF4.DI==1) return 0;
        if (Format.UF4.DI==3) return (((Format.UF4.SD.SD_DI3.SIS >>4) & 0x3) + 1);
        if (Format.UF4.DI==7) return 0;
        return 0xFF;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==0) return 0;
        if (Format.UF5.DI==1) return 0;
        if (Format.UF5.DI==3) return (((Format.UF5.SD.SD_DI3.SIS >>4) & 0x3)+1);
        if (Format.UF5.DI==7) return 0;
        return 0xFF;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==0) return 0;
        if (Format.UF20.DI==1) return 0;
        if (Format.UF20.DI==3) return (((Format.UF20.SD.SD_DI3.SIS >>4) & 0x3) +1);
        if (Format.UF20.DI==7) return 0;
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==0) return 0;
        if (Format.UF21.DI==1) return 0;
        if (Format.UF21.DI==3) return (((Format.UF21.SD.SD_DI3.SIS >>4) & 0x3) +1);
        if (Format.UF21.DI==7) return 0;
        return 0xFF;
    }
    return 0xFF;
}


bool uf_items :: Set_RC(uint8_t Value)
{
    if (UF==24)
    {
        Format.UF24.RC = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_RC() const
{
    if (UF==24)
        return (Format.UF24.RC);

    return 0xFF;
}

bool uf_items :: Set_NC(uint8_t Value)
{
    if (UF==24)
    {
        Format.UF24.NC = Value;
        return true;
    }
    return false;
}

uint8_t uf_items ::  Get_NC() const
{
    if (UF==24)
        return (Format.UF24.NC);

    return 0xFF;
}

bool uf_items :: Set_MC(uint8_t *data)
{
    if (UF==24)
    {
        for(size_t i=0; i<10; i++)
        {
            Format.UF24.MC[i] = data[i];
        }
        return true;
    }
    return false;
}

bool uf_items :: Get_MC(uint8_t *data) const
{
    if (UF==24)
    {
        for(size_t i=0; i<10; i++)
        {
            data[i] = Format.UF24.MC[i];
        }
        return true;
    }
    return false;
}

//nejvyssi bit znamena segment 0
bool uf_items :: Set_MC_SRS(uint16_t Value) {
    if (UF==24) {
        Format.UF24.MC[0] = (Value>>8) & 0xff;
        Format.UF24.MC[1] = Value & 0xff;
        return true;
    }
    return false;
}

uint16_t uf_items :: Get_MC_SRS() const {
    return ((Format.UF24.MC[0] << 8) | Format.UF24.MC[1]);
}


uint8_t uf_items :: Get_Expect_DF() const
{
    if (UF == 4)
    {
        if (Format.UF4.RR < 16) return 4; else return 20;
    }
    if (UF == 20)
    {
        if (Format.UF20.RR < 16) return 4; else return 20;
    }
    if (UF == 5)
    {
        if (Format.UF5.RR < 16) return 5; else return 21;
    }
    if (UF == 21)
    {
        if (Format.UF21.RR < 16) return 5; else return 21;
    }
    if (UF == 24)
    {
        if (Format.UF24.RC < 2) return 0xff; else return 24;
    }
    return UF;
}

uint16_t uf_items :: Get_Expect_BDS() const
{
    if (UF == 4)  return Format.UF4.Get_Expect_BDS();
    if (UF == 5)  return Format.UF5.Get_Expect_BDS();
    if (UF == 20) return Format.UF20.Get_Expect_BDS();
    if (UF == 21) return Format.UF21.Get_Expect_BDS();
    return 0xFFFF;
}

uint8_t uf_items :: Get_LOS() const
{
    if (UF==4)
    {
        if (Format.UF4.DI==1) return (Format.UF4.SD.SD_DI1.LOS);
        if (Format.UF4.DI==3) return (Format.UF4.SD.SD_DI3.LSS);
        if (Format.UF4.DI==7) return (Format.UF4.SD.SD_DI7.LOS);
        return 0xFF;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==1) return (Format.UF5.SD.SD_DI1.LOS);
        if (Format.UF5.DI==3) return (Format.UF5.SD.SD_DI3.LSS);
        if (Format.UF5.DI==7) return (Format.UF5.SD.SD_DI7.LOS);
        return 0xFF;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==1) return (Format.UF20.SD.SD_DI1.LOS);
        if (Format.UF20.DI==3) return (Format.UF20.SD.SD_DI3.LSS);
        if (Format.UF20.DI==7) return (Format.UF20.SD.SD_DI7.LOS);
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==1) return (Format.UF21.SD.SD_DI1.LOS);
        if (Format.UF21.DI==3) return (Format.UF21.SD.SD_DI3.LSS);
        if (Format.UF21.DI==7) return (Format.UF21.SD.SD_DI7.LOS);
        return 0xFF;
    }
    return 0xFF;
}


uint8_t uf_items :: Get_TMS() const
{
// v pripade kratke zpravy UF4,UF5 TMS snad nema vyznam tak se nedekoduje
//    if (UF==4)
//    {
//        if (Format.UF4.DI==1) return (Format.UF4.SD.SD_DI1.TMS);
//        if (Format.UF4.DI==7) return (Format.UF4.SD.SD_DI7.TMS);
//        return 0xFF;
//    }
//    if (UF==5)
//    {
//        if (Format.UF5.DI==1) return (Format.UF5.SD.SD_DI1.TMS);
//        if (Format.UF5.DI==7) return (Format.UF5.SD.SD_DI7.TMS);
//        return 0xFF;
//    }
    if (UF==20)
    {
        if (Format.UF20.DI==1) return (Format.UF20.SD.SD_DI1.TMS);
        if (Format.UF20.DI==7) return (Format.UF20.SD.SD_DI7.TMS);
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==1) return (Format.UF21.SD.SD_DI1.TMS);
        if (Format.UF21.DI==7) return (Format.UF21.SD.SD_DI7.TMS);
        return 0xFF;
    }
    return 0xFF;
}

uint8_t uf_items :: Get_MES() const
{
    if (UF==4)
    {
        if (Format.UF4.DI==1) return (Format.UF4.SD.SD_DI1.MES);
        return 0xFF;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==1) return (Format.UF5.SD.SD_DI1.MES);
        return 0xFF;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==1) return (Format.UF20.SD.SD_DI1.MES);
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==1) return (Format.UF21.SD.SD_DI1.MES);
        return 0xFF;
    }
    return 0xFF;
}

uint8_t uf_items :: Get_MBS() const
{
    if (UF==4)
    {
        if (Format.UF4.DI==1) return (Format.UF4.SD.SD_DI1.MBS);
        return 0xFF;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==1) return (Format.UF5.SD.SD_DI1.MBS);
        return 0xFF;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==1) return (Format.UF20.SD.SD_DI1.MBS);
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==1) return (Format.UF21.SD.SD_DI1.MBS);
        return 0xFF;
    }
    return 0xFF;
}

uint8_t uf_items :: Get_RSS() const
{
    if (UF==4)
    {
        if (Format.UF4.DI==1) return (Format.UF4.SD.SD_DI1.RSS);
        return 0xFF;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==1) return (Format.UF5.SD.SD_DI1.RSS);
        return 0xFF;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==1) return (Format.UF20.SD.SD_DI1.RSS);
        return 0xFF;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==1) return (Format.UF21.SD.SD_DI1.RSS);
        return 0xFF;
    }
    return 0xFF;
}

bool uf_items :: Get_OVC() const
{
    if (UF==4)
    {
        if (Format.UF4.DI==0) return (Format.UF4.SD.SD_DI0.OVC);
        if (Format.UF4.DI==3) return (Format.UF4.SD.SD_DI3.OVC);
        if (Format.UF4.DI==7) return (Format.UF4.SD.SD_DI7.OVC);
        return false;
    }
    if (UF==5)
    {
        if (Format.UF5.DI==0) return (Format.UF5.SD.SD_DI0.OVC);
        if (Format.UF5.DI==3) return (Format.UF5.SD.SD_DI3.OVC);
        if (Format.UF5.DI==7) return (Format.UF5.SD.SD_DI7.OVC);
        return false;
    }
    if (UF==20)
    {
        if (Format.UF20.DI==0) return (Format.UF20.SD.SD_DI0.OVC);
        if (Format.UF20.DI==3) return (Format.UF20.SD.SD_DI3.OVC);
        if (Format.UF20.DI==7) return (Format.UF20.SD.SD_DI7.OVC);
        return false;
    }
    if (UF==21)
    {
        if (Format.UF21.DI==0) return (Format.UF21.SD.SD_DI0.OVC);
        if (Format.UF21.DI==3) return (Format.UF21.SD.SD_DI3.OVC);
        if (Format.UF21.DI==7) return (Format.UF21.SD.SD_DI7.OVC);
        return false;
    }
    return false;
}

bool uf_items :: Data_to_Items(const std::vector<uint8_t> &UF_Data)
{
//    clear();
    if (UF_Data.size() <7) return false;
    uint8_t _UF = (UF_Data[0]>>3);
    if ((_UF & 24) == 24) _UF=24;
    Set_UF(_UF);
    switch (_UF)
    {
        case 0:
            {	//UF=0 Short ACAS
                Set_RL((UF_Data[1]>>7)&0x1);
                Set_AQ((UF_Data[1]>>2)&0x1);
                Set_AP((uint8_t*)(UF_Data.data()+4));
            } break;

        case 4:
            {	//UF=4
                Set_PC(UF_Data[0]&0x7);
                Set_RR((UF_Data[1]>>3)&0x1F);
                Set_DI(UF_Data[1]&0x7);
                Set_SD((UF_Data[2]<<8)|UF_Data[3]);
                Set_AP((uint8_t*)(UF_Data.data()+4));
            } break;

        case 5:
            {	//UF=5
                Set_PC(UF_Data[0]&0x7);
                Set_RR((UF_Data[1]>>3)&0x1F);
                Set_DI(UF_Data[1]&0x7);
                Set_SD((UF_Data[2]<<8)|UF_Data[3]);
                Set_AP((uint8_t*)(UF_Data.data()+4));
            } break;

        case 11:
            {	//UF=11
                Set_PR(((UF_Data[0]&0x7)<<1) | (UF_Data[1]>>7));
                Set_IC((UF_Data[1]>>3)&0xF);
                Set_CL(UF_Data[1] & 0x7);
                Set_AP((uint8_t*)(UF_Data.data()+4));
            } break;

        case 16:
            {	//UF=16 Long ACAS
                Set_RL((UF_Data[1]>>7)&0x1);
                Set_AQ((UF_Data[1]>>2)&0x1);
                Set_MU((uint8_t*)(UF_Data.data()+4));
                Set_AP((uint8_t*)(UF_Data.data()+11));
            } break;

        case 20:
            {	//UF=20
                Set_PC(UF_Data[0]&0x7);
                Set_RR((UF_Data[1]>>3)&0x1F);
                Set_DI(UF_Data[1]&0x7);
                Set_SD((UF_Data[2]<<8)|UF_Data[3]);
//                Set_AP((uint8_t*)(UF_Data.data()+4));
//                Set_MA((uint8_t*)(UF_Data.data()+7));
                Set_MA((uint8_t*)(UF_Data.data()+4));
                Set_AP((uint8_t*)(UF_Data.data()+11));
            } break;

        case 21:
            {	//UF=21
                Set_PC(UF_Data[0]&0x7);
                Set_RR((UF_Data[1]>>3)&0x1F);
                Set_DI(UF_Data[1]&0x7);
                Set_SD((UF_Data[2]<<8)|UF_Data[3]);
//                Set_AP((uint8_t*)(UF_Data.data()+4));
//                Set_MA((uint8_t*)(UF_Data.data()+7));
                Set_MA((uint8_t*)(UF_Data.data()+4));
                Set_AP((uint8_t*)(UF_Data.data()+11));
            } break;

        case 24:
            {	//UF=24
                Set_RC((UF_Data[0]>>4)&0x3);
                Set_NC(UF_Data[0]&0xF);
                Set_MC((uint8_t*)(UF_Data.data()+1));
                Set_AP((uint8_t*)(UF_Data.data()+11));
            } break;

        default : return false;
    }
    return true;
}

bool uf_items :: Items_to_Data(std::vector<uint8_t> &UF_Data) const
{
    UF_Data.clear();
    switch (UF)
    {
        case 0:
            {	//UF=0 Short ACAS
                //UF:5 NU:3 | RL:1 NU:4 AQ:1 NU:2| NU:8 | NU:8 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back((Get_UF()) <<3);
                UF_Data.push_back(((Get_RL())<<7) | ((Get_AQ())<<2));
                UF_Data.push_back(0);
                UF_Data.push_back(0);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 4:
            {	//UF=4
                //UF:5 PC:3 | RR:5 DI:3 | SD:8 | SD:8 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back(((Get_UF()) <<3) | ((Get_PC()) &0x7));
                UF_Data.push_back(((Get_RR())<<3) | ((Get_DI())& 0x7));
                UF_Data.push_back((Get_SD())>>8);
                UF_Data.push_back((Get_SD())&0xFF);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 5:
            {	//UF=5
                //UF:5 PC:3 | RR:5 DI:3 | SD:8 | SD:8 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back(((Get_UF()) <<3) | ((Get_PC()) &0x7));
                UF_Data.push_back(((Get_RR())<<3) | ((Get_DI())& 0x7));
                UF_Data.push_back((Get_SD())>>8);
                UF_Data.push_back((Get_SD())&0xFF);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 11:
            {	//UF=11
                //UF:5 PR:3 | PR:1 IC:4 CL:3 | NU:8 | NU:8 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back(((Get_UF()) <<3) | (((Get_PR())&0xF) >>1));
                UF_Data.push_back( (((Get_PR())&0x1)<<7) | (((Get_IC())&0xF)<<3) | ((Get_CL())&0x7));
                UF_Data.push_back(0);
                UF_Data.push_back(0);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 16:
            {	//UF=16 Long ACAS
                //UF:5 NU:3 | RL:1 NU:4 AQ:1 NU:2| NU:8 | NU:8 | MU:56 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back((Get_UF()) <<3);
                UF_Data.push_back(((Get_RL())<<7) | ((Get_AQ())<<2));
                UF_Data.push_back(0);
                UF_Data.push_back(0);
                for(size_t i=0; i<7; i++)
                    UF_Data.push_back(Format.UF16.MU[i]);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 20:
            {	//UF=20
                //UF:5 PC:3 | RR:5 DI:3 | SD:8 | SD:8 | MA:56 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back(((Get_UF()) <<3) | ((Get_PC()) &0x7));
                UF_Data.push_back(((Get_RR())<<3) | ((Get_DI())& 0x7));
                UF_Data.push_back((Get_SD())>>8);
                UF_Data.push_back((Get_SD())&0xFF);


//                UF_Data.push_back(AP[0]);
//                UF_Data.push_back(AP[1]);
//                UF_Data.push_back(AP[2]);
//                for(size_t i=0; i<7; i++)
//                    UF_Data.push_back(Format.UF20.MA[i]);

                for(size_t i=0; i<7; i++)
                    UF_Data.push_back(Format.UF20.MA[i]);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 21:
            {	//UF=21
                //UF:5 PC:3 | RR:5 DI:3 | SD:8 | SD:8 | MA:56 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back(((Get_UF()) <<3) | ((Get_PC()) &0x7));
                UF_Data.push_back(((Get_RR())<<3) | ((Get_DI())& 0x7));
                UF_Data.push_back((Get_SD())>>8);
                UF_Data.push_back((Get_SD())&0xFF);


//                UF_Data.push_back(AP[0]);
//                UF_Data.push_back(AP[1]);
//                UF_Data.push_back(AP[2]);
//                for(size_t i=0; i<7; i++)
//                    UF_Data.push_back(Format.UF20.MA[i]);

                for(size_t i=0; i<7; i++)
                    UF_Data.push_back(Format.UF20.MA[i]);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        case 24:
            {	//UF=24
                //UF:2 RC:2 NC:4| MC:80 | AP:8 | AP:8 | AP:8 |
                UF_Data.push_back( (0xC0) | (((Get_RC()) &0x3)<<4) | ((Get_NC())&0xF) );
                for(size_t i=0; i<10; i++)
                    UF_Data.push_back(Format.UF24.MC[i]);
                UF_Data.push_back(AP[0]);
                UF_Data.push_back(AP[1]);
                UF_Data.push_back(AP[2]);
            } break;

        default : return false;
    }
    return true;
}
void
uf21::to_stringstream(std::stringstream &out, bool one_line) const {
        uf5::to_stringstream(out, one_line);

     
        out << " MA_hex=";
        for(uint8_t i = 0; i < 7; i++) {
            out << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(MA[i]) << " ";
        }

        out << std::dec << std::setfill(' ');
}

void
uf24::to_stringstream(std::stringstream &out, bool one_line) const {

       if(RC_to_String.end() != RC_to_String.find(RC))
            out << " RC=" << static_cast<int>(RC) << "~\""
                << RC_to_String.find(RC)->second << "\"";
        else
            out << " RC=" << static_cast<int>(RC)
               << "\"Not defined\"";
       out << " NC=" << static_cast<int>(NC);
       out << " MC_hex=";
       for(uint8_t i = 0; i < 10; i++)
           out << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(MC[i]) << " ";

       out << std::dec << std::setfill(' ');
}

void
uf11::to_stringstream(std::stringstream &out, bool one_line) const {

       out << std::dec;

        if(PR_to_String.end() != PR_to_String.find(PR))
            out << " PR=" << static_cast<int>(PR) << "~\""
                << PR_to_String.find(PR)->second << "\"";
        else
            out << " PR=" << static_cast<int>(PR)
               << "\"Not defined\"";

        out << " IC=" << static_cast<int>(IC);
        out << " CL=" << static_cast<int>(CL);

        if(static_cast<int>(CL) != 0) {
           uint8_t SI = ((CL - 1) << 4) | IC;
           out << " SI=" << static_cast<int>(SI);
        } else {
           out << " II=" << static_cast<int>(IC);
        }
};

void
uf_items::to_stringstream(std::stringstream &out, bool one_line) const {

   NEW_LINE(out, 4, ' ')

   out << std::dec << "[UF=" << static_cast<int>(UF);

   switch(UF) {
        case 0:
           Format.UF0.to_stringstream(out, one_line); break;
        case 4:
           Format.UF4.to_stringstream(out, one_line); break;
        case 5:
           Format.UF5.to_stringstream(out, one_line); break;
        case 11:
           Format.UF11.to_stringstream(out, one_line); break;
        case 16:
           Format.UF16.to_stringstream(out, one_line); break;
        case 20:
           Format.UF20.to_stringstream(out, one_line); break;
        case 21:
           Format.UF21.to_stringstream(out, one_line); break;
        case 24:
           Format.UF24.to_stringstream(out, one_line); break;
        default :
           out << " Unknown UF "; break;

   }
   out << std::hex << std::setw(2) << std::setfill('0');
   out << " AP_hex=" << std::noshowbase;
   for(uint8_t i = 0; i < 3; i++)
      out << static_cast<int>(AP[i]) << " ";

   out << std::dec << std::setfill(' ');

   out << " EDF=" << static_cast<int>(Get_Expect_DF());

   if(Get_Expect_BDS() != 0xffff)
      out << std::hex << " BDS_hex=" << std::noshowbase <<
         static_cast<int>(Get_Expect_BDS()) << std::dec;
}
void 
uf4::to_stringstream(std::stringstream &out, bool one_line) const {

  if(PC_to_String.end() != PC_to_String.find(PC)) {
     out << " PC=" << static_cast<int>(PC) << "~" <<
        PC_to_String.find(PC)->second << "";
  } else
     out << " PC=" << "not_defined";

   out << std::hex << std::setw(2) << std::setfill('0');
   out << " RR=" << static_cast<int>(RR) << " DI="
      << static_cast<int>(DI);

   out << " SD_hex=" << std::hex << 
      static_cast<int>(SD.Data) << " " << std::dec;

  switch (DI)  {
      case 0 :
         SD.SD_DI0.to_stringstream(out, one_line); break;
      case 1 :
         SD.SD_DI1.to_stringstream(out, one_line); break;
      case 2 :
         SD.SD_DI2.to_stringstream(out, one_line); break;
      case 3 :
         SD.SD_DI3.to_stringstream(out, one_line); break;
      case 7 :
         SD.SD_DI7.to_stringstream(out, one_line); break;
      default : /*out << "SD=%x ",SD.Data;*/ break;
  }

  if(Get_Expect_BDS()!=0xFFFF)
     out <<  std::hex << "EBDS_hex=" << Get_Expect_BDS() << std::dec;
}
uint16_t 
uf4::Get_Expect_BDS() const  {
  if (RR < 16) return 0xFFFF;
  if (DI==1)
  {
      uint16_t BDS1 = RR & 0xF;
      uint16_t BDS2 = 0;
      return ((BDS1<<4) | BDS2);
  }
  if (DI==3)
  {
      uint16_t BDS1 = RR & 0xF;
      uint16_t BDS2 = SD.SD_DI3.RRS;
      return ((BDS1<<4) | BDS2);
  }
  if (DI==7)
  {
      uint16_t BDS1 = RR & 0xF;
      uint16_t BDS2 = SD.SD_DI7.RRS;
      return ((BDS1<<4) | BDS2);
  }
  return 0xFFFF;
};
void 
uf20::to_stringstream(std::stringstream &out, bool one_line) const {
  uf4::to_stringstream(out, one_line);
    out << " MA=";

  for(uint8_t i = 0; i < 7; i++) {
      out << std::hex << std::setw(2) << std::setfill('0')
          << static_cast<int>(MA[i]) <<  " ";
  }

  out << std::dec << std::setfill(' ');

}
void 
uf16::to_stringstream(std::stringstream &out, bool one_line) const  {
  uf0::to_stringstream(out, one_line);

  
  out << " MU=";
  for(uint8_t i = 0; i < 7; i++) {
      out << std::hex << std::setw(2) << std::setfill('0')
          << static_cast<int>( MU[i]) <<  " ";
  }

  out << std::dec << std::setfill(' ');
}
void 
uf0::to_stringstream(std::stringstream &out, bool one_line) const {
  out << dec << setw(2) << " RL=" << RL <<",";
  out << dec << setw(2) << " AQ=" << AQ <<",";
}

