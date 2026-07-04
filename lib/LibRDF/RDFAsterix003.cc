#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I003_010 0x80
#define FSPEC02_I003_070 0x40
#define FSPEC03_I003_020 0x20
#define FSPEC04_I003_120 0x10
#define FSPEC05_I003_050 0x08
#define FSPEC06_I003_080 0x04
#define FSPEC07_I003_150 0x02

#define FSPEC08_I003_140 0x80
#define FSPEC09_I003_130 0x40
#define FSPEC10_I003_160 0x20
#define FSPEC11_I003_040 0x10
#define FSPEC12_I003_170 0x08
#define FSPEC13_I003_180 0x04
#define FSPEC14_I003_090 0x02

#define FSPEC15_I003_SPA 0x80
#define FSPEC16_I003_SPA 0x40
#define FSPEC17_I003_SPA 0x20
#define FSPEC18_I003_SPA 0x10
#define FSPEC19_I003_SPA 0x08
#define FSPEC20_I003_RFS 0x04
#define FSPEC21_I003_SPA 0x02

bool Asterix003_to_TargetTrack (const void * data, int size, 
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

	if (cat != 3)
	{ 
		cms_error ("unknown category %d", cat);
		return false;
	}

	if (len < size)
	{
		cms_warning ("wrong length %d %d", len, size);
		len = size;
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
		track.CAT = 3;

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

		// source identification
		if (*fspec & FSPEC01_I003_010)
		{
			track.SAC = field[0];
			track.SIC = field[1];
			//cms_info ("SIC %d, SAC %d", track.SIC, track.SAC);
			field += 2;
		}

		// track number
		if (*fspec & FSPEC02_I003_070)
		{
			track.TrackNumber.Present = true;
			track.TrackNumber.Number = field2unsigned (field, 16);
			//cms_info ("TRACK %d", track.TrackID);
			field += 2;
		}

		// track position
		if (*fspec & FSPEC03_I003_020)
		{
			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = (double) field2signed (field, 16) * 1852 / 64;
			track.Cartesian->Y = (double) field2signed (field + 2, 16) * 1852 / 64;
			field += 4;
		}
		else
		{
			track.Cancel = true;
		}

		// track speed and heading
		if (*fspec & FSPEC04_I003_120)
		{
			track.GroundSpeed = new RDFTargetGroundSpeed ();
			track.GroundSpeed->Speed = (double) field2signed (field, 16) * 1852 / (1<<14);
			track.GroundSpeed->Heading = (double) field2signed (field + 2, 16) * (2 * M_PI) / (1<<16);
			field += 4;
		}

		// Flight Level
		if (*fspec & FSPEC05_I003_050)
		{
			track.FlightLevel = new RDFTargetFlightLevel ();
			track.FlightLevel->Valid = true;
			track.FlightLevel->Height = (double) field2signed (field, 16) * 25 * 0.3048;
			field += 2;
		}

		// strack status
		if (*fspec & FSPEC06_I003_080)
		{
			track.SSR = (field[0] & 0x08) ? true : false;
			track.PSR = (field[0] & 0x04) ? true : false;

			if (field[0] & 0x01)
			{
				field += 1;
			}

			field += 1;
		}

		// track quality
		if (*fspec & FSPEC07_I003_150)
		{
			field += 1;
		}

		// fx bit
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// rate of climb/descent
		if (*fspec & FSPEC08_I003_140)
		{
			track.ClimbSpeed = new RDFTargetClimbSpeed ();
			track.ClimbSpeed->Speed = (double) field2unsigned (field, 16) * 100 / 0x0400;
			field += 2;
		}

		// attitude/intention indicator
		if (*fspec & FSPEC09_I003_130)
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
					track.TrackClimb = TRACK_CLIMB_DESCENDING;
					if (track.ClimbSpeed)
					{
						track.ClimbSpeed->Speed = -track.ClimbSpeed->Speed;
					}
					break;
				}
				case 2:
				{
					track.TrackClimb = TRACK_CLIMB_CLIMBING;
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

		// callsign
		if (*fspec & FSPEC10_I003_160)
		{
			char callsign[8];
			memcpy (callsign, field, 7);
			callsign[7] = 0;
			for (int i = 6; i > 0; i--)
			{
				if (callsign[i] == 32)
				{
					callsign[i] = 0;
				}
			}
			if (!track.FlightPlan)
			{
				track.FlightPlan = new RDFTargetFlightPlan ();
			}
			track.FlightPlan->Callsign.assign (callsign);
			field += 7;
		}

		// mode 3/A code
		if (*fspec & FSPEC11_I003_040)
		{
			track.Mode3A = new RDFTargetMode ();
			track.Mode3A->Valid = true;
			track.Mode3A->Code = field2unsigned (field, 12);
			field += 2;
		}

		// current controller in charge
		if (*fspec & FSPEC12_I003_170)
		{
			field += 1;
		}

		// current cleared flight level
		if (*fspec & FSPEC13_I003_180)
		{
			//double cfl = (double) field2unsigned (field[1], field[0]) * 100 * 0.3048;
			field += 2;
		}

		// track category
		if (*fspec & FSPEC14_I003_090)
		{
			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// spare
		if (*fspec & FSPEC15_I003_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC16_I003_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC17_I003_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC18_I003_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC19_I003_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// ???
		if (*fspec & FSPEC20_I003_RFS)
		{
			field += 4;
		}

		// spare 
		if (*fspec & FSPEC21_I003_SPA)
		{
			cms_error ("unable to handle spare bit");
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
		// kontrola track number
		if (!track.TrackNumber.Present)
		{
			return false;
		}

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
	cms_data (size, data);
	track_list.clear ();
	return false;
}

int TargetTrack_to_Asterix003 (deque<RDFTargetTrack> & track_list, 
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
	msg[0] = 3; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if (track_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (track_list.size () > 0)
	{
		RDFTargetTrack & track = track_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// source identification
		{
			//cms_info ("SIC %d, SAC %d", track.SIC, track.SAC);
			*fspec |= FSPEC01_I003_010;
			field[0] = track.SAC;
			field[1] = track.SIC;
			field += 2;
		}

		// track number
		if (!track.TrackNumber.Present)
		{
			cms_error ("track number munst be defined");
			return 0;
		}
		else
		{
			//cms_info ("TRACK %d", track.TrackID);
			*fspec |= FSPEC02_I003_070;
			field[0] = (track.TrackNumber.Number >> 8) & 0xff;
			field[1] = track.TrackNumber.Number & 0xff;
			field += 2;
		}

		// track position
		if (track.Cartesian)
		{
			*fspec |= FSPEC03_I003_020;
			long x = lrint (track.Cartesian->X * 64 / 1852);
			long y = lrint (track.Cartesian->Y * 64 / 1852);
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
			field[0] = (x >> 8) & 0xff;
			field[1] = x & 0xff;
			field[2] = (y >> 8) & 0xff;
			field[3] = y & 0xff;
			field += 4;
		}

		// track speed and heading
		if (track.GroundSpeed)
		{
			*fspec |= FSPEC04_I003_120;
			short speed = (short) rint (track.GroundSpeed->Speed * (1<<14) / 1852);
			int heading = (int) rint (track.GroundSpeed->Heading * (1<<16) / (2 * M_PI));
			if (heading > G_MAXSHORT)
			{
				heading -= 0x10000;
			}
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field[2] = (heading >> 8) & 0xff;
			field[3] = heading & 0xff;
			field += 4;
		}

		// Mode C
		if (!track.Cancel && track.FlightLevel)
		{
			*fspec |= FSPEC05_I003_050;
			int fl = (int) rint (track.FlightLevel->Height / 0.3048 / 25);
			field[0] = (fl >> 8) & 0xff;
			field[1] = fl & 0xff;
			field += 2;
		}

		// track status
		if (track.TrackNumber.Present)
		{
			*fspec |= FSPEC06_I003_080;
			field[0] = 0xc0;
			field[0] |= track.SSR ? 0x08 : 0;
			field[0] |= track.PSR ? 0x04 : 0;
			field += 1;
		}

		if (track.Cancel)
		{
			goto end;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// rate of climb/descent
		if (track.ClimbSpeed)
		{
			*fspec |= FSPEC08_I003_140;
			unsigned short speed = (unsigned short) rint (track.ClimbSpeed->Speed * 0x0400 / 100);
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field += 2;
		}

		if (1)
		{
			*fspec |= FSPEC09_I003_130;
			switch (track.TrackClimb)
			{
				case TRACK_CLIMB_MAINTAINING:
				{
					break;
				}
				case TRACK_CLIMB_DESCENDING:
				{
					field[0] = 1 << 2;
					break;
				}
				case TRACK_CLIMB_CLIMBING:
				{
					field[0] = 2 << 2;
					break;
				}
				case TRACK_CLIMB_UNKNOWN:
				{
					field[0] = 3 << 2;
					break;
				}
			}
			field += 1;
		}

		// callsign
		if (track.FlightPlan && track.FlightPlan->Callsign.size ())
		{
			*fspec |= FSPEC10_I003_160;
			memset (field, 0, 7);
			memcpy (field, track.FlightPlan->Callsign.data (), 
				track.FlightPlan->Callsign.size ());
			field += 7;
		}

		// mod 3/A
		if (track.Mode3A)
		{
			*fspec |= FSPEC11_I003_040;
			field[0] = (track.Mode3A->Code >> 8) & 0x0f;
			field[1] = track.Mode3A->Code & 0xff;
			field += 2;
		}

		// current cleared flight level
		/*if (0)
		{
			*fspec |= FSPEC2_I003_180;
			field[0] = (cfl >> 8) & 0xff;
			field[1] = cfl & 0xff;
			field += 2;
		}*/

end:
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
				track_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime track z fronty
		track_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}
