#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "RDFTargetModeS.hh"
#include "CMSDebug.hh"

unsigned bits2bit (unsigned char * data, unsigned pos)
{
	unsigned pos_byte = pos / 8;
	unsigned pos_byte_offset = pos % 8;

	return (data[pos_byte] >> (7 - pos_byte_offset)) & 1;
}

unsigned bits2unsigned (unsigned char * data, unsigned first, unsigned last)
{
	if (first > last)
	{
		cms_error ("first > last (%d, %d)", first, last);
		return 0;
	}
	
	unsigned first_byte = first / 8;
	unsigned first_byte_offset = first % 8;
	unsigned last_byte = last / 8;
	unsigned last_byte_offset = last % 8;
	unsigned value;

	if (first_byte == last_byte)
	{
		value = ((unsigned char)(data[first_byte] << first_byte_offset))
			>> (first_byte_offset + 7 - last_byte_offset);
	}
	else
	{
		value = ((unsigned char)(data[first_byte] << first_byte_offset))
			>> first_byte_offset;

		for (unsigned i = first_byte + 1; i < last_byte; i++)
		{
			value = (value << 8) | data[i];
		}

		value = (value << (last_byte_offset + 1))
			| (data[last_byte] >> (7 - last_byte_offset));
	}

	return value;
}

int bits2signed (unsigned char * data, unsigned first, unsigned last)
{
	if (first > last)
	{
		cms_error ("first > last (%d, %d)", first, last);
		return 0;
	}
	
	unsigned first_byte = first / 8;
	unsigned first_byte_offset = first % 8;
	unsigned last_byte = last / 8;
	unsigned last_byte_offset = last % 8;
	int value;

	
	if (first_byte == last_byte)
	{
		value = ((char)(data[first_byte] << (first_byte_offset + 24)))
			>> (first_byte_offset + 7 - last_byte_offset + 24);
	}
	else
	{
		value = ((char)(data[first_byte] << first_byte_offset)) >> first_byte_offset;

		for (unsigned i = first_byte + 1; i < last_byte; i++)
		{
			value = (value << 8) | data[i];
		}

		value = (value << (last_byte_offset + 1))
			| (data[last_byte] >> (7 - last_byte_offset));
	}

	return value;
}

RDFTargetModeS::RDFTargetModeS ()
{
	memset (MessageData, 0, sizeof (MessageData));
	Address = 0;
	Age = 0;
}

RDFTargetModeS::RDFTargetModeS (void * data)
{
	memcpy (MessageData, data, 7);
	Address = ((unsigned char *) data)[7];
	Age = 0;
}

bool RDFTargetModeS::equal (const RDFTargetModeS & data)
{
	if (Address != data.Address)
	{
		return false;
	}
	return !memcmp (MessageData, data.MessageData, 7);
}

void RDFTargetModeS::Get20 (
	string & aircraft_identification)
{
	aircraft_identification.erase ();
	
	if (Address != 0x20)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (!MessageData[0] && !MessageData[1] && !MessageData[2] && !MessageData[3]
		&& !MessageData[4] && !MessageData[5] && !MessageData[6])
	{
		return;
	}
		
	if (MessageData[0] != 0x20)
	{
		cms_warning ("wrong message %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx",
			MessageData[0], MessageData[1], MessageData[2], MessageData[3],
			MessageData[4], MessageData[5], MessageData[6]);
		return;
	}
	
	asterix_decode_callsign (MessageData + 1, aircraft_identification);
}

char DekodujZnak2 (unsigned char znak)
{
	char retval;
	if (znak == 0)
	{
		retval = 32;
	}
	else if (znak & 0x20)
	{
		retval = znak;
	}
	else
	{
		retval = znak | 0x40;
	}
	return retval;
}

void RDFTargetModeS::Get05 (
		unsigned char &format_type,
		unsigned char &surveillance_status,
		bool &saff_single,
		bool &time,
		bool &cpr_format_odd,
		bool &alt_available,
		double &altitude)
{
	if (Address != 0x05)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	format_type = bits2unsigned (MessageData, 0, 4);
	surveillance_status = bits2unsigned (MessageData, 5, 6);
	saff_single = bits2bit (MessageData, 7);
	// 8 -- 19 altitude (specified by the format type code)
	
	//TODO dodelat... (btw davaj mi v 05 i jinej format_type nez 19?)
	if (format_type == 19)
	{
		altitude= bits2unsigned (MessageData, 49, 55);
		if (bits2bit (MessageData, 48))
		{
			altitude *= -1;
		}
		alt_available = (altitude != 0);
		if (altitude > 0)
		{
			altitude = (altitude - 1) * 25 * FEET_TO_METER;
		}
		else
		{
			altitude = altitude * 25 * FEET_TO_METER;
		}
	}

	time = bits2bit (MessageData, 20);
	cpr_format_odd = bits2bit (MessageData, 21);
	//TODO komedie, musel bych si ukladat nejakou historii odpovedi,
	// navic time diff < 10 s
	// 22 -- 38 encoded latitude
	// 39 -- 55 encoded longitude

}

void RDFTargetModeS::Get09 (
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
		double &gnss_diff_from_baro)
{
	if (Address != 0x09)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	magnetic_heading = 0.0;

	format_type = bits2unsigned (MessageData, 0, 4);
	subtype = bits2unsigned (MessageData, 5, 7);
	intent_change = bits2bit (MessageData, 8);
	ifr_capability = bits2bit (MessageData, 9);
	nu_category = bits2unsigned (MessageData, 10, 12);

	if (subtype == 1 || subtype == 2)
	{
		ew_direction = bits2bit (MessageData, 13);
		ew_velocity = (bits2unsigned (MessageData, 14, 23) - 1) * KNOT_TO_SPEED;
		ns_direction = bits2bit (MessageData, 24);
		ns_velocity = (bits2unsigned (MessageData, 25, 34) - 1) * KNOT_TO_SPEED;
		if (subtype == 2)
		{
			ew_velocity *= 4;
			ns_velocity *= 4;
		}
	}
	else if (subtype == 3 || subtype == 4)
	{
		if (bits2bit (MessageData, 13))
		{
			magnetic_heading = bits2unsigned (MessageData, 14, 23) * (2 * M_PI) / 1024;
		}
		airspeed_type = bits2bit (MessageData, 24);
		airspeed = (bits2unsigned (MessageData, 25, 34) - 1) * KNOT_TO_SPEED;
		if (subtype == 4)
		{
			airspeed *= 4;
		}
	}

	vertical_rate_src = bits2bit (MessageData, 35);
	vertical_rate = bits2unsigned (MessageData, 37, 45);
	if (bits2bit (MessageData, 36))
	{
		vertical_rate *= -1;
	}
	vertical_rate_available = (vertical_rate != 0);
	if (vertical_rate > 0)
	{
		vertical_rate = (vertical_rate - 1) * 64 * FEET_TO_METER / 60.0;
	}
	else
	{
		vertical_rate = vertical_rate * 64 * FEET_TO_METER / 60.0;
	}

	gnss_diff_from_baro = bits2unsigned (MessageData, 49, 55);
	if (bits2bit (MessageData, 48))
	{
		gnss_diff_from_baro *= -1;
	}
	height_diff_available = (gnss_diff_from_baro != 0);
	if (gnss_diff_from_baro > 0)
	{
		gnss_diff_from_baro = (gnss_diff_from_baro - 1) * 25 * FEET_TO_METER;
	}
	else
	{
		gnss_diff_from_baro = gnss_diff_from_baro * 25 * FEET_TO_METER;
	}
}

void RDFTargetModeS::Get10 (
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
	bool &overlay_command_capability,
	std::vector<int> &status_zero,
	std::vector<int> &status_one)
{
	continuation_flag = bits2bit (MessageData, 8);
	modes_subnetwork_version_number = bits2unsigned (MessageData, 16, 22);
	transponder_enhanced_protocol_indicator = bits2bit (MessageData, 23);
	modes_specific_services_capability = bits2bit (MessageData, 24);
	uplink_elm_capability = bits2unsigned (MessageData, 25, 27);
	downlink_elm_capability = bits2unsigned (MessageData, 28, 31);
	aircraft_identification_capability = bits2bit (MessageData, 32);
	squitter_capability_subfield = bits2bit (MessageData, 33);
	surveilance_identifier = bits2bit (MessageData, 34);
	common_usage_gicb_capability_report = bits2bit (MessageData, 35);
	acas_bit = bits2bit (MessageData, 15);
	acas_data = bits2unsigned (MessageData, 36, 39);

	overlay_command_capability = bits2bit (MessageData, 14); // added 2014-12-17

	for (int i = 0; i < 16; ++i)
	{
		if (bits2bit (MessageData, i + 40) == 1)
		{
			status_one.push_back (i);
		}
		else
		{
			status_zero.push_back (i);
		}
	}
}

void RDFTargetModeS::Get17 (std::vector<int> & registers)
{
	const int bit2reg[] = {	0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x20, 0x21,
							0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x50,
							0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x5f, 0x60,
							0x00, 0x00, 0xe1, 0xe2, 0xf1};
	registers.clear ();

	if (Address != 0x17)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (int i = 0; i < 29; ++i)
	{
		// bits 24, 25: reserved for aircraft capability
		if (bits2bit (MessageData, i) && i != 24 && i != 25)
		{
			registers.push_back (bit2reg[i]);
		}
	}
}

void RDFTargetModeS::Get18 (std::vector<int> & registers)
{
	int i, r;
	
	registers.clear ();

	if (Address != 0x18)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i = 0, r = 0x38; r >= 0x01; i++, r--)
	{
		if (bits2bit (MessageData, i))
		{
			registers.push_back (r);
		}
	}
}

void RDFTargetModeS::Get19 (std::vector<int> & registers)
{
	int i, r;
	
	registers.clear ();

	if (Address != 0x19)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i = 0, r = 0x70; r >= 0x39; i++, r--)
	{
		if (bits2bit (MessageData, i))
		{
			registers.push_back (r);
		}
	}
}

void RDFTargetModeS::Get1A (std::vector<int> & registers)
{
	int i, r;
	
	registers.clear ();

	if (Address != 0x1A)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i = 0, r = 0xA8; r >= 0x71; i++, r--)
	{
		if (bits2bit (MessageData, i))
		{
			registers.push_back (r);
		}
	}
}

void RDFTargetModeS::Get1B (std::vector<int> & registers)
{
	int i, r;
	
	registers.clear ();

	if (Address != 0x1B)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i = 0, r = 0xE0; r >= 0xA9; i++, r--)
	{
		if (bits2bit (MessageData, i))
		{
			registers.push_back (r);
		}
	}
}

void RDFTargetModeS::Get1C (std::vector<int> & registers)
{
	int i, r;
	
	registers.clear ();

	if (Address != 0x1C)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i = 25, r = 0xFF; r >= 0xE1; i++, r--)
	{
		if (bits2bit (MessageData, i))
		{
			registers.push_back (r);
		}
	}
}

void RDFTargetModeS::Get1D (std::vector<int> & MSP_uplink, std::vector<int> & MSP_downlink)
{
	int i, r;
	
	MSP_uplink.clear ();
	MSP_downlink.clear ();

	if (Address != 0x1D)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i =  0, r = 1; r <= 28; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			MSP_uplink.push_back (r);
		}
	}
	for (i = 28, r = 1; r <= 28; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			MSP_downlink.push_back (r);
		}
	}
}

void RDFTargetModeS::Get1E (std::vector<int> & MSP_uplink, std::vector<int> & MSP_downlink)
{
	int i, r;
	
	MSP_uplink.clear ();
	MSP_downlink.clear ();

	if (Address != 0x1E)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i =  0, r = 29; r <= 56; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			MSP_uplink.push_back (r);
		}
	}
	for (i = 28, r = 29; r <= 56; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			MSP_downlink.push_back (r);
		}
	}
}

void RDFTargetModeS::Get1F (std::vector<int> & MSP_uplink, std::vector<int> & MSP_downlink)
{
	int i, r;
	
	MSP_uplink.clear ();
	MSP_downlink.clear ();

	if (Address != 0x1F)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (i =  0, r = 57; r <= 63; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			MSP_uplink.push_back (r);
		}
	}
	for (i = 28, r = 57; r <= 63; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			MSP_downlink.push_back (r);
		}
	}
}



void RDFTargetModeS::Get21 (
	string & aircraft_registration_number, 
	string & icao_airline_rgistration_marking)
{
	char text[8];

	aircraft_registration_number.erase ();
	icao_airline_rgistration_marking.erase ();

	if (Address != 0x21)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		text[0] = DekodujZnak2 (bits2unsigned (MessageData, 1, 6));
		text[1] = DekodujZnak2 (bits2unsigned (MessageData, 7, 12));
		text[2] = DekodujZnak2 (bits2unsigned (MessageData, 13, 18));
		text[3] = DekodujZnak2 (bits2unsigned (MessageData, 19, 24));
		text[4] = DekodujZnak2 (bits2unsigned (MessageData, 25, 30));
		text[5] = DekodujZnak2 (bits2unsigned (MessageData, 31, 36));
		text[6] = DekodujZnak2 (bits2unsigned (MessageData, 37, 42));
		aircraft_registration_number.assign (text, 7);
	}

	if (bits2bit (MessageData, 43))
	{
		text[0] = DekodujZnak2 (bits2unsigned (MessageData, 44, 49));
		text[1] = DekodujZnak2 (bits2unsigned (MessageData, 50, 55));
		icao_airline_rgistration_marking.assign (text, 2);
	}
}

void RDFTargetModeS::Get22 (
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
		unsigned char &ant4_z)
{
	ant1_type = ant1_x = ant1_z = 0;
	ant2_type = ant2_x = ant2_z = 0;
	ant3_type = ant3_x = ant3_z = 0;
	ant4_type = ant4_x = ant4_z = 0;

	if (Address != 0x22)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	ant1_type = (bits2unsigned (MessageData, 0, 2));
	ant1_x = bits2unsigned (MessageData, 3, 8);
	ant1_z = bits2unsigned (MessageData, 9, 13);

	ant2_type = (bits2unsigned (MessageData, 14, 16));
	ant2_x = bits2unsigned (MessageData, 17, 22);
	ant2_z = bits2unsigned (MessageData, 23, 27);

	ant3_type = (bits2unsigned (MessageData, 28, 30));
	ant3_x = bits2unsigned (MessageData, 31, 36);
	ant3_z = bits2unsigned (MessageData, 37, 41);

	ant4_type = (bits2unsigned (MessageData, 42, 44));
	ant4_x = bits2unsigned (MessageData, 45, 50);
	ant4_z = bits2unsigned (MessageData, 51, 55);
}

void RDFTargetModeS::Get25 (
	unsigned char & aircraft_type, 
	unsigned char & number_of_engines,
	unsigned char & engine_type,
	string & model_designation, 
	unsigned char & wake_turbulence_category)
{
	char text[5];
	
	aircraft_type = 0;
	number_of_engines = 0;
	engine_type = 0;
	model_designation.erase ();
	wake_turbulence_category = 0;

	if (Address != 0x25)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	aircraft_type = bits2unsigned (MessageData, 0, 5);
	number_of_engines = bits2unsigned (MessageData, 6, 8);
	engine_type = bits2unsigned (MessageData, 9, 14);
	text[0] = asterix_decode_char (bits2unsigned (MessageData, 15, 20));
	text[1] = asterix_decode_char (bits2unsigned (MessageData, 21, 26));
	text[2] = asterix_decode_char (bits2unsigned (MessageData, 27, 32));
	text[3] = asterix_decode_char (bits2unsigned (MessageData, 33, 38));
	text[4] = asterix_decode_char (bits2unsigned (MessageData, 39, 44));
	model_designation.assign (text, 5);
	wake_turbulence_category = bits2unsigned (MessageData, 45, 50);
}




int RDFTargetModeS::ABCD_gillham (const int ABCD)
{
	int ret_val = ((ABCD & 0x2) << 9) | ((ABCD & 0x4) << 7);
	ret_val |= ((ABCD & 0x920) >> 5) | ((ABCD & 0x490) >> 3) | ((ABCD & 0x248) >> 1); 
	return ret_val;
}

int RDFTargetModeS::gillham_fl (const int ABCD)
{
	int gillham = ABCD_gillham (ABCD);

	if ((gillham & 0x7) == 0 || (gillham & 0x7) == 5  || (gillham & 0x7) == 7)
		return RDF_INVALID_GILLHAM_CODE;

	int alt = gillham >> 3;
	alt ^= alt >> 1;
	alt ^= alt >> 2;
	alt ^= alt >> 4;
	alt ^= alt >> 8;

	int diff = 0;

	switch (gillham % 8)
	{
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


void RDFTargetModeS::Get30(
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
			double & t_bearing)
{
  s_addr_present = false;
  height_present = false;
  t_range_present = false;
  t_bearing_present = false;

  int ARA = bits2unsigned(MessageData,8,8);
  int RA_TERMINATED = bits2unsigned(MessageData,26,26);
  int MTE = bits2unsigned(MessageData,27,27);
  int TTI = bits2unsigned(MessageData,28,29);
  RA_terminated = false;
  if (RA_TERMINATED) RA_terminated = true;

  advisory_type = 0; // UNKNOWN
  if (ARA==1)
  {
    advisory_type = 1; // Single
    advisory_data = bits2unsigned(MessageData, 9, 14);
  } else if (MTE==1)
  {
    advisory_type = 1; // Multi
    advisory_data = bits2unsigned(MessageData, 9, 14);
  }
  if (TTI==1)
  {
    s_addr_present = true;
    s_addr = bits2unsigned(MessageData,30,53);
  } else if (TTI==2)  // vyzkouseno na situaci z 2014/09/18 09:35:18 (radar Warszawa v ELDISu) ... letadlo SAddr 498422 z vladni letky a za nej nalitne SIF vojak
                      // RA corrective, RA positive , Alt=26, Range=0.15-0.25NM , Bearing=204-210dg
  {
    unsigned TIDA = bits2unsigned(MessageData,30,42);
    uint16_t height_loc=0;
    height_loc |= (TIDA&0x0001)<<2;	//D4  0 => 2
    height_loc |= (TIDA&0x0002)<<7;	//B4  1 => 8
    height_loc |= (TIDA&0x0004)>>1;	//D2  2 => 1
    height_loc |= (TIDA&0x0008)<<4;	//B2  3 => 7
    height_loc |= (TIDA&0x0020)<<1;	//B1  5 => 6
    height_loc |= (TIDA&0x0080)<<4;	//A4  7 =>11
    height_loc |= (TIDA&0x0100)>>3;	//C4  8 => 5
    height_loc |= (TIDA&0x0200)<<1;	//A2  9 =>10
    height_loc |= (TIDA&0x0400)>>6;	//C2 10 => 4
    height_loc |= (TIDA&0x0800)>>2;	//A1 11 => 9
    height_loc |= (TIDA&0x1000)>>9;	//C1 12 => 3
    int Altitude=gillham_fl(height_loc);
    if (Altitude != RDF_INVALID_GILLHAM_CODE)
    {
      height_present = true;
      height = ((double)Altitude * 100.) * 0.3048;
    }
    unsigned TIDR = bits2unsigned(MessageData,43,49);
    if (TIDR!=0)
    {
      t_range_present = true;
      t_range = (TIDR-1) * 0.1;
    }
    unsigned TIDB = bits2unsigned(MessageData,50,55);
    t_bearing_present = true;
    t_bearing = ((((TIDB-1)&0x3f) * 6) + 3);
  }
}


void RDFTargetModeS::Get40 (
	unsigned short int &mcp_altitude,
	unsigned short int &fms_altitude,
	double &barometric_pressure_setting,
	bool &modes,
	bool &vnav_mode,
	bool &alt_hold_mode,
	bool &approach_mode,
	bool &altitude_source,
	unsigned char &tgt_alt_src)
{
	mcp_altitude = 0;
	fms_altitude = 0;
	barometric_pressure_setting = 0.0;
	modes = false;
	alt_hold_mode = vnav_mode = approach_mode = false;
	altitude_source = false;
	tgt_alt_src = 0;

	if (Address != 0x40)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		mcp_altitude = bits2unsigned (MessageData, 1, 12) * 16;
	}

	if (bits2bit (MessageData, 13))
	{
		fms_altitude = bits2unsigned (MessageData, 14, 25) * 16;
	}

	if (bits2bit (MessageData, 26))
	{
		barometric_pressure_setting = 800 + (bits2unsigned (MessageData, 27, 38) * 0.1);
	}

	if (bits2bit (MessageData, 47))
	{
		modes = true;
		vnav_mode = bits2bit (MessageData, 48);
		alt_hold_mode = bits2bit (MessageData, 49);
		approach_mode = bits2bit (MessageData, 50);
	}

	if (bits2bit (MessageData, 53))
	{
		altitude_source = true;
		tgt_alt_src = bits2unsigned (MessageData, 54, 55);
	}
}

void RDFTargetModeS::Get41 (
	string & next_waypoint)
{
	char waypoint[9];

	next_waypoint.erase ();
	
	if (Address != 0x41)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (!(MessageData[0] & 0x80))
	{
		return;
	}
		
	waypoint[0] = asterix_decode_char (((MessageData[0] >> 1) & 0x3F));
	waypoint[1] = asterix_decode_char (((MessageData[0] << 5) & 0x20) | ((MessageData[1] >> 3) & 0x1F));
	waypoint[2] = asterix_decode_char (((MessageData[1] << 3) & 0x38) | ((MessageData[2] >> 5) & 0x07));
	waypoint[3] = asterix_decode_char (((MessageData[2] << 1) & 0x3E) | ((MessageData[3] >> 7) & 0x01));
	waypoint[4] = asterix_decode_char (((MessageData[3] >> 1) & 0x3F));
	waypoint[5] = asterix_decode_char (((MessageData[3] << 5) & 0x20) | ((MessageData[4] >> 3) & 0x1F));
	waypoint[6] = asterix_decode_char (((MessageData[4] << 3) & 0x38) | ((MessageData[5] >> 5) & 0x07));
	waypoint[7] = asterix_decode_char (((MessageData[5] << 1) & 0x3E) | ((MessageData[6] >> 7) & 0x01));
	waypoint[8] = asterix_decode_char	 (((MessageData[6] >> 1) & 0x3F));

	next_waypoint.assign (waypoint, 9);
}


void RDFTargetModeS::Get42 (
		double &wp_latitude,
		double &wp_longitude,
		double &cross_altitude)
{
	wp_latitude = 0;
	wp_longitude = 0;
	cross_altitude = 0;

	if (Address != 0x42)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		wp_latitude = bits2signed (MessageData, 1, 19) * 0.5 * M_PI / 131072;
		//if (bits2bit (MessageData, 1)) latitude = - latitude;
	}
	if (bits2bit (MessageData, 20))
	{
		wp_longitude = bits2signed (MessageData, 21, 39) * 0.5 * M_PI / 131072;
		//if (bits2bit (MessageData, 21)) longitude = - longitude;
	}
	if (bits2bit (MessageData, 40))
	{
		cross_altitude = bits2unsigned (MessageData, 41, 55) * 8 * FEET_TO_METER;
		//if (bits2bit (MessageData, 41)) pressure_altitude = - pressure_altitude;
	}
}

void RDFTargetModeS::Get43 (
		double &bearing,
		double &time_to_go,
		double &distance)
{
	bearing = 0;
	time_to_go = 0;
	distance = 0;

	if (Address != 0x43)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		bearing = bits2signed (MessageData, 1, 11) * (M_PI * 2) / 2048;
		if (bearing < 0)
		{
			bearing += 2 * M_PI;
		}
		//if (bits2bit (MessageData, 1)) magnetic_heading = - magnetic_heading;
	}

	if (bits2bit (MessageData, 12))
	{
		time_to_go = bits2unsigned (MessageData, 13, 24) * 0.1 * 60;
	}

	if (bits2bit (MessageData, 25))
	{
		distance = bits2unsigned (MessageData, 26, 41) * 0.1 * NM_TO_METER ;
	}
}



// Meteorological routine air report
void RDFTargetModeS::Get44 (
	unsigned char &fom_source,
	double &wind_speed,
	double &wind_direction,
	double &static_air_temperature,
	double &static_air_presure,
	unsigned char &turbolence,
	double &humidity)
{
	fom_source=0;
	wind_speed=0;
	wind_direction=0;
	static_air_temperature=0;
	static_air_presure=0;
	turbolence=0;
	humidity=0;

	if (Address != 0x44)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	fom_source = bits2unsigned(MessageData, 0, 3);

	if (bits2bit (MessageData, 4))
	{
		wind_speed = bits2unsigned (MessageData, 5, 13) * KNOT_TO_SPEED;
		wind_direction = bits2signed (MessageData,14, 23) * M_PI / 256;
		if (wind_direction < 0)
		{
			wind_direction += 2 * M_PI;
		}
	}

	if (bits2bit (MessageData, 23))
	{
		static_air_temperature = bits2signed (MessageData, 24, 33) * 0.25;
	}

	if (bits2bit (MessageData, 34))
	{
		static_air_presure = bits2unsigned (MessageData, 35, 45) ;
	}
	if (bits2bit (MessageData, 46))
	{
		turbolence = bits2unsigned(MessageData, 47, 48);
	}
	if (bits2bit (MessageData, 49))
	{
		humidity = bits2unsigned(MessageData, 50, 55) * (100. / 64.);
	}
}

// Meteorological hazard report
void RDFTargetModeS::Get45 (
	unsigned char &turbolence,
	unsigned char &wind_shear,
	unsigned char &microburst,
	unsigned char &icing,
	unsigned char &wake_vortex,
	double &static_air_temperature,
	double &average_static_presure,
	double &radio_height)
{

	turbolence=0;
	wind_shear=0;
	microburst=0;
	icing=0;
	wake_vortex=0;
	static_air_temperature=0;
	average_static_presure=0;
	radio_height=0;

	if (Address != 0x45)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		turbolence = bits2unsigned(MessageData, 1, 2);
	}
	if (bits2bit (MessageData, 3))
	{
		wind_shear = bits2unsigned(MessageData, 4, 5);
	}
	if (bits2bit (MessageData, 6))
	{
		microburst = bits2unsigned(MessageData, 7, 8);
	}
	if (bits2bit (MessageData, 9))
	{
		icing = bits2unsigned(MessageData,10,11);
	}
	if (bits2bit (MessageData,12))
	{
		wake_vortex = bits2unsigned(MessageData,13,14);
	}
	if (bits2bit (MessageData, 15))
	{
		static_air_temperature = bits2signed (MessageData, 16, 25) * 0.25;
	}
	if (bits2bit (MessageData, 26))
	{
		average_static_presure = bits2unsigned (MessageData, 27, 37) ;
	}
	if (bits2bit (MessageData, 38))
	{
		radio_height = bits2unsigned (MessageData, 39, 50) * 16 * FEET_TO_METER;
	}
}

// VHF channel report
void GetVHFchannel (
        int offset,
        unsigned char *MessageData,
	double &VHF,
	unsigned char &VHFVDLflags,
	unsigned char &VHFstatus)
{
	if (bits2bit (MessageData, offset + 15))
	{
		VHFstatus = bits2unsigned (MessageData, offset + 16, offset + 17);
		unsigned char VDL = bits2bit (MessageData, offset + 0);
		if (VDL)
		{
			VHFVDLflags = bits2unsigned (MessageData, offset + 1, offset + 4);
		}
		if ((!VDL)&&(bits2bit (MessageData, offset + 1)))
		{
			VHF = ((bits2unsigned (MessageData, offset + 3, offset +14) * 8.33) * 0.001 ) + 118.000;
		}
		else
		{
			VHF = ((bits2unsigned (MessageData, offset + 5, offset +14) * 25  ) * 0.001 ) + 118.000;
		}
	}
}

void RDFTargetModeS::Get48 (
	double &VHF1,
	unsigned char &VHF1VDLflags,
	unsigned char &VHF1status,
	double &VHF2,
	unsigned char &VHF2VDLflags,
	unsigned char &VHF2status,
	double &VHF3,
	unsigned char &VHF3VDLflags,
	unsigned char &VHF3status,
	unsigned char &VHF_EMG_status)
{
	VHF1=VHF2=VHF3=0;
	VHF1status = VHF2status = VHF3status = 0;
	VHF1VDLflags = VHF2VDLflags = VHF3VDLflags = 0;
	VHF_EMG_status = 0;

	if (Address != 0x48)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	GetVHFchannel( 0,MessageData,VHF1,VHF1VDLflags,VHF1status);
	GetVHFchannel(18,MessageData,VHF2,VHF2VDLflags,VHF2status);
	GetVHFchannel(36,MessageData,VHF3,VHF3VDLflags,VHF3status);
	VHF_EMG_status = bits2unsigned (MessageData, 54, 55);
}

void RDFTargetModeS::Get50 (
	double & roll_angle,
	double & true_track_angle, 
	double & ground_speed,
	double & track_angle_rate,
	double & true_airspeed)
{
	roll_angle = 0;
	true_track_angle = 0;
	ground_speed = 0;
	track_angle_rate = 0;
	true_airspeed = 0;

	if (Address != 0x50)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		roll_angle = bits2signed (MessageData, 1, 10) * M_PI_4 / 256;
		//CHECKME
		if (roll_angle < 0)
		{
			roll_angle += 2 * M_PI;
		}
		//if (bits2bit (MessageData, 1)) roll_angle = - roll_angle;
	}

	if (bits2bit (MessageData, 11))
	{
		true_track_angle = bits2signed (MessageData, 12, 22) * M_PI_2 / 512;
		if (true_track_angle < 0)
		{
			true_track_angle += 2 * M_PI;
		}
		//if (bits2bit (MessageData, 12)) true_track_angle = - true_track_angle;
	}

	if (bits2bit (MessageData, 23))
	{
		ground_speed = bits2unsigned (MessageData, 24, 33) * 1024 * KNOT_TO_SPEED / 512;
	}

	if (bits2bit (MessageData, 34))
	{
		track_angle_rate = bits2signed (MessageData, 35, 44) * 8 * M_PI / 180 / 256;
		//if (bits2bit (MessageData, 35)) track_angle_rate = - track_angle_rate;
	}

	if (bits2bit (MessageData, 45))
	{
		true_airspeed = bits2unsigned (MessageData, 46, 55) * 2 * KNOT_TO_SPEED;
	}
}

void RDFTargetModeS::Get51 (
		double &latitude,
		double &longitude,
		double &pressure_altitude)
{
	latitude = 0;
	longitude = 0;
	pressure_altitude = 0;

	if (Address != 0x51)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		latitude = bits2signed (MessageData, 1, 20) * 2.0 * M_PI / 1048576;
		//if (bits2bit (MessageData, 1)) latitude = - latitude;

		longitude = bits2signed (MessageData, 21, 40) * 2.0 * M_PI / 1048576;
		//if (bits2bit (MessageData, 21)) longitude = - longitude;

		pressure_altitude = bits2signed (MessageData, 41, 55) * 8 * FEET_TO_METER;
		//if (bits2bit (MessageData, 41)) pressure_altitude = - pressure_altitude;
	}
}

void RDFTargetModeS::Get52 (
		unsigned char &rnp,
		double &latitude_fine,
		double &longitude_fine,
		double &altitude_or_height)
{
	latitude_fine = 0;
	longitude_fine = 0;
	altitude_or_height = 0;

	if (Address != 0x52)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		rnp = bits2unsigned (MessageData, 1, 4);
		latitude_fine = bits2unsigned (MessageData, 5, 22) * M_PI_2 / 16777216.0;
		longitude_fine = bits2unsigned (MessageData, 23, 40) * M_PI_2 / 16777216.0;

		altitude_or_height = bits2unsigned (MessageData, 42, 55) * 8 * FEET_TO_METER;
		if (bits2bit (MessageData, 41))
		{
			altitude_or_height = - altitude_or_height;
		}
	}
}

void RDFTargetModeS::Get53 (
		double &magnetic_heading,
		double &indicated_airspeed,
		double &mach_number,
		double &true_airspeed,
		double &altitude_rate)
{
	magnetic_heading = 0;
	indicated_airspeed = 0;
	mach_number = 0;
	true_airspeed = 0;
	altitude_rate = 0;

	if (Address != 0x53)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		magnetic_heading = bits2signed (MessageData, 1, 11) * M_PI_2 / 512;
		if (magnetic_heading < 0)
			magnetic_heading += 2 * M_PI;
		//if (bits2bit (MessageData, 1)) magnetic_heading = - magnetic_heading;
	}

	if (bits2bit (MessageData, 12))
	{
		indicated_airspeed = bits2unsigned (MessageData, 13, 22) * KNOT_TO_SPEED;
	}

	if (bits2bit (MessageData, 23))
	{
		mach_number = bits2unsigned (MessageData, 24, 32) * 8.0 / 1000;
	}

	if (bits2bit (MessageData, 33))
	{
		true_airspeed = bits2unsigned (MessageData, 34, 45) * 0.5 * KNOT_TO_SPEED;
	}

	if (bits2bit (MessageData, 46))
	{
		altitude_rate = bits2signed (MessageData, 47, 55) * 64 * FEET_TO_METER / 60.0;
		//if (bits2bit (MessageData, 47)) altitude_rate = - altitude_rate;
	}
}

// Quasi-static parameter monitoring
void RDFTargetModeS::Get5F (
	unsigned char &mcp_selected_alt,
	unsigned char &next_waypoint,
	unsigned char &fms_vertical_mode,
	unsigned char &vhf_channel_report,
	unsigned char &meteo_hazards,
	unsigned char &fms_selected_alt)
{

	mcp_selected_alt=0;
	next_waypoint=0;
	fms_vertical_mode=0;
	vhf_channel_report=0;
	meteo_hazards=0;
	fms_selected_alt=0;

	if (Address != 0x5F)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	mcp_selected_alt  =bits2unsigned (MessageData, 0, 1);
	next_waypoint     =bits2unsigned (MessageData,12,13);
	fms_vertical_mode =bits2unsigned (MessageData,16,17);
	vhf_channel_report=bits2unsigned (MessageData,18,19);
	meteo_hazards     =bits2unsigned (MessageData,20,21);
	fms_selected_alt  =bits2unsigned (MessageData,22,23);
}

void RDFTargetModeS::Get60 (
		double &magnetic_heading,
		double &indicated_airspeed,
		double &mach,
		double &altitude_rate,
		double &vertical_velocity)
{
	magnetic_heading = 0;
	indicated_airspeed = 0;
	mach = 0;
	altitude_rate = 0;
	vertical_velocity = 0;

	if (Address != 0x60)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	if (bits2bit (MessageData, 0))
	{
		magnetic_heading = bits2signed (MessageData, 1, 11) * M_PI_2 / 512;
		if (magnetic_heading < 0)
			magnetic_heading += 2 * M_PI;
		//if (bits2bit (MessageData, 1)) magnetic_heading = - magnetic_heading;
	}

	if (bits2bit (MessageData, 12))
	{
		indicated_airspeed = bits2unsigned (MessageData, 13, 22) * KNOT_TO_SPEED;
	}

	if (bits2bit (MessageData, 23))
	{
		mach = bits2unsigned (MessageData, 24, 33) * 4.0 / 1000;
	}

	if (bits2bit (MessageData, 34))
	{
		altitude_rate = bits2signed (MessageData, 35, 44) * 32 * FEET_TO_METER / 60.0;
		//if (bits2bit (MessageData, 35)) altitude_rate = - altitude_rate;
	}

	if (bits2bit (MessageData, 45))
	{
		vertical_velocity = bits2signed (MessageData, 46, 55) * 32 * FEET_TO_METER / 60.0;
		//if (bits2bit (MessageData, 46)) vertical_velocity = - vertical_velocity;
	}
}

// Extended squitter emergency/priority status
void RDFTargetModeS::Get61 (
	unsigned char &subtype,
	unsigned char &state)
{
	subtype=0;
	state=0;

	if (Address != 0x61)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	if (bits2unsigned (MessageData, 0, 4)!=28)
	{
//		cms_error ("wrong format type code %d .... must be 28", bits2unsigned (MessageData, 0, 4));
		return;
	}

	subtype = bits2unsigned (MessageData, 5, 7);
	state = 0;
	if (subtype == 1)
	{
	  state = bits2unsigned (MessageData, 8,10);
	}
}


void RDFTargetModeS::Get61 (
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
	double & t_bearing)
{
	subtype=0;
	state=0;

	if (Address != 0x61)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	if (bits2unsigned (MessageData, 0, 4)!=28)
	{
//		cms_error ("wrong format type code %d .... must be 28", bits2unsigned (MessageData, 0, 4));
		return;
	}

	subtype = bits2unsigned (MessageData, 5, 7);
	state = 0;

	RA_terminated=0;
	advisory_type=0;
	s_addr_present=false;
	height_present=false;
	t_range_present=false;
	t_bearing_present=false;
	if (subtype == 1)
	{
	  state = bits2unsigned (MessageData, 8,10);
	}
	if (subtype == 2) // version 1b is including Resolution Advisory, like BDS 3,0
	{
	  Get30( RA_terminated, advisory_type, advisory_data, s_addr_present, s_addr, height_present, height, t_range_present, t_range, t_bearing_present, t_bearing);
	}
}

  // target state and status information
void RDFTargetModeS::Get62 (
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
	unsigned char &EMG_status)
{
	subtype=0;
	vert_data_avail = 0;
	target_alt_type = 0;
	target_alt_capa = 0;
	vert_mode_ind = 0;
	target_alt = 0;
	horiz_data_avail = 0;
	target_heading = 0;
	target_heading_track = 0;
	horiz_mode_ind = 0;
	NAC = 0;
	NIC = 0;
	SIL = 0;
	ACAS_not_oper = 0;
	ACAS_RA_active = 0;
	EMG_status = 0;

	if (Address != 0x62)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	if (bits2unsigned (MessageData, 0, 4)!=29)
	{
//		cms_error ("wrong format type code %d .... must be 28", bits2unsigned (MessageData, 0, 4));
		return;
	}

	subtype = bits2unsigned (MessageData, 5, 6);
	if (subtype == 0)
	{
	  vert_data_avail = bits2unsigned (MessageData, 7, 8);
	  target_alt_type = bits2bit (MessageData, 9);
	  target_alt_capa = bits2unsigned (MessageData,11,12);
	  vert_mode_ind = bits2unsigned (MessageData,13,14);
	  target_alt = bits2unsigned (MessageData,15,24);
	  horiz_data_avail = bits2unsigned (MessageData,25,26);
	  target_heading = bits2unsigned (MessageData,27,35);
	  target_heading_track = bits2bit (MessageData,36);
	  horiz_mode_ind = bits2unsigned (MessageData,37,38);
	  NAC = bits2unsigned (MessageData,39,42);
	  NIC = bits2bit (MessageData,43);
	  SIL = bits2unsigned (MessageData,44,45);
	  ACAS_not_oper = bits2bit (MessageData,51);
	  ACAS_RA_active = bits2bit (MessageData,52);
	  EMG_status = bits2unsigned (MessageData,53,55);
	}
}

void RDFTargetModeS::Get65 (
		unsigned char &format_type_code,
		unsigned char &subtype_code,
		unsigned char &en_route_cap,
		unsigned char &terminal_area_cap,
		unsigned char &approach_cap,
		unsigned char &surface_cap,
		unsigned char &en_route_status,
		unsigned char &terminal_area_status,
		unsigned char &approach_status,
		unsigned char &surface_status)
{
	format_type_code = subtype_code = 0;
	en_route_cap = terminal_area_cap = 0;
	approach_cap = surface_cap = 0;
	en_route_status = terminal_area_status = 0;
	approach_status = surface_status = 0;

	if (Address != 0x65)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	format_type_code = bits2unsigned (MessageData, 0, 4);
	subtype_code = bits2unsigned (MessageData, 5, 7);
	
	en_route_cap = bits2unsigned (MessageData, 8, 11);
	terminal_area_cap = bits2unsigned (MessageData, 12, 15);
	approach_cap = bits2unsigned (MessageData, 16, 19);
	surface_cap = bits2unsigned (MessageData, 20, 23);

	en_route_status = bits2unsigned (MessageData, 24, 27);
	terminal_area_status = bits2unsigned (MessageData, 28, 31);
	approach_status = bits2unsigned (MessageData, 32, 35);
	surface_status = bits2unsigned (MessageData, 36, 39);
}

void RDFTargetModeS::GetE3_E4_E5_E6 (
	bool & supported_format,
	string & out_string)
{
	unsigned char format = bits2unsigned (MessageData, 1, 2);
	if (format == 0)
	{
	  supported_format = true; // P/N BCD
	  for (int i=0;i<12;i++)
	  {
	    unsigned char ch = bits2unsigned (MessageData, ((4*(i+0))-0)+3, ((4*(i+1))-1)+3);
	    if (ch>9) ch=9;
	    out_string.push_back(ch + '0');
	  }
	} else if (format == 1)
	{
	  supported_format = true; // character
	  for (int i=0;i<8;i++)
	  {
	    unsigned char ch = bits2unsigned (MessageData, ((6*(i+0))-0)+3, ((6*(i+1))-1)+3);
	    out_string.push_back(asterix_decode_char(ch));
	  }
	} else {
	  supported_format = false;
	}
}
  // Transponder type/part number
void RDFTargetModeS::GetE3 (
	bool & supported_format,
	string & out_string)
{
	out_string.erase ();
	supported_format = false;
	
	if (Address != 0xE3)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	GetE3_E4_E5_E6 ( supported_format, out_string);
}

// Transponder software revision number
void RDFTargetModeS::GetE4 (
	bool & supported_format,
	string & out_string)
{
	out_string.erase ();
	supported_format = false;
	
	if (Address != 0xE4)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	GetE3_E4_E5_E6 ( supported_format, out_string);
}

// ACAS unit part number
void RDFTargetModeS::GetE5 (
	bool & supported_format,
	string & out_string)
{
	out_string.erase ();
	supported_format = false;
	
	if (Address != 0xE5)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	GetE3_E4_E5_E6 ( supported_format, out_string);
}

// ACAS Unit Software Revision
void RDFTargetModeS::GetE6 (
	bool & supported_format,
	string & out_string)
{
	out_string.erase ();
	supported_format = false;
	
	if (Address != 0xE6)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	GetE3_E4_E5_E6 ( supported_format, out_string);
}

void RDFTargetModeS::GetE1_ELDIS_2014 (
	unsigned char & ILA,
	unsigned char & ILB,
	unsigned char & UPS,
	std::vector<int> & II_SI
	)
{
	II_SI.clear ();
	ILA = 0;
	ILB = 0;
	UPS = 0;
	if (Address != 0xE1)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}
	ILA = bits2bit (MessageData, 0);
	ILB = bits2bit (MessageData, 1);
	UPS = bits2unsigned (MessageData, 2, 4);

	for (int i = 32, r = 0; r <=23 ; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			II_SI.push_back (r);
		}
	}
	
  
}

void RDFTargetModeS::GetE2_ELDIS_2014 (
	std::vector<int> & II_SI
	)
{
	II_SI.clear ();
	if (Address != 0xE2)
	{
		cms_error ("wrong address %02x", Address);
		return;
	}

	for (int i = 0, r = 24; r <=79 ; i++, r++)
	{
		if (bits2bit (MessageData, i))
		{
			II_SI.push_back (r);
		}
	}
}

bool is_datalink_capability (const RDFTargetModeS &modes)
{
	return (modes.Address == 0x10);
}


int Get51and52 (unsigned char *Data51, unsigned char *Data52, double &lat, double &lon, double &alt, double &alt_gnss)
            // retval int -2 = invalid_all , -1=only_bds51, 0..15=RNP
            // Data51 or Data52 can be NULL, if not present, else 7 bytes array
            // alt_gnns valid only for rnp = 11..15
{
	int retval = -2;
	lat = 0.0; lon=0.0; alt=0.0; alt_gnss=0.0;
	if ((Data51!=NULL) && (Data51[0] & 0x80))
	{
		retval = -1;
		uint64_t D51 = 0; for (unsigned z=0;z<7;z++) D51 |= ((uint64_t)(Data51[z])) << ((6-z)*8);
		uint32_t lat1 = ((uint32_t)(D51>>35)) & 0x7ffff;
		uint32_t lon1 = ((uint32_t)(D51>>15)) & 0x7ffff;
		uint32_t alt1 = ((uint32_t)(D51>> 0)) &  0x3fff;
		lat1<<=6;
		lon1<<=6;
		if ((Data52!=NULL) && (Data52[0] & 0x80))
		{
			uint64_t D52 = 0; for (unsigned z=0;z<7;z++) D52 |= ((uint64_t)(Data52[z])) << ((6-z)*8);
			uint32_t lat2 = ((uint32_t)(D52>>33)) & 0x3ffff;
			uint32_t lon2 = ((uint32_t)(D52>>15)) & 0x3ffff;
			uint32_t alt2 = ((uint32_t)(D52>> 0)) &  0x3fff;
			if ((((lat1>>6) & 0xfff) == ((lat2>>6) & 0xfff)) && (((lon1>>6) & 0xfff) == ((lon2>>6) & 0xfff)))
			{
				retval = ((int)(D52>>51)) & 0x0f;
				alt_gnss = (double)alt2 * 8 * FEET_TO_METER * (((D52>>14) & 1) ? -1.0 : 1.0);
				lat1 |= lat2 & 0x3f; // 6 more precision bits
				lon1 |= lon2 & 0x3f; // 6 more precision bits
			}
		}
		if (retval<0)
		{
			lat1 |= 0x20; // round
			lon1 |= 0x20; // round
		}
		double recalc_const = 360. / (1048567. * 64.);
		alt = (double)alt1 * 8 * FEET_TO_METER;
		lon = (double)lon1 * (recalc_const) ;
		lat = (double)lat1 * (recalc_const) ;
		if ((D51>>14) & 1) alt = (double)(~alt1 & 0xfff) * 8 * FEET_TO_METER * -1.0;
		if ((D51>>34) & 1) lon = (double)(~lon1 & 0xffffff) * (recalc_const) * -1.0 ;
		if ((D51>>54) & 1) lat = (double)(~lat1 & 0x7fffff) * (recalc_const) * -1.0;
	}
	return retval;
}
