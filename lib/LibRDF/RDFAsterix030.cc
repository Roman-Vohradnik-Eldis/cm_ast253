#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I030_010 0x80
#define FSPEC02_I030_015 0x40
#define FSPEC03_I030_030 0x20
#define FSPEC04_I030_035 0x10
#define FSPEC05_I030_040 0x08
#define FSPEC06_I030_070 0x04
#define FSPEC07_I030_170 0x02

#define FSPEC08_I030_100 0x80
#define FSPEC09_I030_180 0x40
#define FSPEC10_I030_181 0x20
#define FSPEC11_I030_060 0x10
#define FSPEC12_I030_150 0x08
#define FSPEC13_I030_130 0x04
#define FSPEC14_I030_160 0x02

#define FSPEC15_I030_080 0x80
#define FSPEC16_I030_090 0x40
#define FSPEC17_I030_200 0x20
#define FSPEC18_I030_220 0x10
#define FSPEC19_I030_240 0x08
#define FSPEC20_I030_290 0x04
#define FSPEC21_I030_260 0x02

#define FSPEC22_I030_360 0x80
#define FSPEC23_I030_140 0x40
#define FSPEC24_I030_340 0x20
#define FSPEC25_I030_RE  0x10
#define FSPEC26_I030_390 0x08
#define FSPEC27_I030_400 0x04
#define FSPEC28_I030_410 0x02

#define FSPEC29_I030_440 0x80
#define FSPEC30_I030_450 0x40
#define FSPEC31_I030_435 0x20
#define FSPEC32_I030_430 0x10
#define FSPEC33_I030_460 0x08
#define FSPEC34_I030_480 0x04
#define FSPEC35_I030_420 0x02

#define FSPEC36_I030_490 0x80
#define FSPEC37_I030_020 0x40
#define FSPEC38_I030_382 0x20
#define FSPEC39_I030_384 0x10
#define FSPEC40_I030_386 0x08
#define FSPEC41_I030_110 0x04
#define FSPEC42_I030_190 0x02

#define FSPEC43_I030_191 0x80
#define FSPEC44_I030_135 0x40
#define FSPEC45_I030_165 0x20
#define FSPEC46_I030_230 0x10
#define FSPEC47_I030_250 0x08
#define FSPEC48_I030_210 0x04
#define FSPEC49_I030_120 0x02

#define FSPEC50_I030_050 0x80
#define FSPEC51_I030_270 0x40
#define FSPEC52_I030_370 0x20
#define FSPEC53_I030_SPA 0x10
#define FSPEC54_I030_SPA 0x08
#define FSPEC55_I030_SPA 0x04
#define FSPEC56_I030_SPA 0x02

bool Asterix030_to_TargetTrack (const void * data, int size, 
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
	len = (((unsigned) msg[1]) << 8) | msg[2];
	record = msg + 3;

	if (cat != 30)
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
		RDFTargetTrack track;
		track.CAT = 30;
		
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

		// Server Indentification Tag
		if (*fspec & FSPEC01_I030_010)
		{
			track.SAC = field[0];
			track.SIC = field[1];
			field += 2;
		}

		// User Number 
		if (*fspec & FSPEC02_I030_015)
		{
			field += 2;
		}

		// Service Identification
		if (*fspec & FSPEC03_I030_030)
		{
			if (field[0] & 1)
			{
				field += 1;
			}

			field += 1;
		}

		// Type Of Message
		if (*fspec & FSPEC04_I030_035)
		{
			field += 1;
		}

		// Track Number
		if (*fspec & FSPEC05_I030_040)
		{
			track.TrackNumber.Present = true;
			track.TrackNumber.Number = field2unsigned (field, 12);
			field += 2;
		}

		// Time Of Last Update
		if (*fspec & FSPEC06_I030_070)
		{
			//field2unsigned (field, 24)
			field += 3;
		}

		// Track Ages
		if (*fspec & FSPEC07_I030_170)
		{
			field += 4;
		}
		
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Calculated Track Position (Cartesian)
		if (*fspec & FSPEC08_I030_100)
		{
			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = field2signed (field, 16) * 1852 / 64.0;
			track.Cartesian->Y = field2signed (field + 2, 16) * 1852 / 64.0;
			field += 4;
		}

		// Calculated Track Velocity (Polar)
		if (*fspec & FSPEC09_I030_180)
		{
			track.GroundSpeed = new RDFTargetGroundSpeed ();
			track.GroundSpeed->Speed = field2unsigned (field, 16) * 1852 / (1<<14);
			track.GroundSpeed->Heading = field2unsigned (field + 2, 16) * (2 * M_PI) / (1<<16);
			field += 4;
		}

		// Calculated Track Velocity (Cartesian)
		if (*fspec & FSPEC10_I030_181 && !track.GroundSpeed)
		{
			double vx = field2signed (field, 16) * 1852 / (1<<14);
			double vy = field2signed (field + 2, 16) * 1852 / (1<<14);
			track.GroundSpeed = new RDFTargetGroundSpeed ();
			rdf_xy2rt (vx, vy, track.GroundSpeed->Speed, track.GroundSpeed->Heading);
			field += 4;
		}

		// Track Mode 3/A
		if (*fspec & FSPEC11_I030_060)
		{
			track.Mode3A = new RDFTargetMode ();
			track.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			track.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
			//track.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			track.Mode3A->Code = field2unsigned (field, 12);
			field += 2;
		}

		// Measured Track Mode C
		if (*fspec & FSPEC12_I030_150)
		{
			field += 2;
		}

		// Calculated Track Altitude
		if (*fspec & FSPEC13_I030_130)
		{
			field += 2;
		}

		// Calculated Track Flight Level
		if (*fspec & FSPEC14_I030_160)
		{
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Artas Track Status
		if (*fspec & FSPEC15_I030_080)
		{
			track.Simulated = (field[0] & 0x80) ? true : false;
			track.Tentative = (field[0] & 0x40) ? true : false;
			track.CST = (field[0] & 0x10) ? true : false;
			track.PSR = (field[0] & 0x04) ? false : true;
			track.SSR = (field[0] & 0x02) ? false : true;

			if (field[0] & 1)
			{
				field += 1;
				track.Cancel = (field[0] & 0x80) ? true : false;
			}

			if (field[0] & 1)
			{
				field += 1;
				track.SPI = (field[0] & 0x08) ? true : false;
			}

			if (field[0] & 1)
			{
				field += 1;
			}

			if (field[0] & 1)
			{
				cms_error ("errorr");
				goto error;
			}
			
			field += 1;
		}

		// Artas Track Quality
		if (*fspec & FSPEC16_I030_090)
		{
			field += 1;
		}

		// Mode Of Flight
		if (*fspec & FSPEC17_I030_200)
		{
			switch ((field[0] >> 2) & 3)
			{
				case 0:
				{
					track.TrackClimb = TRACK_CLIMB_MAINTAINING;
					break;
				}
				case 1:
				{
					track.TrackClimb = TRACK_CLIMB_CLIMBING;
					break;
				}
				case 2:
				{
					track.TrackClimb = TRACK_CLIMB_DESCENDING;
					break;
				}
				case 3:
				{
					track.TrackClimb = TRACK_CLIMB_UNKNOWN;
					break;
				}
			}
			field += 1;
		}

		// Calculated Rate Of Climb/Descent
		if (*fspec & FSPEC18_I030_220)
		{
			track.ClimbSpeed = new RDFTargetClimbSpeed ();
			track.ClimbSpeed->Speed = field2signed (field, 2) * 0.3048 * 5.86 / 60;
			field += 2;
		}

		// Calculated Rate Of Turn
		if (*fspec & FSPEC19_I030_240)
		{
			field += 1;
		}

		// Plot Ages
		if (*fspec & FSPEC20_I030_290)
		{
			field += 2;
		}

		// Radar Identification Tag
		if (*fspec & FSPEC21_I030_260)
		{
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Measured Position
		if (*fspec & FSPEC22_I030_360)
		{
			track.Polar = new RDFCoorPolar ();
			track.Polar->Rho = field2unsigned (field, 16) * 1852 / 128;
			track.Polar->Theta = field2unsigned (field + 2, 16) * 2 * M_PI / 0x10000;
			field += 4;
		}

		// Last Measured Mode C
		if (*fspec & FSPEC23_I030_140)
		{
			track.FlightLevel = new RDFTargetFlightLevel ();
			track.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
			track.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
			track.FlightLevel->Height = (double) field2signed (field, 14) * 25 * 0.3048;
			field += 2;
		}

		// Last Measured Mode 3/A
		if (*fspec & FSPEC24_I030_340)
		{
			field += 2;
		}

		// Reserved Expension Data Field
		if (*fspec & FSPEC25_I030_RE)
		{
			field += field[0];
		}

		// FPPS Identification Tag
		if (*fspec & FSPEC26_I030_390)
		{
			field += 2;
		}

		// Callsign
		if (*fspec & FSPEC27_I030_400)
		{
			if (!track.FlightPlan)
			{
				track.FlightPlan = new RDFTargetFlightPlan ();
			}
			track.FlightPlan->Callsign.assign ((char *) field, 7);
			field += 7;
		}

		// Pln Number
		if (*fspec & FSPEC28_I030_410)
		{
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Departure Airport
		if (*fspec & FSPEC29_I030_440)
		{
			field += 4;
		}

		// Destination Airport
		if (*fspec & FSPEC30_I030_450)
		{
			field += 4;
		}

		// Category Of Turbulence
		if (*fspec & FSPEC31_I030_435)
		{
			field += 1;
		}

		// Type Of Aircraft
		if (*fspec & FSPEC32_I030_430)
		{
			field += 4;
		}

		// Allocated SSR Codes
		if (*fspec & FSPEC33_I030_460)
		{
			field += 1 + field[0] * 2;
		}

		// Current Cleared Flight Level
		if (*fspec & FSPEC34_I030_480)
		{
			field += 2;
		}

		// Flight Category
		if (*fspec & FSPEC35_I030_420)
		{
			field += 1;
		}
		
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Current Control Position
		if (*fspec & FSPEC36_I030_490)
		{
			field += 2;
		}

		// Time Of Message
		if (*fspec & FSPEC37_I030_020)
		{
			track.Time.Present = true;
			track.Time.Time = field2unsigned (field, 24);
			field += 3;
		}

		// Aircraft Address
		if (*fspec & FSPEC38_I030_382)
		{
			track.AircraftAddress.Present = true;
			track.AircraftAddress.Address = field2unsigned (field, 24);
			field += 3;
		}

		// Aircraft Identification
		if (*fspec & FSPEC39_I030_384)
		{
			asterix_decode_callsign (field, track.AircraftIdentification);
			field += 6;
		}

		// Communications Capability And Flight Status
		if (*fspec & FSPEC40_I030_386)
		{
			field += 1;
		}

		// Estimated Accuracy Of Track Position (Cartesian)
		if (*fspec & FSPEC41_I030_110)
		{
			field += 4;
		}

		// Estimated Accuracy Of Track Velocity (Polar)
		if (*fspec & FSPEC42_I030_190)
		{
			field += 4;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Estimated Accuracy Of Track Velocity (Cartesian)
		if (*fspec & FSPEC43_I030_191)
		{
			field += 4;
		}

		// Estimated Accuracy Of Track Altitude
		if (*fspec & FSPEC44_I030_135)
		{
			field += 2;
		}

		// Estimated Accuracy Of Calculated Track Flight Level
		if (*fspec & FSPEC45_I030_165)
		{
			field += 2;
		}

		// Estimated Accuracy Of Rate Of Climb/Descent
		if (*fspec & FSPEC46_I030_230)
		{
			field += 2;
		}

		// Estimated Accuracy Of Rate Of Turn
		if (*fspec & FSPEC47_I030_250)
		{
			field += 1;
		}

		// Mode Of Flight Probabilities
		if (*fspec & FSPEC48_I030_210)
		{
			field += 3;
		}

		// Track Mode 2 Code
		if (*fspec & FSPEC49_I030_120)
		{
			track.Mode2 = new RDFTargetMode ();
			track.Mode2->Valid = (field[0] & 0x80) ? false : true;
			track.Mode2->Garbled = (field[0] & 0x40) ? true : false;
			//track.Mode2->Tracked = (field[0] & 0x20) ? true : false;
			track.Mode2->Code = field2unsigned (field, 12);
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Artas Track Number
		if (*fspec & FSPEC50_I030_050)
		{
			if (field[2] & 1)
			{
				field += 5;
			}
			else
			{
				field += 3;
			}
		}

		// Local Track Number
		if (*fspec & FSPEC51_I030_270)
		{
			field += 2;
		}

		// Measured 3D Height
		if (*fspec & FSPEC52_I030_370)
		{
			track.MeasuredHeight = new RDFTargetHeight ();
			track.MeasuredHeight->Height = field2signed (field, 16) * 25 * 0.3048;
			field += 2;
		}

		// Spare
		if (*fspec & FSPEC53_I030_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Spare
		if (*fspec & FSPEC54_I030_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Spare
		if (*fspec & FSPEC55_I030_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Spare
		if (*fspec & FSPEC56_I030_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
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
		track_list.push_back (track);

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
	track_list.clear ();
	return false;
}

