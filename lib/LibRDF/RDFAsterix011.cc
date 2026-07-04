#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I011_010 0x80
#define FSPEC02_I011_000 0x40
#define FSPEC03_I011_015 0x20
#define FSPEC04_I011_140 0x10
#define FSPEC05_I011_041 0x08
#define FSPEC06_I011_042 0x04
#define FSPEC07_I011_202 0x02

#define FSPEC08_I011_210 0x80
#define FSPEC09_I011_060 0x40
#define FSPEC10_I011_245 0x20
#define FSPEC11_I011_380 0x10
#define FSPEC12_I011_161 0x08
#define FSPEC13_I011_170 0x04
#define FSPEC14_I011_290 0x02

#define FSPEC15_I011_430 0x80
#define FSPEC16_I011_090 0x40
#define FSPEC17_I011_093 0x20
#define FSPEC18_I011_092 0x10
#define FSPEC19_I011_215 0x08
#define FSPEC20_I011_270 0x04
#define FSPEC21_I011_390 0x02

#define FSPEC22_I011_300 0x80
#define FSPEC23_I011_310 0x40
#define FSPEC24_I011_500 0x20
#define FSPEC25_I011_600 0x10
#define FSPEC26_I011_605 0x08
#define FSPEC27_I011_610 0x04
#define FSPEC28_I011_SP  0x02

#define FSPEC29_I011_RE  0x80
#define FSPEC30_I011_SPA 0x40
#define FSPEC31_I011_SPA 0x20
#define FSPEC32_I011_SPA 0x10
#define FSPEC33_I011_SPA 0x08
#define FSPEC34_I011_SPA 0x04
#define FSPEC35_I011_SPA 0x02

bool Asterix011_to_TargetTrack (const void * data, int size, 
	deque<RDFTargetTrack> & track_list)
{
	bool config_sp4 = false;
	const char * config;
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	if ((config = g_getenv ("ASTERIX011")))
	{
		if (!strcmp (config, "SP4"))
		{
			config_sp4 = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX011 value");
		}
	}
	
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

	if (cat != 11)
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
		track.CAT = 11;
		
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

		// data source identifier (2)
		if (*fspec & FSPEC01_I011_010)
		{
			track.SAC = field[0];
			track.SIC = field[1];
			//cms_info ("SAC %02x SIC %02x", track.SAC, track.SIC);
			field += 2;
		}
		
		// mesage type (1)
		if (*fspec & FSPEC02_I011_000)
		{
			switch (field[0])
			{
				case 1:
				{
					//cms_info ("target report");
					break;
				}
				case 2:
				{
					//cms_info ("manual attachment");
					break;
				}
				case 3:
				{
					//cms_info ("menual detachment");
					break;
				}
				case 4:
				{
					//cms_info ("insertion");
					break;
				}
				case 5:
				{
					//cms_info ("suppression");
					break;
				}
				case 6:
				{
					//cms_info ("modification");
					break;
				}
				case 7:
				{
					//cms_info ("holdbar");
					break;
				}
				default:
				{
					cms_warning ("unknown message type");
				}
			}
			field += 1;
		}

		// service identification (1)
		if (*fspec & FSPEC03_I011_015)
		{
			//cms_info ("service %d", field[0]);
			field += 1;
		}

		// time of track information (3)
		if (*fspec & FSPEC04_I011_140)
		{
			track.Time.Present = true;
			track.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
			//cms_info ("time %d", track.Time);
			field += 3;
		}
		
		// position in WGS-84 coordinates (8)
		if (*fspec & FSPEC05_I011_041)
		{
			//double lat, lon;
			//lat = (double) field2unsigned (field, 32) * M_PI / 0x80000000;
			//lon = (double) field2unsigned (field + 4, 32) * M_PI / 0x80000000;
			//cms_info ("lat %d long %d", lat, lon);
			field += 8;
		}

		// Position in Cartesian Coordinates (4)
		if (*fspec & FSPEC06_I011_042)
		{
			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = field2signed (field, 16);
			track.Cartesian->Y = field2signed (field + 2, 16);
			//cms_info ("x %d y %d", track.Cartesian.X, track.Cartesian.Y);
			// pokud je track na hranici maximalni hodnoty, nebudume ho zobrazovat
			if (track.Cartesian->X == 32767 || track.Cartesian->X == -32768
				|| track.Cartesian->Y == 32767 || track.Cartesian->Y == - 32768)
			{
				track.Cancel = true;
			}
			field += 4;
		}

		// calculated track velocity in cartesian coordinates (4)
		if (*fspec & FSPEC07_I011_202)
		{
			//cms_info ("track velocity");
			//double vx = field2signed (field[1], field[0]) * 0.25;
			//double vy = field2signed (field[3], field[2]) * 0.25;
			field += 4;
		}

		// dalsi fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// calculated acceleration (2)
		if (*fspec & FSPEC08_I011_210)
		{
			//cms_info ("acceleration");
			field += 2;
		}
		
		// mode 3a (2)
		if (*fspec & FSPEC09_I011_060)
		{
			track.Mode3A = new RDFTargetMode ();
			track.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			track.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
			track.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			track.Mode3A->Code = field2unsigned (field, 12);
			//cms_info ("mode 3A %04d", Mode3(track.Mode3A.Code));
			// specialni uprava pro pozemni cile, ktere nemaji odpovidac
			if (track.Mode3A->Code == 0)
			{
				delete track.Mode3A;
				track.Mode3A = NULL;
			}
			field += 2;
		}
		
		// target identification (7)
		if (*fspec & FSPEC10_I011_245)
		{
			asterix_decode_callsign (field + 1, track.AircraftIdentification);
			field += 7;
		}

		// Mode-S
		if (*fspec & FSPEC11_I011_380)
		{
			unsigned char sf1, sf2;

			// primary subfield
			sf1 = field[0];
			if (sf1 & 1)
			{
				sf2 = field[1];
				field += 2;
			}
			else
			{
				sf2 = 0;
				field += 1;
			}

			// subfield #1 Mode S Data (1+n*8)
			if (sf1 & 0x80)
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
			
			// subfield #2 Aircraft Address (3)
			if (sf1 & 0x40)
			{
				track.AircraftAddress.Present = true;
				track.AircraftAddress.Address = field2unsigned (field, 24);
				field += 3;
			}
			// subfiled #3 (0) - never sent

			// subfiled #4 Communications/ACAS Capability and Flight Status (3)
			if (sf1 & 0x10)
			{
				track.CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus ();
				switch ((field[0] >> 5) & 7)
				{
					case 0: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_NONE; break;
					case 1: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B; break;
					case 2: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B_ELMU; break;
					case 3: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B_ELMU_ELMD; break;
					case 4: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_LEVEL_5; break;
					default: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_NONE; break;		// 5 to 7 not assigned
				}
				switch ((field[0] >> 1) & 15)
				{
					case 0: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE; break;
					case 1: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ON_GROUND; break;
					case 2: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE_ALERT; break;
					case 3: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ON_GROUND_ALERT; break;
					case 4: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ALERT_SPI; break;
					case 5: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_SPI; break;
					case 6: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_GENERAL_EMERGENCY; break;
					case 7: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_LIFEGUARD; break;
					case 8: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_MINIMUM_FUEL; break;
					case 9: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_NO_COMMUNICATIONS; break;
					case 10: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_UNLAWFUL_INTERFERENCE; break;
					default: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE; break;		// 11 to 15 not assigned
				}
				track.CommCapabilityAndFlightStatus->ModeSSpecificService = (field[1] & 0x80) ? false : true;
				track.CommCapabilityAndFlightStatus->AltitudeCapability = (field[1] & 0x40) ? true : false;
				track.CommCapabilityAndFlightStatus->IdentificationCapability = (field[1] & 0x20) ? true : false;
				track.CommCapabilityAndFlightStatus->B1A = (field[1] & 0x10) ? true : false;
				track.CommCapabilityAndFlightStatus->B1A = field[1] & 0x0f;
				track.CommCapabilityAndFlightStatus->ACASOperational = (field[2] & 0x80) ? false : true;
				track.CommCapabilityAndFlightStatus->MultipleAids = (field[2] & 0x40) ? true : false;
				track.CommCapabilityAndFlightStatus->DifferentialCorrection = (field[2] & 0x20) ? true : false;
				field += 3;
			}

			// subfiled #5 (0) - never sent
			// subfiled #6 (0) - never sent
			// subfiled #7 (0) - never sent
			
			// subfiled #8 (4)
			if (sf2 & 0x80)
			{
				field += 4;
			}
			
			// subfiled #9 (1)
			if (sf2 & 0x40)
			{
				field += 1;
			}

			// subfield #11 (1)
			if (sf2 & 0x10)
			{
				field += 1;
			}
		}

		// track number (2)
		if (*fspec & FSPEC12_I011_161)
		{
			track.TrackNumber.Present = true;
			track.TrackNumber.Number = field2unsigned (field, 12);
			//cms_info ("track number %d", track.TrackID);
			field += 2;
		}

		// track status 1+
		if (*fspec & FSPEC13_I011_170)
		{
			//cms_info ("track status");
			if (field[0] & 0x01)
			{
				field += 1;
				//cms_info ("status1 %d", field[1]);
				if (field[0] & 0x40) track.Cancel = true;
			}
			if (field[0] & 0x01)
			{
				field += 1;
				//cms_info ("status2 %d", field[1]);
				track.CST = (field[0] & 0x20) ? true : false;
			}
			if (field[0] & 0x01)
			{
				field += 1;
				//cms_info ("status3 %d", field[1]);
			}
			if (field[0] & 0x01)
			{
				cms_error ("");
				goto error;
			}
			field += 1;
		}

		// system track update ages (1+)
		if (*fspec & FSPEC14_I011_290)
		{
			unsigned char sf1, sf2;

			//cms_info ("system track update ages");

			// primary subfield
			sf1 = field[0];
			if (sf1 & 1)
			{
				sf2 = field[1];
				field += 2;
			}
			else
			{
				sf2 = 0;
				field += 1;
			}
			
			// subfield #1 (1)
			if (sf1 & 0x80)
			{
				field += 1;
			}

			// subfield #2 (1)
			if (sf1 & 0x40)
			{
				field += 1;
			}
			
			// subfield #3 (1)
			if (sf1 & 0x20)
			{
				field += 1;
			}
			
			// subfield #4 (1)
			if (sf1 & 0x10)
			{
				field += 1;
			}
			
			// subfield #5 (1)
			if (sf1 & 0x08)
			{
				field += 1;
			}
			
			// subfield #6 (2)
			if (sf1 & 0x04)
			{
				field += 2;
			}

			// subfield #7 (1)
			if (sf1 & 0x02)
			{
				field += 1;
			}

			// subfield #8 (1)
			if (sf2 & 0x80)
			{
				field += 1;
			}
			
			// subfield #9 (1)
			if (sf2 & 0x40)
			{
				field += 1;
			}

			// subfield #10 (1)
			if (sf2 & 0x20)
			{
				field += 1;
			}

			// subfield #11 (1)
			if (sf2 & 0x10)
			{
				field += 1;
			}

			// subfield #12 (1)
			if (sf2 & 0x08)
			{
				field += 1;
			}
		}
		
		// dalsi fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// phase of flight (1)
		if (*fspec & FSPEC15_I011_430)
		{
			//cms_info ("phase of flight");
			field += 1;
		}

		// measured flight level (2)
		if (*fspec & FSPEC16_I011_090)
		{
			track.FlightLevel = new RDFTargetFlightLevel ();
			track.FlightLevel->Valid = true;
			track.FlightLevel->Garbled = false;
			track.FlightLevel->Height = (double) field2signed (field, 14) * 0.3048 * 25;
			//cms_info ("flight level %d", track.FlightLevel.Height);
			field += 2;
		}

		// calculated track barometric altitude (2)
		if (*fspec & FSPEC17_I011_093)
		{
			//cms_info ("barometric altitude");
			field += 2;
		}

		// calculated track geometric descend (2)
		if (*fspec & FSPEC18_I011_092)
		{
			//cms_info ("geometric descend");
			field += 2;
		}
		
		// calculated rate of climb/descent (2)
		if (*fspec & FSPEC19_I011_215)
		{
			track.ClimbSpeed = new RDFTargetClimbSpeed ();
			track.ClimbSpeed->Speed = field2signed (field, 16) * 0.3048 * 6.25 / 60;
			//cms_info ("rate of climb");
			field += 2;
		}

		// target size (1+)
		if (*fspec & FSPEC20_I011_270)
		{
			//cms_info ("target size");
			while ((field[0] & 1) && field < msg + len)
			{
				field += 1;
			}
			field += 1;
		}

		// flight plan related data (1+1+)
		if (*fspec & FSPEC21_I011_390)
		{
			unsigned char sf1, sf2;

			//cms_info ("flight plan related data");
			
			// primary subfield
			sf1 = field[0];
			if (sf1 & 1)
			{
				sf2 = field[1];
				field += 2;
			}
			else
			{
				sf2 = 0;
				field += 1;
			}

			// subfield #1 (2)
			if (sf1 & 0x80)
			{
				field += 2;
			}

			// subfield #2 (7)
			if (sf1 & 0x40)
			{
				field += 7;
			}

			// subfield #3 (4)
			if (sf1 & 0x20)
			{
				field += 4;
			}

			// subfield #4 (1)
			if (sf1 & 0x10)
			{
				field += 1;
			}

			// subfield #5 (4)
			if (sf1 & 0x08)
			{
				field += 4;
			}

			// subfield #6 (1)
			if (sf1 & 0x04)
			{
				field += 1;
			}

			// subfield #7 (4)
			if (sf1 & 0x02)
			{
				field += 4;
			}

			// subfield #8 (4)
			if (sf2 & 0x80)
			{
				field += 4;
			}

			// subfield #9 (3)
			if (sf2 & 0x40)
			{
				field += 3;
			}

			// subfield #10 (2)
			if (sf2 & 0x20)
			{
				field += 2;
			}

			// subfield #11 (2)
			if (sf2 & 0x10)
			{
				field += 2;
			}

			// subfield #12 (1+n*4)
			if (sf2 & 0x08)
			{
				field += 1 + field[0] * 4;
			}

			// subfield #13 (6)
			if (sf2 & 0x04)
			{
				field += 6;
			}

			// subfield #14 (1)
			if (sf2 & 0x02)
			{
				field += 1;
			}
		}

		// dalsi fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// vehicle fleet identification (1)
		if (*fspec & FSPEC22_I011_300)
		{
			//cms_info ("vehicle fleet identification");
			field += 1;
		}

		// pre-programmed message (1)
		if (*fspec & FSPEC23_I011_310)
		{
			//cms_info ("pre-programmed message");
			field += 1;
		}

		// astimated accuracies (1+)
		if (*fspec & FSPEC24_I011_500)
		{
			unsigned char sf = field[0];
			field += 1;

			//cms_info ("astimated accuracies");
			
			// subfield #1 (2)
			if (sf & 0x80)
			{
				field += 2;
			}

			// subfield #2 (4)
			if (sf & 0x40)
			{
				field += 4;
			}

			// subfield #3 (2)
			if (sf & 0x20)
			{
				field += 2;
			}

			// subfield #4 (2)
			if (sf & 0x10)
			{
				field += 2;
			}

			// subfield #5 (1)
			if (sf & 0x08)
			{
				field += 1;
			}

			// subfield #6 (2)
			if (sf & 0x40)
			{
				field += 2;
			}
		}
		
		// alert messages (3)
		if (*fspec & FSPEC25_I011_600)
		{
			field += 3;
		}

		// tracks in alert (1+n*2)
		if (*fspec & FSPEC26_I011_605)
		{
			field += 1 + field[0] * 2;
		}

		// holdbar status (1+2*n)
		if (*fspec & FSPEC27_I011_610)
		{
			field += 1 + field[0] * 2;
		}

		// special purpose field
		if (*fspec & FSPEC28_I011_SP)
		{
			if (config_sp4)
			{
				field += 4;
			}
			else
			{
				field += field[0];
			}
		}

		// dalsi fspec
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// reserved expansion field
		if (*fspec & FSPEC29_I011_RE)
		{
			field += field[0];
			goto error;
		}

		// spare
		if (*fspec & FSPEC30_I011_SPA)
		{
			cms_error ("unble to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC31_I011_SPA)
		{
			cms_error ("unble to handle spare bit");
			goto error;
		}
		
		// spare
		if (*fspec & FSPEC32_I011_SPA)
		{
			cms_error ("unble to handle spare bit");
			goto error;
		}
		
		// spare
		if (*fspec & FSPEC33_I011_SPA)
		{
			cms_error ("unble to handle spare bit");
			goto error;
		}
		
		// spare
		if (*fspec & FSPEC34_I011_SPA)
		{
			cms_error ("unble to handle spare bit");
			goto error;
		}
		
		// spare
		if (*fspec & FSPEC35_I011_SPA)
		{
			cms_error ("unble to handle spare bit");
			goto error;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		cms_error("unable to handle more fspec");
		goto error;

end:
		// zkontrolujeme track number
		if (!track.TrackNumber.Present)
		{
			goto error;
		}
		
		// zkontrolujeme, zda to neni track cancel
		if (!track.Polar && !track.Cartesian)
		{
			track.Cancel = true;
		}
			
		// pridame track do seznamu
		track_list.push_back (track);

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
	track_list.clear ();
	return false;
}
