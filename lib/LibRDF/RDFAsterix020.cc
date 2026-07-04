#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I020_010 0x80
#define FSPEC02_I020_020 0x40
#define FSPEC03_I020_140 0x20
#define FSPEC04_I020_041 0x10
#define FSPEC05_I020_042 0x08
#define FSPEC06_I020_161 0x04
#define FSPEC07_I020_170 0x02

#define FSPEC08_I020_070 0x80
#define FSPEC09_I020_202 0x40
#define FSPEC10_I020_090 0x20
#define FSPEC11_I020_100 0x10
#define FSPEC12_I020_220 0x08
#define FSPEC13_I020_245 0x04
#define FSPEC14_I020_110 0x02

#define FSPEC15_I020_105 0x80
#define FSPEC16_I020_210 0x40
#define FSPEC17_I020_300 0x20
#define FSPEC18_I020_310 0x10
#define FSPEC19_I020_500 0x08
#define FSPEC20_I020_400 0x04
#define FSPEC21_I020_250 0x02

#define FSPEC22_I020_230 0x80
#define FSPEC23_I020_260 0x40
#define FSPEC24_I020_030 0x20
#define FSPEC25_I020_055 0x10
#define FSPEC26_I020_050 0x08
#define FSPEC27_I020_RE  0x04
#define FSPEC28_I020_SP  0x02

bool Asterix020_to_TargetAdsb (const void * data, int size, 
	deque<RDFTargetAdsb> & adsb_list)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	adsb_list.clear ();
	
	if (!data)
	{
		cms_error ("wrong data");
		return false;
	}

	if (size < 0 || size > 0xffff)
	{
		cms_error ("wrong size %d", size);
		return false;
	}

	msg = (unsigned char *) data;
	cat = msg[0];
	len = (((unsigned) msg[1]) << 8) | msg[2];
	record = msg + 3;

	if (cat != 20)
	{ 
		cms_error ("unknown category %d", cat);
		return false;
	}

	if (len < size)
	{
		cms_warning ("wrong length %d %d", len, size);
		size = len;
	}
	else if (len > size)
	{
		cms_error ("wrong length %d %d", len, size);
		goto error;
	}
	
	while (record < msg + len)
	{
		// vytvorime novou polozku
		RDFTargetAdsb adsb;
		
		// najdeme zacatek datovych poli
		fspec = field = record;
		while (*field & FSPEC_FX)
		{
			if (++field >= msg + len)
			{
				cms_error ("end of message");
				goto error;
			}
		}
		field++;

		// Data Source Identifier
		if (*fspec & FSPEC01_I020_010)
		{
			adsb.SAC = field[0];
			adsb.SIC = field[1];
			field += 2;
		}

		// Target Report Descriptor
		if (*fspec & FSPEC02_I020_020)
		{
			if (field[0] & 1)
			{
				field += 1;
				adsb.SPI = (field[0] & 0x40) ? true : false;
				adsb.Simulated = (field[0] & 0x04) ? true : false;
				adsb.Tested = (field[0] & 0x02) ? true : false;
			}

			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents");
				goto error;
			}

			field += 1;
		}

		// Time of Day
		if (*fspec & FSPEC03_I020_140)
		{
			adsb.Time.Present = true;
			adsb.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
			field += 3;
		}

		// Position in WGS-84 Coordinates
		if (*fspec & FSPEC04_I020_041)
		{
			adsb.Geographic = new RDFCoorGeographic ();
			adsb.Geographic->Latitude = (double) 
				field2signed (field, 32) * M_PI / (1 << 25);
			adsb.Geographic->Longitude = (double) 
				field2signed (field + 4, 32) * M_PI / (1 << 25);
			field += 8;
		}

		// Position in Cartesian Coordinates
		if (*fspec & FSPEC05_I020_042)
		{
			adsb.Cartesian = new RDFCoorCartesian ();
			adsb.Cartesian->X = (double) field2signed (field, 24) / 2;
			adsb.Cartesian->Y = (double) field2signed (field + 3, 24) / 2;
			field += 6;
		}

		// Track Number
		if (*fspec & FSPEC06_I020_161)
		{
			adsb.TrackNumber.Present = true;
			adsb.TrackNumber.Number = field2unsigned (field, 12);
			field += 2;
		}

		// Track Status
		if (*fspec & FSPEC07_I020_170)
		{
			adsb.Tentative = (field[0] & 0x80) ? true : false;
			adsb.Cancel = (field[0] & 0x40) ? true : false;
			adsb.CST = (field[0] & 0x20) ? true : false;
			
			switch ((field[0] >> 3) & 0x03)
			{
				case 0: adsb.TrackClimb = TRACK_CLIMB_MAINTAINING; break;
				case 1: adsb.TrackClimb = TRACK_CLIMB_CLIMBING; break;
				case 2: adsb.TrackClimb = TRACK_CLIMB_DESCENDING; break;
				case 3: adsb.TrackClimb = TRACK_CLIMB_UNKNOWN; break;
			}

			if (field[0] & 1)
			{
				field++;
			}
			
			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents");
				goto error;
			}

			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Mode-3/A Code in Octal Representation
		if (*fspec & FSPEC08_I020_070)
		{
			adsb.Mode3A = new RDFTargetMode ();
			adsb.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			adsb.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
			adsb.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			adsb.Mode3A->Code = field2unsigned (field, 12);
			field += 2;
		}

		// Calculated Track Velocity in Cartesian Coordinates
		if (*fspec & FSPEC09_I020_202)
		{
			double vx = (double) field2signed (field, 16) / 4;
			double vy = (double) field2signed (field + 2, 16) / 4;
			adsb.GroundSpeed = new RDFTargetGroundSpeed ();
			rdf_xy2rt (vx, vy, adsb.GroundSpeed->Speed, adsb.GroundSpeed->Heading);
			field += 4;
		}

		// Flight Level in Binary Representation
		if (*fspec & FSPEC10_I020_090)
		{
			adsb.FlightLevel = new RDFTargetFlightLevel ();
			adsb.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
			adsb.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
			adsb.FlightLevel->Height = (double) field2signed (field, 14) * 25 * 0.3048;
			field += 2;
		}

		// Mode-C Code
		if (*fspec & FSPEC11_I020_100)
		{
			adsb.ModeC = new RDFTargetMode ();
			adsb.ModeC->Valid = (field[0] & 0x80) ? false : true;
			adsb.ModeC->Garbled = (field[0] & 0x40) ? true : false;
			adsb.ModeC->Code = field2unsigned (field, 12);
			adsb.ModeC->ConfidencePresent = true;
			adsb.ModeC->ConfidenceCode = field2unsigned (field + 2, 12);
			field += 4;
		}

		// Target Address
		if (*fspec & FSPEC12_I020_220)
		{
			adsb.AircraftAddress.Present = true;
			adsb.AircraftAddress.Address = field2unsigned (field, 24);
			field += 3;
		}

		// Target Identification
		if (*fspec & FSPEC13_I020_245)
		{
			asterix_decode_callsign (field + 1, adsb.AircraftIdentification);
			field += 7;
		}

		// Measured Height (Cartesian Coordinates)
		if (*fspec & FSPEC14_I020_110)
		{
			adsb.MeasuredHeight = new RDFTargetHeight ();
			adsb.MeasuredHeight->Height = field2signed (field, 16) * 6.25 * 0.3048;
			field += 2;
		}
		
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Geometric Height WGS-84
		if (*fspec & FSPEC15_I020_105)
		{
			adsb.GeometricHeight = new RDFTargetHeight ();
			adsb.GeometricHeight->Height = field2signed (field, 16) * 6.25 * 0.3048;
			field += 2;
		}

		// Calculated Acceleration
		if (*fspec & FSPEC16_I020_210)
		{
			field += 2;
		}

		// Vehicle Fleet Identification
		if (*fspec & FSPEC17_I020_300)
		{
			field += 1;
		}

		// Pre-programmed Message
		if (*fspec & FSPEC18_I020_310)
		{
			field += 1;
		}

		// Position Accuracy
		if (*fspec & FSPEC19_I020_500)
		{
			unsigned char * subfield = field;
			field += 1;

			// Subfield #1: DOP of Position
			if (*subfield & 0x80)
			{
				field += 6;
			}

			// Subfield #2: Standart Deviation of Position
			if (*subfield & 0x40)
			{
				field += 6;
			}

			// Subfield #3: Standart Deviation of Geometric Height
			if (*subfield & 0x20)
			{
				field += 2;
			}

			// Spare
			if (*subfield & 0x1F)
			{
				cms_error ("unable to handle more subfield");
				goto error;
			}
		}

		// Contributing Receivers
		if (*fspec & FSPEC20_I020_400)
		{
			unsigned char rep = *field;
			field += 1 + rep;
		}

		// Mode S MB Data
		if (*fspec & FSPEC21_I020_250)
		{
			unsigned char rep = *field;
			field += 1;
			for (int i = 0; i < rep; i++)
			{
				RDFTargetModeS modes;
				memcpy (modes.MessageData, field, 7);
				modes.Address = field[7];
				adsb.ModeS.push_back (modes);
				field += 8;
			}
		}
		
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Comms/ACAS Capability and Flight Status
		if (*fspec & FSPEC22_I020_230)
		{
			field += 2;
		}

		// ACAS Resolution Advisory Report
		if (*fspec & FSPEC23_I020_260)
		{
			field += 7;
		}

		// Warning/Error Conditions
		if (*fspec & FSPEC24_I020_030)
		{
			adsb.WarningError.push_back (field[0] >> 1);
			while (field[0] & 1)
			{
				field += 1;
				adsb.WarningError.push_back (field[0] >> 1);
			}
			field += 1;
		}

		// Mode-1 Code in Octal Representation
		if (*fspec & FSPEC25_I020_055)
		{
			adsb.Mode1 = new RDFTargetMode ();
			adsb.Mode1->Valid = (field[0] & 0x80) ? false : true;
			adsb.Mode1->Garbled = (field[0] & 0x40) ? true : false;
			adsb.Mode1->Tracked = (field[0] & 0x20) ? true : false;
			adsb.Mode1->Code = ((field[0] & 0x1c) << 7) | ((field[0] & 0x3) << 6);
			field += 1;
		}

		// Mode-2 Code in Octal Representation
		if (*fspec & FSPEC26_I020_050)
		{
			adsb.Mode2 = new RDFTargetMode ();
			adsb.Mode2->Valid = (field[0] & 0x80) ? false : true;
			adsb.Mode2->Garbled = (field[0] & 0x40) ? true : false;
			adsb.Mode2->Tracked = (field[0] & 0x20) ? true : false;
			adsb.Mode2->Code = field2unsigned (field, 12);
			field += 2;
		}

		// Reserved Expansion Field
		if (*fspec & FSPEC27_I020_RE)
		{
			field += field[0];
		}

		// Special Purpose Field
		if (*fspec & FSPEC28_I020_SP)
		{
			field += field[0];
		}
			
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		cms_error ("unable to handle more fspec");
		goto error;

end:
		// pridame polozku do seznamu
		adsb_list.push_back (adsb);

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error ("wrong packet length (%d > %d)", record - msg, size);
		goto error;
	}

	return true;

error:
	cms_data (size, msg);
	adsb_list.clear ();
	return false;
}

int TargetAdsb_to_Asterix020 (deque<RDFTargetAdsb> & adsb_list, 
	void * data, int maxsize)
{
	unsigned char buffer[4096];
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int fspec_size;
	int field_size;
	int size;

	memset (data, 0, maxsize);
	msg = (unsigned char *) data;
	msg[0] = 20; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;	

	// seznam je prazdny, neni co zpracovavat
	if (adsb_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (adsb_list.size () > 0)
	{
		RDFTargetAdsb & adsb = adsb_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// Data Source Identifier
		{
			*fspec |= FSPEC01_I020_010;
			field[0] = adsb.SAC;
			field[1] = adsb.SIC;
			field += 2;
		}

		// Target Report Descriptor
		{
			*fspec |= FSPEC02_I020_020;
			field[0] = 0;

			field[1] = 0;
			if (adsb.SPI) field[1] |= 0x40;
			if (adsb.Simulated) field[1] |= 0x04;
			if (adsb.Tested) field[1] |= 0x02;

			if (field[1])
			{
				field[0] |= 0x01;
				field += 1;
			}

			field += 1;
		}

		// Time of Day
		if (adsb.Time.Present)
		{
			*fspec |= FSPEC03_I020_140;
			int time = RDF_time_to_asterix_time(adsb.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Position in WGS-84 Coordinates
		if (adsb.Geographic)
		{
			*fspec |= FSPEC04_I020_041;
			long long lat = llrint (adsb.Geographic->Latitude * (1 << 25) / M_PI);
			long long lon = llrint (adsb.Geographic->Longitude * (1 << 25) / M_PI);
			if (lat > INT_MAX || lat < INT_MIN)
			{
				cms_warning ("lat out of limit");
				goto skip;
			}
			if (lon > INT_MAX || lon < INT_MIN)
			{
				cms_warning ("lon out of limit");
				goto skip;
			}
			field[0] = (lat >> 24) & 0xff;
			field[1] = (lat >> 16) & 0xff;
			field[2] = (lat >> 8) & 0xff;
			field[3] = lat & 0xff;
			field[4] = (lon >> 24) & 0xff;
			field[5] = (lon >> 16) & 0xff;
			field[6] = (lon >> 8) & 0xff;
			field[7] = lon & 0xff;
			field += 8;
		}

		// Position in Cartesian Coordinates
		if (adsb.Cartesian)
		{
			*fspec |= FSPEC05_I020_042;
			long x = lrint (adsb.Cartesian->X * 2);
			long y = lrint (adsb.Cartesian->Y * 2);
			if (x > 0x7FFFFF || x < -0x800000)
			{
				cms_warning ("x out of limit");
				goto skip;
			}
			if (y > 0x7FFFFF || y < -0x800000)
			{
				cms_warning ("y out of limit");
				goto skip;
			}
			field[0] = (x >> 16) & 0xff;
			field[1] = (x >> 8) & 0xff;
			field[2] = x & 0xff;
			field[3] = (y >> 16) & 0xff;
			field[4] = (y >> 8) & 0xff;
			field[5] = y & 0xff;
			field += 6;
		}

		// Track Number
		if (adsb.TrackNumber.Present)
		{
			*fspec |= FSPEC06_I020_161;
			field[0] = (adsb.TrackNumber.Number >> 8) & 0xff;
			field[1] = adsb.TrackNumber.Number & 0xff;
			field += 2;
		}

		// Track Status
		if (adsb.TrackNumber.Present)
		{
			*fspec |= FSPEC07_I020_170;

			if (adsb.Tentative) field[0] |= 0x80;
			if (adsb.Cancel) field[0] |= 0x40;
			if (adsb.CST) field[0] |= 0x20;

			switch (adsb.TrackClimb)
			{
				case TRACK_CLIMB_MAINTAINING: break;
				case TRACK_CLIMB_CLIMBING: field[0] |= (1 << 3);
				case TRACK_CLIMB_DESCENDING: field[0] |= (2 << 3);
				case TRACK_CLIMB_UNKNOWN: field[0] |= (3 << 3);
			}

			field += 1;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Mode-3/A Code in Octal Representation
		if (adsb.Mode3A)
		{
			*fspec |= FSPEC08_I020_070;
			adsb.Mode3A = new RDFTargetMode ();
			field[0] = adsb.Mode3A->Valid ? 0 : 0x80;
			field[0] |= adsb.Mode3A->Garbled ? 0x40 : 0;
			field[0] |= adsb.Mode3A->Tracked ? 0x20 : 0;
			field[0] |= (adsb.Mode3A->Code >> 8) & 0x0f;
			field[1] = adsb.Mode3A->Code & 0xff;
			field += 2;
		}

		// Calculated Track Velocity in Cartesian Coordinates
		/*XXXif (*fspec & FSPEC09_I020_202)
		{
			double vx = (double) field2signed (field, 16) / 4;
			double vy = (double) field2signed (field + 2, 16) / 4;
			adsb.GroundSpeed = new RDFTargetGroundSpeed ();
			rdf_xy2rt (vx, vy, adsb.GroundSpeed->Speed, adsb.GroundSpeed->Heading);
			field += 4;
		}*/

		// Flight Level in Binary Representation
		if (adsb.FlightLevel)
		{
			*fspec |= FSPEC10_I020_090;
			int fl = (int) rint (adsb.FlightLevel->Height / 0.3048 / 25);
			field[0] = adsb.FlightLevel->Valid ? 0 : 0x80;
			field[0] |= adsb.FlightLevel->Garbled ? 0x40 : 0;
			field[0] |= (fl >> 8) & 0x3f;
			field[1] = fl & 0xff;
			field += 2;
		}

		// Mode-C Code
		if (adsb.ModeC)
		{
			*fspec |= FSPEC11_I020_100;
			field[0] = adsb.ModeC->Valid ? 0 : 0x80;
			field[0] |= adsb.ModeC->Garbled ? 0x40 : 0;
			field[0] |= (adsb.ModeC->Code >> 8) & 0x0f;
			field[1] = adsb.ModeC->Code & 0xff;
			field[2] = (adsb.ModeC->ConfidenceCode >> 8) & 0x0f;
			field[3] = adsb.ModeC->ConfidenceCode & 0xff;
			field += 4;
		}

		// Target Address
		if (adsb.AircraftAddress.Present)
		{
			*fspec |= FSPEC12_I020_220;
			field[0] = (adsb.AircraftAddress.Address >> 16) & 0xff;
			field[1] = (adsb.AircraftAddress.Address >> 8) & 0xff;
			field[2] = adsb.AircraftAddress.Address & 0xff;
			field += 3;
		}

		// Target Identification
		if (adsb.AircraftIdentification.size ())
		{
			*fspec |= FSPEC13_I020_245;
			asterix_encode_callsign (adsb.AircraftIdentification, field + 1);
			field += 7;
		}

		// Measured Height (Cartesian Coordinates)
		if (adsb.MeasuredHeight)
		{
			*fspec |= FSPEC14_I020_110;
			int height = (int) rint (adsb.MeasuredHeight->Height / 6.25 / 0.3048);
			field[0] = (height >> 8) & 0xff;
			field[1] = height & 0xff;
			field += 2;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Geometric Height WGS-84
		if (adsb.GeometricHeight)
		{
			*fspec |= FSPEC15_I020_105;
			int height = (int) rint (adsb.GeometricHeight->Height / 6.25 / 0.3048);
			field[0] = (height >> 8) & 0xff;
			field[1] = height & 0xff;
			field += 2;
		}

		// Calculated Acceleration
		/*if (*fspec & FSPEC16_I020_210)
		{
			field += 2;
		}*/

		// Vehicle Fleet Identification
		/*if (*fspec & FSPEC17_I020_300)
		{
			field += 1;
		}*/

		// Pre-programmed Message
		/*if (*fspec & FSPEC18_I020_310)
		{
			field += 1;
		}*/

		// Position Accuracy
		/*if (*fspec & FSPEC19_I020_500)
		{
			unsigned char * subfield = field;
			field += 1;

			// Subfield #1: DOP of Position
			if (*subfield & 0x80)
			{
				field += 6;
			}

			// Subfield #2: Standart Deviation of Position
			if (*subfield & 0x40)
			{
				field += 6;
			}

			// Subfield #3: Standart Deviation of Geometric Height
			if (*subfield & 0x20)
			{
				field += 2;
			}

			// Spare
			if (*subfield & 0x1F)
			{
				cms_error ("unable to handle more subfield");
				goto error;
			}
		}*/

		// Contributing Receivers
		/*if (*fspec & FSPEC20_I020_400)
		{
			unsigned char rep = *field;
			field += 1 + rep;
		}*/

		// Mode S MB Data
		if (adsb.ModeS.size ())
		{
			*fspec |= FSPEC21_I020_250;
			field[0] = adsb.ModeS.size ();
			field += 1;
			for (unsigned i = 0; i < adsb.ModeS.size (); i++)
			{
				memcpy (field, adsb.ModeS[i].MessageData, 7);
				field[7] = adsb.ModeS[i].Address;
				field += 8;
			}
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Comms/ACAS Capability and Flight Status
		/*if (*fspec & FSPEC22_I020_230)
		{
			field += 2;
		}*/

		// ACAS Resolution Advisory Report
		/*if (*fspec & FSPEC23_I020_260)
		{
			field += 7;
		}*/

		// Warning/Error Conditions
		if (adsb.WarningError.size ())
		{
			*fspec |= FSPEC24_I020_030;
			for (unsigned i = 0; i < adsb.WarningError.size (); i++)
			{
				field[0] = (adsb.WarningError[i] << 1);
				if (i + 1 != adsb.WarningError.size ())
				{
					field[0] |= 1;
				}
				field += 1;
			}
		}

		// Mode-1 Code in Octal Representation
		if (adsb.Mode1)
		{
			*fspec |= FSPEC25_I020_055;
			field[0] = adsb.Mode1->Valid ? 0 : 0x80;
			field[0] |= adsb.Mode1->Garbled ? 0x40 : 0;
			field[0] |= adsb.Mode1->Tracked ? 0x20 : 0;
			field[0] |= ((adsb.Mode1->Code >> 7) & 0x1c) 
				| ((adsb.Mode1->Code >> 6) & 0x03);
			field += 1;
		}

		// Mode-2 Code in Octal Representation
		if (adsb.Mode2)
		{
			*fspec |= FSPEC26_I020_050;
			field[0] = adsb.Mode2->Valid ? 0 : 0x80;
			field[0] |= adsb.Mode2->Garbled ? 0x40 : 0;
			field[0] |= adsb.Mode2->Tracked ? 0x20 : 0;
			field[0] |= (adsb.Mode2->Code >> 8) & 0x0f;
			field[1] = adsb.Mode2->Code & 0xff;
			field += 2;
		}

		// Reserved Expansion Field
		/*if (*fspec & FSPEC27_I020_RE)
		{
			field += field[0];
		}*/

		// Special Purpose Field
		/*if (*fspec & FSPEC28_I020_SP)
		{
			field += field[0];
		}*/
			
		// vymazeme nevyuzite polozky fspec
		while (!(*fspec & ~FSPEC_FX))
		{
			fspec--;
		}
		
		// konec polozky fspec
		*fspec++ &= ~FSPEC_FX;

		// zkontrolujeme, jestli se polozka vejde do zaznamu
		fspec_size = fspec - record;
		field_size = field - buffer;
		if (size + fspec_size + field_size > maxsize)
		{
			if (size == 3)
			{
				cms_error ("not enough space in output buffer");
				adsb_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime cil z fronty
		adsb_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}

