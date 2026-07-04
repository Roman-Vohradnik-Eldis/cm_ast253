#ifndef UF_ITEMS_HH_INCLUDED
#define UF_ITEMS_HH_INCLUDED
#include "UF_Subfields.hh"
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

union sd_field
{
    uint16_t Data;
    sd_di0 SD_DI0;
    sd_di1 SD_DI1;
    sd_di2 SD_DI2;
    sd_di3 SD_DI3;
    sd_di7 SD_DI7;
};

//Short Air-air Surveillance
struct uf0
{
    uint8_t RL;                 // :1
    uint8_t AQ;                 // :1

    void to_stringstream(std::stringstream &out, bool one_line = false) const; 
};

//Long Air-air Surveillance
struct uf16 : public uf0
{
    uint8_t MU[7];              // :56
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Surveillance , Altitude Request
struct uf4
{
    uint8_t PC;                 // :3
    uint8_t RR;                 // :5
    uint8_t DI;                 // :3
    sd_field SD;                // :16
    void to_stringstream(std::stringstream &out, bool one_line= false) const;

    uint16_t Get_Expect_BDS() const;
};

//Surveillance , Identity Request
struct uf5 : public uf4
{

};

//Comm-A, Altitude Request
struct uf20 : public uf4
{
    uint8_t MA[7];              // :56
    void to_stringstream(std::stringstream &out, bool one_line = false) const; 
};

//Comm-A, Identity Request
struct uf21 : public uf5
{
    uint8_t MA[7];              // :56
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

struct uf11
{
    uint8_t PR;                 // :4
    uint8_t IC;                 // :4
    uint8_t CL;                 // :3
    void to_stringstream(std::stringstream &out, bool one_line = false)const;
};

struct uf24
{
    uint8_t RC;                 // :2
    uint8_t NC;                 // :4
    uint8_t MC[10];             // :80
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};


union spec_fields
{
    uf0 UF0;
    uf4 UF4;
    uf5 UF5;
    uf11 UF11;
    uf16 UF16;
    uf20 UF20;
    uf21 UF21;
    uf24 UF24;
};

struct uf_items
{
    uint8_t UF;                 // :5
    spec_fields Format;
    uint8_t AP[3];              // :24

    void Set_UF(uint8_t Value);
    uint8_t Get_UF() const;

    bool Set_RL(uint8_t Value);
    uint8_t Get_RL() const;

    bool Set_AQ(uint8_t Value);
    uint8_t Get_AQ() const;

    bool Set_MU(uint8_t *data);
    bool Get_MU(uint8_t *data) const;

    bool Set_PC(uint8_t Value);
    uint8_t Get_PC() const;

    bool Set_RR(uint8_t Value);
    uint8_t Get_RR() const;

    bool Set_DI(uint8_t Value);
    uint8_t Get_DI() const;

    bool Set_SD(uint16_t Value);
    bool Set_SD(sd_di0 SD);
    bool Set_SD(sd_di1 SD);
    bool Set_SD(sd_di2 SD);
    bool Set_SD(sd_di3 SD);
    bool Set_SD(sd_di7 SD);
    uint16_t Get_SD() const;
    sd_di0 Get_SD_DI0() const;
    sd_di1 Get_SD_DI1() const;
    sd_di2 Get_SD_DI2() const;
    sd_di3 Get_SD_DI3() const;
    sd_di7 Get_SD_DI7() const;

    bool Set_MA(uint8_t *data);
    bool Get_MA(uint8_t *data) const;

    bool Set_PR(uint8_t Value);
    uint8_t Get_PR() const;

    bool Set_IC(uint8_t Value);
    uint8_t Get_IC() const;

    bool Set_CL(uint8_t Value);
    uint8_t Get_CL() const;

    bool Set_RC(uint8_t Value);
    uint8_t Get_RC() const;

    bool Set_NC(uint8_t Value);
    uint8_t Get_NC() const;

    bool Set_MC(uint8_t *data);
    bool Get_MC(uint8_t *data) const;
    
    bool Set_MC_SRS(uint16_t Value);        //nejvyssi bit znamena segment 0
    uint16_t Get_MC_SRS() const;

    void Set_AP(uint8_t *data);
    void Set_AP(uint32_t Value);
    void Get_AP(uint8_t *data) const;
    uint32_t Get_AP() const;

    uint8_t Get_Expect_DF() const;
    uint16_t Get_Expect_BDS() const;        //vraci 0xFFFF, pokud jsou nekorektni data pro urceni BDS
    uint8_t Get_LOS() const;                // vraci 0xFF pokud polozka chybi, dekoduje z SD fields
    uint8_t Get_TMS() const;                // vraci 0xFF pokud polozka chybi, dekoduje z SD fields, Tactical message subfield :4
    uint8_t Get_MES() const;                // vraci 0xFF pokud polozka chybi, dekoduje z SD fields, Multisite ELM subfield :3
    uint8_t Get_MBS() const;                // vraci 0xFF pokud polozka chybi, dekoduje z SD fields, Multisite Comm-B subfield :2
    uint8_t Get_RSS() const;                // vraci 0xFF pokud polozka chybi, dekoduje z SD fields, Reservation status subfield :2
    bool Get_OVC() const;


    bool Data_to_Items(const std::vector<uint8_t> &UF_Data);
    bool Items_to_Data(std::vector<uint8_t> &UF_Data) const;
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};


#endif // UF_ITEMS_HH_INCLUDED
