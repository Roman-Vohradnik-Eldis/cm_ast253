#include "RDFAsterix.hh"
#include "CMSDebug.hh"


#define FSPEC_FX 0x01

#define FSPEC01_I018_036 0x80
#define FSPEC02_I018_037 0x40
#define FSPEC03_I018_000 0x20
#define FSPEC04_I018_001 0x10
#define FSPEC05_I018_005 0x08
#define FSPEC06_I018_016 0x04
#define FSPEC07_I018_017 0x02

#define FSPEC08_I018_018 0x80
#define FSPEC09_I018_019 0x40
#define FSPEC10_I018_028 0x20
#define FSPEC11_I018_030 0x10
#define FSPEC12_I018_025 0x08
#define FSPEC13_I018_027 0x04
#define FSPEC14_I018_029 0x02

#define FSPEC15_I018_002 0x80
#define FSPEC16_I018_006 0x40
#define FSPEC17_I018_007 0x20
#define FSPEC18_I018_008 0x10
#define FSPEC19_I018_009 0x08
#define FSPEC20_I018_010 0x04
#define FSPEC21_I018_011 0x02

#define FSPEC22_I018_014 0x80
#define FSPEC23_I018_015 0x40
#define FSPEC24_I018_020 0x20
#define FSPEC25_I018_021 0x10
#define FSPEC26_I018_022 0x08
#define FSPEC27_I018_023 0x04
#define FSPEC28_I018_004 0x02

#define FSPEC29_I018_031 0x80
#define FSPEC30_I018_032 0x40
#define FSPEC31_I018_033 0x20
#define FSPEC32_I018_034 0x10
#define FSPEC33_I018_035 0x08
#define FSPEC34_I018_012 0x04
#define FSPEC35_I018_013 0x02

bool Asterix018_to_DLFunction ( const unsigned char *data, int size, deque<RDFDatalinkFunction> &dlf_list )
{
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int cat;
	int len;


	dlf_list.clear ();
	
	if ( !data )
	{
		cms_error ( "wrong data" );
		return false;
	}

	if ( size < 0 || size > 0xFFFF )
	{
		cms_error ( "wrong size %d", size );
		return false;
	}

	msg = ( unsigned char * ) data;
	cat = msg[0];
	len = ( ((unsigned) msg[1]) << 8 ) | msg[2];
	record = msg + 3;

	if ( cat != DLF_CAT )
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
		RDFDatalinkFunction dlf;
		dlf.CAT = DLF_CAT;
		
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

		// data source identifier (2)
		if ( *fspec & FSPEC01_I018_036 )
		{
			dlf.Source.SAC = field[0];
			dlf.Source.SIC = field[1];
			//INFO ("SAC %d SIC %d", dlf.SAC, dlf.SIC);
			field += 2;
		}

		// data destination identifier (2)
		if ( *fspec & FSPEC02_I018_037 )
		{
			dlf.Destination.SAC = field[0];
			dlf.Destination.SIC = field[1];
			//INFO ("SAC %d SIC %d", dlf.SAC, dlf.SIC);
			field += 2;
		}

		/* message type (1) */
		if (*fspec & FSPEC03_I018_000)
		{
			dlf.Message_type = static_cast<ENUM_MESSAGE_TYPE_18>(field[0]);
			field += 1;
		}

		/* result (1) */
		if (*fspec & FSPEC04_I018_001)
		{
			dlf.Result.Present = true;
			dlf.Result.Cause = static_cast<ENUM_RESULT_CAUSE>(field[0] >> 4);
			dlf.Result.Diag = static_cast<ENUM_RESULT_DIAG>(field[0] & 0x0f);
			field += 1;
		}

		/* Mode S address (3) */
		if (*fspec & FSPEC05_I018_005)
		{
			dlf.AircraftAddress.Present = true;
			dlf.AircraftAddress.Address = field2unsigned (field, 24);
			field += 3;
		}

		/* Packet number (4) */
		if (*fspec & FSPEC06_I018_016)
		{
			dlf.Packet_Num.Present = true;
			dlf.Packet_Num.Value = field2unsigned (field, 32);
			field += 4;
		}

		/* packet number list (1 + 4*N) */
		if (*fspec & FSPEC07_I018_017)
		{
			size_t rep = field[0];
			field += 1;

			for (size_t i = 0; i < rep; ++i)
			{
				dlf.Packet_List.push_back (field2unsigned (field, 32));
				field += 4;
			}
		}

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		/* packet properties (1) */
		if (*fspec & FSPEC08_I018_018)
		{
			dlf.Packet_Prop.Present = true;
			dlf.Packet_Prop.Priority = (field[0] >> 2) & 0x1F;
			dlf.Packet_Prop.Type = static_cast<ENUM_PACKET_TYPE>(field[0] & 0x3);
			field += 1;
		}

		/* Mode S packet (3+) */
		if (*fspec & FSPEC09_I018_019)
		{
			dlf.ModeS_Packet.assign (field + 1, field + field[0]);
			field += field[0];
		}

		/* gicb periodicity (2) */
		if (*fspec & FSPEC10_I018_028)
		{
			dlf.GICB_Periodicity.Present = true;
			dlf.GICB_Periodicity.Value = field2unsigned (field, 16);
			field += 2;
		}

		/* gicb properties (2) */
		if (*fspec & FSPEC11_I018_030)
		{
			dlf.GICB_Prop.Present = true;
			dlf.GICB_Prop.Prop.Priority = field[0] >> 3;
			dlf.GICB_Prop.Prop.PC = field[1] >> 7;
			dlf.GICB_Prop.Prop.AU = (field[1] >> 6) & 0x1;
			dlf.GICB_Prop.Prop.NE = (field[1] >> 5) & 0x1;
			dlf.GICB_Prop.Prop.RD = (field[1] >> 3) & 0x3;
			field += 2;
		}

		/* gicb number (4) */
		if (*fspec & FSPEC12_I018_025)
		{
			dlf.GICB_Num.Present = true;
			dlf.GICB_Num.Value = field2unsigned (field, 32);
			field += 4;
		}

		/* BDS code (1) */
		if (*fspec & FSPEC13_I018_027)
		{
			dlf.BDS_Code.Present = true;
			dlf.BDS_Code.Value = field[0];
			field += 1;
		}

		/* gicb extracted (7) */
		if (*fspec & FSPEC14_I018_029)
		{
			dlf.GICB_Extracted = new RDFTargetModeS ();
			memcpy (dlf.GICB_Extracted->MessageData, field, 7);
			field += 7;
		}

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		/* time (3) */
		if (*fspec & FSPEC15_I018_002)
		{
			dlf.Time.Present = true;
			dlf.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
			field += 3;
		}

		/* Mode S address list (1 + 3*N) */
		if (*fspec & FSPEC16_I018_006)
		{
			size_t rep = field[0];
			field += 1;

			for (size_t i = 0; i < rep; ++i)
			{
				dlf.ModeS_addr_list.push_back (field2unsigned (field, 24));
				field += 3;
			}
		}

		/* data link command (1) */
		if (*fspec & FSPEC17_I018_007)
		{
			dlf.DL_Command.Present = true;
			dlf.DL_Command.Command.ULM = static_cast<ENUM_LINK_MASK>((field[0] >> 7) & 0x1);
			dlf.DL_Command.Command.DLM = static_cast<ENUM_LINK_MASK>((field[0] >> 6) & 0x1);
			dlf.DL_Command.Command.ULC = static_cast<ENUM_LINK_COMMAND>((field[0] >> 5) & 0x1);
			dlf.DL_Command.Command.DLC = static_cast<ENUM_LINK_COMMAND>((field[0] >> 4) & 0x1);
			field += 1;
		}

		/* data link status (1+) */
		if (*fspec & FSPEC18_I018_008)
		{
			dlf.DL_Status.Present = true;
			dlf.DL_Status.UDS = static_cast<ENUM_LINK_COMMAND>((field[0] >> 7) & 0x1);
			dlf.DL_Status.DDS = static_cast<ENUM_LINK_COMMAND>((field[0] >> 6) & 0x1);
			dlf.DL_Status.UCS = static_cast<ENUM_LINK_COMMAND>((field[0] >> 5) & 0x1);
			dlf.DL_Status.DCS = static_cast<ENUM_LINK_COMMAND>((field[0] >> 4) & 0x1);
			dlf.DL_Status.EI = (field[0] >> 1) & 0x1;
			
			if (field[0] & 0x1)
			{
				field += 1;
				dlf.DL_Status.IC = field[0] >> 7;
			}

			/* no more extents defined */
			if (field[0] & 0x1)
			{
				cms_error ("unable to handle more extents");
				goto error;
			}

			field += 1;
		}

		/* data link report request (1+) */
		if (*fspec & FSPEC19_I018_009)
		{
			dlf.DL_Report_Req.Present = true;
			dlf.DL_Report_Req.DL_Status = field[0] >> 7;
			dlf.DL_Report_Req.D_COM = (field[0] >> 6) & 0x1;
			dlf.DL_Report_Req.D_ECA = (field[0] >> 5) & 0x1;
			dlf.DL_Report_Req.D_CQF = (field[0] >> 4) & 0x1;
			dlf.DL_Report_Req.D_CQF_Method = (field[0] >> 3) & 0x1;
			dlf.DL_Report_Req.D_Polar_Pos = (field[0] >> 2) & 0x1;
			dlf.DL_Report_Req.D_Cart_Pos = (field[0] >> 1) & 0x1;

			if (field[0] & 0x1)
			{
				field += 1;
				dlf.DL_Report_Req.ID = field[0] >> 7;
				dlf.DL_Report_Req.Mode_A = (field[0] >> 6) & 0x1;
				dlf.DL_Report_Req.Speed = (field[0] >> 5) & 0x1;
				dlf.DL_Report_Req.Height = (field[0] >> 4) & 0x1;
				dlf.DL_Report_Req.Heading = (field[0] >> 3) & 0x1;
			}

			field += 1;
		}

		/* transponder communications capability (1) */
		if (*fspec & FSPEC20_I018_010)
		{
			dlf.D_Comm.Present = true;
			dlf.D_Comm.ComCap = static_cast<COMM_CAPABILITY>(field[0] & 0x7);
			field += 1;
		}

		/* capability report (7) */
		if (*fspec & FSPEC21_I018_011)
		{
			dlf.CapReport = new RDFTargetModeS ();
			memcpy (dlf.CapReport->MessageData, field, 7);
			field += 7;
		}

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		/* position in polar coordinates (4) */
		if (*fspec & FSPEC22_I018_014)
		{
			dlf.Polar_Pos = new RDFCoorPolar ();
			dlf.Polar_Pos->Rho = field2unsigned (field, 16) * 1852 / 256;
			dlf.Polar_Pos->Theta = ((double) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000);
			field += 4;
		}

		/* position in cartesian coordinate (4) */
		if (*fspec & FSPEC23_I018_015)
		{
			dlf.Cartesian_Pos = new RDFCoorCartesian ();
			dlf.Cartesian_Pos->X = field2signed (field, 16) * 1852 / 128.0;
			dlf.Cartesian_Pos->Y = field2signed (field + 2, 16) * 1852 / 128.0;
			field += 4;
		}

		/* broadcast number (4) */
		if (*fspec & FSPEC24_I018_020)
		{
			dlf.Broadcast_Num.Present = true;
			dlf.Broadcast_Num.Value = field2unsigned (field, 32);
			field += 4;
		}

		/* broadcast properties (6) */
		if (*fspec & FSPEC25_I018_021)
		{
			dlf.Broadcast_Prop.Present = true;
			dlf.Broadcast_Prop.Prop.Priority = field[0] >> 4;
			dlf.Broadcast_Prop.Prop.Power = field[0] & 0xF;
			dlf.Broadcast_Prop.Prop.Duration = field[1];
			dlf.Broadcast_Prop.Prop.Coverage = field2unsigned (field + 2, 32);
			field += 6;
		}

		/* broadcast prefix (4) */
		if (*fspec & FSPEC26_I018_022)
		{
			dlf.Broadcast_Prefix.Present = true;
			dlf.Broadcast_Prefix.Prefix.Prefix = field2unsigned (field, 27);
			field += 4;
		}

		/* uplink or downlink broadcast (7) */
		if (*fspec & FSPEC27_I018_023)
		{
			dlf.Broadcast = new RDFTargetModeS ();
			memcpy (dlf.Broadcast->MessageData, field, 7);
			field += 7;
		}

		/* II code (1) */
		if (*fspec & FSPEC28_I018_004)
		{
			dlf.II_Code.Present = true;
			dlf.II_Code.Former = field[0] >> 4;
			dlf.II_Code.Current = field[0] & 0xF;
			field += 1;
		}

		if (!(*fspec & FSPEC_FX)) goto end;
		fspec++;

		/* aircraft identity (6) */
		if (*fspec & FSPEC29_I018_031)
		{
			asterix_decode_callsign (field, dlf.AircraftIdentity);
			field += 6;
		}

		/* aircraft Mode A (2) */
		if (*fspec & FSPEC30_I018_032)
		{
			dlf.Mode3A = new RDFTargetMode ();
			dlf.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			
			dlf.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			dlf.Mode3A->Code = field2unsigned (field, 12);
			field += 2;
		}

		/* aircraft height (2) */
		if (*fspec & FSPEC31_I018_033)
		{
			dlf.FlightLevel = new RDFTargetFlightLevel ();
			dlf.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
			dlf.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
			dlf.FlightLevel->Height = (double) field2signed (field, 14) * 25 * 0.3048;
			field += 2;
		}

		/* aircraft speed (2) */
		if (*fspec & FSPEC32_I018_034)
		{
			dlf.Speed = new RDFTargetSpeed (field);
			field += 2;
		}

		/* aircraft heading (2) */
		if (*fspec & FSPEC33_I018_035)
		{
			dlf.Heading = new RDFTargetHeading (field);
			field += 2;
		}

		/* aircraft coverage quality factor (1) */
		if (*fspec & FSPEC34_I018_012)
		{
			dlf.CQF.Present = true;
			dlf.CQF.FlightStatus = static_cast<AIRCRAFT_CQF>(field[0] >> 7);
			dlf.CQF.CQF = field[0] & 0x7F;
			field += 1;
		}

		/* aircraft CQF calculation method (1) */
		if (*fspec & FSPEC35_I018_013)
		{
			dlf.CQF_Method.Present = true;
			dlf.CQF_Method.Value = field[0];
			field += 1;
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
		dlf_list.push_back (dlf);

		// presuneme se na dalsi zaznam
		record = field;

	}

	if ( record > msg + len )
	{
		cms_error ( "wrong packet length (%d > %d)", record - msg, size );
		goto error;
	}

	return true;

error:
	cms_data  (size, msg );
	dlf_list.clear ();
	return false;
}


#if 	0
bool Asterix018_to_TargetTrack (const void * data, int size, 
	deque<RDFDatalinkFunction> & dlf_list)
{
	deque<RDFDatalinkFunction> target_list;
	
	dlf_list.clear ();

	if (!Asterix018_to_Target (data, size, target_list))
	{
		//cms_info ("cat 18 target failed");
		return false;
	}

	while (!target_list.empty ())
	{
		if (target_list.front().TrackNumber.Present)
		{
			dlf_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}


bool Asterix018_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list)
{
	deque<RDFDatalinkFunction> target_list;
	
	plot_list.clear ();

	if (!Asterix018_to_Target (data, size, target_list))
	{
		//cms_info ("cat 18 target failed");
		return false;
	}

	while (!target_list.empty ())
	{
		if (!target_list.front().TrackNumber.Present)
		{
			plot_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}
#endif


int DLFunction_to_Asterix018 ( deque<RDFDatalinkFunction> &dlf_list, unsigned char *data, int maxsize )
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

	msg[0] = DLF_CAT;	// kategorie
	msg[1] = 0;				// delka
	msg[2] = 0;	

	// seznam je prazdny, neni co zpracovavat
	if ( dlf_list.size () == 0 ) return 0;
	
	size = 3;
	while ( dlf_list.size () > 0 )
	{
		RDFDatalinkFunction dlf = dlf_list.front ();
		memset ( buffer, 0, sizeof ( buffer ) );
		record	= fspec = msg + size;
		field		= buffer;
		*fspec = 0;

//*******************************************************************************

		// data source identifier
		//INFO ("SIC %d, SAC %d", dlf.SIC, dlf.SAC);
		*fspec |= FSPEC01_I018_036;
		field[0] = dlf.Source.SAC;
		field[1] = dlf.Source.SIC;
		field += 2;

		// data destination identifier
		//INFO ("SIC %d, SAC %d", dlf.SIC, dlf.SAC);
		*fspec |= FSPEC02_I018_037;
		field[0] = dlf.Destination.SAC;
		field[1] = dlf.Destination.SIC;
		field += 2;

		// typ zpravy
		//INFO ("SIC %d, SAC %d", dlf.SIC, dlf.SAC);
		*fspec |= FSPEC03_I018_000;
		field[0] = dlf.Message_type;
		field += 1;

		// action result
		if ( dlf.Result.Present )
		{
			*fspec |= FSPEC04_I018_001;
			field[0] = ( ( dlf.Result.Cause >> 4 ) & 0xF0 ) + ( dlf.Result.Diag & 0x0F );
			field += 1;
		}

		// aircraft address
		if ( dlf.AircraftAddress.Present )
		{
			*fspec |= FSPEC05_I018_005;
			field[0] = ( dlf.AircraftAddress.Address >> 16 ) & 0xFF;
			field[1] = ( dlf.AircraftAddress.Address >> 8 ) & 0xFF;
			field[2] = dlf.AircraftAddress.Address & 0xFF;
			field += 3;
		}

		// packet number
		if ( dlf.Packet_Num.Present )
		{
			*fspec |= FSPEC06_I018_016;
			field[0] = ( dlf.Packet_Num.Value >> 24 ) & 0xFF;
			field[1] = ( dlf.Packet_Num.Value >> 16 ) & 0xFF;
			field[2] = ( dlf.Packet_Num.Value >>  8 ) & 0xFF;
			field[3] = dlf.Packet_Num.Value & 0xFF;
			field += 4;
		}

		// packet number list
		if (dlf.Packet_List.size () > 0)
		{
			*fspec |= FSPEC07_I018_017;
			field[0] = dlf.Packet_List.size ();
			field += 1;

			for (size_t i = 0; i < dlf.Packet_List.size (); ++i)
			{
				uint32_t tmp = dlf.Packet_List.at (i);
				field[0] = (tmp >> 24) & 0xFF;
				field[1] = (tmp >> 16) & 0xFF;
				field[2] = (tmp >> 8) & 0xFF;
				field[3] = tmp & 0xFF;
				field += 4;
			}
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

//*******************************************************************************

		// packet properties
		if ( dlf.Packet_Prop.Present )
		{
			*fspec |= FSPEC08_I018_018;
			field[0] = ( dlf.Packet_Prop.Priority >> 2 ) || dlf.Packet_Prop.Type;
			field += 1;
		}

		// mode S packet
		if ( dlf.ModeS_Packet.size () > 0 )
		{
			*fspec |= FSPEC09_I018_019;
			field[0] = dlf.ModeS_Packet.size ();
			std::copy (dlf.ModeS_Packet.begin (), dlf.ModeS_Packet.end (), field + 1);
			field += 1 + dlf.ModeS_Packet.size ();
		}

		// GICB periodicity
		if ( dlf.GICB_Periodicity.Present )
		{
			*fspec |= FSPEC10_I018_028;
			field[0] = ( dlf.GICB_Periodicity.Value >> 8 ) & 0xFF;
			field[1] = dlf.GICB_Periodicity.Value & 0xFF;
			field += 2;
		}

		// GICB properties
		if ( dlf.GICB_Prop.Present )
		{
			*fspec |= FSPEC11_I018_030;
			/* TODO: check byte order (20141001) */
			memcpy ( field, &dlf.GICB_Prop.Prop, 2 );
			field += 2;
		}

		// GICB number
		if ( dlf.GICB_Num.Present )
		{
			*fspec |= FSPEC12_I018_025;
			field[0] = (dlf.GICB_Num.Value >> 24) & 0xFF;
			field[1] = (dlf.GICB_Num.Value >> 16) & 0xFF;
			field[2] = (dlf.GICB_Num.Value >> 8) & 0xFF;
			field[3] = dlf.GICB_Num.Value & 0xFF;
			field += 4;
		}

		// BDS code
		if ( dlf.BDS_Code.Present )
		{
			*fspec |= FSPEC13_I018_027;
			field[0] = dlf.BDS_Code.Value;
			field += 1;
		}

		// GICB_Extracted = BDS message 56bit
		if ( dlf.GICB_Extracted )
		{
			*fspec |= FSPEC14_I018_029;
			memcpy ( field, dlf.GICB_Extracted->MessageData, 7 );
			field += 7;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

//*******************************************************************************

		// time of day
		if ( dlf.Time.Present )
		{
			*fspec |= FSPEC15_I018_002;
			int time = RDF_time_to_asterix_time(dlf.Time.Time);
			field[0] = ( time >> 16 ) & 0xFF;
			field[1] = ( time >>  8 ) & 0xFF;
			field[2] = time & 0xFF;
			field += 3;
		}

		// ModeS address list
		if ( dlf.ModeS_addr_list.size () > 0 )
		{
			*fspec |= FSPEC16_I018_006;
			field[0] = dlf.ModeS_addr_list.size ();
			field += 1;

			for (size_t i = 0; i < dlf.ModeS_addr_list.size (); ++i)
			{
				uint32_t tmp = dlf.ModeS_addr_list.at (i);
				field[0] = (tmp >> 16) & 0xFF;
				field[1] = (tmp >> 8) & 0xFF;
				field[2] = tmp & 0xFF;
				field += 3;
			}
		}

		// Datalink command
		if ( dlf.DL_Command.Present )
		{
			*fspec |= FSPEC17_I018_007;
			memcpy ( field, &dlf.DL_Command.Command, 1 );
			field += 1;
		}

		// Datalink status
		if ( dlf.DL_Status.Present )
		{
			*fspec |= FSPEC18_I018_008;
			field[0] = dlf.DL_Status.UDS << 7;
			field[0] |= dlf.DL_Status.DDS << 6;
			field[0] |= dlf.DL_Status.UCS << 5;
			field[0] |= dlf.DL_Status.DCS << 4;
			field[0] |= dlf.DL_Status.EI << 1;

			if (dlf.DL_Status.IC)
			{
				field[0] |= 0x01;
				field += 1;
				field[0] = dlf.DL_Status.IC << 7;
			}

			field += 1;
		}

		// Report request
		if ( dlf.DL_Report_Req.Present )
		{
			*fspec |= FSPEC19_I018_009;
			field[0] = field[1] = 0;

			field[0] |= dlf.DL_Report_Req.DL_Status << 7;
			field[0] |= dlf.DL_Report_Req.D_COM << 6;
			field[0] |= dlf.DL_Report_Req.D_ECA << 5;
			field[0] |= dlf.DL_Report_Req.D_CQF << 4;
			field[0] |= dlf.DL_Report_Req.D_CQF_Method << 3;
			field[0] |= dlf.DL_Report_Req.D_Polar_Pos << 2;
			field[0] |= dlf.DL_Report_Req.D_Cart_Pos << 1;

			/* no encoding rule what to do when all bits in extent
			 * are set to 0 */
			field[1] |= dlf.DL_Report_Req.ID << 7;
			field[1] |= dlf.DL_Report_Req.Mode_A << 6;
			field[1] |= dlf.DL_Report_Req.Speed << 5;
			field[1] |= dlf.DL_Report_Req.Height << 4;
			field[1] |= dlf.DL_Report_Req.Heading << 3;

			field += 2;
		}

		// Transponder Comm capability
		if ( dlf.D_Comm.Present )
		{
			*fspec |= FSPEC20_I018_010;
			field[0] = dlf.D_Comm.ComCap;
			field += 1;
		}

		// Capability Report
		if ( dlf.CapReport )
		{
			*fspec |= FSPEC21_I018_011;
			memcpy ( field, dlf.CapReport->MessageData, 7 );
			field += 7;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

//*******************************************************************************

		// polar co-ordinates
		if ( dlf.Polar_Pos )
		{
			*fspec |= FSPEC22_I018_014;
			long rho = lrint (dlf.Polar_Pos->Rho * 256 / 1852 );
			long theta = lrint (dlf.Polar_Pos->Theta * 0x10000 / (2 * M_PI));
			if (rho < 0 || rho > USHRT_MAX)
			{
				cms_warning ("rho out of limit");
				goto skip;
			}
			field[0] = ( rho >> 8 ) & 0xFF;
			field[1] = rho & 0xFF;
			field[2] = ( theta >> 8 ) & 0xFF;
			field[3] = theta & 0xFF;
			field += 4;
		}

		// cartesian co-ordinates
		if ( dlf.Cartesian_Pos )
		{
			*fspec |= FSPEC23_I018_015;
			long x = lrint (dlf.Cartesian_Pos->X * 128 / 1852);
			long y = lrint (dlf.Cartesian_Pos->Y * 128 / 1852);
			if (x > SHRT_MAX || x < SHRT_MIN)
			{
				cms_warning ("x out of limit");
				goto skip;
			}
			if (y > SHRT_MAX || y < SHRT_MIN)
			{
				cms_warning ("y out of limit");
				goto skip;
			}
			field[0] = ( x >> 8 ) & 0xFF;
			field[1] = x & 0xFF;
			field[2] = ( y >> 8 ) & 0xFF;
			field[3] = y & 0xFF;
			field += 4;
		}

		// Broadcast number
		if ( dlf.Broadcast_Num.Present )
		{
			*fspec |= FSPEC24_I018_020;
			field[0] = (dlf.Broadcast_Num.Value >> 24) & 0xFF;
			field[1] = (dlf.Broadcast_Num.Value >> 16) & 0xFF;
			field[2] = (dlf.Broadcast_Num.Value >> 8) & 0xFF;
			field[3] = dlf.Broadcast_Num.Value & 0xFF;
			field += LEN4;
		}

		// Broadcast properties
		if ( dlf.Broadcast_Prop.Present )
		{
			*fspec |= FSPEC25_I018_021;
			field[0] = (dlf.Broadcast_Prop.Prop.Priority << 4) | dlf.Broadcast_Prop.Prop.Power;
			field[1] = dlf.Broadcast_Prop.Prop.Duration;
			field[2] = (dlf.Broadcast_Prop.Prop.Coverage >> 24) & 0xFF;
			field[3] = (dlf.Broadcast_Prop.Prop.Coverage >> 16) & 0xFF;
			field[4] = (dlf.Broadcast_Prop.Prop.Coverage >> 8) & 0xFF;
			field[5] = dlf.Broadcast_Prop.Prop.Coverage & 0xFF;

			field += LEN6;
		}

		// Broadcast prefix
		if ( dlf.Broadcast_Prefix.Present )
		{
			*fspec |= FSPEC26_I018_022;
			field[0] = (dlf.Broadcast_Prefix.Prefix.Prefix >> 24) & 0x7;
			field[1] = (dlf.Broadcast_Prefix.Prefix.Prefix >> 16) & 0xFF;
			field[2] = (dlf.Broadcast_Prefix.Prefix.Prefix >> 8) & 0xFF;
			field[3] = dlf.Broadcast_Prefix.Prefix.Prefix & 0xFF;
			field += LEN4;
		}

		// Broadcast
		if ( dlf.Broadcast )
		{
			*fspec |= FSPEC27_I018_023;
			memcpy ( field, dlf.Broadcast->MessageData, LEN7 );
			field += LEN7;
		}

		// II Code
		if ( dlf.II_Code.Present )
		{
			*fspec |= FSPEC28_I018_004;
			field[0] = ( dlf.II_Code.Former << 4 ) | dlf.II_Code.Current;
			field += 1;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

//*******************************************************************************

		// Aircraft identity
		if ( !dlf.AircraftIdentity.empty () )
		{
			*fspec |= FSPEC29_I018_031;
			asterix_encode_callsign (dlf.AircraftIdentity, field);
			field += LEN6;
		}

		// Mode 3/A code in octal representation
		if ( dlf.Mode3A )
		{
			*fspec |= FSPEC30_I018_032;
			field[0]	= dlf.Mode3A->Valid ? 0 : 0x80;
			field[0] |= dlf.Mode3A->Garbled ? 0x40 : 0;
			field[0] |= dlf.Mode3A->Tracked ? 0x20 : 0;
			field[0] |= ( dlf.Mode3A->Code >> 8 ) & 0x0F;
			field[1]	= dlf.Mode3A->Code & 0xFF;
			field += 2;
		}

		// Aircraft height - flight level
		if ( dlf.FlightLevel )
		{
			*fspec |= FSPEC31_I018_033;

			int fl = (int) rint ( dlf.FlightLevel->Height / FEET / 25 );
			field[0]	= dlf.FlightLevel->Valid ? 0 : 0x80;
			field[0] |= dlf.FlightLevel->Garbled ? 0x40 : 0;
			field[0] |= ( fl >> 8 ) & 0x3F;
			field[1] = fl & 0xFF;
			field += 2;
		}

		// ground speed
		if ( dlf.Speed )
		{
			*fspec |= FSPEC32_I018_034;

			unsigned short speed = (unsigned short) ( dlf.Speed->Speed * (1<<14) / 1852 );
			field[0] = ( speed >> 8 ) & 0xFF;
			field[1] = speed & 0xFF;
			field += 2;
		}

		// ground speed
		if ( dlf.Heading )
		{
			*fspec |= FSPEC33_I018_035;

			unsigned short heading = (unsigned short) ( dlf.Heading->Heading * (1<<16) / ( 2 * M_PI ) );
			field[0] = ( heading >> 8 ) & 0xFF;
			field[1] = heading & 0xFF;
			field += 2;
		}

		// CQF
		if ( dlf.CQF.Present )
		{
			*fspec |= FSPEC34_I018_012;
			field[0] = ( dlf.CQF.FlightStatus << 8 ) | ( dlf.CQF.CQF & 0x7F );
			field += 1;
		}

		// CQF
		if ( dlf.CQF_Method.Present )
		{
			*fspec |= FSPEC35_I018_013;
			field[0] = dlf.CQF_Method.Value;
			field += 1;
		}

#if 	0
		// 
		if ( dlf. )
		{
			*fspec |= FSPEC_I018_0;
			field[0] = dlf.;
			field += 1;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

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
		// odstranime dlf z fronty
		dlf_list.pop_front ();

	}		//while ( dlf_list.size () > 0 )

	msg[1] = ( size >> 8 ) & 0xFF;
	msg[2] = size & 0xFF;

	return size;
}


#if 	0
int TargetTrack_to_Asterix018 (deque<RDFDatalinkFunction> & dlf_list, void * data, int size)
{
	deque<RDFDatalinkFunction> tmp;

	while (!dlf_list.empty ())
	{
		if (dlf_list.front().TrackNumber.Present)
		{
			tmp.push_back (dlf_list.front ());
		}
		dlf_list.pop_front ();
	}

	int send_size = Target_to_Asterix018 (tmp, data, size);

	if (tmp.size ())
	{
		dlf_list.resize (tmp.size ());
		copy (tmp.begin (), tmp.end (), dlf_list.begin ());
	}

	return send_size;
}


int TargetPlot_to_Asterix018 (deque<RDFTargetPlot> & plot_list, void * data, int size)
{
	deque<RDFDatalinkFunction> tmp;

	while (!plot_list.empty ())
	{
		tmp.push_back (plot_list.front ());
		plot_list.pop_front ();
	}

	int send_size = Target_to_Asterix018 (tmp, data, size);


	if (tmp.size ())
	{
		plot_list.resize (tmp.size ());
		copy (tmp.begin (), tmp.end (), plot_list.begin ());
	}

	return send_size;
}
#endif

