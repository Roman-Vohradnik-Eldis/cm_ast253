#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I010_010 0x80
#define FSPEC02_I010_000 0x40
#define FSPEC03_I010_020 0x20
#define FSPEC04_I010_140 0x10
#define FSPEC05_I010_041 0x08
#define FSPEC06_I010_040 0x04
#define FSPEC07_I010_042 0x02

#define FSPEC08_I010_200 0x80
#define FSPEC09_I010_202 0x40
#define FSPEC10_I010_161 0x20
#define FSPEC11_I010_170 0x10
#define FSPEC12_I010_060 0x08
#define FSPEC13_I010_220 0x04
#define FSPEC14_I010_245 0x02

#define FSPEC15_I010_250 0x80
#define FSPEC16_I010_300 0x40
#define FSPEC17_I010_090 0x20
#define FSPEC18_I010_091 0x10
#define FSPEC19_I010_270 0x08
#define FSPEC20_I010_550 0x04
#define FSPEC21_I010_310 0x02

#define FSPEC22_I010_500 0x80
#define FSPEC23_I010_280 0x40
#define FSPEC24_I010_131 0x20
#define FSPEC25_I010_210 0x10
#define FSPEC26_I010_SPA 0x08
#define FSPEC27_I010_SP  0x04
#define FSPEC28_I010_RE  0x02

bool Asterix010_to_Target (const void * data, int size, 
	deque<RDFTargetTrack> & track_list)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	track_list.clear ();
	
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
	len = (((unsigned) msg[1]) << 8) | + msg[2];
	record = msg + 3;

	if (cat != 10)
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
		bool target_report = false;
		RDFTargetTrack track;
		track.CAT = 10;
		
		// prvni fspec
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

		// Data Source Identifier (2)
		if (*fspec & FSPEC01_I010_010)
		{
			track.SAC = field[0];
			track.SIC = field[1];
			//cms_info ("SAC %02x SIC %02x", track.SAC, track.SIC);
			field += 2;
		}
		
		// MessageType (1)
		if (*fspec & FSPEC02_I010_000)
		{
			switch (field[0])
			{
				case 1:
				{
					//cms_info ("Target Report");
					target_report = true;
					break;
				}
				case 2:
				{
					//cms_info ("Start of Update Cycle");
					break;
				}
				case 3:
				{
					//cms_info ("Periodic Status Message");
					break;
				}
				case 4:
				{
					//cms_info ("Event-triggered Status Message");
					break;
				}
				default:
				{
					cms_error ("unknown message type");
				}
			}
			field += 1;
		}
		
		// Target Report Descriptor (1+)
		if (*fspec & FSPEC03_I010_020)
		{
			//cms_info ("target1 %02x", field[0]);
			if (*field & FSPEC_FX)
			{
				field += 1;
				//cms_info ("target2 %02x", field[0]);
				if (*field & FSPEC_FX)
				{
					field += 1;
					//cms_info ("target3 %02x", field[0]);
				}
			}
			field += 1;
		}
		
		// Time Of Day (3)
		if (*fspec & FSPEC04_I010_140)
		{
			track.Time.Present = true;
			track.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
			//cms_info ("time %d", track.Time);
			field += 3;
		}
		
		// Position in WGS-84 Coord (8)
		if (*fspec & FSPEC05_I010_041)
		{
			track.Geographic = new RDFCoorGeographic ();
			track.Geographic->Latitude = (double) field2unsigned (field, 32) * M_PI / 0x80000000;
			track.Geographic->Longitude = (double) field2unsigned (field + 4, 32) * M_PI / 0x80000000;
			//cms_info ("geo %f %f", track.Geographic->Latitude, track.Geographic->Longitude);
			field += 8;
		}
		
		// Position in Polar Coordinates (4)
		if (*fspec & FSPEC06_I010_040)
		{
			track.Polar = new RDFCoorPolar ();
			track.Polar->Rho = field2unsigned (field, 16);
			track.Polar->Theta = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
			//cms_info ("polar %f %f", track.Polar->Rho, track.Polar->Theta);
			field += 4;
		}
		
		// Position in Cartesian Coordinates (4)
		if (*fspec & FSPEC07_I010_042)
		{
			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = field2signed (field, 16);
			track.Cartesian->Y = field2signed (field + 2, 16);
			//cms_info ("polar %f %f", track.Cartesian->X, track.Cartesian->Y);
			field += 4;
		}

		// druhy fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Calculated Track Velocity in Polar Coordinates (2)
		if (*fspec & FSPEC08_I010_200)
		{
			track.GroundSpeed = new RDFTargetGroundSpeed ();
			track.GroundSpeed->Speed = (double) field2signed (field, 16) * 1852 / (1<<14);
			track.GroundSpeed->Heading = (double) field2signed (field + 2, 16) * (2 * M_PI) / (1<<16);
			//cms_info ("speed %f heading %f", track.GroundSpeed->Speed, track.GroundSpeed->Heading);
			field += 4;
		}
		
		// Calculated Track Velocity in Cartesian Coordinates (3)
		if (*fspec & FSPEC09_I010_202)
		{
			//cms_info ("track velocity");
			field += 4;
		}
		
		// Track Number (2)
		if (*fspec & FSPEC10_I010_161)
		{
			track.TrackNumber.Present = true;
			track.TrackNumber.Number = field2unsigned (field, 12);
			//cms_info ("number %d", track.TrackID);
			field += 2;
		}

		// Track Status (1+)
		if (*fspec & FSPEC11_I010_170)
		{
			//cms_info ("track status1 %02x", field[0]);
			track.CST = (field[0] & 0x30) ? true : false;
			
			if (*field & FSPEC_FX)
			{
				field += 1;
				//cms_info ("track status2 %02x", field[0]);
			}

			if (*field & FSPEC_FX)
			{
				field += 1;
				//cms_info ("track status3 %02x", field[0]);
			}

			field += 1;
		}
		
		// Mode-3/A Code in Octal Representation (2)
		if (*fspec & FSPEC12_I010_060)
		{
			track.Mode3A = new RDFTargetMode ();
			track.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			track.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
			track.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			track.Mode3A->Code = field2unsigned (field, 12);
			//cms_info ("mode 3a");
			field += 2;
		}
		
		// Aircraft Address (3)
		if (*fspec & FSPEC13_I010_220)
		{
			track.AircraftAddress.Present = true;
			track.AircraftAddress.Address = field2unsigned (field, 24);
			//cms_info ("aircraft address");
			field += 3;
		}
		
		// Target Identification (7)
		if (*fspec & FSPEC14_I010_245)
		{
			asterix_decode_callsign (field + 1, track.AircraftIdentification);
			field += 7;
		}

		// treti fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Mode S MB Data  (1+8n)
		if (*fspec & FSPEC15_I010_250)
		{
			int rep = field[0];
			field += 1;
			for (int i = 0; i < rep; i++)
			{
				// vytvorime novou polozku
				RDFTargetModeS modes;
				memcpy (modes.MessageData, field, 7);
				modes.Address = field[7];
				track.ModeS.push_back (modes);
				field += 8;
			}
		}
		
		// Vehicle Fleet Idenfificastion (1)
		if (*fspec & FSPEC16_I010_300)
		{
			//cms_info ("vehicle");
			field += 1;
		}
		
		// Flight Level in Binary Representation (2)
		if (*fspec & FSPEC17_I010_090)
		{
			track.FlightLevel = new RDFTargetFlightLevel ();
			track.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
			track.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
			track.FlightLevel->Height = (double) field2signed (field, 14) * 0.3048 * 25;
			//cms_info ("flight level %d", track.FlightLevel.Height);
			field += 2;
		}
		
		// Measured Height (2)
		if (*fspec & FSPEC18_I010_091)
		{
			track.MeasuredHeight = new RDFTargetHeight ();
			track.MeasuredHeight->Height = (double) field2signed (field, 16) * 0.3048 / 6.25;
			//cms_info ("measured height %d", track.Height.Height);
			field += 2;
		}
		
		// Target Size & Orientation (1+)
		if (*fspec & FSPEC19_I010_270)
		{
			//cms_info ("Size & Orientation %02x", field[0]);
			if (*field & FSPEC_FX)
			{
				field += 1;
				//cms_info ("Size & Orientation %02x", field[0]);
			}
			field += 1;
		}
		
		// System Status (1)
		if (*fspec & FSPEC20_I010_550)
		{
			//cms_info ("status %02x", field[0]);
			field += 1;
		}
		
		// Pre-programmed Message (1)
		if (*fspec & FSPEC21_I010_310)
		{
			//cms_info ("message %02", field[0]);
			field += 1;
		}

		// ctvrty fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// standart deviation of position (4)
		if (*fspec & FSPEC22_I010_500)
		{
			//cms_info ("deviation");
			field += 4;
		}
		
		// Presence (1+2n)
		if (*fspec & FSPEC23_I010_280)
		{
			//cms_info ("presence");
			field += 1 + 2 * field[0];
		}
		
		// Amplitude of Primary Plot (1)
		if (*fspec & FSPEC24_I010_131)
		{
			//cms_info ("amplitude");
			field += 1;
		}
		
		// Calculated Acceleration (2)
		if (*fspec & FSPEC25_I010_210)
		{
			//cms_info ("Acceleration");
			field += 2;
		}

		// spare bit
		if (*fspec & FSPEC26_I010_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Special Purpose Field
		if (*fspec & FSPEC27_I010_SP)
		{
			field += field[0];
		}

		// Reserved Expansion Field
		if (*fspec & FSPEC28_I010_RE)
		{
			field += field[0];
		}

		if (*fspec & FSPEC_FX)
		{
			cms_error("unable to handle more fspec");
			goto error;
		}

end:
		// zkontrolujeme, zda se nejedna o track cancel
		if (!track.Cartesian && !track.Polar && !track.Geographic)
		{
			track.Cancel = true;
		}

		// pridame track do seznamu
		if (target_report)
		{
			track_list.push_back (track);
		}

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error("wrong packet length (%d > %d)", record - msg, size);
		goto error;
	}
	
	return true;

error:
	cms_data (size, data);
	track_list.clear ();
	return false;
}

bool Asterix010_to_TargetTrack (const void * data, int size, 
	deque<RDFTargetTrack> & track_list)
{
	deque<RDFTargetTrack> target_list;
	
	track_list.clear ();

	if (!Asterix010_to_Target (data, size, target_list))
	{
		//cms_info ("cat 48 target failed");
		return false;
	}

	while (!target_list.empty ())
	{
		if (target_list.front().TrackNumber.Present)
		{
			track_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}

bool Asterix010_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list)
{
	deque<RDFTargetTrack> target_list;
	
	plot_list.clear ();

	if (!Asterix010_to_Target (data, size, target_list))
	{
		//cms_info ("cat 48 target failed");
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

