#ifndef DF_ITEMS_HH_INCLUDED
#define DF_ITEMS_HH_INCLUDED
#include "DF_Subfields.hh"

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <iostream>

int16_t decode_id(const uint16_t & code);
double decode_ac(const uint16_t & code);
int16_t decode_ii();
//Short air-air surveillance (ACAS)
struct df0
{
    uint8_t VS;             // :1
    uint8_t RI;             // :4
    uint16_t AC;            // :13
    uint8_t AA[3];          // Adddress spocita se jako CRC
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Surveillance, altitude reply
struct df4
{
    uint8_t FS;             //Flight_Status:3
    uint8_t DR;             //Downlink Request:5
    uint8_t UM;             //Utility Message:6
    uint16_t AC;            //Altitude_Code:13
    uint8_t AA[3];          // Adddress spocita se jako CRC
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Surveillance, identity reply
struct df5
{
    uint8_t FS;             //Flight_Status:3
    uint8_t DR;             //Downlink Request:5
    uint8_t UM;             //Utility Message:6
    uint16_t ID;            //Identification Code:13
    uint8_t AA[3];          // Adddress spocita se jako CRC
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//All-call reply
struct df11
{
    uint8_t CA;             //Capability:3
    uint8_t AA[3];          //Aircraft Address:24
    uint8_t II_SI[3];          //spocita se jako CRC
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};


//Long air-air surveillance (ACAS)
struct df16 : public df0
{
    uint8_t MV[7];          // :56
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Extended squitter
struct df17
{
    uint8_t CA;             //Capability:3
    uint8_t AA[3];          //Aircraft Address:24
    uint8_t ME[7];          //Extended squiter Message:56
    void ME_Data_to_Items(me_items & me) { me.Data_to_Items(ME);}
    void ME_Items_to_Data(const me_items & me) {me.Items_to_Data(ME); };
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//ADS-B messages with DF=18 with CF=0,1 are from non-transponder surface and airborne targets.
//CF=1 means that targets use anonymous mode.
struct df18
{
    uint8_t CF;             //Control:3
    uint8_t AA[3];          //Aircraft Address:24
    uint8_t ME[7];          //Extended squiter Message:56
    void ME_Data_to_Items(me_items & me) { me.Data_to_Items(ME);}
    void ME_Items_to_Data(const me_items & me) {me.Items_to_Data(ME); };
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//ADS-B messages with DF=19 with AF=0 are from military targets.
struct df19
{
    uint8_t AF;             //Application:3
    uint8_t AA[3];          //Aircraft Address:24
    uint8_t ME[7];          //Extended squiter Message:56
    void ME_Data_to_Items(me_items & me) { me.Data_to_Items(ME);}
    void ME_Items_to_Data(const me_items & me) {me.Items_to_Data(ME); };
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Comm-B, altitude reply
struct df20 : public df4
{
    uint8_t MB[7];          //COMM B Message:56
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Comm-B, identity reply
struct df21 : public df5
{
    uint8_t MB[7];          //COMM B Message:56
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

//Comm-D (ELM)
struct df24
{
    uint8_t KE;             //ELM Control:1
    uint8_t ND;             //Number of D segment:4
    uint8_t MD[10];         //COMM D Message:80
    uint8_t AA[3];          // Adddress spocita se jako CRC
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

union df_spec_fields
{
    df0 DF0;
    df4 DF4;
    df5 DF5;
    df11 DF11;
    df16 DF16;
    df17 DF17;
    df18 DF18;
    df19 DF19;
    df20 DF20;
    df21 DF21;
    df24 DF24;
};

struct df_items
{
    uint8_t  DF;                            //Downlink Format:5
    df_spec_fields SPEC;
    uint8_t AP_PI[3];                       // Adresss Parity:24

    void Set_DF(uint8_t Value);
    uint8_t Get_DF() const;

    bool Set_VS(uint8_t Value);
    uint8_t Get_VS() const;

    bool Set_RI(uint8_t Value);
    uint8_t Get_RI() const;

    bool Set_AC(uint16_t Value);
    uint16_t Get_AC() const;

    bool Set_ID(uint16_t Value);
    uint16_t Get_ID() const;

    bool Set_FS(uint8_t Value);
    uint8_t Get_FS() const;

    bool Set_DR(uint8_t Value);
    uint8_t Get_DR() const;

    bool Set_UM(uint8_t Value);
    uint8_t Get_UM() const;

    bool Set_CA(uint8_t Value);
    uint8_t Get_CA() const;

    bool Set_CF(uint8_t Value);
    uint8_t Get_CF() const;

    bool Set_AF(uint8_t Value);
    uint8_t Get_AF() const;

    bool Set_AA(uint8_t *data);
    bool Set_AA(uint32_t Value);
    bool Get_AA(uint8_t *data) const;

    uint32_t Get_AA() const;
    uint32_t Get_AA(bool OVC,uint16_t BDS) const;

    bool Set_II_SI(uint32_t Value);
    uint32_t Get_II_SI() const;

    bool Set_KE(uint8_t Value);
    uint8_t Get_KE() const;

    bool Set_ND(uint8_t Value);
    uint8_t Get_ND() const;

    bool Set_MB(uint8_t *data);
    bool Get_MB(uint8_t *data) const;

    bool Set_MV(uint8_t *data);
    bool Get_MV(uint8_t *data) const;

    bool Set_ME(uint8_t *data);
    bool Get_ME(uint8_t *data) const;

    bool Set_MD(uint8_t *data);
    bool Get_MD(uint8_t *data) const;
    
    bool Set_MD_TAS(uint16_t data);
    uint16_t Get_MD_TAS() const;

    void Set_AP_PI(uint8_t *data);
    void Set_AP_PI(uint32_t Value);
    void Get_AP_PI(uint8_t *data) const;

    bool Data_to_Items(const std::vector<uint8_t> &DF_Data);
    bool Items_to_Data(std::vector<uint8_t> &DF_Data) const;

    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

#endif // DF_ITEMS_HH_INCLUDED
