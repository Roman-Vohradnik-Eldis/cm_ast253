//#include "RDFAsterix017.hh"
#include "CMSDebug.hh"
#include "RDFAsterix.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I017_010 0x80
#define FSPEC02_I017_012 0x40
#define FSPEC03_I017_000 0x20
#define FSPEC04_I017_350 0x10
#define FSPEC05_I017_220 0x08
#define FSPEC06_I017_221 0x04
#define FSPEC07_I017_140 0x02

#define FSPEC08_I017_045 0x80
#define FSPEC09_I017_070 0x40
#define FSPEC10_I017_050 0x20
#define FSPEC11_I017_200 0x10
#define FSPEC12_I017_230 0x08
#define FSPEC13_I017_240 0x04
#define FSPEC14_I017_210 0x02

#define FSPEC15_I017_360 0x80

//#define			JEDNA23			0x800000
//const	int		SCF_CAT	=	17;
const	uint	JEDNA23	=	0x800000;


bool Asterix017_to_SurvCoordFunction ( const unsigned char  *data, int size, std::deque < RDFSurvCoordFunction > &scf_list )
{
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int cat;
	int len;

	scf_list.clear ();

	if ( !data )
	{
		cms_error ( "wrong data" );
		return false;
	}

	if ( size < 0 || size > 0xFFFF )
	{
		cms_error  ( "wrong size %d", size );
		return false;
	}

	msg = (unsigned char *) data;
	cat = msg[0];
	len = (((unsigned) msg[1]) << 8) | msg[2];
	record = msg + 3;

	if ( cat != SCF_CAT )
	{ 
		cms_error ( "unknown category %d", cat );
		return false;
	}

	if ( len < size )
	{
		cms_warning ( "wrong length %d %d", len, size );
		size = len;
	}
	else if ( len > size )
	{
		cms_error ( "wrong length %d %d", len, size );
		goto error;
	}

	while ( record < msg + len )
	{
		// vytvorime novou polozku

		RDFSurvCoordFunction scf;
		scf.CAT = SCF_CAT;

		// najdeme zacatek datovych poli

		fspec = field = record;

		while ( *field & FSPEC_FX )
		{
			if ( ++field >= msg + len )
			{
				cms_error ( "end of message" );
				goto error;
			}
		}

		field++;

		// Data Source Identifier
		if ( *fspec & FSPEC01_I017_010 )
		{
			scf.Source.SAC = field[0];
			scf.Source.SIC = field[1];
			field += 2;
		}

		// Data Destination Identifier
		if ( *fspec & FSPEC02_I017_012 )
		{
			scf.Destination.SAC = field[0];
			scf.Destination.SIC = field[1];
			field += 2;
		}

		// typ zpravy
		if ( *fspec & FSPEC03_I017_000 )
		{
		//INFO ("SIC %d, SAC %d", scf.SIC, scf.SAC);
			scf.Message_type	= (ENUM_MESSAGE_TYPE_17) field[0];
			field += 1;
		}

		// station/node list
		if ( *fspec & FSPEC04_I017_350 )
		{
			scf.Station_list.Present	= field[0];
			field += 1;

			if ( scf.Station_list.Present > 0 )
			{
				for ( uint i=0; i<scf.Station_list.Present; i++ )
				{
					RADAR_ID	rid;
					rid.SAC	= field [ 2 * i ];
					rid.SIC	= field [ 2 * i + 1 ];
					scf.Station_list.ins_ss ( rid );
				}
				field += 2 * scf.Station_list.Present;
			/*
				unsigned	len = 2 * scf.Station_list.Present;
				scf.Station_list.Data	= new unsigned char [ len ];
				memcpy ( scf.Station_list.Data, field, len );
				field += len;
			*/
			}
		}

		// Target Address
		if ( *fspec & FSPEC05_I017_220 )
		{
			scf.AircraftAddress.Present	= true;
			scf.AircraftAddress.Address	= field2unsigned ( field, 24 );
			field += 3;
		}

		// Track Number
		if ( *fspec & FSPEC06_I017_221 )
		{
			scf.DuplAddrRefNum.Present	= true;
			scf.DuplAddrRefNum.Number		= field2unsigned ( field, 12 );
			field += 2;
		}

		// Time of Day
		if ( *fspec & FSPEC07_I017_140 )
		{
			scf.Time.Present	= true;
			scf.Time.Time			= ((long long) field2unsigned ( field, 24 ) * 1000 + 64 ) / 128;
			field += 3;
		}

		if ( !( *fspec & FSPEC_FX )) goto end;
		fspec++;

//*******************************************************************************

		// Position in WGS-84 Coordinates
		if ( *fspec & FSPEC08_I017_045 )
		{
			scf.Geographic = new RDFCoorGeographic ();
			scf.Geographic->Latitude	= (double) field2signed ( field, 24 ) * M_PI / JEDNA23;
			field += 3;
			scf.Geographic->Longitude	= (double) field2signed ( field, 24 ) * M_PI / JEDNA23;
			field += 3;
		}

		// Mode-3/A Code in Octal Representation
		if ( *fspec & FSPEC09_I017_070 )
		{
			scf.Mode3A = new RDFTargetMode ();
			scf.Mode3A->Valid		= ( field[0] & 0x80 ) ? false	: true;
			scf.Mode3A->Garbled = ( field[0] & 0x40 ) ? true	: false;
			scf.Mode3A->Tracked = ( field[0] & 0x20 ) ? true	: false;
			scf.Mode3A->Code = field2unsigned ( field, 12 );
			field += 2;
		}

		// Flight Level in Binary Representation
		if ( *fspec & FSPEC10_I017_050 )
		{
			scf.FlightLevel	= new RDFTargetFlightLevel ();
			scf.FlightLevel->Valid		= ( field[0] & 0x80 ) ? false	: true;
			scf.FlightLevel->Garbled	= ( field[0] & 0x40 ) ? true	: false;
			scf.FlightLevel->Height		= (double) field2signed ( field, 14 ) * 25 * FEET;
			field += 2;
		}

		// Calculated Track Velocity in Cartesian Coordinates
		if ( *fspec & FSPEC11_I017_200 )
		{
			scf.GroundSpeed	= new RDFTargetGroundSpeed ();
			scf.GroundSpeed->Speed		= (double) field2unsigned ( field, 16 ) * 1852 / ( 1<<14 );
			scf.GroundSpeed->Heading	= (double) field2unsigned ( field + 2, 16 ) * ( 2 * M_PI ) / ( 1<<16 );
/*
			double vx = (double) field2signed ( field, 16 ) / 4;
			double vy = (double) field2signed ( field + 2, 16 ) / 4;
			scf.GroundSpeed = new RDFTargetGroundSpeed ();
			rdf_xy2rt ( vx, vy, scf.GroundSpeed->Speed, scf.GroundSpeed->Heading );
*/
			field += 4;
		}

		// Transponder Capabilty
		if ( *fspec & FSPEC12_I017_230 )
		{
			scf.TransponderCap.Present	= true;
			scf.TransponderCap.Cap			= (ENUM_COMM_CAP) field[0];
			field += 1;
		}

		// Track Status
		if ( *fspec & FSPEC13_I017_240 )
		{
			scf.TrackStatus.Present	= true;
			memcpy ( &scf.TrackStatus.StatusTrack, field, 1 );
			field += 1;
		}

		// ModeS address list
		if ( *fspec & FSPEC14_I017_210 )
		{
			field[0] = scf.ModeS_addr_list.Present;
			field += 1;

			if ( scf.ModeS_addr_list.Present > 0 )
			{
				unsigned len = 3 * scf.ModeS_addr_list.Present;
				scf.ModeS_addr_list.Data	= new unsigned char [ len ];
				memcpy ( scf.ModeS_addr_list.Data, field, len );
				field += len;
			}
		}

		// konec polozky fspec
		*fspec |= FSPEC_FX;
		fspec++;

//*******************************************************************************

		// Cluster Controller Command State
		if ( *fspec & FSPEC15_I017_360 )
		{
			scf.CC_State.Present	= true;
			scf.CC_State.Value		= field[0];
			field += 1;
		}



/*
		// Target Report Descriptor
		if (*fspec & FSPEC02_I020_020)
		{
			if (field[0] & 1)
			{
				field += 1;
				scf.SPI = (field[0] & 0x40) ? true : false;
				scf.Simulated = (field[0] & 0x04) ? true : false;
				scf.Tested = (field[0] & 0x02) ? true : false;
			}

			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents");
				goto error;
			}

			field += 1;
		}

		// Position in Cartesian Coordinates
		if (*fspec & FSPEC05_I020_042)
		{
			scf.Cartesian = new RDFCoorCartesian ();
			scf.Cartesian->X = (double) field2signed (field, 24) / 2;
			scf.Cartesian->Y = (double) field2signed (field + 3, 24) / 2;
			field += 6;
		}

		// Track Status
		if (*fspec & FSPEC07_I020_170)
		{
			scf.Tentative = (field[0] & 0x80) ? true : false;
			scf.Cancel = (field[0] & 0x40) ? true : false;
			scf.CST = (field[0] & 0x20) ? true : false;

			switch ((field[0] >> 3) & 0x03)
			{
				case 0: scf.TrackClimb = TRACK_CLIMB_MAINTAINING; break;
				case 1: scf.TrackClimb = TRACK_CLIMB_CLIMBING; break;
				case 2: scf.TrackClimb = TRACK_CLIMB_DESCENDING; break;
				case 3: scf.TrackClimb = TRACK_CLIMB_UNKNOWN; break;
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

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		// Mode-C Code
		if (*fspec & FSPEC11_I020_100)
		{
			scf.ModeC = new RDFTargetMode ();
			scf.ModeC->Valid = (field[0] & 0x80) ? false : true;
			scf.ModeC->Garbled = (field[0] & 0x40) ? true : false;
			scf.ModeC->Code = field2unsigned (field, 12);
			scf.ModeC->ConfidencePresent = true;
			scf.ModeC->ConfidenceCode = field2unsigned (field + 2, 12);
			field += 4;
		}

		// Target Identification
		if (*fspec & FSPEC13_I020_245)
		{
			char callsign[8];
			DekodujCallSign (field + 1, callsign);
			if (CheckCallSign (callsign, 8))
			{
				scf.AircraftIdentification.assign (callsign, 8);
			}
			field += 7;
		}

		// Measured Height (Cartesian Coordinates)
		if (*fspec & FSPEC14_I020_110)
		{
			scf.MeasuredHeight = new RDFTargetHeight ();
			scf.MeasuredHeight->Height = field2signed (field, 16) * 6.25 * 0x3048;
			field += 2;
		}

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		// Geometric Height WGS-84
		if (*fspec & FSPEC15_I020_105)
		{
			scf.GeometricHeight = new RDFTargetHeight ();
			scf.GeometricHeight->Height = field2signed (field, 16) * 6.25 * 0x3048;
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
				scf.ModeS.push_back (modes);
				field += 8;
			}
		}

		if (!(*fspec & FSPEC_FX)) goto end;
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
			scf.WarningError.push_back (field[0] >> 1);
			while (field[0] & 1)
			{
				field += 1;
				scf.WarningError.push_back (field[0] >> 1);
			}
			field += 1;
		}

		// Mode-1 Code in Octal Representation
		if (*fspec & FSPEC25_I020_055)
		{
			scf.Mode1 = new RDFTargetMode ();
			scf.Mode1->Valid = (field[0] & 0x80) ? false : true;
			scf.Mode1->Garbled = (field[0] & 0x40) ? true : false;
			scf.Mode1->Tracked = (field[0] & 0x20) ? true : false;
			scf.Mode1->Code = ((field[0] & 0x1c) << 7) | ((field[0] & 0x3) << 6);
			field += 1;
		}

		// Mode-2 Code in Octal Representation
		if (*fspec & FSPEC26_I020_050)
		{
			scf.Mode2 = new RDFTargetMode ();
			scf.Mode2->Valid = (field[0] & 0x80) ? false : true;
			scf.Mode2->Garbled = (field[0] & 0x40) ? true : false;
			scf.Mode2->Tracked = (field[0] & 0x20) ? true : false;
			scf.Mode2->Code = field2unsigned (field, 12);
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

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		cms_error ("unable to handle more fspec");
		goto error;
*/

end:
		// pridame polozku do seznamu
		scf_list.push_back ( scf );
		// presuneme se na dalsi zaznam
		record = field;

	}	//while ( record < msg + len )

	if ( record > msg + len )
	{
		cms_error ( "wrong packet length (%d > %d)", record - msg, size );
		goto error;
	}

	return true;

error:
	cms_data ( size, msg );
	scf_list.clear ();
	return false;
}


int SurvCoordFunction_to_Asterix017 ( std::deque < RDFSurvCoordFunction > &scf_list, unsigned char *data, int maxsize )
{
	unsigned char buffer[4096];
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int fspec_size;
	int field_size;
	int size;

	memset ( data, 0, maxsize );
	msg = (unsigned char *) data;

	msg[0] = SCF_CAT;	// kategorie
	msg[1] = 0;				// delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if ( scf_list.size () == 0 ) return 0;

	size = 3;
	while ( scf_list.size () > 0 )
	{
		RDFSurvCoordFunction scf = scf_list.front ();
		memset ( buffer, 0, sizeof ( buffer ) );
		record	= fspec = msg + size;
		field		= buffer;
		*fspec = 0;

//*******************************************************************************

		// data source identifier
		//INFO ("SIC %d, SAC %d", scf.SIC, scf.SAC);
		*fspec |= FSPEC01_I017_010;
		field[0] = scf.Source.SAC;
		field[1] = scf.Source.SIC;
		field += 2;

		// data destination identifier
		//INFO ("SIC %d, SAC %d", scf.SIC, scf.SAC);
		*fspec |= FSPEC02_I017_012;
		field[0] = scf.Destination.SAC;
		field[1] = scf.Destination.SIC;
		field += 2;

		// typ zpravy
		//INFO ("SIC %d, SAC %d", scf.SIC, scf.SAC);
		*fspec |= FSPEC03_I017_000;
		field[0] = scf.Message_type;
		field += 1;

		// station/node list
		if ( scf.Station_list.Present > 0 )
		{
			*fspec |= FSPEC04_I017_350;
			field[0] = scf.Station_list.Present;
			field += 1;

			for ( uint i=0; i<scf.Station_list.Present; i++ )
			{
				RADAR_ID	rid = scf.Station_list.Data [ i ];
				field[2*i]		= rid.SAC;
				field[2*i+1]	= rid.SIC;
			}

//			memcpy ( field, scf.Station_list.Data, 2*scf.Station_list.Present );
			field += 2 * scf.Station_list.Present;
		}

		// aircraft address
		if ( scf.AircraftAddress.Present )
		{
			*fspec |= FSPEC05_I017_220;
			field[0] = ( scf.AircraftAddress.Address >> 16 ) & 0xFF;
			field[1] = ( scf.AircraftAddress.Address >>  8 ) & 0xFF;
			field[2] = scf.AircraftAddress.Address & 0xFF;
			field += 3;
		}

		// Duplicated Address Ref Num
		if ( scf.DuplAddrRefNum.Present )
		{
			*fspec |= FSPEC06_I017_221;
			//memcpy ( field, &scf.DuplAddrRefNum.Num, 2 );

			unsigned short num = scf.DuplAddrRefNum.Number & 0xFFFF;
			memcpy ( field, &num, 2 );

			field[0] = ( scf.DuplAddrRefNum.Number >> 8 ) & 0xFF;
			field[1] = scf.DuplAddrRefNum.Number & 0xFF;

			field += 2;
		}

		// time of day
		if ( scf.Time.Present )
		{
			*fspec |= FSPEC07_I017_140;
			int time = RDF_time_to_asterix_time(scf.Time.Time);
			field[0] = ( time >> 16 ) & 0xFF;
			field[1] = ( time >>  8 ) & 0xFF;
			field[2] = time & 0xFF;
			field += 3;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

//*******************************************************************************

		if ( scf.Geographic )
		{
			*fspec |= FSPEC08_I017_045;
			long lat = lrint (scf.Geographic->Latitude * JEDNA23 / M_PI);
			long lon = lrint (scf.Geographic->Longitude * JEDNA23 / M_PI);
			if (lat > 0x7FFFFF || lat < -0x800000)
			{
				cms_warning ("lat out of limit");
				goto skip;
			}
			if (lon > 0x7FFFFF || lon < -0x800000)
			{
				cms_warning ("lon out of limit");
				goto skip;
			}
			field[0] = ( lat >> 16 ) & 0xFF;
			field[1] = ( lat >>  8 ) & 0xFF;
			field[2] = lat & 0xFF;
			field[3] = ( lon >> 16 ) & 0xFF;
			field[4] = ( lon >>  8 ) & 0xFF;
			field[5] = lon & 0xFF;

			field += 6;
		}

		// Mode 3/A code in octal representation
		if ( scf.Mode3A )
		{
			*fspec |= FSPEC09_I017_070;
			field[0]	= scf.Mode3A->Valid ? 0 : 0x80;
			field[0] |= scf.Mode3A->Garbled ? 0x40 : 0;
			field[0] |= scf.Mode3A->Tracked ? 0x20 : 0;
			field[0] |= ( scf.Mode3A->Code >> 8 ) & 0x0F;
			field[1]	= scf.Mode3A->Code & 0xFF;
			field += 2;
		}

		// Aircraft height - flight level
		if ( scf.FlightLevel )
		{
			*fspec |= FSPEC10_I017_050;

			int fl = (int) rint ( scf.FlightLevel->Height / FEET / 25 );
			field[0]	= scf.FlightLevel->Valid ? 0 : 0x80;
			field[0] |= scf.FlightLevel->Garbled ? 0x40 : 0;
			field[0] |= ( fl >> 8 ) & 0x3F;
			field[1] = fl & 0xFF;
			field += 2;
		}

		// ground speed
		if ( scf.GroundSpeed )
		{
			*fspec |= FSPEC11_I017_200;

			unsigned short speed = (unsigned short) ( scf.GroundSpeed->Speed * (1<<14) / 1852 );
			field[0] = ( speed >> 8 ) & 0xFF;
			field[1] = speed & 0xFF;
			field += 2;

			unsigned short heading = (unsigned short) ( scf.GroundSpeed->Heading * (1<<16) / ( 2 * M_PI ) );
			field[0] = ( heading >> 8 ) & 0xFF;
			field[1] = heading & 0xFF;
			field += 2;
		}

		// Transponder Capabilty
		if ( scf.TransponderCap.Present )
		{
			*fspec |= FSPEC12_I017_230;
			field[0] = scf.TransponderCap.Cap;
			field += 1;
		}

		// Track Status
		if ( scf.TrackStatus.Present )
		{
			*fspec |= FSPEC13_I017_240;
			memcpy ( field, &scf.TrackStatus.StatusTrack, 1 );
			field += 1;
		}

		// ModeS address list
		if ( scf.ModeS_addr_list.Present > 0 )
		{
			*fspec |= FSPEC14_I017_210;
			field[0] = scf.ModeS_addr_list.Present;
			field += 1;

			memcpy ( field, scf.ModeS_addr_list.Data, 3*scf.ModeS_addr_list.Present );
			field += 3*scf.ModeS_addr_list.Present;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

//*******************************************************************************

		// Cluster Controller Command State
		if ( scf.CC_State.Present )
		{
			*fspec |= FSPEC15_I017_360;
			field[0] = scf.CC_State.Value;
			field += 1;
		}


	#if 	0
		// 
		if ( scf. )
		{
			*fspec |= FSPEC_I017_0;
			field[0] = scf.;
			field += 1;
		}

		// konec polozky fspec
		*fspec |= FSPEC_FX;
		fspec++;

//*******************************************************************************
#endif

		// FSPEC4_RE_Data

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
		if ( size + fspec_size + field_size > maxsize ) break;

		// zkopirujeme polozku do zaznamu
		memcpy ( fspec, buffer, field_size );
		size += fspec_size + field_size;

skip:
		// odstranime scf z fronty
		scf_list.pop_front ();

	}		//while ( scf_list.size () > 0 )

	msg[1] = ( size >> 8 ) & 0xFF;
	msg[2] = size & 0xFF;

	return size;
}

