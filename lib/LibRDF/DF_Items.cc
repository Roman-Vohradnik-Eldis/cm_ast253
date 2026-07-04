#include "DF_Items.hh"
#include "UF_DF_Data.hh"
#include <sstream>
#include <iomanip>
#include <iostream>

void df_items :: Set_DF(uint8_t Value)
{
    DF = Value;
}

uint8_t df_items ::  Get_DF() const
{
    return (DF);
}

bool df_items :: Set_VS(uint8_t Value)
{
    if (DF==0)
    {
        SPEC.DF0.VS=Value;
        return true;
    }
    if (DF==16)
    {
        SPEC.DF16.VS=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_VS() const
{
    if (DF==0)
        return SPEC.DF0.VS;
    if (DF==16)
        return SPEC.DF16.VS;
    return false;
}

bool df_items :: Set_RI(uint8_t Value)
{
    if (DF==0)
    {
        SPEC.DF0.RI=Value;
        return true;
    }
    if (DF==16)
    {
        SPEC.DF16.RI=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_RI() const
{
    if (DF==0)
        return SPEC.DF0.RI;
    if (DF==16)
        return SPEC.DF16.RI;
    return false;
}

bool df_items :: Set_AC(uint16_t Value)
{
    if (DF==0)
    {
        SPEC.DF0.AC=Value;
        return true;
    }
    if (DF==16)
    {
        SPEC.DF16.AC=Value;
        return true;
    }
    if (DF==4)
    {
        SPEC.DF4.AC=Value;
        return true;
    }
    if (DF==20)
    {
        SPEC.DF20.AC=Value;
        return true;
    }
    return false;
}


uint16_t df_items :: Get_AC() const
{
    if (DF==0)
        return SPEC.DF0.AC;
    if (DF==16)
        return SPEC.DF16.AC;
    if (DF==4)
        return SPEC.DF4.AC;
    if (DF==20)
        return SPEC.DF20.AC;
    return false;
}

bool df_items :: Set_ID(uint16_t Value)
{
    if (DF==5)
    {
        SPEC.DF5.ID=Value;
        return true;
    }
    if (DF==21)
    {
        SPEC.DF21.ID=Value;
        return true;
    }
    return false;
}

uint16_t df_items :: Get_ID() const
{
    if (DF==5)
        return SPEC.DF5.ID;
    if (DF==21)
        return SPEC.DF21.ID;
    return false;
}

bool df_items :: Set_FS(uint8_t Value)
{
    if (DF==4)
    {
        SPEC.DF4.FS=Value;
        return true;
    }
    if (DF==5)
    {
        SPEC.DF5.FS=Value;
        return true;
    }
    if (DF==20)
    {
        SPEC.DF20.FS=Value;
        return true;
    }
    if (DF==21)
    {
        SPEC.DF21.FS=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_FS() const
{
    if (DF==4)
        return SPEC.DF4.FS;
    if (DF==5)
        return SPEC.DF5.FS;
    if (DF==20)
        return SPEC.DF20.FS;
    if (DF==21)
        return SPEC.DF21.FS;
    return false;
}

bool df_items :: Set_DR(uint8_t Value)
{
    if (DF==4)
    {
        SPEC.DF4.DR=Value;
        return true;
    }
    if (DF==5)
    {
        SPEC.DF5.DR=Value;
        return true;
    }
    if (DF==20)
    {
        SPEC.DF20.DR=Value;
        return true;
    }
    if (DF==21)
    {
        SPEC.DF21.DR=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_DR() const
{
    if (DF==4)
        return SPEC.DF4.DR;
    if (DF==5)
        return SPEC.DF5.DR;
    if (DF==20)
        return SPEC.DF20.DR;
    if (DF==21)
        return SPEC.DF21.DR;
    return false;
}

bool df_items :: Set_UM(uint8_t Value)
{
    if (DF==4)
    {
        SPEC.DF4.UM=Value;
        return true;
    }
    if (DF==5)
    {
        SPEC.DF5.UM=Value;
        return true;
    }
    if (DF==20)
    {
        SPEC.DF20.UM=Value;
        return true;
    }
    if (DF==21)
    {
        SPEC.DF21.UM=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_UM() const
{
    if (DF==4)
        return SPEC.DF4.UM;
    if (DF==5)
        return SPEC.DF5.UM;
    if (DF==20)
        return SPEC.DF20.UM;
    if (DF==21)
        return SPEC.DF21.UM;
    return false;
}

bool df_items :: Set_CA(uint8_t Value)
{
    if (DF==11)
    {
        SPEC.DF11.CA=Value;
        return true;
    }
    if (DF==17)
    {
        SPEC.DF17.CA=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_CA() const
{
    if (DF==11)
        return SPEC.DF11.CA;
    if (DF==17)
        return SPEC.DF17.CA;
    return false;
}

bool df_items :: Set_CF(uint8_t Value)
{
    if (DF==18)
    {
        SPEC.DF18.CF=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_CF() const
{
    if (DF==18)
    {
        return SPEC.DF18.CF;
    }
    return false;
}

bool df_items :: Set_AF(uint8_t Value)
{
    if (DF==19)
    {
        SPEC.DF19.AF=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_AF() const
{
    if (DF==19)
    {
        return SPEC.DF19.AF;
    }
    return false;
}

bool df_items :: Set_AA(uint8_t *data)
{
    if (DF==11)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF11.AA[i] = data[i];
        return true;
    }
    if (DF==4)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF4.AA[i] = data[i];
        return true;
    }
    if (DF==5)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF5.AA[i] = data[i];
        return true;
    }
    if (DF==20)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF20.AA[i] = data[i];
        return true;
    }
    if (DF==21)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF21.AA[i] = data[i];
        return true;
    }
    if (DF==24)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF24.AA[i] = data[i];
        return true;
    }
    if (DF==17)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF17.AA[i] = data[i];
        return true;
    }
    if (DF==18)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF18.AA[i] = data[i];
        return true;
    }
    if (DF==19)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF19.AA[i] = data[i];
        return true;
    }
    if (DF==0)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF0.AA[i] = data[i];
        return true;
    }

    if (DF==16)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF16.AA[i] = data[i];
        return true;
    }
    return false;
}

bool df_items :: Set_AA(uint32_t Value)
{
    if (DF==11)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF11.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    if (DF==4)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF4.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    if (DF==5)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF5.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    if (DF==20)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF20.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    if (DF==21)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF21.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    if (DF==24)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF24.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    if (DF==17)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF17.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }

    if (DF==18)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF18.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }

    if (DF==19)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF19.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }

    if (DF==0)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF0.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }

    if (DF==16)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF16.AA[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    return false;
}

bool df_items :: Get_AA(uint8_t *data) const
{

    if (DF==11)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF11.AA[i];
        return true;
    }

    if (DF==4)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF4.AA[i];
        return true;
    }

    if (DF==5)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF5.AA[i];
        return true;
    }

    if (DF==20)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF20.AA[i];
        return true;
    }

    if (DF==21)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF21.AA[i];
        return true;
    }

    if (DF==24)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF24.AA[i];
        return true;
    }

    if (DF==17)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF17.AA[i];
        return true;
    }

    if (DF==18)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF18.AA[i];
        return true;
    }

    if (DF==19)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF19.AA[i];
        return true;
    }

    if (DF==0)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF0.AA[i];
        return true;
    }

    if (DF==16)
    {
        for(size_t i=0; i<3; i++)
            data[i] = SPEC.DF16.AA[i];
        return true;
    }
    return false;
}

uint32_t df_items :: Get_AA() const
{
    if (DF==11)
    {
        return (((uint32_t)(SPEC.DF11.AA[0]))<<16) | ((uint32_t)(SPEC.DF11.AA[1])<<8) | (uint32_t)(SPEC.DF11.AA[2]);
    }
    if (DF==4)
    {
        return (((uint32_t)(SPEC.DF4.AA[0]))<<16) | ((uint32_t)(SPEC.DF4.AA[1])<<8) | (uint32_t)(SPEC.DF4.AA[2]);
    }
    if (DF==5)
    {
        return (((uint32_t)(SPEC.DF5.AA[0]))<<16) | ((uint32_t)(SPEC.DF5.AA[1])<<8) | (uint32_t)(SPEC.DF5.AA[2]);
    }
    if (DF==20)
    {
        return (((uint32_t)(SPEC.DF20.AA[0]))<<16) | ((uint32_t)(SPEC.DF20.AA[1])<<8) | (uint32_t)(SPEC.DF20.AA[2]);
    }
    if (DF==21)
    {
        return (((uint32_t)(SPEC.DF21.AA[0]))<<16) | ((uint32_t)(SPEC.DF21.AA[1])<<8) | (uint32_t)(SPEC.DF21.AA[2]);
    }
    if (DF==24)
    {
        return (((uint32_t)(SPEC.DF24.AA[0]))<<16) | ((uint32_t)(SPEC.DF24.AA[1])<<8) | (uint32_t)(SPEC.DF24.AA[2]);
    }
    if (DF==17)
    {
        return (((uint32_t)(SPEC.DF17.AA[0]))<<16) | ((uint32_t)(SPEC.DF17.AA[1])<<8) | (uint32_t)(SPEC.DF17.AA[2]);
    }
    if (DF==18)
    {
        return (((uint32_t)(SPEC.DF18.AA[0]))<<16) | ((uint32_t)(SPEC.DF18.AA[1])<<8) | (uint32_t)(SPEC.DF18.AA[2]);
    }
    if (DF==19)
    {
        return (((uint32_t)(SPEC.DF19.AA[0]))<<16) | ((uint32_t)(SPEC.DF19.AA[1])<<8) | (uint32_t)(SPEC.DF19.AA[2]);
    }
    if (DF==0)
    {
        return (((uint32_t)(SPEC.DF0.AA[0]))<<16) | ((uint32_t)(SPEC.DF0.AA[1])<<8) | (uint32_t)(SPEC.DF0.AA[2]);
    }
    if (DF==16)
    {
        return (((uint32_t)(SPEC.DF16.AA[0]))<<16) | ((uint32_t)(SPEC.DF16.AA[1])<<8) | (uint32_t)(SPEC.DF16.AA[2]);
    }
    return 0xFFFFFFFF;
}

uint32_t df_items :: Get_AA(bool OVC,uint16_t BDS) const
{
    uint32_t Address = Get_AA();
    if (OVC)
    {
        return (Address ^ (BDS<<16));
    }
    return Address;
}


bool df_items :: Set_II_SI(uint32_t Value)
{
    if (DF==11)
    {
        for(size_t i=0; i<3; i++)
            SPEC.DF11.II_SI[i] = (Value>>((2-i)*8)) & 0xFF;
        return true;
    }
    return false;
}

uint32_t df_items :: Get_II_SI() const
{
    if (DF==11)
    {
        return (((uint32_t)(SPEC.DF11.II_SI[0]))<<16) | ((uint32_t)(SPEC.DF11.II_SI[1])<<8) | (uint32_t)(SPEC.DF11.II_SI[2]);
    }
    return 0xFFFFFFFF;
}

bool df_items :: Set_KE(uint8_t Value)
{
    if (DF==24)
    {
        SPEC.DF24.KE=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_KE() const
{
    if (DF==24)
        return SPEC.DF24.KE;
    return false;
}

bool df_items :: Set_ND(uint8_t Value)
{
    if (DF==24)
    {
        SPEC.DF24.ND=Value;
        return true;
    }
    return false;
}

uint8_t df_items :: Get_ND() const
{
    if (DF==24)
        return SPEC.DF24.ND;
    return false;
}

bool df_items :: Set_MB(uint8_t *data)
{
    if (DF==20)
    {
        for(size_t i=0; i<7; i++)
        {
            SPEC.DF20.MB[i] = data[i];
        }
        return true;
    }
    if (DF==21)
    {
        for(size_t i=0; i<7; i++)
        {
            SPEC.DF21.MB[i] = data[i];
        }
        return true;
    }
    return false;
}

bool df_items :: Get_MB(uint8_t *data) const
{
    if (DF==20)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = SPEC.DF20.MB[i];
        }
        return true;
    }
    if (DF==21)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = SPEC.DF21.MB[i];
        }
        return true;
    }
    return false;
}


bool df_items :: Set_MV(uint8_t *data)
{
    if (DF==16)
    {
        for(size_t i=0; i<7; i++)
        {
            SPEC.DF16.MV[i] = data[i];
        }
        return true;
    }
    return false;
}

bool df_items :: Get_MV(uint8_t *data) const
{
    if (DF==16)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = SPEC.DF16.MV[i];
        }
        return true;
    }
    return false;
}

bool df_items :: Set_ME(uint8_t *data)
{
    if (DF==17)
    {
        for(size_t i=0; i<7; i++)
        {
            SPEC.DF17.ME[i] = data[i];
        }
        return true;
    }
    else if (DF==18)
    {
        for(size_t i=0; i<7; i++)
        {
            SPEC.DF18.ME[i] = data[i];
        }
        return true;
    }
    else if (DF==19)
    {
        for(size_t i=0; i<7; i++)
        {
            SPEC.DF19.ME[i] = data[i];
        }
        return true;
    }
    return false;
}

bool df_items :: Get_ME(uint8_t *data) const
{
    if (DF==17)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = SPEC.DF17.ME[i];
        }
        return true;
    }
    else if (DF==18)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = SPEC.DF18.ME[i];
        }
        return true;
    }
    else if (DF==19)
    {
        for(size_t i=0; i<7; i++)
        {
            data[i] = SPEC.DF19.ME[i];
        }
        return true;
    }

    return false;
}

bool df_items :: Set_MD(uint8_t *data)
{
    if (DF==24)
    {
        for(size_t i=0; i<10; i++)
        {
            SPEC.DF24.MD[i] = data[i];
        }
        return true;
    }
    return false;
}

bool df_items :: Get_MD(uint8_t *data) const
{
    if (DF==24)
    {
        for(size_t i=0; i<10; i++)
        {
            data[i] = SPEC.DF24.MD[i];
        }
        return true;
    }
    return false;
}

bool df_items :: Set_MD_TAS(uint16_t data) {
    if (DF==24) {
        SPEC.DF24.MD[1] = (data>>8) & 0xFF;
        SPEC.DF24.MD[2] = data & 0xFF;
        return true;
    }
    return false;
}

uint16_t df_items :: Get_MD_TAS() const {
    return ((SPEC.DF24.MD[1] << 8) | SPEC.DF24.MD[2]);
}


void df_items :: Set_AP_PI(uint8_t *data)
{
    for(size_t i=0; i<3; i++)
        AP_PI[i] = data[i];
}

void df_items :: Set_AP_PI(uint32_t Value)
{
    for(size_t i=0; i<3; i++)
        AP_PI[i] = (Value>>((2-i)*8)) & 0xFF;
}

void df_items :: Get_AP_PI(uint8_t *data) const
{
    for(size_t i=0; i<3; i++)
        data[i] = AP_PI[i];
}

bool df_items :: Data_to_Items(const std::vector<uint8_t> &DF_Data)
{
    if (DF_Data.size() < 7) return false;

    uint8_t _DF = (DF_Data[0] >> 3);

    if ((_DF & 24) == 24) _DF=24;

    Set_DF(_DF);

    switch (_DF)
    {
        case 0:
            {	//DF=0 Short ACAS
                Set_VS((DF_Data[0]>>2) & 0x1);
                Set_RI(((DF_Data[1]<<1) | (DF_Data[2]>>7))& 0xF);
                Set_AC(((DF_Data[2] & 0x1F) << 8) | DF_Data[3]);
                Set_AP_PI((uint8_t*)(DF_Data.data()+4));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;

        case 4:
            {	//DF=4
                Set_FS(DF_Data[0] & 0x7);
                Set_DR((DF_Data[1]>>3) & 0x1F);
                Set_UM(((DF_Data[1]<<3) | (DF_Data[2]>>5)) & 0x3F);
                Set_AC(((DF_Data[2] & 0x1F) << 8) | DF_Data[3]);
                Set_AP_PI((uint8_t*)(DF_Data.data()+4));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;

        case 5:
            {	//DF=5
                Set_FS(DF_Data[0] & 0x7);
                Set_DR((DF_Data[1]>>3) & 0x1F);
                Set_UM(((DF_Data[1]<<3) | (DF_Data[2]>>5)) & 0x3F);
                Set_ID(((DF_Data[2] & 0x1F) << 8) | DF_Data[3]);
                Set_AP_PI((uint8_t*)(DF_Data.data()+4));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;

        case 11:
            {	//DF=11
                Set_CA(DF_Data[0] & 0x7);
                Set_AA((uint8_t*)(DF_Data.data()+1));
                Set_AP_PI((uint8_t*)(DF_Data.data()+4));
                Set_II_SI(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
//                printf("DF11\n");
//                printf("%X%X%X\n",DF_Data[1],DF_Data[2],DF_Data[3]);
            } break;

        case 16:
            {	//DF=16 Long ACAS
                if (DF_Data.size() <14) return false;
                Set_VS((DF_Data[0]>>2) & 0x1);
                Set_RI(((DF_Data[1]<<1) | (DF_Data[2]>>7))& 0xF);
                Set_AC(((DF_Data[2] & 0x1F) << 8) | DF_Data[3]);
                Set_MV((uint8_t*)(DF_Data.data()+4));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;

        case 17:
            {	//DF=17
                if (DF_Data.size() <14) return false;
                Set_CA(DF_Data[0] & 0x7);
                Set_AA((uint8_t*)(DF_Data.data()+1));
                Set_ME((uint8_t*)(DF_Data.data()+4));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
            } break;

        case 18:
            {	//DF=18
                if (DF_Data.size() <14) return false;
                Set_CF(DF_Data[0] & 0x7);
                if (Get_CF() != 0 && Get_CF() != 1) return false; // supported ADS-B only
                Set_AA((uint8_t*)(DF_Data.data()+1));
                Set_ME((uint8_t*)(DF_Data.data()+4));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
            } break;

        case 19:
            {	//DF=19
                if (DF_Data.size() <14) return false;
                Set_AF(DF_Data[0] & 0x7);
                if (Get_AF() != 0) return false; // supported ADS-B only
                Set_AA((uint8_t*)(DF_Data.data()+1));
                Set_ME((uint8_t*)(DF_Data.data()+4));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
            } break;

        case 20:
            {	//DF=20
                if (DF_Data.size() <14) return false;
                Set_FS(DF_Data[0] & 0x7);
                Set_DR((DF_Data[1]>>3) & 0x1F);
                Set_UM(((DF_Data[1]<<3) | (DF_Data[2]>>5)) & 0x3F);
                Set_AC(((DF_Data[2] & 0x1F) << 8) | DF_Data[3]);
                Set_MB((uint8_t*)(DF_Data.data()+4));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;

        case 21:
            {	//DF=21
                if (DF_Data.size() <14) return false;
                Set_FS(DF_Data[0] & 0x7);
                Set_DR((DF_Data[1]>>3) & 0x1F);
                Set_UM(((DF_Data[1]<<3) | (DF_Data[2]>>5)) & 0x3F);
                Set_ID(((DF_Data[2] & 0x1F) << 8) | DF_Data[3]);
                Set_MB((uint8_t*)(DF_Data.data()+4));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;

        case 24:
            {	//DF=24
                if (DF_Data.size() <14) return false;
                Set_KE((DF_Data[0]>>4) & 0x1);
                Set_ND(DF_Data[0] & 0xF);
                Set_MD((uint8_t*)(DF_Data.data()+1));
                Set_AP_PI((uint8_t*)(DF_Data.data()+11));
                Set_AA(((uf_df_data*)&DF_Data)->Compute_DF_CRC_Rest());
            } break;
        default : return false;
    }

    return true;
}

bool df_items :: Items_to_Data(std::vector<uint8_t> &DF_Data) const
{
    DF_Data.clear();
    uint8_t _DF = Get_DF();
    switch (_DF)
    {
        case 0:
            {	//DF=0 Short ACAS
                //DF:5 VS:1 NU:2 | NU:5 RI:3 | RI:1 NU:2 AC:5 | AC:8 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((VS==NULL) || (RI==NULL) || (AC==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | (((Get_VS())) <<2));
                DF_Data.push_back((Get_RI())>>1);
                DF_Data.push_back(((Get_RI()) <<7) | ((Get_AC())>>8));
                DF_Data.push_back((Get_AC()) & 0xFF);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;

        case 4:
            {	//DF=4
                //DF:5 FS:3 | DR:5 UM:3 | UM:3 AC:3 | AC:8 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (DR==NULL) || (UM==NULL) || (AC==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_FS()) & 0x7));
                DF_Data.push_back(((Get_DR()) <<3) | (((Get_UM()) & 0x3F) >>3));
                DF_Data.push_back((((Get_UM()) & 0x3F) <<5) | (((Get_AC()) & 0x1FFF)>>8));
                DF_Data.push_back((Get_AC()) & 0xFF);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;

        case 5:
            {	//DF=5
                //DF:5 FS:3 | DR:5 UM:3 | UM:3 ID:3 | ID:8 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (DR==NULL) || (UM==NULL) || (ID==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_FS()) & 0x7));
                DF_Data.push_back(((Get_DR()) <<3) | (((Get_UM()) & 0x3F) >>3));
                DF_Data.push_back((((Get_UM()) & 0x3F) <<5) | (((Get_ID()) & 0x1FFF)>>8));
                DF_Data.push_back((Get_ID()) & 0xFF);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));

            } break;

        case 11:
            {	//DF=11
                //DF:5 CA:3 | AA:8 | AA:8 | AA:8 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (CA==NULL) || (AA==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_CA()) & 0x7));
                DF_Data.push_back(SPEC.DF11.AA[0]);
                DF_Data.push_back(SPEC.DF11.AA[1]);
                DF_Data.push_back(SPEC.DF11.AA[2]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;

        case 16:
            {	//DF=16 Long ACAS
                //DF:5 VS:1 NU:2 | NU:5 RI:3 | RI:1 NU:2 AC:5 | AC:8 | MV:56 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((VS==NULL) || (RI==NULL) || (AC==NULL) || (MV==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | (((Get_VS())) <<2));
                DF_Data.push_back((Get_RI())>>1);
                DF_Data.push_back(((Get_RI()) <<7) | ((Get_AC())>>8));
                DF_Data.push_back((Get_AC()) & 0xFF);
                for(size_t i=0; i<7; i++)
                    DF_Data.push_back(SPEC.DF16.MV[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;

        case 17:
            {	//DF=17
                //DF:5 CA:3 | AA:8 | AA:8 | AA:8 | ME:56 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (CA==NULL) || (AA==NULL) || (ME==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_CA()) & 0x7));
                DF_Data.push_back(SPEC.DF17.AA[0]);
                DF_Data.push_back(SPEC.DF17.AA[1]);
                DF_Data.push_back(SPEC.DF17.AA[2]);
                for(size_t i=0; i<7; i++)
                    DF_Data.push_back(SPEC.DF17.ME[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));

            } break;

        case 18:
            {	//DF=18
                //DF:5 CF:3 | AA:8 | AA:8 | AA:8 | ME:56 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (CF==NULL) || (AA==NULL) || (ME==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_CF()) & 0x7));
                DF_Data.push_back(SPEC.DF18.AA[0]);
                DF_Data.push_back(SPEC.DF18.AA[1]);
                DF_Data.push_back(SPEC.DF18.AA[2]);
                for(size_t i=0; i<7; i++)
                    DF_Data.push_back(SPEC.DF18.ME[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));

            } break;

        case 19:
            {	//DF=19
                //DF:5 AF:3 | AA:8 | AA:8 | AA:8 | ME:56 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (AF==NULL) || (AA==NULL) || (ME==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_AF()) & 0x7));
                DF_Data.push_back(SPEC.DF19.AA[0]);
                DF_Data.push_back(SPEC.DF19.AA[1]);
                DF_Data.push_back(SPEC.DF19.AA[2]);
                for(size_t i=0; i<7; i++)
                    DF_Data.push_back(SPEC.DF19.ME[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));

            } break;

        case 20:
            {	//DF=20
                //DF:5 FS:3 | DR:5 UM:3 | UM:3 AC:3 | AC:8 | MB:56 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (DR==NULL) || (UM==NULL) || (AC==NULL) || (MB==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_FS()) & 0x7));
                DF_Data.push_back(((Get_DR()) <<3) | (((Get_UM()) & 0x3F) >>3));
                DF_Data.push_back((((Get_UM()) & 0x3F) <<5) | (((Get_AC()) & 0x1FFF)>>8));
                DF_Data.push_back((Get_AC()) & 0xFF);
                for(size_t i=0; i<7; i++)
                    DF_Data.push_back(SPEC.DF20.MB[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;

        case 21:
            {	//DF=21
                //DF:5 FS:3 | DR:5 UM:3 | UM:3 AC:3 | ID:8 | MB:56 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((FS==NULL) || (DR==NULL) || (UM==NULL) || (ID==NULL) || (MB==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(((_DF) <<3) | ((Get_FS()) & 0x7));
                DF_Data.push_back(((Get_DR()) <<3) | (((Get_UM()) & 0x3F) >>3));
                DF_Data.push_back((((Get_UM()) & 0x3F) <<5) | (((Get_ID()) & 0x1FFF)>>8));
                DF_Data.push_back((Get_ID()) & 0xFF);
                for(size_t i=0; i<7; i++)
                    DF_Data.push_back(SPEC.DF21.MB[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;

        case 24:
            {	//DF=24
                // 111 KE:1 ND:4 | MD:80 | AP_PI:8 | AP_PI:8 | AP_PI:8 |
//                if ((KE==NULL) || (ND==NULL) || (MD==NULL) || (AP_PI==NULL)) return false;
                DF_Data.push_back(0xE0 | (((Get_KE()) & 0x1)<<4) | (Get_ND() & 0xF));
                for(size_t i=0; i<10; i++)
                    DF_Data.push_back(SPEC.DF24.MD[i]);
                DF_Data.push_back(*AP_PI);
                DF_Data.push_back(*(AP_PI +1));
                DF_Data.push_back(*(AP_PI +2));
            } break;
        default : return false;
    }
    return true;
}
//305
void df_items::to_stringstream(std::stringstream &out, bool one_line)
   const {

    NEW_LINE(out, 4, ' ')

    out << "[DF=" << static_cast<int>(Get_DF());

    switch (DF)
    {
        case 0:  SPEC.DF0.to_stringstream(out, one_line); break;
        case 4:  SPEC.DF4.to_stringstream(out, one_line); break;
        case 5:  SPEC.DF5.to_stringstream(out, one_line); break;
        case 11: SPEC.DF11.to_stringstream(out, one_line); break;
        case 16: SPEC.DF16.to_stringstream(out, one_line); break;
        case 17: SPEC.DF17.to_stringstream(out, one_line); break;
        case 18: SPEC.DF18.to_stringstream(out, one_line); break;
        case 19: SPEC.DF19.to_stringstream(out, one_line); break;
        case 20: SPEC.DF20.to_stringstream(out, one_line); break;
        case 21: SPEC.DF21.to_stringstream(out, one_line); break;
        case 24: SPEC.DF24.to_stringstream(out, one_line); break;
        default : out << " Unknown UF "; break;
    }

    uint32_t II_SI = Get_II_SI();
    int8_t IC = II_SI & 0xE;
    int8_t CL = (II_SI & 0x7F) >> 4;

    if(static_cast<int>(CL) != 0 && II_SI != 0xFFFFFFFF) {
      uint8_t SI = ((CL - 1) << 4) | IC;
      out << std::dec << " SI=" << static_cast<int>(SI);
    } else if(II_SI != 0xFFFFFFFF) {
      out << std::dec << " II=" << static_cast<int>(IC);
    }

    uint32_t __attribute__((unused)) aa = Get_AA();
    out << "]";
    out << std::dec << std::setfill(' ');

    NEW_LINE(out, 0, '\0')
}
void
df21::to_stringstream(std::stringstream &out, bool one_line) const
{
     df5::to_stringstream(out,one_line);

     out << std::hex << std::noshowbase;

     out << " MB_hex=";
     for(uint8_t i = 0; i < 7; i++)
         out << std::setw(2) << std::setfill('0')  << static_cast<int>(MB[i]) <<  " ";

     out << std::dec << std::setfill(' ');
};

void
df24::to_stringstream(std::stringstream &out, bool one_line) const
{
     out << " KE=" << static_cast<int>(KE);
     out << " ND=" << static_cast<int>(ND);

     out << " MD_hex=";
     for(uint8_t i = 0; i < 10; i++)
     {
        out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(MD[i]);
     }

     out << " AA_hex=" << std::hex << std::setw(6) << std::setfill('0') << std::fixed
         << ((((uint32_t)(AA[0]))<<16) | ((uint32_t)(AA[1])<<8) | (uint32_t)(AA[2]));

     out << std::dec << std::setfill(' ');
};

void
df20::to_stringstream(std::stringstream &out, bool one_line) const
{
     df4::to_stringstream(out, one_line);
     out << std::hex << std::noshowbase;

     out << " MB_hex=";
     for(uint8_t i = 0; i < 7; i++)
         out << std::setw(2) << std::setfill('0') <<
            static_cast<int>(MB[i]) << " ";

     out << std::dec << std::setfill(' ');
};

void
df17::to_stringstream(std::stringstream &out, bool one_line) const
{
     if(CA_to_String.end() != CA_to_String.find(CA))
         out << " CA=" << static_cast<int>(CA) << "~\""
             << CA_to_String.find(CA)->second << "\"";
     else
         out << " CA=" << static_cast<int>(CA)
            << "\"Not defined\"";

     out << std::hex << " AA_hex=" << std::hex << std::setw(6) << std::setfill('0') << std::fixed <<
        ((((uint32_t)(AA[0]))<<16)
        | ((uint32_t)(AA[1])<<8)
        | (uint32_t)(AA[2]));

     out << std::setw(2) << std::setfill('0');

     out << " ME_hex=" << std::noshowbase;
     for(uint8_t i = 0; i < 7; i++)
         out << static_cast<int>(ME[i]) << " ";

     out << std::dec << std::setfill(' ');
};

void
df18::to_stringstream(std::stringstream &out, bool one_line) const
{
     if(CF_to_String.end() != CF_to_String.find(CF))
         out << " CF=" << static_cast<int>(CF) << "~\""
             << CA_to_String.find(CF)->second << "\"";
     else
         out << " CF=" << static_cast<int>(CF)
            << "\"Not defined\"";

     out << std::hex << " AA_hex=" << std::hex << std::setw(6) << std::setfill('0') << std::fixed <<
        ((((uint32_t)(AA[0]))<<16)
        | ((uint32_t)(AA[1])<<8)
        | (uint32_t)(AA[2]));

     out << std::setw(2) << std::setfill('0');

     out << " ME_hex=" << std::noshowbase;
     for(uint8_t i = 0; i < 7; i++)
         out << static_cast<int>(ME[i]) << " ";

     out << std::dec << std::setfill(' ');
};

void
df19::to_stringstream(std::stringstream &out, bool one_line) const
{
     if(AF_to_String.end() != AF_to_String.find(AF))
         out << " AF=" << static_cast<int>(AF) << "~\""
             << AF_to_String.find(AF)->second << "\"";
     else
         out << " AF=" << static_cast<int>(AF)
            << "\"Not defined\"";

     out << std::hex << " AA_hex=" << std::hex << std::setw(6) << std::setfill('0') << std::fixed <<
        ((((uint32_t)(AA[0]))<<16)
        | ((uint32_t)(AA[1])<<8)
        | (uint32_t)(AA[2]));

     out << std::setw(2) << std::setfill('0');

     out << " ME_hex=" << std::noshowbase;
     for(uint8_t i = 0; i < 7; i++)
         out << static_cast<int>(ME[i]) << " ";

     out << std::dec << std::setfill(' ');
};

void
df16::to_stringstream(std::stringstream &out, bool one_line) const
{
    df0::to_stringstream(out,one_line );

    out << std::hex << std::setw(2) << std::setfill('0');
    out << " MV_hex=";
    for(uint8_t i = 0; i < 7; i++)
        out << static_cast<int>(MV[i]) << " ";

    out << std::dec << std::setfill(' ');
};

void
df11::to_stringstream(std::stringstream &out, bool one_line) const
{
     if(CA_to_String.end() != CA_to_String.find(CA))
         out << " CA=" << static_cast<int>(CA) << "~\""
             << CA_to_String.find(CA)->second << "\"";
     else
         out << " CA=" << static_cast<int>(CA)
            << "\"Not defined\"";

     out << std::hex << " AA_hex=" <<  std::hex << std::setw(6) << std::setfill('0')
         << std::fixed << ((((uint32_t)(AA[0]))<<16) | ((uint32_t)(AA[1])<<8) | (uint32_t)(AA[2]));

     out << std::dec;
};

int16_t decode_id(const uint16_t &code) {
   uint16_t nCode = 0;

   nCode |= (code&0x0001)<<(2+0);  //D4
   nCode |= (code&0x0002)<<(7+2);   //B4+2
   nCode |= (code&0x0004)>>(1-0); //D2
   nCode |= (code&0x0008)<<(4+2);  //B2+2
   nCode |= (code&0x0010)>>(4-0);   //D1
   nCode |= (code&0x0020)<<(1+2); //B1+2
   nCode |= (code&0x0080)<<(4+3);  //A4+3
   nCode |= (code&0x0100)>>(3-1);   //C4+1
   nCode |= (code&0x0200)<<(1+3); //A2+3
   nCode |= (code&0x0400)>>(6-1);  //C2+1
   nCode |= (code&0x0800)<<(1);//2-3   //A1+3
   nCode |= (code&0x1000)>>(9-1); //C1=1

   return nCode;
}

void
df5::to_stringstream(std::stringstream &out, bool one_line) const
{
    if(FS_to_String.end() != FS_to_String.find(FS))
        out << " FS=" << static_cast<int>(FS) << "~\"" << FS_to_String.find(FS)->second << "\"";
    else
        out << " FS=" << static_cast<int>(FS) << "\"Not defined\"";

    out << " DR=" << static_cast<int>(DR);
    out << " UM=" << static_cast<int>(UM);
    out << std::hex << " ID=" << static_cast<int>(ID) <<  "(" << decode_id(ID) << ")";

    out << std::hex << " AA_hex=" <<  std::hex << std::setw(6) << std::setfill('0')
        << std::fixed << ((((uint32_t)(AA[0]))<<16) | ((uint32_t)(AA[1])<<8) | (uint32_t)(AA[2]));

    out << std::dec;
};

void
df4::to_stringstream(std::stringstream &out, bool one_line) const
{
    if(FS_to_String.end() != FS_to_String.find(FS))
        out << " FS=" << static_cast<int>(FS) << "~\""
            << FS_to_String.find(FS)->second << "\"";
    else
        out << " FS=" << static_cast<int>(FS)
           << "\"Not defined\"";

    out << std::dec;
    out << " DR=" << static_cast<int>(DR);
    out << " UM=" << static_cast<int>(UM);

    out << std::hex << " AC_hex=" << AC  << "(" << std::setprecision(2) << std::fixed
       << std::setw(5) << std::setfill('0') << decode_ac(AC) << ")";

    out << std::hex << " AA_hex=" <<  std::hex << std::setw(6) << std::setfill('0')
        << std::fixed << ((((uint32_t)(AA[0]))<<16) | ((uint32_t)(AA[1])<<8) | (uint32_t)(AA[2]));

};

void
df0::to_stringstream(std::stringstream &out, bool one_line) const
{
     out << std::hex << std::noshowbase;
     out << " VS_hex=" << static_cast<int>(VS);
     out << " RI_hex=" << static_cast<int>(RI);
     out << " ID_hex=" << static_cast<int>(AC);
     out << std::dec;
};

static const int32_t INVALID_GILLHAM_CODE = -10000;
static const int32_t INVALID_FL = -10001;

static int ABCD_gillham (const int ABCD) {
   int ret_val = ((ABCD & 0x2) << 9) | ((ABCD & 0x4) << 7);
   ret_val |= ((ABCD & 0x920) >> 5)
      | ((ABCD & 0x490) >> 3)
      | ((ABCD & 0x248) >> 1);

   return ret_val;
}

static int gillham_fl (const int ABCD) {

   int gillham = ABCD_gillham (ABCD);

   if ((gillham & 0x7) == 0
        || (gillham & 0x7) == 5
        || (gillham & 0x7) == 7) {
      return INVALID_GILLHAM_CODE;
   }

   int alt = gillham >> 3;

   alt ^= alt >> 1;
   alt ^= alt >> 2;
   alt ^= alt >> 4;
   alt ^= alt >> 8;

   int diff = 0;

   switch (gillham % 8) {

      case 1: diff = -2; break;
      case 3: diff = -1; break;
      case 2: diff = 0; break;
      case 6: diff = 1; break;
      case 4: diff = 2; break;
   }

   if (alt & 0x1)
   diff = -diff;

   return (alt * 5 - 10) + diff;
}

double
decode_ac(const uint16_t & code) {

   //code = code & 0x1FFF;

   bool meters=false;
   bool hft=false;
   uint16_t height_loc=0;

   double Altitude = 0;

   if(code & 0x1FFF) {
      if(code&0x0040)
         meters=true;
      if(!(code&0x0010))
         hft=true;
      if(!meters) {
         height_loc=(code&0x000f);
         height_loc|=((code&0x0020)>>1);
         height_loc|=((code&0x1f80)>>2);

         if(!hft) {
            Altitude = (double)height_loc* 0.25 ;
            Altitude -= 10;
         } else {
            height_loc = (code&0x0001)<<2;   //D4
            height_loc |= (code&0x0002)<<7;  //B4
            height_loc |= (code&0x0004)>>1;  //D2
            height_loc |= (code&0x0008)<<4;  //B2
            height_loc |= (code&0x0020)<<1;  //B1
            height_loc |= (code&0x0080)<<4;  //A4
            height_loc |= (code&0x0100)>>3;  //C4
            height_loc |= (code&0x0200)<<1;  //A2
            height_loc |= (code&0x0400)>>6;  //C2
            height_loc |= (code&0x0800)>>2;  //A1
            height_loc |= (code&0x1000)>>9;  //C1

            Altitude=gillham_fl(height_loc);
            if (Altitude == INVALID_GILLHAM_CODE) {
               height_loc &= 0xFFF7;
            }
         }
      }
   }

   return Altitude;
}

