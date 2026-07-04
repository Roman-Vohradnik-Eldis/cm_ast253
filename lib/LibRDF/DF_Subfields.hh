#ifndef DF_SUBFIELDS_HH_INCLUDED
#define DF_SUBFIELDS_HH_INCLUDED
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <map>
#include "boost/assign.hpp"
#include <string>
#include <math.h>

using namespace boost::assign;

unsigned asterix_decode_mode123 (unsigned code);
unsigned asterix_encode_mode123 (unsigned value);

//Definition of CA field
const uint8_t CA_Level1 = 0;                        //signifies Level 1 transponder (surveillance only), and no ability to set CA code 7 and either airborne or on the ground
const uint8_t CA_Not_Assigned_1 = 1;                //reserved
const uint8_t CA_Not_Assigned_2 = 2;                //reserved
const uint8_t CA_Not_Assigned_3 = 3;                //reserved
const uint8_t CA_Level2_GND = 4;                    //signifies Level 2 or above transponder and ability to set CA code 7 and on the ground
const uint8_t CA_Level2_AIR = 5;                    //signifies Level 2 or above transponder and ability to set CA code 7 and airborne
const uint8_t CA_Level2_AIR_GND = 6;                //signifies Level 2 or above transponder and ability to set CA code 7 and either airborne or on the ground
const uint8_t CA_DR_Set = 7;                        //signifies the DR field is not equal to 0 or the FS field equals 2, 3, 4 or 5, and either airborne or on the ground

//Definition of CF field
const uint8_t CF_0 = 0;
const uint8_t CF_1 = 1;
const uint8_t CF_2 = 2;
const uint8_t CF_3 = 3;
const uint8_t CF_4 = 4;
const uint8_t CF_5 = 5;
const uint8_t CF_6 = 6;
const uint8_t CF_7 = 7;

//Definition of AF field
const uint8_t AF_0 = 0;
const uint8_t AF_1 = 1;
const uint8_t AF_2 = 2;
const uint8_t AF_3 = 3;
const uint8_t AF_4 = 4;
const uint8_t AF_5 = 5;
const uint8_t AF_6 = 6;
const uint8_t AF_7 = 7;

const std::map<int8_t,std::string> CA_to_String = map_list_of
    (CA_Level1,"Level 1 transponder ")
    (CA_Not_Assigned_1,"Not Assigned ")
    (CA_Not_Assigned_2,"Not Assigned ")
    (CA_Not_Assigned_3,"Not Assigned ")
    (CA_Level2_GND,"Level 2 Ground")
    (CA_Level2_AIR,"Level 2 Airborne")
    (CA_Level2_AIR_GND,"Level 2")
    (CA_DR_Set,"DR Set");

const std::map<int8_t,std::string> CF_to_String = map_list_of
    (CF_0,"ADS-B with ICAO AA ")
    (CF_1,"ADS-B with anonymous AA ")
    (CF_2,"Fine TIS-B ")
    (CF_3,"Coarse TIS-B ")
    (CF_4,"TIS-B and ADS-R Management ")
    (CF_5,"Fine TIS-B Message ")
    (CF_6,"ADS-R ")
    (CF_7,"Reserved ");

const std::map<int8_t,std::string> AF_to_String = map_list_of
    (AF_0,"ADS-B ")
    (AF_1,"Reserved ")
    (AF_2,"Reserved ")
    (AF_3,"Reserved ")
    (AF_4,"Reserved ")
    (AF_5,"Reserved ")
    (AF_6,"Reserved ")
    (AF_7,"Reserved ");

//Definition of FS field
const uint8_t FS_AIR = 0;                           //signifies no alert and no SPI, aircraft is airborne
const uint8_t FS_GND = 1;                           //signifies no alert and no SPI, aircraft is on the ground
const uint8_t FS_Alert_AIR = 2;                     //signifies alert, no SPI, aircraft is airborne
const uint8_t FS_Alert_GND = 3;                     //signifies alert, no SPI, aircraft is on the ground
const uint8_t FS_Alert_SPI = 4;                     //signifies alert and SPI, aircraft is airborne or on the ground
const uint8_t FS_SPI = 5;                           //signifies no alert and SPI, aircraft is airborne or on the ground
const uint8_t FS_Not_Assigned_6 = 6;                //not assigned
const uint8_t FS_Not_Assigned_7 = 7;                //not assigned

const std::map<int8_t,std::string> FS_to_String = map_list_of
    (FS_AIR,"Airborne ")
    (FS_GND,"Ground ")
    (FS_Alert_AIR,"Airborne Alert ")
    (FS_Alert_GND,"Ground Alert ")
    (FS_Alert_SPI,"Alert and SPI")
    (FS_SPI,"SPI ")
    (FS_Not_Assigned_6," ")
    (FS_Not_Assigned_7," ");

//Definition of DR field
const uint8_t DR_No_Request = 0;                    //signifies no downlink request
const uint8_t DR_Commb_Request = 1;                 //signifies request to send Comm-B message
const uint8_t DR_Broadcast_1 = 4;                   //signifies Comm-B broadcast message 1 available
const uint8_t DR_Broadcast_2 = 5;                   //signifies Comm-B broadcast message 2 available


const uint8_t ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL = 1;
const uint8_t ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC = 2;
const uint8_t ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL = 3;
const uint8_t ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC = 4;

const uint8_t ME_AIRCRAFT_OPERATIONAL_STATUS_AIRBORNE = 0;
const uint8_t ME_AIRCRAFT_OPERATIONAL_STATUS_SURFACE = 1;

const std::map<int8_t,std::string> ME_AIRBORNE_VELOCITY_SUBTYPE_to_String = map_list_of
    (0,"Reserved ")
    (ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL,"Ground speed Normal ")
    (ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC, "Ground speed Supersonic ")
    (ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL,"Air speed Normal ")
    (ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC, "Air speed Supersonic ")
    (5,"Reserved ")
    (6,"Reserved ")
    (7,"Reserved ");

const uint8_t ME_AIRBORNE_VELOCITY_EAST_WEST_DIRECTION_EAST = 0;
const uint8_t ME_AIRBORNE_VELOCITY_EAST_WEST_DIRECTION_WEST = 1;

const uint8_t ME_AIRBORNE_VELOCITY_NORTH_SOUTH_DIRECTION_NORTH = 0;
const uint8_t ME_AIRBORNE_VELOCITY_NORTH_SOUTH_DIRECTION_SOUTH = 1;

const uint8_t ME_AIRBORNE_VELOCITY_VERTICAL_RATE_SOURCE_GEOMETRIC = 0;
const uint8_t ME_AIRBORNE_VELOCITY_VERTICAL_RATE_SOURCE_BAROMETRIC = 1;

const uint8_t ME_AIRBORNE_VELOCITY_VERTICAL_RATE_SIGN_UP = 0;
const uint8_t ME_AIRBORNE_VELOCITY_VERTICAL_RATE_SIGN_DOWN = 1;

const uint8_t ME_AIRBORNE_VELOCITY_HEADING_STATUS_DATA_NOT_VALID = 0;  // Heading data NOT Available
const uint8_t ME_AIRBORNE_VELOCITY_HEADING_STATUS_DATA_VALID = 1;  // Heading data Available

const uint8_t ME_AIRBORNE_VELOCITY_AIR_SPEED_TYPE_IAS = 0;      //Indicated AIR SPEED
const uint8_t ME_AIRBORNE_VELOCITY_AIR_SPEED_TYPE_TAS = 1;      //True AIR SPEED

struct me_airborne_position {
    uint8_t NIC_B           :1;
    uint8_t SS              :2;
    uint8_t TC              :5;
    
    uint8_t AC_HI           :8;
    
    uint8_t CPR_LAT_HI      :2;
    uint8_t F               :1; 
    uint8_t T               :1;
    uint8_t AC_LO           :4;
    
    uint8_t CPR_LAT_MID     :8;
    
    uint8_t CPR_LON_HI      :1;
    uint8_t CPR_LAT_LO      :7;
    
    uint8_t CPR_LON_MID     :8;
    
    uint8_t CPR_LON_LO      :8;    
};

struct me_surface_position {
    uint8_t MOVEMENT_HI     :3;
    uint8_t TC              :5;
    
    uint8_t HEADING_HI      :3;
    uint8_t HEADING_VALID   :1;
    uint8_t MOVEMENT_LO     :4;
    
    uint8_t CPR_LAT_HI      :2;
    uint8_t F               :1; 
    uint8_t T               :1;
    uint8_t HEADING_LO      :4;
    
    uint8_t CPR_LAT_MID     :8;
    
    uint8_t CPR_LON_HI      :1;
    uint8_t CPR_LAT_LO      :7;
    
    uint8_t CPR_LON_MID     :8;
    
    uint8_t CPR_LON_LO      :8;   
};

struct me_aircraft_ident {
    uint8_t AIRCRAFT_TYPE   :3; 
    uint8_t TC              :5;
    
    uint8_t CHAR1_HI        :2;
    uint8_t CHAR0           :6;
    
    uint8_t CHAR2_HI        :4;
    uint8_t CHAR1_LO        :4;
    
    uint8_t CHAR3           :6;
    uint8_t CHAR2_LO        :2;
    
    uint8_t CHAR5_HI        :2;
    uint8_t CHAR4           :6;
    
    uint8_t CHAR6_HI        :4;
    uint8_t CHAR5_LO        :4;
    
    uint8_t CHAR7           :6;
    uint8_t CHAR6_LO        :2;
};

struct me_airborne_velocity_1_2 {
    uint8_t SUBTYPE                 :3;
    uint8_t TC                      :5;
    
    uint8_t EAST_WEST_VELOCITY_HI   :2;
    uint8_t EAST_WEST_DIRECTION     :1;
    uint8_t NACV                    :3;
    uint8_t RESERVED_A              :1;
    uint8_t INTENT                  :1;
    
    uint8_t EAST_WEST_VELOCITY_LO   :8;
    
    uint8_t NORTH_SOUTH_VELOCITY_HI :7;
    uint8_t NORTH_SOUTH_DIRECTION   :1;
    
    uint8_t VERTICAL_RATE_HI        :3;
    uint8_t VERTICAL_RATE_SIGN      :1;
    uint8_t VERTICAL_RATE_SOURCE    :1;
    uint8_t NORTH_SOUTH_VELOCITY_LO :3;

    uint8_t RESERVED_B              :2;
    uint8_t VERTICAL_RATE_LO        :6;
    
    uint8_t DIFF_BARO               :7;
    uint8_t DIFF_BARO_SIGN          :1;
};

struct me_airborne_velocity_3_4 {
    uint8_t SUBTYPE                 :3;
    uint8_t TC                      :5;
    
    uint8_t HEADING_HI              :2;
    uint8_t HEADING_STATUS          :1;
    uint8_t NACV                    :3;
    uint8_t RESERVED_A              :1;
    uint8_t INTENT                  :1;
    
    uint8_t HEADING_LO              :8;
    
    uint8_t AIR_SPEED_HI            :7;
    uint8_t AIR_SPEED_TYPE          :1;
    
    uint8_t VERTICAL_RATE_HI        :3;
    uint8_t VERTICAL_RATE_SIGN      :1;
    uint8_t VERTICAL_RATE_SOURCE    :1;
    uint8_t AIR_SPEED_LO            :3;

    uint8_t RESERVED_B              :2;
    uint8_t VERTICAL_RATE_LO        :6;
    
    uint8_t DIFF_BARO               :7;
    uint8_t DIFF_BARO_SIGN          :1;
};

struct me_aircraft_status {
    uint8_t SUBTYPE                 :3;
    uint8_t TC                      :5;
    uint8_t MODE_A_HI               :5;
    uint8_t EMERGENCY               :3;
    uint8_t MODE_A_LO               :8;
    uint8_t RESERVED[4];
};

struct me_operational_status_surface {
    uint8_t SUBTYPE                 :3;
    uint8_t TC                      :5;

    uint8_t CC_UAT_IN               :1;
    uint8_t CC_B2_LOW               :1;
    uint8_t CC_RESERVED_2           :2;
    uint8_t CC_ES_IN                :1;
    uint8_t CC_RESERVED_1           :1;
    uint8_t CC_FORMAT               :2;
    uint8_t LW                      :4;
    uint8_t CC_NIC_C                :1;
    uint8_t CC_NACV                 :3;

    uint8_t OM_SDA                  :2;
    uint8_t OM_SINGLE_ANT           :1;
    uint8_t OM_RAS                  :1;
    uint8_t OM_IDENT                :1;
    uint8_t OM_TCAS_RA              :1;
    uint8_t OM_FORMAT               :2;
    uint8_t OM_LON_ANT_OFFSET       :5;
    uint8_t OM_LAT_ANT_OFFSET       :3;

    uint8_t NACP                    :4;
    uint8_t NIC_A                   :1;
    uint8_t MOPS_VER                :3;
    uint8_t RESERVED_2              :1;
    uint8_t SIL_SUPP                :1;
    uint8_t HRD                     :1;
    uint8_t TRK_HDG                 :1;
    uint8_t SIL                     :2;
    uint8_t RESERVED_1              :2;
};

struct me_operational_status_airborne {
    uint8_t SUBTYPE                 :3;
    uint8_t TC                      :5;

    uint8_t CC_TS                   :1;
    uint8_t CC_ARV                  :1;
    uint8_t CC_RESERVED_1           :2;
    uint8_t CC_ES_IN                :1;
    uint8_t CC_TCAS                 :1;
    uint8_t CC_FORMAT               :2;
    uint8_t CC_RESERVED_2           :5;
    uint8_t CC_UAT_IN               :1;
    uint8_t CC_TC                   :2;

    uint8_t OM_SDA                  :2;
    uint8_t OM_SINGLE_ANT           :1;
    uint8_t OM_RAS                  :1;
    uint8_t OM_IDENT                :1;
    uint8_t OM_TCAS_RA              :1;
    uint8_t OM_FORMAT               :2;
    uint8_t OM_RESERVED             :8;

    uint8_t NACP                    :4;
    uint8_t NIC_A                   :1;
    uint8_t MOPS_VER                :3;
    uint8_t RESERVED                :1;
    uint8_t SIL_SUPP                :1;
    uint8_t HRD                     :1;
    uint8_t NIC_BARO                :1;
    uint8_t SIL                     :2;
    uint8_t GVA                     :2;
};

struct me_target_state_and_status0 {
    uint8_t VDATA_HI                :1;
    uint8_t SUBTYPE                 :2;
    uint8_t TC                      :5;

    uint8_t TARGET_ALT_HI           :1;
    uint8_t VMODE                   :2;
    uint8_t TARGET_ALT_CAPABILITY   :2;
    uint8_t COMPATIBILITY           :1;
    uint8_t TARGET_ALT_TYPE         :1;
    uint8_t VDATA_LO                :1;

    uint8_t TARGET_ALT_MID          :8;

    uint8_t TARGET_HEADING_HI       :5;
    uint8_t HDATA                   :2;
    uint8_t TARGET_ALT_LO           :1;

    uint8_t NACP_HI                 :1;
    uint8_t HMODE                   :2;
    uint8_t TARGET_HEADING_TRACK    :1;
    uint8_t TARGET_HEADING_LO       :4;

    uint8_t RESERVED_HI             :2;
    uint8_t SIL                     :2;
    uint8_t NIC_BARO                :1;
    uint8_t NACP_LO                 :3;

    uint8_t EMERGENCY               :3;
    uint8_t CAPABILITY              :2;
    uint8_t RESERVED_LO             :3;
};

struct me_target_state_and_status1 {
    uint8_t SIL_SUPP                :1;
    uint8_t SUBTYPE                 :2;
    uint8_t TC                      :5;

    uint8_t SEL_ALT_HI              :7;
    uint8_t SEL_ALT_TYPE            :1;

    uint8_t BARO_HI                 :4;
    uint8_t SEL_ALT_LO              :4;

    uint8_t SEL_HEADING_HI          :2;
    uint8_t SEL_HEADING_STATUS      :1;
    uint8_t BARO_LO                 :5;

    uint8_t NACP_HI                 :1;
    uint8_t SEL_HEADING_LO          :7;

    uint8_t AUTOPILOT               :1;
    uint8_t MCP_FCU                 :1;
    uint8_t SIL                     :2;
    uint8_t NIC_BARO                :1;
    uint8_t NACP_LO                 :3;

    uint8_t RESERVED_2              :2;
    uint8_t LNAV                    :1;
    uint8_t TCAS                    :1;
    uint8_t APPROACH                :1;
    uint8_t RESERVED_1              :1;
    uint8_t ALT_HOLD                :1;
    uint8_t VNAV                    :1;
};

struct me_airborne_position_ac_subfield {
    uint16_t ALT_LO         : 4;
    uint16_t Q              : 1;
    uint16_t ALT_HI         : 7;
    uint16_t none           : 4;
    
    uint16_t operator=(uint16_t value);
    
    void set(uint16_t value);
    operator uint16_t() const { return(get());} 
    uint16_t get() const;
    void set_Alt (int32_t feets);
    int32_t get_Alt() const;
};

const double CPR_2NB_17 = 131072.0;
const double CPR_2NB_19 = 524288.0;

class me_cpr_position_subfields {
public:    
    uint8_t T;          // Time :1
    uint8_t F;          // CPR Format :1 0-even, 1-odd
    int32_t CPR_LAT;    // CPR Latitude :17
    int32_t CPR_LON;    // CPR Longitude :17
    
    void encode_Airborne_LatLon (double Lat, double Lon);
    void encode_Surface_LatLon (double Lat, double Lon);
    bool decode_Airborne_LatLon(const me_cpr_position_subfields & pair,double &Lat, double &Lon) const;
    bool decode_Local_LatLon (double Ref_Lat, double Ref_Lon, double &Lat, double &Lon, bool airborne) const;
    bool decode_Surface_LatLon(const me_cpr_position_subfields & pair,double &Lat, double &Lon) const;
//    bool decode_Surface_LatLon (const double &Ref_Lat, double &Ref_Lon, double &Lat, double &Lon) const;
private:
    void encode_LatLon (double Lat, double Lon, double NB);
    int cpr_NL(double lat) const { return cpr_NL_precomputed(lat);};
    int cpr_NL_precomputed(double lat) const;
    int cpr_NL_computed(double lat) const;
    double cpr_MOD(double num,double denom) const;
};

class me_airborne_position_fields : public me_cpr_position_subfields {
public:
    uint8_t                             SS;         // Surveilance status :2
    uint8_t                             NIC_B;      // NIC Supplement-B :1, DO-260, DO260A: NIC_B -> SINGLE_ANT
    me_airborne_position_ac_subfield    AC;         // Altitude_Code:12
    
    void set_LatLon (double Lat, double Long);
    bool get_LatLon (const me_cpr_position_subfields & pair, double &Lat, double &Lon) const {return decode_Airborne_LatLon(pair,Lat,Lon);};
    bool get_LatLon (double Ref_Lat, double Ref_Lon, double &Lat, double &Lon) const {return decode_Local_LatLon(Ref_Lat,Ref_Lon,Lat,Lon,true);};
};

class me_surface_position_fields :public me_cpr_position_subfields {
public:   
    uint8_t                             Movement;
    uint8_t                             Heading_Valid;
    uint8_t                             Heading;
    
    void set_Movement_unknown () { Movement = 0; };
    void set_Movement_decelarate () { Movement = 125; };
    void set_Movement_acecelarate () { Movement = 126; };
    void set_Movement_speed_km_h (double speed);
    bool get_Movement_speed_km_h (double &speed);
    void set_Movement_speed_knot (double speed) { speed=speed*1.852; set_Movement_speed_km_h(speed); }
    bool get_Movement_speed_knot (double &speed) { speed=speed/1.852; return get_Movement_speed_km_h(speed); }
    void set_Heading_deg(double heading) { Heading = round((heading/360.0)*128); }
    double get_Heading_deg() { return ((Heading*360.0)/128); }
    void set_LatLon (double Lat, double Lon); 
    bool get_LatLon (const me_cpr_position_subfields & pair, double &Lat, double &Lon) const {return decode_Surface_LatLon(pair,Lat,Lon);};
    bool get_LatLon (double Ref_Lat, double Ref_Lon, double &Lat, double &Lon) const {return decode_Local_LatLon(Ref_Lat,Ref_Lon,Lat,Lon,false);};
};

struct me_aircraft_ident_fields {
    uint8_t                         Aircraft_type;  //:3
    uint8_t                         Callsign[8];    // 8* 6bits (8 characters)
    char decode_char(uint8_t coded_ch);
    uint8_t encode_char(char ch);
	void encode_Callsing(uint8_t *callsign);
	void decode_Callsing(uint8_t *callsign);
};

struct me_velocity_1_2 {
    uint8_t EAST_WEST_DIRECTION;    // :1
    uint16_t EAST_WEST_VELOCITY;    // :10
    uint8_t NORTH_SOUTH_DIRECTION;  // :1
    uint16_t NORTH_SOUTH_VELOCITY;  // :10
    void set_EAST_WEST_VELOCITY_unknown() { EAST_WEST_VELOCITY=0;};
    void set_EAST_WEST_VELOCITY_NORMAL_knot(double value) { 
        if (value > 1021.0) { EAST_WEST_VELOCITY= 1023; return;};
        if (value < 0) { set_EAST_WEST_VELOCITY_unknown(); return;};
        EAST_WEST_VELOCITY = round(value) +1;
    };
    void set_EAST_WEST_VELOCITY_NORMAL_km_h(double value) {
        set_EAST_WEST_VELOCITY_NORMAL_knot(value/1.852);
    };
    void set_EAST_WEST_VELOCITY_SUPERSONIC_knot(double value) { 
        set_EAST_WEST_VELOCITY_NORMAL_knot(value/4);
    };
    void set_EAST_WEST_VELOCITY_SUPERSONIC_km_h(double value) {
        set_EAST_WEST_VELOCITY_NORMAL_knot(value/1.852/4);
    };
    bool get_EAST_WEST_VELOCITY_NORMAL_knot(double & value) {
        if (EAST_WEST_VELOCITY==0) return false;
        value = EAST_WEST_VELOCITY-1;
        return true;
    };   
    bool get_EAST_WEST_VELOCITY_SUPERSONIC_knot(double & value) {
        if (EAST_WEST_VELOCITY==0) return false;
        value = (EAST_WEST_VELOCITY-1)*4;
        return true;
    }
    
    void set_NORTH_SOUTH_VELOCITY_unknown() { NORTH_SOUTH_VELOCITY=0;};
    void set_NORTH_SOUTH_VELOCITY_NORMAL_knot(double value) { 
        if (value > 1021.0) { NORTH_SOUTH_VELOCITY= 1023; return;};
        if (value < 0) { set_NORTH_SOUTH_VELOCITY_unknown(); return;};
        NORTH_SOUTH_VELOCITY = round(value) +1;
    };
    void set_NORTH_SOUTH_VELOCITY_NORMAL_km_h(double value) {
        set_NORTH_SOUTH_VELOCITY_NORMAL_knot(value/1.852);
    };
    void set_NORTH_SOUTH_VELOCITY_SUPERSONIC_knot(double value) { 
        set_NORTH_SOUTH_VELOCITY_NORMAL_knot(value/4);
    };
    void set_NORTH_SOUTH_VELOCITY_SUPERSONIC_km_h(double value) {
        set_NORTH_SOUTH_VELOCITY_NORMAL_knot(value/1.852/4);
    };
    bool get_NORTH_SOUTH_VELOCITY_NORMAL_knot(double & value) {
        if (NORTH_SOUTH_VELOCITY==0) return false;
        value = NORTH_SOUTH_VELOCITY-1;
        return true;
    };   
    bool get_NORTH_SOUTH_VELOCITY_SUPERSONIC_knot(double & value) {
        if (NORTH_SOUTH_VELOCITY==0) return false;
        value = (NORTH_SOUTH_VELOCITY-1)*4;
        return true;
    }
};

struct me_velocity_3_4 {
    uint8_t HEADING_STATUS;         // :1
    uint16_t HEADING;               // :10
    uint8_t AIR_SPEED_TYPE;         // :1
    uint16_t AIR_SPEED;             // :10
    
    void set_AIR_SPEED_unknown() { AIR_SPEED = 0;};
    
    void set_AIR_SPEED_NORMAL_knot(double value) {
        if (value > 1021.0) { AIR_SPEED= 1023; return;};
        if (value < 0) { set_AIR_SPEED_unknown(); return;};
        AIR_SPEED = round(value) +1;
    };
    void set_AIR_SPEED_NORMAL_km_h(double value) {
        set_AIR_SPEED_NORMAL_knot(value/1.852);
    };
    void set_AIR_SPEED_SUPERSONIC_knot(double value) {
        set_AIR_SPEED_NORMAL_knot(value/4);
    };
    void set_AIR_SPEED_SUPERSONIC_km_h(double value) {
        set_AIR_SPEED_NORMAL_knot(value/1.852/4);
    };
    bool get_AIR_SPEED_NORMAL_knot(double & value) {
        if (AIR_SPEED==0) return false;
        value = AIR_SPEED-1;
        return true;
    };   
    bool get_AIR_SPEED_SUPERSONIC_knot(double & value) {
        if (AIR_SPEED==0) return false;
        value = (AIR_SPEED-1)*4;
        return true;
    }
    void set_HEADING(double value) {HEADING = round((value/360.0)*1024);};
    double get_HEADING() {return ((HEADING *360.0) /1024);  };
    
};

union me_velocity_1_2_3_4 {
    me_velocity_1_2 SUBTYPE_1_2;
    me_velocity_3_4 SUBTYPE_3_4;
};

struct me_airborne_velocity_fields {
    uint8_t                         SUBTYPE;                // :3
    uint8_t                         INTENT;                 // :1
    uint8_t                         RESERVED_A;             // :1, DO-260, DO-260A: RESERVED_A -> IFR
    uint8_t                         NACV;                   // :3, DO-260: NACV -> NUCR
    me_velocity_1_2_3_4             VELOCITY;
    uint8_t                         VERTICAL_RATE_SOURCE;   // :1
    uint8_t                         VERTICAL_RATE_SIGN;     // :1
    uint16_t                        VERTICAL_RATE;          // :9
    uint8_t                         RESERVED_B;             // :2
    uint8_t                         DIFF_BARO_SIGN;         // :1
    uint8_t                         DIFF_BARO;              // :7
    
    void set_GROUND_VELOCITY_unknown(); 
    void set_GROUND_VELOCITY_knot(double east_west, double north_south);
    void set_GROUND_VELOCITY_km_h(double east_west, double north_south);
    bool get_EAST_WEST_VELOCITY_knot(double value);
    bool get_NORTH_SOUTH_VELOCITY_knot(double value);
    void set_AIR_SPEED_unknown();
    void set_AIR_SPEED_knot(double value);
    void set_AIR_SPEED_km_h(double value);
    void set_AIR_HEADING(double value);
    bool get_AIR_SPEED_knot(double & value);
    bool get_AIR_HEADING(double & value);
    void set_VERTICAL_RATE_unknown();
    void set_VERTICAL_RATE_feet_min(double value);
    bool get_VERTICAL_RATE_feet_min(double & value);
    void set_DIFF_BARO_unknown();
    void set_DIFF_BARO_feet(uint16_t value);
    bool get_DIFF_BARO_feet(uint16_t &value);
};

struct me_m3a_subfield
{
    uint16_t M3A;            // :13, in format C1|A1|C2|A2|C4|A4|ZERO|B1|D1|B2|D2|B4|D4

    uint16_t get_M3A_octal();                               // C1|A1|C2|A2|C4|A4|ZERO|B1|D1|B2|D2|B4|D4 -> A4|A2|A1|B4|B2|B1|C4|C2|C1|D4|D2|D1
    uint16_t get_M3A() { return asterix_decode_mode123(get_M3A_octal()); }

    void set_M3A_octal(uint16_t m3a);                       // A4|A2|A1|B4|B2|B1|C4|C2|C1|D4|D2|D1 -> C1|A1|C2|A2|C4|A4|ZERO|B1|D1|B2|D2|B4|D4
    void set_M3A(uint16_t m3a) { set_M3A_octal(asterix_encode_mode123(m3a)); }
};

struct me_aircraft_status_fields : public me_m3a_subfield {
    uint8_t                         SUBTYPE;                // :3
    uint8_t                         EMERGENCY;              // :8
};

struct me_operational_cc_airbrone {
    uint8_t                         FORMAT;                 // :2
    uint8_t                         TCAS;                   // :1, DO-260A: TCAS -> NOT_TCAS
    uint8_t                         ES_IN;                  // :1, DO-260A: ES_IN -> CDTI
    uint8_t                         ARV;                    // :1
    uint8_t                         TS;                     // :1
    uint8_t                         TC;                     // :2
    uint8_t                         UAT_IN;                 // :1
};

struct me_operational_cc_surface {
    uint8_t                         FORMAT;                 // :2
    uint8_t                         RESERVED;               // :1,  DO-260A: RESERVED -> POA
    uint8_t                         ES_IN;                  // :1,  DO-260A: ES_IN -> CDTI
    uint8_t                         B2_LOW;                 // :1
    uint8_t                         UAT_IN;                 // :1
    uint8_t                         NACV;                   // :3
    uint8_t                         NIC_C;                  // :1
};

union me_operational_cc_0_1 {
    me_operational_cc_airbrone AIRBORNE;
    me_operational_cc_surface  SURFACE;
};

struct me_operational_om_airbrone {
    uint8_t                         FORMAT;                 // :2
    uint8_t                         TCAS_RA;                // :1
    uint8_t                         IDENT;                  // :1
    uint8_t                         RAS;                    // :1
    uint8_t                         SINGLE_ANT;             // :1
    uint8_t                         SDA;                    // :1
};

struct me_operational_om_surface : public me_operational_om_airbrone {
    uint8_t                         LAT_ANT_OFFSET;         // :3
    uint8_t                         LON_ANT_OFFSET;         // :5
};

union me_operational_om_0_1 {
    me_operational_om_airbrone AIRBORNE;
    me_operational_om_surface  SURFACE;
};

struct me_aircraft_operational_status_fields {
    uint8_t                         SUBTYPE;                // :3
    me_operational_cc_0_1           CC;
    uint8_t                         LW;                     // :4
    me_operational_om_0_1           OM;
    uint8_t                         MOPS_VER;               // :3
    uint8_t                         NIC_A;                  // :1, DO-260A: NIC_A -> NIC_SUPP
    uint8_t                         NACP;                   // :4
    uint8_t                         GVA;                    // :2
    uint8_t                         SIL;                    // :2
    uint8_t                         NIC_BARO;               // :1
    uint8_t                         TRK_HDG;                // :1
    uint8_t                         HRD;                    // :1
    uint8_t                         SIL_SUPP;               // :1
};

struct me_target_state_and_status_subtype0
{
    uint8_t                         VDATA;                  // :2
    uint16_t                        TARGET_ALT_TYPE;        // :1
    uint8_t                         TARGET_ALT_CAPABILITY;  // :2
    uint8_t                         VMODE;                  // :2
    uint16_t                        TARGET_ALT;             // :10
    uint8_t                         HDATA;                  // :2
    uint16_t                        TARGET_HEADING;         // :9
    uint8_t                         TARGET_HEADING_TRACK;   // :1
    uint8_t                         HMODE;                  // :2
    uint8_t                         NACP;                   // :4
    uint8_t                         NIC_BARO;               // :1
    uint8_t                         SIL;                    // :2
    uint8_t                         CAPABILITY;             // :2
    uint8_t                         EMERGENCY;              // :3

    bool is_TARGET_ALT_invalid() { return TARGET_ALT > 1010; }
	void set_TARGET_ALT_unknown() { TARGET_ALT = 0; }
    void set_TARGET_ALT_feet(double value) {
        if (value > 100000) { TARGET_ALT = 1011; return; }
        TARGET_ALT = (value + 1000)/100;
    }
    bool get_TARGET_ALT_feet(double &value) {
        if (TARGET_ALT > 1010) { value = 0; return false; }
        value = ((TARGET_ALT*100) - 1000);
        return true;
    }
    bool is_TARGET_HEADING_invalid() { return TARGET_HEADING > 359; }
	void set_TARGET_HEADING(double value) { TARGET_HEADING = round(value); }
    double get_TARGET_HEADING() {
		if (TARGET_HEADING > 359) { return 0; }
		return TARGET_HEADING;
	}
};

struct me_target_state_and_status_subtype1
{
    uint8_t                         SIL_SUPP;               // :1
    uint16_t                        SEL_ALT;                // :11
    uint8_t                         SEL_ALT_TYPE;           // :1
    uint16_t                        BARO;                   // :9
    uint8_t                         SEL_HEADING_STATUS;     // :1
    uint16_t                        SEL_HEADING;            // :9
    uint8_t                         NACP;                   // :4
    uint8_t                         NIC_BARO;               // :1
    uint8_t                         SIL;                    // :2
    uint8_t                         MCP_FCU;                // :1
    uint8_t                         AUTOPILOT;              // :1
    uint8_t                         VNAV;                   // :1
    uint8_t                         ALT_HOLD;               // :1
    uint8_t                         APPROACH;               // :1
    uint8_t                         TCAS;                   // :1
    uint8_t                         LNAV;                   // :1

    void set_SEL_ALT_unknown() { SEL_ALT = 0; }
    void set_SEL_ALT_feet(double value) {
        if (value > 65472) { SEL_ALT = 2047; return; }
        SEL_ALT = round(value/32.0) + 1;
    }
    bool get_SEL_ALT_feet(double &value) {
        if (SEL_ALT == 0) { value = 0; return false; }
        value = ((SEL_ALT - 1)*32);
        return true;
    }

    void set_BARO_unknown() { BARO = 0; }
    void set_BARO_milibar(double value) {
        if (value < 800.0 || value > 1208.4) { BARO = 0; return; }
        BARO = round((value -= 800.0)/0.8) + 1;
    }
    bool get_BARO_milibar(double &value) {
        if (BARO == 0) { value = 0; return false; }
        value = (((BARO - 1)*0.8) + 800.0);
        return true;
    }

   void set_SEL_HEADING(double value) { SEL_HEADING = round((value/360.0)*512); }
   double get_SEL_HEADING() { return ((SEL_HEADING*360.0)/512); }
};

union me_target_state_and_status_subtypes
{
    me_target_state_and_status_subtype0 ST0;
    me_target_state_and_status_subtype1 ST1;
};

struct me_target_state_and_status_fields {
    uint8_t                             SUBTYPE;                // :2
    me_target_state_and_status_subtypes SUBTYPES;
};

union me_spec_fields {
    me_airborne_position_fields AIRBORNE_POSITION;
    me_surface_position_fields SURFACE_POSITION;
    me_aircraft_ident_fields AIRCRAFT_IDENT;
    me_airborne_velocity_fields AIRBORNE_VELOCITY;
    me_aircraft_status_fields AIRCRAFT_STATUS;
    me_aircraft_operational_status_fields AIRCRAFT_OPERATIONAL_STATUS;
    me_target_state_and_status_fields TARGET_STATE_AND_STATUS;
};

struct me_items {
    uint8_t TC;       // Squiter TYPE Code subfield :5
    me_spec_fields SPEC;
    
    void Set_TC(uint8_t Value);
    uint8_t Get_TC() const;
    
    void Data_to_Items(uint8_t * me);
    bool Items_to_Data(uint8_t * me) const;
};

#endif // DF_SUBFIELDS_HH_INCLUDED
