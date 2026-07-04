#ifndef UF_DF_DATA_HH_INCLUDED
#define UF_DF_DATA_HH_INCLUDED
#include "DF_Items.hh"
#include "UF_Items.hh"


struct uf_df_data
{
    std::vector<uint8_t> Data;

    uf_df_data();

    void Set_Data(uint16_t *input_data,const uint32_t &Length);
    void Set_Data(const uint64_t &SData1,const uint64_t &SData0);   //naplni uf daty, hodnotami z driveru-delka se urci z UF
    void Get_Data(uint16_t *output_data,size_t &Length);
    bool Set_AP_PI(uint8_t *_AP_PI);
    bool Set_AP_PI(const uint32_t &_AP_PI);
    uint32_t Compute_DF_CRC_Rest();
    void Compute_DF_AP_PI(const uint32_t &CRC_Rest);
    uint32_t Compute_UF_CRC_Rest();
    void Compute_UF_AP_PI(const uint32_t &CRC_Rest);

    bool Decode_DF(df_items &Items) const;
    bool Encode_DF(const df_items &Items);
    bool Decode_UF(uf_items &Items) const;
    bool Encode_UF(const uf_items &Items);
    void print();
};
#endif // UF_DF_DATA_HH_INCLUDED
