#include <iostream>
#include <iomanip>
#include "UF_DF_Data.hh"

uf_df_data :: uf_df_data()
{
    Data.clear();
}

bool uf_df_data :: Set_AP_PI(uint8_t *_AP_PI)
{
    if(Data.size()==7)
    {
        for(size_t i=0;i<3;i++)
            Data[i+4]=_AP_PI[i];
        return true;
    }
    if(Data.size()==14)
    {
        for(size_t i=0;i<3;i++)
            Data[i+11]=_AP_PI[i];
        return true;
    }
    return false;
}

bool uf_df_data :: Set_AP_PI(const uint32_t &_AP_PI)
{
    if(Data.size()==7)
    {
        for(size_t i=0;i<3;i++)
            Data[i+4]=(_AP_PI >> ((2-i)*8)) & 0xFF;
        return true;
    }
    if(Data.size()==14)
    {
        for(size_t i=0;i<3;i++)
            Data[i+11]=(_AP_PI >> ((2-i)*8)) & 0xFF;
        return true;
    }
    return false;
}

uint32_t uf_df_data :: Compute_DF_CRC_Rest()
{
const uint32_t GP = 0x1FFF409;
const uint32_t CRC_Rest_Mask = 0xFFFFFF;

uint32_t           i, j;
uint32_t iAcc;
uint8_t iMask;
uint32_t aiMesi;
uint32_t iMesLen;

  /* NEJDRIV URCIME DELKU ZPRAVY */
  //iMesLen = adsMesLen(aiMes[0]>>3)/8;
  iMesLen = Data.size();
  /* NASOUKAME TAM CELOU ZPRAVU */

  iAcc = 0x00000000;
  for(i=0;i<iMesLen;i++)
  {
     iMask = 0x80;
//     aiMesi=(aiMes[i/4]>>((3-(i%4))*8)) & 0x0ff;
//     aiMesi=(aiMes[i/2]>>((1-(i%2))*8)) & 0x0ff;
     aiMesi=Data[i];
     for(j=0;j<8;j++)
     {
		iAcc <<= 1;
		iAcc |= (aiMesi&iMask) ? 0x00000001 : 0x00000000;
		iAcc ^= (iAcc&0x01000000) ? GP : 0x00000000;
		iMask >>= 1;
	 }
  }

  return((iAcc&CRC_Rest_Mask));
}

void uf_df_data :: Compute_DF_AP_PI(const uint32_t &CRC_Rest)
{
    Set_AP_PI(CRC_Rest);
    Set_AP_PI(Compute_DF_CRC_Rest());
}

void uf_df_data :: Set_Data(uint16_t *input_data,const uint32_t &Length)
{
    Data.resize(Length);
    for(size_t i=0; i<Length; i++)
        Data[i]= (input_data[i/2]>>((1-(i%2))*8)) & 0x0ff;
}

void uf_df_data :: Set_Data(const uint64_t &SData1,const uint64_t &SData0)
{
    //dekodujeme UF, pro urceni delky
    size_t _DF = SData1>>51;
    //printf("UF=%d\n",_DF);
    if (_DF<16) Data.resize(7); else Data.resize(14);

    for(size_t i=0; i<7;i++)
    {
        Data[i]=(SData1>>(6-i)*8) & 0xFF;
    }
    if (_DF>=16)
    {
        for(size_t i=0; i<7;i++)
        {
            Data[i+7]=(SData0>>(6-i)*8) & 0xFF;
        }
    }
}

void uf_df_data :: Get_Data(uint16_t *output_data,size_t &Length)
{
    Length=Data.size();
    for(size_t i=0; i<(Length/2); i++)
        output_data[i]= Data[2*i]<<8 | Data[2*i+1];
    if (Length%2) output_data[(Length/2)]= Data[Length-1]<<8;
}

bool uf_df_data :: Decode_DF(df_items &Items) const      // Data -> Items
{
    return Items.Data_to_Items(Data);
}
bool uf_df_data :: Encode_DF(const df_items &Items)  // Items -> Data
{
    return Items.Items_to_Data(Data);
}

bool uf_df_data :: Decode_UF(uf_items &Items) const
{
    return Items.Data_to_Items(Data);
}
bool uf_df_data :: Encode_UF(const uf_items &Items)
{
    return Items.Items_to_Data(Data);
}

void uf_df_data :: print()
{
    for(uint32_t i=0;i<Data.size();i++)
        cout <<hex <<setw(2) <<setfill('0') << (uint)(Data[i]);
    cout << " ";
}

