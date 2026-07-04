#ifndef UF_SUBFIELDS_HH_INCLUDED
#define UF_SUBFIELDS_HH_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <map>
#include "boost/assign.hpp"
#include <string>

#include <sstream>
#include <iomanip>

#ifndef NEW_LINE
#define NEW_LINE(st,cnt,c) \
   if((void *)&st != NULL) {\
      if(one_line == false) { \
         st << "\n";          \
         int i = 0;           \
         while(i++ < (int)cnt)\
            st << (c);        \
                              \
       } else {               \
         st << ';';           \
       }\
   }
#endif

using namespace boost::assign;

// Definition of PR field
const uint8_t PR_1 = 0;                             //signifies reply with probability of 1
const uint8_t PR_1_2 = 1;                           //signifies reply with probability of 1/2
const uint8_t PR_1_4 = 2;                           //signifies reply with probability of 1/4
const uint8_t PR_1_8 = 3;                           //signifies reply with probability of 1/8
const uint8_t PR_1_16 = 4;                          //signifies reply with probability of 1/16
const uint8_t PR_Not_Assigned_5 = 5;                //Not Assigned
const uint8_t PR_Not_Assigned_6 = 6;                //Not Assigned
const uint8_t PR_Not_Assigned_7 = 7;                //Not Assigned
const uint8_t PR_1_Override = 8;                    //signifies disregard lockout, reply with probability of 1
const uint8_t PR_1_2_Override = 9;                  //signifies disregard lockout, reply with probability of 1/2
const uint8_t PR_1_4_Override = 10;                 //signifies disregard lockout, reply with probability of 1/4
const uint8_t PR_1_8_Override = 11;                 //signifies disregard lockout, reply with probability of 1/8
const uint8_t PR_1_16_Override = 12;                //signifies disregard lockout, reply with probability of 1/16
const uint8_t PR_Not_Assigned_13 = 13;              //Not Assigned
const uint8_t PR_Not_Assigned_14 = 14;              //Not Assigned
const uint8_t PR_Not_Assigned_15 = 15;              //Not Assigned

const std::map<int8_t,std::string> PR_to_String = map_list_of
    (PR_1,"Probability of reply 1 ")
    (PR_1_2,"Probability of reply 1/2 ")
    (PR_1_4,"Probability of reply 1/4 ")
    (PR_1_8,"Probability of reply 1/8 ")
    (PR_1_16,"Probability of reply 1/16 ")
    (PR_Not_Assigned_5,"Not Assigned")
    (PR_Not_Assigned_6,"Not Assigned")
    (PR_Not_Assigned_7,"Not Assigned")
    (PR_1_Override,"Probability of reply 1 Override ")
    (PR_1_2_Override,"Probability of reply 1/2 Override ")
    (PR_1_4_Override,"Probability of reply 1/4 Override ")
    (PR_1_8_Override,"Probability of reply 1/8 Override ")
    (PR_1_16_Override,"Probability of reply 1/16 Override ")
    (PR_Not_Assigned_13,"Not Assigned")
    (PR_Not_Assigned_14,"Not Assigned")
    (PR_Not_Assigned_15,"Not Assigned");

//Definition of PC field
const uint8_t PC_No_Action = 0;                     //signifies no action
const uint8_t PC_Lockout = 1;                       //signifies non-selective all-call lockout
const uint8_t PC_Not_Assigned_2 = 2;                //not assigned
const uint8_t PC_Not_Assigned_3 = 3;                //not assigned
const uint8_t PC_Closeout_CommB = 4;                //signifies close out Comm-B
const uint8_t PC_Uplink_Closeout = 5;               //signifies close out uplink ELM
const uint8_t PC_Downlink_Closeout = 6;             //signifies close out downlink ELM
const uint8_t PC_Not_Assigned_7 = 7;                //not assigned

const std::map<int8_t,std::string> PC_to_String = map_list_of
    (PC_No_Action,"No Action ")
    (PC_Lockout,"non-selective all-call lockout ")
    (PC_Not_Assigned_2,"Not Assigned ")
    (PC_Not_Assigned_3,"Not Assigned ")
    (PC_Closeout_CommB,"close out Comm-B ")
    (PC_Uplink_Closeout,"close out uplink ELM")
    (PC_Downlink_Closeout,"close out downlink ELM")
    (PC_Not_Assigned_7,"Not Assigned ");

// Definition of RC field
const uint8_t RC_Init_Segment = 0;                  //signifies uplink ELM initial segment in MC
const uint8_t RC_Intermediate_Segment = 1;          //signifies uplink ELM intermediate segment in MC
const uint8_t RC_Final_Segment = 2;                 //signifies uplink ELM final segment in MC
const uint8_t RC_Req_Downlink_ELM = 3;              //signifies a request for downlink ELM delivery

const std::map<int8_t,std::string> RC_to_String = map_list_of
    (RC_Init_Segment,"Initial Segment")
    (RC_Intermediate_Segment,"Intermediate Segment")
    (RC_Final_Segment,"Final Segment")
    (RC_Req_Downlink_ELM,"Downlink ELM Request");

//Definition of MBS subfield
const uint8_t MBS_No_Action_CommB = 0;              //signifies no Comm-B action
const uint8_t MBS_Air_Init_CommB = 1;               //signifies air-initiated Comm-B reservation request
const uint8_t MBS_Closeout_CommB = 2;               //signifies Comm-B closeout
const uint8_t MBS_Not_Assigned   = 3;               //not assigned

// c++11
//std::map<int8_t,std::string> MBS_to_String= {
//    {MBS_No_Action_CommB,"No Action "},
//    {MBS_Air_Init_CommB,"Air Init COMM-B "},
//    {MBS_Closeout_CommB,"Closeout COMM-B "},
//    {MBS_No_Action_CommB,"Not Defined "},
//};

const std::map<int8_t,std::string> MBS_to_String = map_list_of
    (MBS_No_Action_CommB,"No COMM-B Action ")
    (MBS_Air_Init_CommB, "Air Initiated COMM-B reservation Request ")
    (MBS_Closeout_CommB, "Closeout COMM-B ")
    (MBS_No_Action_CommB,"Not Defined ");

//Definition of MES subfield
const uint8_t MES_No_Action = 0;                            //signifies no ELM action
const uint8_t MES_Uplink_Reserv = 1;                        //signifies uplink ELM reservation request
const uint8_t MES_Uplink_Closeout = 2;                      //signifies uplink ELM closeout
const uint8_t MES_Downlink_Reserv =3;                       //signifies downlink ELM reservation request
const uint8_t MES_Downlink_Closeout = 4;                    //signifies downlink ELM closeout
const uint8_t MES_Uplink_Reserv_Downlink_Closeout = 5;      //signifies uplink ELM reservation request and downlink ELM closeout
const uint8_t MES_Uplink_Closeout_Downlink_Reserv = 6;      //signifies uplink ELM closeout and downlink ELM reservation request
const uint8_t MES_Uplink_Downlink_Closeout = 7;             //signifies uplink ELM and downlink ELM closeouts

const std::map<int8_t,std::string> MES_to_String = map_list_of
    (MES_No_Action,"no ELM action ")
    (MES_Uplink_Reserv,"uplink ELM reservation request ")
    (MES_Uplink_Closeout,"uplink ELM closeout ")
    (MES_Downlink_Reserv,"downlink ELM reservation request ")
    (MES_Downlink_Closeout, "downlink ELM closeout ")
    (MES_Uplink_Reserv_Downlink_Closeout,"uplink ELM reservation request and downlink ELM closeout")
    (MES_Uplink_Closeout_Downlink_Reserv,"uplink ELM closeout and downlink ELM reservation request")
    (MES_Uplink_Downlink_Closeout,"uplink ELM and downlink ELM closeouts");

//Definition of RSS subfield
const uint8_t RSS_No_Request = 0;                           //signifies no request
const uint8_t RSS_CommB_Reserv_Status_Req = 1;              //signifies report Comm-B reservation status in UM
const uint8_t RSS_Uplink_ELM_Reserv_Status_Req = 2;         //signifies report uplink ELM reservation status in UM
const uint8_t RSS_Downlink_ELM_Reserv_Status_Req = 3;       //signifies report downlink ELM reservation status in UM

const std::map<int8_t,std::string> RSS_to_String = map_list_of
    (RSS_No_Request,"No Request ")
    (RSS_CommB_Reserv_Status_Req,"Comm-B reservation status in UM ")
    (RSS_Uplink_ELM_Reserv_Status_Req,"uplink ELM reservation status in UM ")
    (RSS_Downlink_ELM_Reserv_Status_Req,"downlink ELM reservation status in UM ");

//Definition of TCS
const uint8_t TCS_No_Commnad = 0;                           //signifies no position type command
const uint8_t TCS_Possition_for_15s = 1;                    //signifies use surface position type for the next 15 seconds
const uint8_t TCS_Possition_for_60s = 2;                    //signifies use surface position type for the next 60 seconds
const uint8_t TCS_Cancel_Command =3;                        //signifies cancel surface type command
const uint8_t TCS_Not_Assigned_4 = 4;                       //not assigned
const uint8_t TCS_Not_Assigned_5 = 5;                       //not assigned
const uint8_t TCS_Not_Assigned_6 = 6;                       //not assigned
const uint8_t TCS_Not_Assigned_7 = 7;                       //not assigned

const std::map<int8_t,std::string> TCS_to_String = map_list_of
    (TCS_No_Commnad,"No Command ")
    (TCS_Possition_for_15s,"Possition for 15s ")
    (TCS_Possition_for_60s,"Possition for 15s ")
    (TCS_Cancel_Command,"Command Cancel ")
    (TCS_Not_Assigned_4,"Not Assigned ")
    (TCS_Not_Assigned_5,"Not Assigned ")
    (TCS_Not_Assigned_6,"Not Assigned ")
    (TCS_Not_Assigned_7,"Not Assigned ");

//Definition of RCS subfield
const uint8_t RCS_No_Command = 0;                           //signifies no surface squitter rate command
const uint8_t RCS_Hi_Rate_for_60s = 1;                      //signifies report high surface squitter rate for 60 seconds
const uint8_t RCS_Lo_Rate_for_60s = 2;                      //signifies report low surface squitter rate for 60 seconds
const uint8_t RCS_Suppress_for_60s = 3;                     //signifies suppress all surface squitters for 60 seconds
const uint8_t RCS_Suppress_for_120s = 4;                    //signifies suppress all surface squitters for 120 seconds
const uint8_t RCS_Not_Assigned_5 = 5;                       //not assigned
const uint8_t RCS_Not_Assigned_6 = 6;                       //not assigned
const uint8_t RCS_Not_Assigned_7 = 7;                       //not assigned

const std::map<int8_t,std::string> RCS_to_String = map_list_of
    (RCS_No_Command,"No Command ")
    (RCS_Hi_Rate_for_60s,"high squitter rate for 60 seconds ")
    (RCS_Lo_Rate_for_60s,"low squitter rate for 60 seconds ")
    (RCS_Suppress_for_60s,"suppress squitters for 60 seconds")
    (RCS_Suppress_for_120s,"suppress squitters for 120 seconds")
    (RCS_Not_Assigned_5,"Not Assigned ")
    (RCS_Not_Assigned_6,"Not Assigned ")
    (RCS_Not_Assigned_7,"Not Assigned ");


//Definition of SAS subfield
const uint8_t SAS_No_Command = 0;                           //signifies no antenna command
const uint8_t SAS_Alternate_Ant_for_120s = 1;               //signifies alternate top and bottom antennas for 120 seconds
const uint8_t SAS_Use_Bottom_Ant_for_120s = 2;              //signifies use bottom antenna for 120 seconds
const uint8_t SAS_Use_Default = 3;                          //signifies return to the default

const std::map<int8_t,std::string> SAS_to_String = map_list_of
    (SAS_No_Command,"No Command ")
    (SAS_Alternate_Ant_for_120s,"alternate top and bottom antennas for 120 seconds ")
    (SAS_Use_Bottom_Ant_for_120s,"use bottom antenna for 120 seconds ")
    (SAS_Use_Default,"antenna return to the default");

struct sd_di0
{
    uint16_t NU1:4;         //Not used
    uint16_t OVC:1;         //Overlay control
    uint16_t NU :7;
    uint16_t IIS:4;         //Interrogator identifier subfield

    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

struct sd_di1
{
    uint16_t TMS:4;         //Tactical message subfield
    uint16_t RSS:2;         //Reservation status subfield
    uint16_t LOS:1;         //Lockout subfield
    uint16_t MES:3;         //Multisite ELM subfield
    uint16_t MBS:2;         //Multisite Comm-B subfield
    uint16_t IIS:4;         //Interrogator identifier subfield

    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

struct sd_di2
{
    uint16_t NU1:4;         //Not used
    uint16_t SAS:2;         //surface antenna subfield
    uint16_t RCS:3;         //Rate control subfield
    uint16_t TCS:3;         //Type control subfield
    uint16_t NU:4;
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

struct sd_di3
{
    uint16_t NU1:4;         //Not used
    uint16_t OVC:1;         //Overlay control
    uint16_t RRS:4;         //Reply request subfield (contain the BDS2 code of a requested GICB register)
    uint16_t LSS:1;         //Lockout surveillance subfield (Lockout for SI code)
    uint16_t SIS:6;         //Surveillance identifier subfield (SI code)
    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};

struct sd_di7
{
    uint16_t TMS:4;         //Tactical message subfield
    uint16_t OVC:1;         //Overlay control
    uint16_t NU1:1;         //Not used
    uint16_t LOS:1;         //Lockout subfield
    uint16_t NU:1;          //Not Used
    uint16_t RRS:4;         //Reply request subfield (contain the BDS2 code of a requested GICB register)
    uint16_t IIS:4;         //Interrogator identifier subfield

    void to_stringstream(std::stringstream &out, bool one_line = false) const;
};


#endif // UF_SUBFIELDS_HH_INCLUDED
