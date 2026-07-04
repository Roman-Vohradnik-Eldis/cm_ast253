#ifndef _RDF_TARGET_MODES_HH
#define _RDF_TARGET_MODES_HH

#include "RDFHeader.hh"

#define RDF_INVALID_GILLHAM_CODE	-10000
#define RDF_INVALID_FL			-10001

enum AIRCRAFT_CATEGORY
{
	AIRCRAFT_CATEGORY_A_NONE,			// sada A, 0 - nejsou k dispozici zadne udaje o kategorii letadla
	AIRCRAFT_CATEGORY_A_1,				// sada A, 1 - lehke (15500 liber nebo 7031 kg)
	AIRCRAFT_CATEGORY_A_2,				// sada A, 2 - stredni c. 1 (15500 az 75000 liber nebo 7031 az 34019 kg)
	AIRCRAFT_CATEGORY_A_3,				// sada A, 3 - stredni c. 2 (75000 az 300000 liber nebo 34019 az 136078 kg)
	AIRCRAFT_CATEGORY_A_4,				// sada A, 4 - letadlo s vysokymi turbulencemi v uplavu
	AIRCRAFT_CATEGORY_A_5,				// sada A, 5 - tezke (> 300000 liber nebo 136078 kg)
	AIRCRAFT_CATEGORY_A_6,				// sada A, 6 - vysoky vykon (zrychleni s vice nez 5 G), vysoka rychlost (> 400 uzlu)
	AIRCRAFT_CATEGORY_A_7,				// sada A, 7 - vrtulnik
	AIRCRAFT_CATEGORY_B_NONE,			// sada B, 0 - nejsou k dispozici zadne udaje o kategorii letadla
	AIRCRAFT_CATEGORY_B_1,				// sada B, 1 - kluzak / vetron
	AIRCRAFT_CATEGORY_B_2,				// sada B, 2 - lehci nez vzduch
	AIRCRAFT_CATEGORY_B_3,				// sada B, 3 - parasutista / parasutista-akrobat
	AIRCRAFT_CATEGORY_B_4,				// sada B, 4 - ultralehke / zavesny kluzak / padakovy kluzak
	AIRCRAFT_CATEGORY_B_5,				// sada B, 5 - rezervovano
	AIRCRAFT_CATEGORY_B_6,				// sada B, 6 - bezpilotni letadlo
	AIRCRAFT_CATEGORY_B_7,				// sada B, 7 - vesmirna lod / transatmosfericke vozidlo
	AIRCRAFT_CATEGORY_C_NONE,			// sada C, 0 - nejsou k dispozici zadne udaje o kategorii letadla
	AIRCRAFT_CATEGORY_C_1,				// sada C, 1 - pozemni vozidlo - pohotovostni vozidlo
	AIRCRAFT_CATEGORY_C_2,				// sada C, 2 - pozemni vozidlo - servisni vozidlo
	AIRCRAFT_CATEGORY_C_3,				// sada C, 3 - pevna pozemni prekazka nebo prekazka upevnena retezem
	AIRCRAFT_CATEGORY_C_4,				// sada C, 4 az 7 - rezervovano
	AIRCRAFT_CATEGORY_D_NONE,			// sada D, 0 - nejsou k dispozici zadne udaje o kategorii letadla - rezervovano
};

enum FOM_SOURCE
{
	FOM_SOURCE_INVALID,				// 0 - Invalid
	FOM_SOURCE_INS,					// 1 - INS
	FOM_SOURCE_GNSS,				// 2 - GNSS
	FOM_SOURCE_DME_DME,				// 3 - DME/DME
	FOM_SOURCE_VOR_DME,				// 4 - VOR/DME
};

enum HAZARD_CODING
{
	HAZARD_CODING_NIL,				// 0 - NIL
	HAZARD_CODING_LIGHT,				// 1 - LIGHT
	HAZARD_CODING_MODERATE,				// 2 - MODERATE
	HAZARD_CODING_SEVERE,				// 3 - SEVERE
};

struct RDFTargetModeS
{
	unsigned char MessageData[7];		// 56-bit Mode S message
	unsigned char Address;				// 4-bit Data Buffer Store 1 Address
										// + 4-bit Data Buffer Store 2 Address
	unsigned char Age;					// stari BDS registru (sec)

	RDFTargetModeS ();
	RDFTargetModeS (void * data);

	void assign (void * data);
	void erase ();
	bool equal (const RDFTargetModeS & data);
	bool operator == (const RDFTargetModeS & data) { return equal (data); }
	bool operator != (const RDFTargetModeS & data) { return ! equal (data); }

	int ABCD_gillham (const int ABCD);
	int gillham_fl (const int ABCD);

	void Get05 (
			unsigned char &format_type,
			unsigned char &surveillance_status,
			bool &saff_single,
			bool &time,
			bool &cpr_format_odd,
			bool &alt_available,
			double &altitude
			);

	void Get09 (
		unsigned char &format_type,
		unsigned char &subtype,
		bool &intent_change,
		bool &ifr_capability,
		unsigned char &nu_category,
		bool &ew_direction,
		double &ew_velocity,	//unsigned/nebo signed i s ew_dir?!
		bool &ns_direction,
		double &ns_velocity,		//dtto
		double &magnetic_heading,
		bool &airspeed_type,
		double &airspeed,
		bool &vertical_rate_src,
		bool &vertical_rate_available,
		double &vertical_rate,
		bool &height_diff_available,
		double &gnss_diff_from_baro);

	// data link capability report
	void Get10 (
		bool &continuation_flag,
		unsigned char & modes_subnetwork_version_number,
		unsigned char & transponder_enhanced_protocol_indicator,
		unsigned char & modes_specific_services_capability,
		unsigned char & uplink_elm_capability,
		unsigned char & downlink_elm_capability,
		unsigned char & aircraft_identification_capability,
		unsigned char & squitter_capability_subfield,
		unsigned char & surveilance_identifier,
		unsigned char & common_usage_gicb_capability_report,
		bool & acas_bit,
		unsigned char &acas_data,
		bool & overlay_command_capability,
		std::vector<int> &status_zero,
		std::vector<int> &status_one);

	// Common usage GICB capability report
	void Get17 (std::vector<int> & registers);
	void Get18 (std::vector<int> & registers);
	void Get19 (std::vector<int> & registers);
	void Get1A (std::vector<int> & registers);
	void Get1B (std::vector<int> & registers);
	void Get1C (std::vector<int> & registers);

	void Get1D (std::vector<int> & MSP_uplink, std::vector<int> & MSP_downlink);
	void Get1E (std::vector<int> & MSP_uplink, std::vector<int> & MSP_downlink);
	void Get1F (std::vector<int> & MSP_uplink, std::vector<int> & MSP_downlink);
	
	// Aircraft identification
	void Get20 (
		string & aircraft_identification);

	// Aircraft and airline registration markings
	void Get21 (
		string & aircraft_registration_number, 
		string & icao_airline_rgistration_marking);

	void Get22 (
		unsigned char &ant1_type,
		unsigned char &ant1_x,
		unsigned char &ant1_z,
		unsigned char &ant2_type,
		unsigned char &ant2_x,
		unsigned char &ant2_z,
		unsigned char &ant3_type,
		unsigned char &ant3_x,
		unsigned char &ant3_z,
		unsigned char &ant4_type,
		unsigned char &ant4_x,
		unsigned char &ant4_z);

	// Aircraft type
	void Get25 (
		unsigned char & aircraft_type, 
		unsigned char & number_of_engines,
		unsigned char & engine_type,
		string & model_designation, 
		unsigned char & wake_turbulence_category);

	void Get30 (
		bool &RA_terminated,
		unsigned char & advisory_type,
		unsigned int & advisory_data,
		bool & s_addr_present,
		unsigned int & s_addr,
		bool & height_present,
		double & height,
		bool & t_range_present,
		double & t_range,
		bool & t_bearing_present,
		double & t_bearing);

	void Get40 (
		unsigned short int &mcp_altitude,
		unsigned short int &fms_altitude,
		double &barometric_pressure_setting,
		bool &modes,
		bool &vnav_mode,
		bool &alt_hold_mode,
		bool &approach_mode,
		bool &altitude_source,
		unsigned char &tgt_alt_src);

	// Next waypoint details
	void Get41 (
		string & next_waypoint);

	// Next waypoint details
	void Get42 (
		double &wp_latitude,
		double &wp_longitude,
		double &cross_altitude);

	// Next waypoint details
	void Get43 (
		double &bearing,
		double &time_to_go,
		double &distance);

	// Meteorological routine air report
	void Get44 (
		unsigned char &fom_source,
		double &wind_speed,
		double &wind_direction,
		double &static_air_temperature,
		double &static_air_presure,
		unsigned char &turbolence,
		double &humidity);

	// Meteorological hazard report
	void Get45 (
		unsigned char &turbolence,
		unsigned char &wind_shear,
		unsigned char &microburst,
		unsigned char &icing,
		unsigned char &wake_vortex,
		double &static_air_temperature,
		double &average_static_presure,
		double &radio_height);

	// VHF channel report
	void Get48 (
		double &VHF1,
		unsigned char &VHF1VDLflags,
		unsigned char &VHF1status,
		double &VHF2,
		unsigned char &VHF2VDLflags,
		unsigned char &VHF2status,
		double &VHF3,
		unsigned char &VHF3VDLflags,
		unsigned char &VHF3status,
		unsigned char &VHF_EMG_status);

	// Track and turn report
	void Get50 (
		double & roll_angle,
		double & true_track_angle, 
		double & ground_speed,
		double & track_angle_rate,
		double & true_airspeed);

	// Position report coarse
	void Get51 (
		double &latitude,
		double &longitude,
		double &pressure_altitude);

	// Position report fine
	void Get52 (
		unsigned char &rnp,
		double &latitude_fine,
		double &longitude_fine,
		double &altitude_or_height);
		
	// Air-referenced state vector
	void Get53 (
		double &magnetic_heading,
		double &indicated_airspeed,
		double &mach_number,
		double &true_airspeed,
		double &altitude_rate);

	// Quasi-static parameter monitoring
	void Get5F (
		unsigned char &mcp_selected_alt,
		unsigned char &next_waypoint,
		unsigned char &fms_vertical_mode,
		unsigned char &vhf_channel_report,
		unsigned char &meteo_hazards,
		unsigned char &fms_selected_alt);

	// Heading and speed report
	void Get60 (
		double &magnetic_heading,
		double &indicated_airspeed,
		double &mach,
		double &altitude_rate,
		double &vertical_velocity);

	// Extended squitter emergency/priority status
	void Get61 (
		unsigned char &subtype,
		unsigned char &state);

	void Get61 (
		unsigned char &subtype,
		unsigned char &state,
		bool &RA_terminated,
		unsigned char & advisory_type,
		unsigned int & advisory_data,
		bool & s_addr_present,
		unsigned int & s_addr,
		bool & height_present,
		double & height,
		bool & t_range_present,
		double & t_range,
		bool & t_bearing_present,
		double & t_bearing);

	// Target state and status information
	void Get62 (
		unsigned char &subtype,
		unsigned char &vert_data_avail,
		unsigned char &target_alt_type,
		unsigned char &target_alt_capa,
		unsigned char &vert_mode_ind,
		unsigned int  &target_alt,
		unsigned char &horiz_data_avail,
		unsigned int  &target_heading,
		unsigned char &target_heading_track,
		unsigned char &horiz_mode_ind,
		unsigned char &NAC,
		unsigned char &NIC,
		unsigned char &SIL,
		unsigned char &ACAS_not_oper,
		unsigned char &ACAS_RA_active,
		unsigned char &EMG_status);

	// Aircraft operational status
	void Get65 (	
		unsigned char &format_type_code,
		unsigned char &subtype_code,
		unsigned char &en_route_cap,
		unsigned char &terminal_area_cap,
		unsigned char &approach_cap,
		unsigned char &surface_cap,
		unsigned char &en_route_status,
		unsigned char &terminal_area_status,
		unsigned char &approach_status,
		unsigned char &surface_status);

	// shared part for 4 functions
	void GetE3_E4_E5_E6 (
		bool & supported_format,
		string & out_string);

	// Transponder type/part number
	void GetE3 (
		bool & supported_format,
		string & out_string);


	// Transponder software revision number
	void GetE4 (
		bool & supported_format,
		string & out_string);

	// ACAS unit part number
	void GetE5 (
		bool & supported_format,
		string & out_string);

	// ACAS Unit Software Revision
	void GetE6 (
		bool & supported_format,
		string & out_string);

	void GetE1_ELDIS_2014 (
		unsigned char & ILA,
		unsigned char & ILB,
		unsigned char & UPS,
		std::vector<int> & II_SI);

	void GetE2_ELDIS_2014 (
		std::vector<int> & II_SI);
};

bool is_datalink_capability (const RDFTargetModeS &modes);

int Get51and52 (unsigned char *Data51, unsigned char *Data52, double &lat, double &lon, double &alt, double &alt_gnss);
            // retval int -2 = invalid_all , -1=only_bds51, 0..15=RNP
            // Data51 or Data52 can be NULL, if not present, else 7 bytes array
            // alt_gnns valid only for rnp = 11..15

#endif /* _RDF_TARGET_MODES_HH */
