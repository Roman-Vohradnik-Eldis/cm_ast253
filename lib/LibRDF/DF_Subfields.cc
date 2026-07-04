#include "DF_Subfields.hh"

void me_items::Data_to_Items(uint8_t * me) {
    TC = ((me_airborne_position*)me)->TC;
    switch (TC) {
        case 0:
        case 9:
        case 10 :
        case 11 :
        case 12 :
        case 13 :
        case 14 :
        case 15 :
        case 16 :
        case 17 :
        case 18 :
        case 20 :
        case 21 :
        case 22 :
            SPEC.AIRBORNE_POSITION.SS = ((me_airborne_position*)me)->SS;
            SPEC.AIRBORNE_POSITION.NIC_B = ((me_airborne_position*)me)->NIC_B;
            SPEC.AIRBORNE_POSITION.AC =  (uint16_t) (((((me_airborne_position*)me)->AC_HI) <<4) | ((me_airborne_position*)me)->AC_LO) ;      
            SPEC.AIRBORNE_POSITION.T = ((me_airborne_position*)me)->T;
            SPEC.AIRBORNE_POSITION.F = ((me_airborne_position*)me)->F;
            SPEC.AIRBORNE_POSITION.CPR_LAT = (((me_airborne_position*)me)->CPR_LAT_HI << 15)  | (((me_airborne_position*)me)->CPR_LAT_MID << 7) | (((me_airborne_position*)me)->CPR_LAT_LO);
            SPEC.AIRBORNE_POSITION.CPR_LON = (((me_airborne_position*)me)->CPR_LON_HI << 16)  | (((me_airborne_position*)me)->CPR_LON_MID << 8) | (((me_airborne_position*)me)->CPR_LON_LO);
        break;
        case 5:
        case 6:
        case 7:
        case 8:    
            SPEC.SURFACE_POSITION.Movement = ((((me_surface_position*)me)->MOVEMENT_HI)<<4) | (((me_surface_position*)me)->MOVEMENT_LO);
            SPEC.SURFACE_POSITION.Heading_Valid = ((me_surface_position*)me)->HEADING_VALID;
            SPEC.SURFACE_POSITION.Heading = ((((me_surface_position*)me)->HEADING_HI)<<4) | (((me_surface_position*)me)->HEADING_LO);
            SPEC.SURFACE_POSITION.T = ((me_surface_position*)me)->T;
            SPEC.SURFACE_POSITION.F = ((me_surface_position*)me)->F;
            SPEC.SURFACE_POSITION.CPR_LAT = (((me_surface_position*)me)->CPR_LAT_HI << 15)  | (((me_surface_position*)me)->CPR_LAT_MID << 7) | (((me_surface_position*)me)->CPR_LAT_LO);
            SPEC.SURFACE_POSITION.CPR_LON = (((me_surface_position*)me)->CPR_LON_HI << 16)  | (((me_surface_position*)me)->CPR_LON_MID << 8) | (((me_surface_position*)me)->CPR_LON_LO);
        break;  
        
        case 1:
        case 2:
        case 3:
        case 4:
            SPEC.AIRCRAFT_IDENT.Aircraft_type = ((me_aircraft_ident*)me)->AIRCRAFT_TYPE;
            SPEC.AIRCRAFT_IDENT.Callsign[0] = ((me_aircraft_ident*)me)->CHAR0;
            SPEC.AIRCRAFT_IDENT.Callsign[1] = ((((me_aircraft_ident*)me)->CHAR1_HI) << 4) | ((me_aircraft_ident*)me)->CHAR1_LO;
            SPEC.AIRCRAFT_IDENT.Callsign[2] = ((((me_aircraft_ident*)me)->CHAR2_HI) << 2) | ((me_aircraft_ident*)me)->CHAR2_LO;
            SPEC.AIRCRAFT_IDENT.Callsign[3] = ((me_aircraft_ident*)me)->CHAR3;
            SPEC.AIRCRAFT_IDENT.Callsign[4] = ((me_aircraft_ident*)me)->CHAR4;
            SPEC.AIRCRAFT_IDENT.Callsign[5] = ((((me_aircraft_ident*)me)->CHAR5_HI) << 4) | ((me_aircraft_ident*)me)->CHAR5_LO;
            SPEC.AIRCRAFT_IDENT.Callsign[6] = ((((me_aircraft_ident*)me)->CHAR6_HI) << 2) | ((me_aircraft_ident*)me)->CHAR6_LO;
            SPEC.AIRCRAFT_IDENT.Callsign[7] = ((me_aircraft_ident*)me)->CHAR7;
            
        break;
                  
        case 19:
            SPEC.AIRBORNE_VELOCITY.SUBTYPE =((me_airborne_velocity_1_2*)me)->SUBTYPE;
            switch (SPEC.AIRBORNE_VELOCITY.SUBTYPE) {
                case ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL:
                case ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC:
                    SPEC.AIRBORNE_VELOCITY.INTENT = ((me_airborne_velocity_1_2*)me)->INTENT;
                    SPEC.AIRBORNE_VELOCITY.RESERVED_A = ((me_airborne_velocity_1_2*)me)->RESERVED_A;
                    SPEC.AIRBORNE_VELOCITY.NACV = ((me_airborne_velocity_1_2*)me)->NACV;
                    
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.EAST_WEST_DIRECTION = ((me_airborne_velocity_1_2*)me)->EAST_WEST_DIRECTION;
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.EAST_WEST_VELOCITY = ((((me_airborne_velocity_1_2*)me)->EAST_WEST_VELOCITY_HI) << 8) | ((me_airborne_velocity_1_2*)me)->EAST_WEST_VELOCITY_LO;
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.NORTH_SOUTH_DIRECTION = ((me_airborne_velocity_1_2*)me)->NORTH_SOUTH_DIRECTION;
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.NORTH_SOUTH_VELOCITY = ((((me_airborne_velocity_1_2*)me)->NORTH_SOUTH_VELOCITY_HI) << 3) | ((me_airborne_velocity_1_2*)me)->NORTH_SOUTH_VELOCITY_LO;
                    
                    SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SOURCE = ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_SOURCE;
                    SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SIGN = ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_SIGN;
                    SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE = ((((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_HI) << 6) | ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_LO;
                    SPEC.AIRBORNE_VELOCITY.RESERVED_B = ((me_airborne_velocity_1_2*)me)->RESERVED_B;
                    SPEC.AIRBORNE_VELOCITY.DIFF_BARO_SIGN = ((me_airborne_velocity_1_2*)me)->DIFF_BARO_SIGN;
                    SPEC.AIRBORNE_VELOCITY.DIFF_BARO = ((me_airborne_velocity_1_2*)me)->DIFF_BARO;
                break;
                case ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL:
                case ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC: 
                    SPEC.AIRBORNE_VELOCITY.INTENT = ((me_airborne_velocity_3_4*)me)->INTENT;
                    SPEC.AIRBORNE_VELOCITY.RESERVED_A = ((me_airborne_velocity_3_4*)me)->RESERVED_A;
                    SPEC.AIRBORNE_VELOCITY.NACV = ((me_airborne_velocity_3_4*)me)->NACV;
                    
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.HEADING_STATUS = ((me_airborne_velocity_3_4*)me)->HEADING_STATUS;
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.HEADING = ((((me_airborne_velocity_3_4*)me)->HEADING_HI) << 8) | ((me_airborne_velocity_3_4*)me)->HEADING_LO;
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.AIR_SPEED_TYPE = ((me_airborne_velocity_3_4*)me)->AIR_SPEED_TYPE;
                    SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.AIR_SPEED = ((((me_airborne_velocity_3_4*)me)->AIR_SPEED_HI) << 3) | ((me_airborne_velocity_3_4*)me)->AIR_SPEED_LO;
                    
                    SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SOURCE = ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_SOURCE;
                    SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SIGN = ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_SIGN;
                    SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE = ((((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_HI) << 6) | ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_LO;
                    SPEC.AIRBORNE_VELOCITY.RESERVED_B = ((me_airborne_velocity_3_4*)me)->RESERVED_B;
                    SPEC.AIRBORNE_VELOCITY.DIFF_BARO_SIGN = ((me_airborne_velocity_3_4*)me)->DIFF_BARO_SIGN;
                    SPEC.AIRBORNE_VELOCITY.DIFF_BARO = ((me_airborne_velocity_3_4*)me)->DIFF_BARO;
                break;
            }

        break;

        case 28:
            SPEC.AIRCRAFT_STATUS.SUBTYPE = ((me_aircraft_status*)me)->SUBTYPE;
            SPEC.AIRCRAFT_STATUS.EMERGENCY = ((me_aircraft_status*)me)->EMERGENCY;
            SPEC.AIRCRAFT_STATUS.M3A = (((me_aircraft_status*)me)->MODE_A_HI << 8) | ((me_aircraft_status*)me)->MODE_A_LO;
        break;

        case 29:
            SPEC.TARGET_STATE_AND_STATUS.SUBTYPE = ((me_target_state_and_status0*)me)->SUBTYPE;
            switch (SPEC.TARGET_STATE_AND_STATUS.SUBTYPE) {
                case 0:
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.VDATA = (((me_target_state_and_status0*)me)->VDATA_HI << 1) | ((me_target_state_and_status0*)me)->VDATA_LO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT_TYPE = ((me_target_state_and_status0*)me)->TARGET_ALT_TYPE;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT_CAPABILITY = ((me_target_state_and_status0*)me)->TARGET_ALT_CAPABILITY;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.VMODE = ((me_target_state_and_status0*)me)->VMODE;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT = (((me_target_state_and_status0*)me)->TARGET_ALT_HI << 9) | (((me_target_state_and_status0*)me)->TARGET_ALT_MID << 1) | (((me_target_state_and_status0*)me)->TARGET_ALT_LO);
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.HDATA = ((me_target_state_and_status0*)me)->HDATA;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_HEADING = (((me_target_state_and_status0*)me)->TARGET_HEADING_HI << 4) | ((me_target_state_and_status0*)me)->TARGET_HEADING_LO;
					SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_HEADING_TRACK = ((me_target_state_and_status0*)me)->TARGET_HEADING_TRACK;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.HMODE = ((me_target_state_and_status0*)me)->HMODE;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.NACP = (((me_target_state_and_status0*)me)->NACP_HI << 3) | ((me_target_state_and_status0*)me)->NACP_LO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.NIC_BARO = ((me_target_state_and_status0*)me)->NIC_BARO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.SIL = ((me_target_state_and_status0*)me)->SIL;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.CAPABILITY = ((me_target_state_and_status0*)me)->CAPABILITY;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.EMERGENCY = ((me_target_state_and_status0*)me)->EMERGENCY;
                    break;
                case 1:
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SIL_SUPP = ((me_target_state_and_status1*)me)->SIL_SUPP;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_ALT = (((me_target_state_and_status1*)me)->SEL_ALT_HI << 4) | ((me_target_state_and_status1*)me)->SEL_ALT_LO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_ALT_TYPE = ((me_target_state_and_status1*)me)->SEL_ALT_TYPE;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.BARO = (((me_target_state_and_status1*)me)->BARO_HI << 5) | ((me_target_state_and_status1*)me)->BARO_LO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_HEADING_STATUS = ((me_target_state_and_status1*)me)->SEL_HEADING_STATUS;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_HEADING = (((me_target_state_and_status1*)me)->SEL_HEADING_HI << 7) | ((me_target_state_and_status1*)me)->SEL_HEADING_LO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.NACP = (((me_target_state_and_status1*)me)->NACP_HI << 3) | ((me_target_state_and_status1*)me)->NACP_LO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.NIC_BARO = ((me_target_state_and_status1*)me)->NIC_BARO;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SIL = ((me_target_state_and_status1*)me)->SIL;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.MCP_FCU = ((me_target_state_and_status1*)me)->MCP_FCU;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.AUTOPILOT = ((me_target_state_and_status1*)me)->AUTOPILOT;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.VNAV = ((me_target_state_and_status1*)me)->VNAV;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.ALT_HOLD = ((me_target_state_and_status1*)me)->ALT_HOLD;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.APPROACH = ((me_target_state_and_status1*)me)->APPROACH;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.TCAS = ((me_target_state_and_status1*)me)->TCAS;
                    SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.LNAV = ((me_target_state_and_status1*)me)->LNAV;
                break;
            }
        break;

        case 31:
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.SUBTYPE = ((me_operational_status_surface*)me)->SUBTYPE;
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.MOPS_VER = ((me_operational_status_surface*)me)->MOPS_VER;
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.NIC_A = ((me_operational_status_surface*)me)->NIC_A;
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.NACP = ((me_operational_status_surface*)me)->NACP;
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.SIL = ((me_operational_status_surface*)me)->SIL;
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.HRD = ((me_operational_status_surface*)me)->HRD;
            SPEC.AIRCRAFT_OPERATIONAL_STATUS.SIL_SUPP = ((me_operational_status_surface*)me)->SIL_SUPP;
            switch (SPEC.AIRCRAFT_OPERATIONAL_STATUS.SUBTYPE) {
                case ME_AIRCRAFT_OPERATIONAL_STATUS_SURFACE:
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.FORMAT = ((me_operational_status_surface*)me)->CC_FORMAT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.RESERVED = ((me_operational_status_surface*)me)->CC_RESERVED_1;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.ES_IN = ((me_operational_status_surface*)me)->CC_ES_IN;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.B2_LOW = ((me_operational_status_surface*)me)->CC_B2_LOW;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.UAT_IN = ((me_operational_status_surface*)me)->CC_UAT_IN;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.NACV = ((me_operational_status_surface*)me)->CC_NACV;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.NIC_C = ((me_operational_status_surface*)me)->CC_NIC_C;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.FORMAT = ((me_operational_status_surface*)me)->OM_FORMAT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.TCAS_RA = ((me_operational_status_surface*)me)->OM_TCAS_RA;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.IDENT = ((me_operational_status_surface*)me)->OM_IDENT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.RAS = ((me_operational_status_surface*)me)->OM_RAS;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.SINGLE_ANT = ((me_operational_status_surface*)me)->OM_SINGLE_ANT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.SDA = ((me_operational_status_surface*)me)->OM_SDA;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.LON_ANT_OFFSET = ((me_operational_status_surface*)me)->OM_LON_ANT_OFFSET;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.LAT_ANT_OFFSET = ((me_operational_status_surface*)me)->OM_LAT_ANT_OFFSET;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.LW = ((me_operational_status_surface*)me)->LW;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.TRK_HDG = ((me_operational_status_surface*)me)->TRK_HDG;
                break;
                case ME_AIRCRAFT_OPERATIONAL_STATUS_AIRBORNE:
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.FORMAT = ((me_operational_status_airborne*)me)->CC_FORMAT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.TCAS = ((me_operational_status_airborne*)me)->CC_TCAS;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.ES_IN = ((me_operational_status_airborne*)me)->CC_ES_IN;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.ARV = ((me_operational_status_airborne*)me)->CC_ARV;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.TS = ((me_operational_status_airborne*)me)->CC_TS;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.TC = ((me_operational_status_airborne*)me)->CC_TC;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.UAT_IN = ((me_operational_status_airborne*)me)->CC_UAT_IN;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.FORMAT = ((me_operational_status_airborne*)me)->OM_FORMAT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.TCAS_RA = ((me_operational_status_airborne*)me)->OM_TCAS_RA;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.IDENT = ((me_operational_status_airborne*)me)->OM_IDENT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.RAS = ((me_operational_status_airborne*)me)->OM_RAS;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.SINGLE_ANT = ((me_operational_status_airborne*)me)->OM_SINGLE_ANT;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.SDA = ((me_operational_status_airborne*)me)->OM_SDA;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.NIC_BARO = ((me_operational_status_airborne*)me)->NIC_BARO;
                    SPEC.AIRCRAFT_OPERATIONAL_STATUS.GVA = ((me_operational_status_airborne*)me)->GVA;
                 break;
            }
    }
}

bool me_items::Items_to_Data(uint8_t * me) const {
 
    switch (TC) {
        case 0:
        case 9:
        case 10 :
        case 11 :
        case 12 :
        case 13 :
        case 14 :
        case 15 :
        case 16 :
        case 17 :
        case 18 :
        case 20 :
        case 21 :
        case 22 :
            ((me_airborne_position*)me)->TC = TC;
            ((me_airborne_position*)me)->SS = (SPEC.AIRBORNE_POSITION.SS & 0x3);
            ((me_airborne_position*)me)->NIC_B = SPEC.AIRBORNE_POSITION.NIC_B & 0x1;
            ((me_airborne_position*)me)->AC_HI = (SPEC.AIRBORNE_POSITION.AC.get()>>4) & 0xFF;
            ((me_airborne_position*)me)->AC_LO = SPEC.AIRBORNE_POSITION.AC.get() & 0xF;
            ((me_airborne_position*)me)->T =(SPEC.AIRBORNE_POSITION.T & 0x1);
            ((me_airborne_position*)me)->F = (SPEC.AIRBORNE_POSITION.F & 0x1);
            ((me_airborne_position*)me)->CPR_LAT_LO = SPEC.AIRBORNE_POSITION.CPR_LAT & 0x7F;
            ((me_airborne_position*)me)->CPR_LAT_MID = (SPEC.AIRBORNE_POSITION.CPR_LAT>>7) & 0xFF;
            ((me_airborne_position*)me)->CPR_LAT_HI = (SPEC.AIRBORNE_POSITION.CPR_LAT>>15) & 0x3;
            ((me_airborne_position*)me)->CPR_LON_LO = SPEC.AIRBORNE_POSITION.CPR_LON & 0xFF;
            ((me_airborne_position*)me)->CPR_LON_MID = (SPEC.AIRBORNE_POSITION.CPR_LON>>8) & 0xFF;
            ((me_airborne_position*)me)->CPR_LON_HI = (SPEC.AIRBORNE_POSITION.CPR_LON>>16) & 0x1;
            return true;
        break;
        case 5:
        case 6:
        case 7:
        case 8:
            ((me_surface_position*)me)->TC = TC;
            ((me_surface_position*)me)->MOVEMENT_HI = (SPEC.SURFACE_POSITION.Movement >> 4) & 0x7;
            ((me_surface_position*)me)->MOVEMENT_LO = SPEC.SURFACE_POSITION.Movement & 0xF;
            ((me_surface_position*)me)->HEADING_VALID = SPEC.SURFACE_POSITION.Heading_Valid & 0x1;
            ((me_surface_position*)me)->HEADING_HI = (SPEC.SURFACE_POSITION.Heading >> 4) & 0x7;
            ((me_surface_position*)me)->HEADING_LO = SPEC.SURFACE_POSITION.Heading & 0xF;
            ((me_surface_position*)me)->T =(SPEC.SURFACE_POSITION.T & 0x1);
            ((me_surface_position*)me)->F = (SPEC.SURFACE_POSITION.F & 0x1);
            ((me_surface_position*)me)->CPR_LAT_LO = SPEC.SURFACE_POSITION.CPR_LAT & 0x7F;
            ((me_surface_position*)me)->CPR_LAT_MID = (SPEC.SURFACE_POSITION.CPR_LAT>>7) & 0xFF;
            ((me_surface_position*)me)->CPR_LAT_HI = (SPEC.SURFACE_POSITION.CPR_LAT>>15) & 0x3;
            ((me_surface_position*)me)->CPR_LON_LO = SPEC.SURFACE_POSITION.CPR_LON & 0xFF;
            ((me_surface_position*)me)->CPR_LON_MID = (SPEC.SURFACE_POSITION.CPR_LON>>8) & 0xFF;
            ((me_surface_position*)me)->CPR_LON_HI = (SPEC.SURFACE_POSITION.CPR_LON>>16) & 0x1;
            return true;
        break;
        
        case 1:
        case 2:
        case 3:
        case 4:
            ((me_aircraft_ident*)me)->TC = TC;
            ((me_aircraft_ident*)me)->AIRCRAFT_TYPE = SPEC.AIRCRAFT_IDENT.Aircraft_type;
            ((me_aircraft_ident*)me)->CHAR0 = SPEC.AIRCRAFT_IDENT.Callsign[0];
            ((me_aircraft_ident*)me)->CHAR1_LO = SPEC.AIRCRAFT_IDENT.Callsign[1] & 0xF;
            ((me_aircraft_ident*)me)->CHAR1_HI = (SPEC.AIRCRAFT_IDENT.Callsign[1] >> 4) & 0x3;
            ((me_aircraft_ident*)me)->CHAR2_LO = SPEC.AIRCRAFT_IDENT.Callsign[2] & 0x3;
            ((me_aircraft_ident*)me)->CHAR2_HI = (SPEC.AIRCRAFT_IDENT.Callsign[2] >> 2) & 0xF;
            ((me_aircraft_ident*)me)->CHAR3 = SPEC.AIRCRAFT_IDENT.Callsign[3];
            
            ((me_aircraft_ident*)me)->CHAR4 = SPEC.AIRCRAFT_IDENT.Callsign[4];
            ((me_aircraft_ident*)me)->CHAR5_LO = SPEC.AIRCRAFT_IDENT.Callsign[5] & 0xF;
            ((me_aircraft_ident*)me)->CHAR5_HI = (SPEC.AIRCRAFT_IDENT.Callsign[5] >> 4) & 0x3;
            ((me_aircraft_ident*)me)->CHAR6_LO = SPEC.AIRCRAFT_IDENT.Callsign[6] & 0x3;
            ((me_aircraft_ident*)me)->CHAR6_HI = (SPEC.AIRCRAFT_IDENT.Callsign[6] >> 2) & 0xF;
            ((me_aircraft_ident*)me)->CHAR7 = SPEC.AIRCRAFT_IDENT.Callsign[7];
            return true;
            
        case 19:
            ((me_airborne_velocity_1_2*)me)->TC = TC;
            ((me_airborne_velocity_1_2*)me)->SUBTYPE = SPEC.AIRBORNE_VELOCITY.SUBTYPE;
            switch (SPEC.AIRBORNE_VELOCITY.SUBTYPE) {
                case ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL:
                case ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC:
                    ((me_airborne_velocity_1_2*)me)->INTENT = SPEC.AIRBORNE_VELOCITY.INTENT;
                    ((me_airborne_velocity_1_2*)me)->RESERVED_A = SPEC.AIRBORNE_VELOCITY.RESERVED_A;
                    ((me_airborne_velocity_1_2*)me)->NACV = SPEC.AIRBORNE_VELOCITY.NACV;
                    
                    ((me_airborne_velocity_1_2*)me)->EAST_WEST_DIRECTION = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.EAST_WEST_DIRECTION;
                    ((me_airborne_velocity_1_2*)me)->EAST_WEST_VELOCITY_LO = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.EAST_WEST_VELOCITY & 0xFF ;
                    ((me_airborne_velocity_1_2*)me)->EAST_WEST_VELOCITY_HI = (SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.EAST_WEST_VELOCITY >> 8) & 0x3 ;
                    ((me_airborne_velocity_1_2*)me)->NORTH_SOUTH_DIRECTION = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.NORTH_SOUTH_DIRECTION;
                    ((me_airborne_velocity_1_2*)me)->NORTH_SOUTH_VELOCITY_LO = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.NORTH_SOUTH_VELOCITY & 0x7;
                    ((me_airborne_velocity_1_2*)me)->NORTH_SOUTH_VELOCITY_HI = (SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_1_2.NORTH_SOUTH_VELOCITY >> 3) & 0x7F; 
                    
                    ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_SOURCE = SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SOURCE;
                    ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_SIGN = SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SIGN;
                    ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_LO = SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE & 0x3F;
                    ((me_airborne_velocity_1_2*)me)->VERTICAL_RATE_HI = (SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE >> 6) & 0x7 ;
                    ((me_airborne_velocity_1_2*)me)->RESERVED_B = SPEC.AIRBORNE_VELOCITY.RESERVED_B;
                    ((me_airborne_velocity_1_2*)me)->DIFF_BARO_SIGN = SPEC.AIRBORNE_VELOCITY.DIFF_BARO_SIGN;
                    ((me_airborne_velocity_1_2*)me)->DIFF_BARO = SPEC.AIRBORNE_VELOCITY.DIFF_BARO;
                break;    
                case ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL:
                case ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC: 
                    ((me_airborne_velocity_3_4*)me)->INTENT = SPEC.AIRBORNE_VELOCITY.INTENT;
                    ((me_airborne_velocity_3_4*)me)->RESERVED_A = SPEC.AIRBORNE_VELOCITY.RESERVED_A;
                    ((me_airborne_velocity_3_4*)me)->NACV = SPEC.AIRBORNE_VELOCITY.NACV;
                    
                    ((me_airborne_velocity_3_4*)me)->HEADING_STATUS = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.HEADING_STATUS;
                    ((me_airborne_velocity_3_4*)me)->HEADING_LO = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.HEADING & 0xFF ;
                    ((me_airborne_velocity_3_4*)me)->HEADING_HI = (SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.HEADING >> 8) & 0x3 ;
                    ((me_airborne_velocity_3_4*)me)->AIR_SPEED_TYPE = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.AIR_SPEED_TYPE;
                    ((me_airborne_velocity_3_4*)me)->AIR_SPEED_LO = SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.AIR_SPEED & 0x7;
                    ((me_airborne_velocity_3_4*)me)->AIR_SPEED_HI = (SPEC.AIRBORNE_VELOCITY.VELOCITY.SUBTYPE_3_4.AIR_SPEED >> 3) & 0x7F; 
                    
                    ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_SOURCE = SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SOURCE;
                    ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_SIGN = SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE_SIGN;
                    ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_LO = SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE & 0x3F;
                    ((me_airborne_velocity_3_4*)me)->VERTICAL_RATE_HI = (SPEC.AIRBORNE_VELOCITY.VERTICAL_RATE >> 6) & 0x7;
                    ((me_airborne_velocity_3_4*)me)->RESERVED_B = SPEC.AIRBORNE_VELOCITY.RESERVED_B;
                    ((me_airborne_velocity_3_4*)me)->DIFF_BARO_SIGN = SPEC.AIRBORNE_VELOCITY.DIFF_BARO_SIGN;
                    ((me_airborne_velocity_3_4*)me)->DIFF_BARO = SPEC.AIRBORNE_VELOCITY.DIFF_BARO;
            }
            return true;

        case 28:
            ((me_aircraft_status*)me)->TC = TC;
            ((me_aircraft_status*)me)->SUBTYPE = SPEC.AIRCRAFT_STATUS.SUBTYPE;
            ((me_aircraft_status*)me)->EMERGENCY = SPEC.AIRCRAFT_STATUS.EMERGENCY;
            ((me_aircraft_status*)me)->MODE_A_LO = SPEC.AIRCRAFT_STATUS.M3A;
            ((me_aircraft_status*)me)->MODE_A_HI = (SPEC.AIRCRAFT_STATUS.M3A >> 8) & 0x1F;
            memset(((me_aircraft_status*)me)->RESERVED, 0, 4);
            return true;
        break;

        case 29:
            ((me_target_state_and_status0*)me)->TC = TC;
            ((me_target_state_and_status0*)me)->SUBTYPE = SPEC.TARGET_STATE_AND_STATUS.SUBTYPE;
            switch (SPEC.TARGET_STATE_AND_STATUS.SUBTYPE) {
                case(0):
                    ((me_target_state_and_status0*)me)->VDATA_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.VDATA >> 1) & 0x1;
                    ((me_target_state_and_status0*)me)->VDATA_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.VDATA & 0x1;
                    ((me_target_state_and_status0*)me)->TARGET_ALT_TYPE = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT_TYPE;
                    ((me_target_state_and_status0*)me)->COMPATIBILITY = 0;
                    ((me_target_state_and_status0*)me)->TARGET_ALT_CAPABILITY = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT_CAPABILITY;
                    ((me_target_state_and_status0*)me)->VMODE = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.VMODE;
                    ((me_target_state_and_status0*)me)->TARGET_ALT_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT >> 9) & 0x1;
                    ((me_target_state_and_status0*)me)->TARGET_ALT_MID = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT >> 1) & 0xFF;
                    ((me_target_state_and_status0*)me)->TARGET_ALT_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_ALT & 0x1;
                    ((me_target_state_and_status0*)me)->HDATA = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.HDATA;
                    ((me_target_state_and_status0*)me)->TARGET_HEADING_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_HEADING >> 4) & 0x1F;
                    ((me_target_state_and_status0*)me)->TARGET_HEADING_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_HEADING & 0xF;
                    ((me_target_state_and_status0*)me)->TARGET_HEADING_TRACK = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.TARGET_HEADING_TRACK;
                    ((me_target_state_and_status0*)me)->HMODE = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.HMODE;
                    ((me_target_state_and_status0*)me)->NACP_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.NACP >> 3) & 0x1F;
                    ((me_target_state_and_status0*)me)->NACP_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.NACP & 0x7;
                    ((me_target_state_and_status0*)me)->NIC_BARO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.NIC_BARO;
                    ((me_target_state_and_status0*)me)->SIL = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.SIL;
                    ((me_target_state_and_status0*)me)->RESERVED_HI = 0x0;
                    ((me_target_state_and_status0*)me)->RESERVED_LO = 0x0;
                    ((me_target_state_and_status0*)me)->CAPABILITY = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.CAPABILITY;
                    ((me_target_state_and_status0*)me)->EMERGENCY = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST0.EMERGENCY;
                break;
                case(1):
                    ((me_target_state_and_status1*)me)->SIL_SUPP = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SIL_SUPP;
                    ((me_target_state_and_status1*)me)->SEL_ALT_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_ALT >> 4) & 0x7F;
                    ((me_target_state_and_status1*)me)->SEL_ALT_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_ALT & 0x0F;
                    ((me_target_state_and_status1*)me)->SEL_ALT_TYPE = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_ALT_TYPE;
                    ((me_target_state_and_status1*)me)->BARO_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.BARO >> 5) & 0x0F;
                    ((me_target_state_and_status1*)me)->BARO_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.BARO & 0x1F;
                    ((me_target_state_and_status1*)me)->SEL_HEADING_STATUS = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_HEADING_STATUS;
                    ((me_target_state_and_status1*)me)->SEL_HEADING_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_HEADING >> 7) & 0x3;
                    ((me_target_state_and_status1*)me)->SEL_HEADING_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SEL_HEADING & 0x7F;
                    ((me_target_state_and_status1*)me)->NACP_HI = (SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.NACP >> 3) & 0x1;
                    ((me_target_state_and_status1*)me)->NACP_LO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.NACP & 0x07;
                    ((me_target_state_and_status1*)me)->NIC_BARO = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.NIC_BARO;
                    ((me_target_state_and_status1*)me)->SIL = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.SIL;
                    ((me_target_state_and_status1*)me)->MCP_FCU = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.MCP_FCU;
                    ((me_target_state_and_status1*)me)->AUTOPILOT = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.AUTOPILOT;
                    ((me_target_state_and_status1*)me)->VNAV = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.VNAV;
                    ((me_target_state_and_status1*)me)->ALT_HOLD = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.ALT_HOLD;
                    ((me_target_state_and_status1*)me)->APPROACH = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.APPROACH;
                    ((me_target_state_and_status1*)me)->TCAS = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.TCAS;
                    ((me_target_state_and_status1*)me)->LNAV = SPEC.TARGET_STATE_AND_STATUS.SUBTYPES.ST1.LNAV;
                break;
            default:
                return false;
            }
            return true;
        break;

        case 31:
            ((me_operational_status_surface*)me)->TC = TC;
            ((me_operational_status_surface*)me)->SUBTYPE = SPEC.AIRCRAFT_OPERATIONAL_STATUS.SUBTYPE;
            switch (SPEC.AIRBORNE_VELOCITY.SUBTYPE) {
                case ME_AIRCRAFT_OPERATIONAL_STATUS_SURFACE:
                    ((me_operational_status_surface*)me)->CC_UAT_IN = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.UAT_IN;
                    ((me_operational_status_surface*)me)->CC_B2_LOW = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.B2_LOW;
                    ((me_operational_status_surface*)me)->CC_RESERVED_1 = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.RESERVED;
                    ((me_operational_status_surface*)me)->CC_ES_IN = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.ES_IN;
                    ((me_operational_status_surface*)me)->CC_RESERVED_2 = 0;
                    ((me_operational_status_surface*)me)->CC_FORMAT = 0;
                    ((me_operational_status_surface*)me)->LW = SPEC.AIRCRAFT_OPERATIONAL_STATUS.LW;
                    ((me_operational_status_surface*)me)->CC_NIC_C = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.NIC_C;
                    ((me_operational_status_surface*)me)->CC_NACV = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.SURFACE.NACV;
                    ((me_operational_status_surface*)me)->OM_SDA = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.SDA;
                    ((me_operational_status_surface*)me)->OM_SINGLE_ANT = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.SINGLE_ANT;
                    ((me_operational_status_surface*)me)->OM_RAS = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.RAS;
                    ((me_operational_status_surface*)me)->OM_IDENT = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.IDENT;
                    ((me_operational_status_surface*)me)->OM_TCAS_RA = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.TCAS_RA;
                    ((me_operational_status_surface*)me)->OM_FORMAT = 0;
                    ((me_operational_status_surface*)me)->OM_LON_ANT_OFFSET = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.LON_ANT_OFFSET;
                    ((me_operational_status_surface*)me)->OM_LAT_ANT_OFFSET = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.SURFACE.LAT_ANT_OFFSET;
                    ((me_operational_status_surface*)me)->NACP = SPEC.AIRCRAFT_OPERATIONAL_STATUS.NACP;
                    ((me_operational_status_surface*)me)->NIC_A = SPEC.AIRCRAFT_OPERATIONAL_STATUS.NIC_A;
                    ((me_operational_status_surface*)me)->MOPS_VER = SPEC.AIRCRAFT_OPERATIONAL_STATUS.MOPS_VER;
                    ((me_operational_status_surface*)me)->RESERVED_2 = 0;
                    ((me_operational_status_surface*)me)->SIL_SUPP = SPEC.AIRCRAFT_OPERATIONAL_STATUS.SIL_SUPP;
                    ((me_operational_status_surface*)me)->HRD = SPEC.AIRCRAFT_OPERATIONAL_STATUS.HRD;
                    ((me_operational_status_surface*)me)->TRK_HDG = SPEC.AIRCRAFT_OPERATIONAL_STATUS.TRK_HDG;
                    ((me_operational_status_surface*)me)->SIL = SPEC.AIRCRAFT_OPERATIONAL_STATUS.SIL;
                    ((me_operational_status_surface*)me)->RESERVED_1 = 0;
                break;
                case ME_AIRCRAFT_OPERATIONAL_STATUS_AIRBORNE:
                    ((me_operational_status_airborne*)me)->CC_TS = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.TS;
                    ((me_operational_status_airborne*)me)->CC_ARV = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.ARV;
                    ((me_operational_status_airborne*)me)->CC_RESERVED_1 = 0;
                    ((me_operational_status_airborne*)me)->CC_ES_IN = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.ES_IN;
                    ((me_operational_status_airborne*)me)->CC_TCAS = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.TCAS;
                    ((me_operational_status_airborne*)me)->CC_FORMAT = 0;
                    ((me_operational_status_airborne*)me)->CC_RESERVED_2 = 0;
                    ((me_operational_status_airborne*)me)->CC_UAT_IN = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.UAT_IN;
                    ((me_operational_status_airborne*)me)->CC_TC = SPEC.AIRCRAFT_OPERATIONAL_STATUS.CC.AIRBORNE.TC;
                    ((me_operational_status_airborne*)me)->OM_SDA = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.SDA;
                    ((me_operational_status_airborne*)me)->OM_SINGLE_ANT = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.SINGLE_ANT;
                    ((me_operational_status_airborne*)me)->OM_RAS = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.RAS;
                    ((me_operational_status_airborne*)me)->OM_IDENT = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.IDENT;
                    ((me_operational_status_airborne*)me)->OM_TCAS_RA = SPEC.AIRCRAFT_OPERATIONAL_STATUS.OM.AIRBORNE.TCAS_RA;
                    ((me_operational_status_airborne*)me)->OM_FORMAT = 0;
                    ((me_operational_status_airborne*)me)->OM_RESERVED = 0;
                    ((me_operational_status_airborne*)me)->NACP = SPEC.AIRCRAFT_OPERATIONAL_STATUS.NACP;
                    ((me_operational_status_airborne*)me)->NIC_A = SPEC.AIRCRAFT_OPERATIONAL_STATUS.NIC_A;
                    ((me_operational_status_airborne*)me)->MOPS_VER = SPEC.AIRCRAFT_OPERATIONAL_STATUS.MOPS_VER;
                    ((me_operational_status_airborne*)me)->RESERVED = 0;
                    ((me_operational_status_airborne*)me)->SIL_SUPP = SPEC.AIRCRAFT_OPERATIONAL_STATUS.SIL_SUPP;
                    ((me_operational_status_airborne*)me)->HRD = SPEC.AIRCRAFT_OPERATIONAL_STATUS.HRD;
                    ((me_operational_status_airborne*)me)->NIC_BARO = SPEC.AIRCRAFT_OPERATIONAL_STATUS.NIC_BARO;
                    ((me_operational_status_airborne*)me)->SIL = SPEC.AIRCRAFT_OPERATIONAL_STATUS.SIL;
                    ((me_operational_status_airborne*)me)->GVA = SPEC.AIRCRAFT_OPERATIONAL_STATUS.GVA;
                break;

                default:
                    return false;
              }
              return true;
        break;    
        default:
            return false;
    }
}


double me_cpr_position_subfields::cpr_MOD(double num,double denom) const {
    double val = num-(denom*floor(num/denom));
    if (val<0) 
        val += denom;
    return val;
}


/* The NL function uses the precomputed table from 1090-WP-9-14 */
int me_cpr_position_subfields::cpr_NL_precomputed(double lat) const {
    if (lat < 0) lat = -lat; /* Table is simmetric about the equator. */
    if (lat < 10.47047130) return 59;
    if (lat < 14.82817437) return 58;
    if (lat < 18.18626357) return 57;
    if (lat < 21.02939493) return 56;
    if (lat < 23.54504487) return 55;
    if (lat < 25.82924707) return 54;
    if (lat < 27.93898710) return 53;
    if (lat < 29.91135686) return 52;
    if (lat < 31.77209708) return 51;
    if (lat < 33.53993436) return 50;
    if (lat < 35.22899598) return 49;
    if (lat < 36.85025108) return 48;
    if (lat < 38.41241892) return 47;
    if (lat < 39.92256684) return 46;
    if (lat < 41.38651832) return 45;
    if (lat < 42.80914012) return 44;
    if (lat < 44.19454951) return 43;
    if (lat < 45.54626723) return 42;
    if (lat < 46.86733252) return 41;
    if (lat < 48.16039128) return 40;
    if (lat < 49.42776439) return 39;
    if (lat < 50.67150166) return 38;
    if (lat < 51.89342469) return 37;
    if (lat < 53.09516153) return 36;
    if (lat < 54.27817472) return 35;
    if (lat < 55.44378444) return 34;
    if (lat < 56.59318756) return 33;
    if (lat < 57.72747354) return 32;
    if (lat < 58.84763776) return 31;
    if (lat < 59.95459277) return 30;
    if (lat < 61.04917774) return 29;
    if (lat < 62.13216659) return 28;
    if (lat < 63.20427479) return 27;
    if (lat < 64.26616523) return 26;
    if (lat < 65.31845310) return 25;
    if (lat < 66.36171008) return 24;
    if (lat < 67.39646774) return 23;
    if (lat < 68.42322022) return 22;
    if (lat < 69.44242631) return 21;
    if (lat < 70.45451075) return 20;
    if (lat < 71.45986473) return 19;
    if (lat < 72.45884545) return 18;
    if (lat < 73.45177442) return 17;
    if (lat < 74.43893416) return 16;
    if (lat < 75.42056257) return 15;
    if (lat < 76.39684391) return 14;
    if (lat < 77.36789461) return 13;
    if (lat < 78.33374083) return 12;
    if (lat < 79.29428225) return 11;
    if (lat < 80.24923213) return 10;
    if (lat < 81.19801349) return 9;
    if (lat < 82.13956981) return 8;
    if (lat < 83.07199445) return 7;
    if (lat < 83.99173563) return 6;
    if (lat < 84.89166191) return 5;
    if (lat < 85.75541621) return 4;
    if (lat < 86.53536998) return 3;
    if (lat < 87.00000000) return 2;
    else return 1;
}

int me_cpr_position_subfields::cpr_NL_computed(double lat) const {
    if (lat==0) return 59;
    if ((lat==87) || (lat==-87)) return 2;
    if ((lat > 87) || (lat <-87)) return 1;
    double nz =15;
    double a = 1- cos(M_PI/(2*nz));
    double b = pow (cos(M_PI/180.0*fabs(lat)),2);
    double n1 = 2* M_PI/(acos(1- a/b));
    return floor(n1);
}

void me_cpr_position_subfields::encode_LatLon (double Lat, double Lon, double NB) {
    uint8_t nz =15;
    double dlat=360.0 /(4* nz-F);
    CPR_LAT = floor(NB*(cpr_MOD(Lat,dlat)/dlat) +0.5);
    double Rlat = dlat * ((CPR_LAT/NB) + floor(Lat/dlat));
    int nn= cpr_NL(Rlat);
    double dlon;
    if ((nn-F)>0) 
        dlon=360.0/(nn-F);
    else
        dlon=360.0;
    CPR_LON = floor(NB*(cpr_MOD(Lon,dlon)/dlon) + 0.5);
    CPR_LAT = cpr_MOD(CPR_LAT,CPR_2NB_17);
    CPR_LON = cpr_MOD(CPR_LON,CPR_2NB_17);
}

void me_cpr_position_subfields::encode_Airborne_LatLon (double Lat, double Lon) {
    encode_LatLon (Lat, Lon, CPR_2NB_17);
} 

void me_cpr_position_subfields::encode_Surface_LatLon (double Lat, double Lon) {
    encode_LatLon (Lat, Lon, CPR_2NB_19);
} 

bool me_cpr_position_subfields::decode_Airborne_LatLon(const me_cpr_position_subfields & pair,double &Lat, double &Lon) const {
    double dlat0, dlat1, dlon0, dlon1;
    double RLat0, RLat1;
    double result_Lat, result_Lon;
    double YZ0,YZ1,XZ0,XZ1;
    bool time_even;
    uint8_t NZ =15;
    
    if (F==pair.F) return false;
    if (F==0) {
        XZ0 = CPR_LON;
        YZ0 = CPR_LAT;
        XZ1 = pair.CPR_LON;
        YZ1 = pair.CPR_LAT;
        time_even = true;
    } else {
        XZ1 = CPR_LON;
        YZ1 = CPR_LAT;
        XZ0 = pair.CPR_LON;
        YZ0 = pair.CPR_LAT;
        time_even = false;
    }
        
    dlat0 = 90.0 / NZ;
    dlat1 = 90.0 / (NZ - 0.25);

    int j = floor (0.5 + (59 * YZ0 - 60 * YZ1) / CPR_2NB_17);

    RLat0 = dlat0 * (cpr_MOD (j, 60) + (YZ0 / CPR_2NB_17));
    RLat1 = dlat1 * (cpr_MOD (j, 59) + (YZ1 / CPR_2NB_17));

    /* southern hemisphere values of RLati will fall in the range from 270 to
     * 360 degrees. subtract 360 deg from such values, thereby restoring Rlati
     * to the range from -90 to 90 degrees. */
    if ((RLat0 >= 270) && (RLat0 <= 360))
            RLat0 -= 360;

    if ((RLat1 >= 270) && (RLat1 <= 360))
            RLat1 -= 360;

    /* positions straddle a transition latitude */
    if (cpr_NL(RLat0) != cpr_NL(RLat1))
            return false;

    if (time_even)
    {
            double N0 = std::max (cpr_NL (RLat0), 1);
            dlon0 = 360.0 / N0;
            int m = floor (0.5 + ((XZ0 * (cpr_NL (RLat0) - 1) - XZ1 * cpr_NL (RLat0)) / CPR_2NB_17));
            result_Lon = dlon0 * (cpr_MOD (m, N0) + (XZ0 / CPR_2NB_17));
            result_Lat = RLat0;
    }
    else
    {
            double N1 = std::max (cpr_NL (RLat1) - 1, 1);
            dlon1 = 360.0 / N1;
            int m = floor (0.5 + ((XZ0 * (cpr_NL (RLat1) - 1) - XZ1 * cpr_NL (RLat1)) / CPR_2NB_17));
            result_Lon = dlon1 * (cpr_MOD (m, N1) + (XZ1 / CPR_2NB_17));
            result_Lat = RLat1;
    }

    if ((result_Lon >= 180) && (result_Lon <= 360))
            result_Lon -= 360;
    Lat = result_Lat;
    Lon = result_Lon;

    return true;
}

bool me_cpr_position_subfields::decode_Local_LatLon (double Ref_Lat, double Ref_Lon, double &Lat, double &Lon, bool airborne) const {
    double dlat, dlon;
	double UHEL;
        uint8_t NZ=15;
        
	if (airborne)
		UHEL = 360.0;
	else
		UHEL = 90.0;

	/* step a. */
	if (F==0)
		dlat = UHEL / (4 * NZ);
	else
		dlat = UHEL / (4 * NZ - 1);

	/* step b: latitude zone index number */
	int j =  floor (Ref_Lat / dlat) + floor (0.5 + cpr_MOD (Ref_Lat, dlat) / dlat - (CPR_LAT / CPR_2NB_17));

	/* step c: decoded position latitude */
	Lat = dlat * (j + (CPR_LAT / CPR_2NB_17));

	/* step d: longitude zone size */
	if (F==0)
	{
		dlon = UHEL / cpr_NL (Lat);
	}
	else
	{
		int nn = cpr_NL (Lat) - 1;
		if (nn > 0)
			dlon = UHEL / nn;
		else
			dlon = UHEL;
	}

	/* step e: longitude zone coordinate */
	int m = floor (Ref_Lon / dlon) + floor (0.5 + cpr_MOD (Ref_Lon, dlon) / dlon - (CPR_LON / CPR_2NB_17));
	

	/* step f: decoded position longitude */
	Lon = dlon * (m + (CPR_LON / CPR_2NB_17));
        return true;
}

bool me_cpr_position_subfields::decode_Surface_LatLon(const me_cpr_position_subfields & pair,double &Lat, double &Lon) const {
    double dlat0, dlat1, dlon0, dlon1;
    double RLat0, RLat1;
    double result_Lat, result_Lon;
    double YZ0,YZ1,XZ0,XZ1;
    bool time_even;
//    uint8_t NZ =15;
    
    if (F==pair.F) return false;
    if (F==0) {
        XZ0 = CPR_LON;
        YZ0 = CPR_LAT;
        XZ1 = pair.CPR_LON;
        YZ1 = pair.CPR_LAT;
        time_even = true;
    } else {
        XZ1 = CPR_LON;
        YZ1 = CPR_LAT;
        XZ0 = pair.CPR_LON;
        YZ0 = pair.CPR_LAT;
        time_even = false;
    }
        
    dlat0 = 90.0 / 60.0;
    dlat1 = 90.0 / 59.0;

    int j = floor (0.5 + (59 * YZ0 - 60 * YZ1) / CPR_2NB_17);

    RLat0 = dlat0 * (cpr_MOD (j, 60) + (YZ0 / CPR_2NB_17));
    RLat1 = dlat1 * (cpr_MOD (j, 59) + (YZ1 / CPR_2NB_17));

    /* southern hemisphere values of RLati will fall in the range from 270 to
     * 360 degrees. subtract 360 deg from such values, thereby restoring Rlati
     * to the range from -90 to 90 degrees. */
    if ((RLat0 >= 270) && (RLat0 <= 360))
            RLat0 -= 360;

    if ((RLat1 >= 270) && (RLat1 <= 360))
            RLat1 -= 360;

    /* positions straddle a transition latitude */
    if (cpr_NL(RLat0) != cpr_NL(RLat1))
            return false;

    if (time_even)
    {
            double N0 = std::max (cpr_NL (RLat0), 1);
            dlon0 = 90.0 / N0;
            int m = floor (0.5 + ((XZ0 * (cpr_NL (RLat0) - 1) - XZ1 * cpr_NL (RLat0)) / CPR_2NB_17));
            result_Lon = dlon0 * (cpr_MOD (m, N0) + (XZ0 / CPR_2NB_17));
            result_Lat = RLat0;
    }
    else
    {
            double N1 = std::max (cpr_NL (RLat1) - 1, 1);
            dlon1 = 90.0 / N1;
            int m = floor (0.5 + ((XZ0 * (cpr_NL (RLat1) - 1) - XZ1 * cpr_NL (RLat1)) / CPR_2NB_17));
            result_Lon = dlon1 * (cpr_MOD (m, N1) + (XZ1 / CPR_2NB_17));
            result_Lat = RLat1;
    }

    if ((result_Lon >= 180) && (result_Lon <= 360))
            result_Lon -= 360;
    Lat = result_Lat;
    Lon = result_Lon;

    return true;
}


void me_airborne_position_fields::set_LatLon (double Lat, double Lon) {
    encode_Airborne_LatLon (Lat, Lon);
} 

void me_surface_position_fields::set_Movement_speed_km_h (double speed) {
    if (speed < 0) Movement = 0;
    else if (speed == 0) Movement = 1;
    else if (speed <= 0.2315) Movement = 2;
    else if (speed <= 1.852) Movement = (speed-0.2315) / ((1.852-0.2315)/6) + 3;
    else if (speed <= 3.704) Movement = (speed-1.852) / ((3.704-1.852)/4) + 9;
    else if (speed <= 27.78) Movement = (speed-3.704) / ((27.78-3.704)/16) + 13;
    else if (speed <= 129.64) Movement = (speed-27.78) / ((129.64-27.78)/55) + 39;
    else if (speed <= 185.2) Movement = (speed-129.64) / ((185.2-129.64)/15) + 94;
    else if (speed <= 324.1) Movement = (speed-185.2) / ((324.1-185.2)/15) + 109;
    else Movement = 124;
}

bool me_surface_position_fields::get_Movement_speed_km_h (double &speed) {
    if (Movement == 0) { speed = 0;return false; }
    else if (Movement == 1) speed = 0;
    else if (Movement == 2) speed = 0.2315;
    else if (Movement <= 8) speed = (Movement - 3) * ((1.852-0.2315)/6) + 0.2315;
    else if (Movement <= 12) speed = (Movement - 9) * ((3.704-1.852)/4) + 1.852;
    else if (Movement <= 38) speed = (Movement - 13) * ((27.78-3.704)/16) + 3.704;
    else if (Movement <= 93) speed = (Movement - 39) * ((129.64-27.78)/55) + 27.78;
    else if (Movement <= 108) speed = (Movement - 94) * ((185.2-129.64)/15) + 129.64;
    else if (Movement <= 123) speed = (Movement - 109) * ((324.1-185.2)/15) + 185.2;
    else speed = 324.1;

    return true;
}

void me_surface_position_fields::set_LatLon (double Lat, double Lon) {
    encode_Surface_LatLon (Lat, Lon);
} 


uint16_t me_airborne_position_ac_subfield::operator=(uint16_t value) {
    set(value);
    return value;
}

void me_airborne_position_ac_subfield::set (uint16_t value) {
    ALT_LO = value & 0xF;
    Q= (value >> 4) & 0x1;
    ALT_HI = (value >> 5) & 0x7f;
}

uint16_t me_airborne_position_ac_subfield::get() const {
    return (ALT_LO | (Q<<4) | (ALT_HI << 5)); 
}

void me_airborne_position_ac_subfield::set_Alt (int32_t feets) {
    uint32_t N;
    if (feets <= 50188) {
        Q = 1;
        N = (feets + 1000) / 25;
        ALT_LO = N & 0xF;
        ALT_HI = N >> 4;
        return;
    }
    Q = 0;
    N = (feets + 1000) / 100;
    ALT_LO = N & 0xF;
    ALT_HI = N >> 4;
}

int32_t me_airborne_position_ac_subfield::get_Alt() const {
    uint32_t N = ALT_LO | (ALT_HI<<4); 
    int32_t result;
    if (Q==1) {
        result = N*25-1000;
        return result;
    }
    result = N*100 -1000;
    return result;
}

char me_aircraft_ident_fields::decode_char(uint8_t coded_ch) {
    char retval;
    // not defined -> return exclamation
    if (coded_ch == 0x0 ||
    (coded_ch > 0x1a && coded_ch < 0x20) ||
    (coded_ch > 0x20 && coded_ch < 0x30) ||
    (coded_ch > 0x39))
        retval = 0x3f;
    // separation & digits
    else if (coded_ch & 0x20)
        retval = coded_ch;
    // chars
    else
        retval = coded_ch | 0x40;

    return retval;
}

uint8_t me_aircraft_ident_fields::encode_char(char ch) {
    if (ch & 0x20) return ch;
    return ch & 0x3f;
}

void me_aircraft_ident_fields::encode_Callsing(uint8_t *callsign) {
    for (int i = 0; i < 8; i++)
        Callsign[i] = encode_char(callsign[i]);
}

void me_aircraft_ident_fields::decode_Callsing(uint8_t *callsign) {
    for (int i = 0; i < 8; i++)
        callsign[i] = decode_char(Callsign[i]);
}

void me_airborne_velocity_fields::set_GROUND_VELOCITY_unknown() {
    SUBTYPE = ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL;
    VELOCITY.SUBTYPE_1_2.set_EAST_WEST_VELOCITY_unknown();
    VELOCITY.SUBTYPE_1_2.set_NORTH_SOUTH_VELOCITY_unknown();
}

void me_airborne_velocity_fields::set_GROUND_VELOCITY_knot(double east_west, double north_south) {
    if ((east_west>1021.0) || (north_south>1021.0)) {
        SUBTYPE = ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC;    
        VELOCITY.SUBTYPE_1_2.set_EAST_WEST_VELOCITY_SUPERSONIC_knot(east_west);
        VELOCITY.SUBTYPE_1_2.set_NORTH_SOUTH_VELOCITY_SUPERSONIC_knot(north_south);

    } else {
        SUBTYPE = ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL;
        VELOCITY.SUBTYPE_1_2.set_EAST_WEST_VELOCITY_NORMAL_knot(east_west);
        VELOCITY.SUBTYPE_1_2.set_NORTH_SOUTH_VELOCITY_NORMAL_knot(north_south);
    }    
}

void me_airborne_velocity_fields::set_GROUND_VELOCITY_km_h(double east_west, double north_south) {
    set_GROUND_VELOCITY_knot(east_west/1.852,north_south/1.852);
}

bool me_airborne_velocity_fields::get_EAST_WEST_VELOCITY_knot(double value) {
    if (SUBTYPE == ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL) {
        return VELOCITY.SUBTYPE_1_2.get_EAST_WEST_VELOCITY_NORMAL_knot(value);
    }
    if (SUBTYPE == ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC) {
        return VELOCITY.SUBTYPE_1_2.get_EAST_WEST_VELOCITY_SUPERSONIC_knot(value);
    }
    return false;
} 

bool me_airborne_velocity_fields::get_NORTH_SOUTH_VELOCITY_knot(double value) {
    if (SUBTYPE == ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_NORMAL) {
        return VELOCITY.SUBTYPE_1_2.get_NORTH_SOUTH_VELOCITY_NORMAL_knot(value);
    }
    if (SUBTYPE == ME_AIRBORNE_VELOCITY_SUBTYPE_GROUND_SPEED_SUPERSONIC) {
        return VELOCITY.SUBTYPE_1_2.get_NORTH_SOUTH_VELOCITY_SUPERSONIC_knot(value);
    }
    return false;
}

void me_airborne_velocity_fields::set_AIR_SPEED_unknown() {
    SUBTYPE = ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL;
    VELOCITY.SUBTYPE_3_4.set_AIR_SPEED_unknown();
}

void me_airborne_velocity_fields::set_AIR_SPEED_knot(double value) {
    if (value > 1021.0) {
        SUBTYPE = ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC; 
        VELOCITY.SUBTYPE_3_4.set_AIR_SPEED_SUPERSONIC_knot(value);
        return;
    }
    SUBTYPE = ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL;
    VELOCITY.SUBTYPE_3_4.set_AIR_SPEED_NORMAL_knot(value);
}

void me_airborne_velocity_fields::set_AIR_SPEED_km_h(double value) {
    set_AIR_SPEED_knot(value / 1.852);
}

void me_airborne_velocity_fields::set_AIR_HEADING(double value) {
    VELOCITY.SUBTYPE_3_4.set_HEADING(value);
}


bool me_airborne_velocity_fields::get_AIR_HEADING(double & value) {
    if ((SUBTYPE != ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC) && (SUBTYPE != ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL)) return false;
    value = VELOCITY.SUBTYPE_3_4.get_HEADING();
    return true;
}

bool me_airborne_velocity_fields::get_AIR_SPEED_knot(double & value) {
    if (SUBTYPE == ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_NORMAL) {
        return VELOCITY.SUBTYPE_3_4.get_AIR_SPEED_NORMAL_knot(value);
    }
    if (SUBTYPE == ME_AIRBORNE_VELOCITY_SUBTYPE_AIR_SPEED_SUPERSONIC) {
        return VELOCITY.SUBTYPE_3_4.get_AIR_SPEED_SUPERSONIC_knot(value);
    }
    return false;
}

void me_airborne_velocity_fields::set_VERTICAL_RATE_unknown() {
    VERTICAL_RATE=0;
}

void me_airborne_velocity_fields::set_VERTICAL_RATE_feet_min(double value) {
    if (value > 32608) { VERTICAL_RATE=511; return;}
    VERTICAL_RATE = (value/64)+1;
}

bool me_airborne_velocity_fields::get_VERTICAL_RATE_feet_min(double & value) {
    if (VERTICAL_RATE==0) return false;
    value = (VERTICAL_RATE-1)*64;
    return true;
}

void me_airborne_velocity_fields::set_DIFF_BARO_unknown() {
    DIFF_BARO=0;
}

void me_airborne_velocity_fields::set_DIFF_BARO_feet(uint16_t value) {
    if (value > 3137) { DIFF_BARO = 127; return;}
    DIFF_BARO = (value/25)+1;
}

bool me_airborne_velocity_fields::get_DIFF_BARO_feet(uint16_t &value) {
    if (DIFF_BARO==0) return false;
    value = (DIFF_BARO-1)*25;
    return true;
}

uint16_t me_m3a_subfield::get_M3A_octal() {
    bool a1, a2, a4, b1, b2, b4, c1, c2, c4, d1, d2, d4;

    c1 = (M3A & 0x1000) >> 12;
    a1 = (M3A & 0x800) >> 11;
    c2 = (M3A & 0x400) >> 10;
    a2 = (M3A & 0x200) >> 9;
    c4 = (M3A & 0x100) >> 8;
    a4 = (M3A & 0x80) >> 7;
    b1 = (M3A & 0x20) >> 5;
    d1 = (M3A & 0x10) >> 4;
    b2 = (M3A & 0x8) >> 3;
    d2 = (M3A & 0x4) >> 2;
    b4 = (M3A & 0x2) >> 1;
    d4 = M3A & 0x1;

    return (a4 << 11) | (a2 << 10) | (a1 << 9) | (b4 << 8) | (b2 << 7) | (b1 << 6) | (c4 << 5) | (c2 << 4) | (c1 << 3) | (d4 << 2) | (d2 << 1) | d1;
}

void me_m3a_subfield::set_M3A_octal(uint16_t m3a) {
    bool a1, a2, a4, b1, b2, b4, c1, c2, c4, d1, d2, d4;

    a4 = (m3a & 0x800) >> 11;
    a2 = (m3a & 0x400) >> 10;
    a1 = (m3a & 0x200) >> 9;
    b4 = (m3a & 0x100) >> 8;
    b2 = (m3a & 0x80) >> 7;
    b1 = (m3a & 0x40) >> 6;
    c4 = (m3a & 0x20) >> 5;
    c2 = (m3a & 0x10) >> 4;
    c1 = (m3a & 0x8) >> 3;
    d4 = (m3a & 0x4) >> 2;
    d2 = (m3a & 0x2) >> 1;
    d1 = m3a & 0x1;

    M3A = (c1 << 12) | (a1 << 11) | (c2 << 10) | (a2 << 9) | (c4 << 8) | (a4 << 7) | (b1 << 5) | (d1 << 4) | (b2 << 3) | (d2 << 2) | (b4 << 1) | d4;
}
