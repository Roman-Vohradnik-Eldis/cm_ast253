#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I001_010 0x80
#define FSPEC02_I001_020 0x40
#define FSPEC03_I001_161 0x20
#define FSPEC04_I001_040 0x10
#define FSPEC05_I001_042 0x08
#define FSPEC06_I001_200 0x04
#define FSPEC07_I001_070 0x02

#define FSPEC08_I001_090 0x80
#define FSPEC09_I001_141 0x40
#define FSPEC10_I001_130 0x20
#define FSPEC11_I001_131 0x10
#define FSPEC12_I001_120 0x08
#define FSPEC13_I001_170 0x04
#define FSPEC14_I001_210 0x02

#define FSPEC15_I001_050 0x80
#define FSPEC16_I001_080 0x40
#define FSPEC17_I001_100 0x20
#define FSPEC18_I001_060 0x10
#define FSPEC19_I001_030 0x08
#define FSPEC20_I001_SP  0x04
#define FSPEC21_I001_RS  0x02

#define FSPEC22_I001_150 0x80
#define FSPEC23_I001_INDRA 0x40
#define FSPEC24_I001_SPA 0x20
#define FSPEC25_I001_SPA 0x10
#define FSPEC26_I001_SPA 0x08
#define FSPEC27_I001_SPA 0x04
#define FSPEC28_I001_SPA 0x02

bool Asterix001_to_TargetTrack (const void * data, int size, 
	deque<RDFTargetTrack> & track_list, int scaling_factor,
   std::stringstream *out_str, bool one_line)
{

	bool config_alenia = false;
	const char * config;
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

//	const char *config;
	int sp_len = -1;
	if ((config = g_getenv ("ASTERIX001_SP_LEN")))
	{
		sp_len = atoi(config);
	}
	int rs_len = -1;
	if ((config = g_getenv ("ASTERIX001_RS_LEN")))
	{
		rs_len = atoi(config);
	}

	if ((config = g_getenv ("ASTERIX001")))
	{
		if (!strcmp (config, "TRAD"))
		{
			config_alenia = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX001 value");
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

	if (cat != 1)
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
		track.CAT = 1;

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

		// data source identifier
		if (*fspec & FSPEC01_I001_010)
		{
         if(out_str) {
             *out_str << "010 [sac=" << static_cast<int>(field[0])
                      << " sic=" << static_cast<int>(field[1]) << "]";

              NEW_LINE(*out_str, 0, '\0');
         }

			track.SAC = field[0];
			track.SIC = field[1];
			//cms_info ("SIC %d, SAC %d", track.SIC, track.SAC);
			field += 2;
		}

		// target report description
		if (*fspec & FSPEC02_I001_020)
		{
         if(out_str) {

            if (field[0] & 0x80)
               *out_str << "020 [TRACK ";
            else
               *out_str << "020 [PLOT ";

            if (field[0] & 0x40)
               *out_str << "SIM";
            else
               *out_str << "ACT";

            switch ((field[0] >> 4) & 3) {
               case 1: *out_str << " PSR"; break;
               case 2: *out_str << " SSR"; break;
               case 3: *out_str << " COMBINED"; break;
               default: *out_str << " NO_DETECTION"; break;
            }

            *out_str << " ANT_" << 1 + ((field[0] >> 3) & 0x1);

            if (field[0] & 0x04)
               *out_str << " SPI";
            else
               *out_str << "  - ";

            if (field[0] & 0x2)
               *out_str << " FIX]";
            else
               *out_str << "  - ]";

            if (field[0] & 1) {
               NEW_LINE(*out_str, 4, ' ');

               if (field[0] & 0x80)
                  *out_str << "[TEST ";
               else
                  *out_str << "[REAL ";

               if (field[0] & 0x10)
                  *out_str << "ME ";
               else
                  *out_str << " - ";

               if (field[0] & 0x8)
                  *out_str << "MI ";
               else
                  *out_str << " - ";

               switch (field[0] & 0x60) {
                  case 0: *out_str << "-]"; break;
                  case 0x20: *out_str << "HIJACK (7500)]"; break;
                  case 0x40: *out_str << "RADIO_FAIL (7600)]"; break;
                  case 0x60: *out_str << "EMERGENCY (7700)]"; break;
               }
            }
            NEW_LINE(*out_str, 0, '\0');
         }

			//cms_info ("target1 %02x", field[0]);

			// kontrola priznaku plot/track
			if (!(field[0] & 0x80))
			{
				return false;
			}

			switch ((field[0] >> 4) & 3)
			{
				case 0:
				{
					track.TargetType = TARGET_TYPE_UNKNOWN;
				}
				case 1:
				{
					track.TargetType = TARGET_TYPE_PRIMARY;
					track.PSR = true;
					break;
				}
				case 2:
				{
					track.TargetType = TARGET_TYPE_SECONDARY;
					track.SSR = true;
					break;
				}
				case 3:
				{
					track.TargetType = TARGET_TYPE_COMBINED;
					track.PSR = true;
					track.SSR = true;
					break;
				}
			}

			track.Simulated = (field[0] & 0x40) ? true : false;
			track.SPI = (field[0] & 0x04) ? true : false;

			if (field[0] & 1)
			{
				field += 1;
				//cms_info ("target2 %02x", field[0]);
				track.Tested = (field[0] & 0x80) ? true : false;
			}

			field += 1;
		}

		// track number
		if (*fspec & FSPEC03_I001_161)
		{
         if(out_str) {
            *out_str << "161 [track=" << field2unsigned (field, 16) << "]";
            NEW_LINE(*out_str, 0, '\0');
         }

			track.TrackNumber.Present = true;
			track.TrackNumber.Number = field2unsigned (field, 16);
			//DEBUG ("trackid %d", track.TrackID);
			field += 2;
		}

		// measured position in polar co-ordinates
		if (*fspec & FSPEC04_I001_040)
		{
         if(out_str) {
            double rho = (double)field2unsigned (field, 16) * 1852 / 128.0;
				double theta = (double) field2unsigned (field + 2, 16) * 360 / 0x10000;

				*out_str << "040 [rho=" << static_cast<int>(rho) <<" m (" 
               << std::setprecision(2) << std::fixed
					<< rho / 1852.0 << " NM) theta=" << theta << " deg]";

            NEW_LINE(*out_str, 0, '\0');
         }

			track.Polar = new RDFCoorPolar ();
			track.Polar->Rho = (double) field2unsigned (field, 16) * 1852 / 128;
			track.Polar->Theta = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
			//DEBUG ("rho %d, theta %f", track.Rho, track.Theta);
			field += 4;
		}

		// calculated position in cartesian coordinates
		if (*fspec & FSPEC05_I001_042)
		{
         if(out_str) {

         	double x = (double) field2signed(field, 16) / 64;
				double y = (double) field2signed(field + 2, 16) / 64;

				*out_str << "042 [x=" << rint (x * 1852) << " m (" << x << " NM) y="
					<< rint (y * 1852) << " m (" << y << " NM)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = (double) field2signed (field, 16) 
				* 1852 * exp2 (scaling_factor - 6);
			track.Cartesian->Y = (double) field2signed (field + 2, 16) 
				* 1852 * exp2 (scaling_factor - 6);
			//DEBUG ("x %d, y %d", track.X, track.Y);
			field += 4;
		}

		// calculated track velocity in polar coordinates
		if (*fspec & FSPEC06_I001_200)
		{
         if(out_str) {
            double speed = (double) field2unsigned (field, 16) / 0x4000;
				double heading = (double) field2unsigned (field + 2, 16) * 360 / 0x10000;

				*out_str << "200 [speed=" << speed * 1852 << " m/s (" << speed * 3600
					<< " kt) heading=" << heading << " deg]";

            NEW_LINE(*out_str, 0, '\0');
         }

			track.GroundSpeed = new RDFTargetGroundSpeed ();
			track.GroundSpeed->Speed = (double) field2unsigned (field, 16) * 1852 / (1<<14);
			track.GroundSpeed->Heading = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / (1<<16);
			//DEBUG ("speed %d, heading %d", track.GroundSpeed, track.Heading);
			field += 4;
		}

		// mode 3/A code in octal representation
		if (*fspec & FSPEC07_I001_070)
		{
         if(out_str) {
     			if (field[0] & 0x80)
					*out_str << "070 [INVALID";
				else
					*out_str << "070 [VALID";

				if (field[0] & 0x40)
					*out_str << " GARBLED";
				else
					*out_str << " -";

				if (field[0] & 0x20)
					*out_str << " TRACKED";
				else
					*out_str << " - ";

				*out_str << " code_oct=" << std::oct << std::setw(4) << 
               std::fixed << std::setfill('0') << field2unsigned (field, 12)
					<< std::dec << std::noshowbase << "]";

         }

			track.Mode3A = new RDFTargetMode ();
			track.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			track.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
			track.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			track.Mode3A->Code = field2unsigned (field, 12);
			//DEBUG ("mode3a");
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// flight level
		if (*fspec & FSPEC08_I001_090)
		{
         if(out_str) {

				if (field[0] & 0x80)
					*out_str << "090 [INVALID";
				else
					*out_str << "090 [VALID";

				if (field[0] & 0x40)
					*out_str << " GARBLED";
				else
					*out_str << " - ";

				double fl = field2signed (field, 14) / 4.0;
				*out_str << " fl=" << std::setprecision (2) << fl << " ("
					<< std::setprecision (3) << fl * 100 * 0.3048 << " m)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			track.FlightLevel = new RDFTargetFlightLevel ();
			track.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
			track.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
			track.FlightLevel->Height = (double) field2signed (field, 14) * 25 * 0.3048;
			//DEBUG ("flight level %d", track.FlightLevel.Height);
			field += 2;
		}

		// trancated time of day
		if (*fspec & FSPEC09_I001_141)
		{
         if(out_str) {
			   *out_str << "141 [time=" << field2unsigned (field, 16) / 128.0
					<< " sec]";

            NEW_LINE(*out_str, 0, '0');
         }


			track.Time.Present = true;
			track.Time.Time = (field2unsigned (field, 16) * 1000 + 64) / 128;
			//cms_info ("time %d", time);
			field += 2;
		}

		// radar track charactreistics
		if (*fspec & FSPEC10_I001_130)
		{
         if(out_str) {
         	*out_str << "130 [hex=";
         }

         uint16_t pos = 0;

			while (field[0] & 1) {
            pos++;

            if(out_str) {
               *out_str << std::hex << std::setw(2) << std::fixed 
                  << static_cast<int>(field[0] >> 1);

               if(pos % 10 == 0) {
                  *out_str << "]";
                  NEW_LINE(*out_str, 4, ' ');
               } else 
                  *out_str << " "; 
            }

				field += 1;
			}

         if(out_str) {
            if(pos % 10 != 0) {
               *out_str << "]";
               NEW_LINE(*out_str, 0, '\0');
            }
         }

			field += 1;

      }

		// received power 
		if (*fspec & FSPEC11_I001_131)
		{
         
         if(out_str) {
            *out_str << "131 [power=" << field2signed (field, 8) << " dBm]";
            NEW_LINE(*out_str, 0, '\0');
         }

			//cms_info ("received power");
			field += 1;
		}

		// measured radial doppler speed
		if (*fspec & FSPEC12_I001_120)
		{
         
         if(out_str) {
            double speed = (double) field2signed (field, 8) / 256;
				*out_str << "120 [speed" << speed * 1852 << " m/s ("
					<< speed * 3600 << " kt)]";

            NEW_LINE(*out_str, 0, '\0');
         }


			//cms_info ("measured radial doppler speed");
			field += 1;
		}

		// track status
		if (*fspec & FSPEC13_I001_170)
      {
         if(out_str) {

            *out_str << "170 [";
            if (field[0] & 0x80)
               *out_str << "TENTATIVE";
            else
               *out_str << "CONFIRMED";

            if (field[0] & 0x40)
               *out_str << " SSR/Combined";
            else
               *out_str << " PSR";

            if (field[0] & 0x20)
               *out_str << " MAN]";
            else
               *out_str << " - ]";

            NEW_LINE(*out_str, 4, ' ');

            if (field[0] & 0x10)
               *out_str << "[DOU";
            else
               *out_str << "[ - ";

            *out_str << " RDPC_" << 1 + ((field[0] >> 3) & 0x1);

            if (field[0] & 0x02)
               *out_str << " GHOST";
            else
               *out_str << " - ";

            if (field[0] & 0x01) {

               if (field[1] & 0x80)
                  *out_str << " CANCEL";
               else
                  *out_str << " ALIVE";
            }

            *out_str << "]";
            NEW_LINE(*out_str, 0, '0');
         }
			//cms_info ("track status %02x", field[0]);
			track.Tentative = (field[0] & 0x80) ? true : false;
			track.PSR = (field[0] & 0x40) ? false : true;
			track.PSR = (field[0] & 0x40) ? true : false;

			if (field[0] & 1)
			{
				//cms_info ("track status 2");
				track.Cancel = (field[0] & 0x80) ? true : false;
				field += 1;
			}

			field += 1;
		}

		// track quality
		if (*fspec & FSPEC14_I001_210)
		{
         if(out_str) {
            if(field[0] & 0x1) {
               *out_str << "210 [quality= " << static_cast<int>(field[0] >> 1) 
                  << "]";

               NEW_LINE(*out_str, 0, '\0');
            }
         }
			//cms_info ("track quality");
			if (field[0] & 1)
			{
				//cms_info ("track quality 2");
				field += 1;
			}
			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// mode-2 code in octal representation
		if (*fspec & FSPEC15_I001_050)
		{
			track.Mode2 = new RDFTargetMode ();
			track.Mode2->Valid = (field[0] & 0x80) ? false : true;
			track.Mode2->Garbled = (field[0] & 0x40) ? true : false;
			track.Mode2->Tracked = (field[0] & 0x20) ? true : false;
			track.Mode2->Code = field2unsigned (field, 12);
			//cms_info ("mode2 %d %d", track.Mode2Validated, mode3 (track.Mode2));
			field += 2;
		}

		// mode 3/A code dconfidence indicator
		if (*fspec & FSPEC16_I001_080 && track.Mode3A)
		{
			track.Mode3A->ConfidenceCode = field2unsigned (field, 12);
			//cms_info ("mode3 %d", mode3);
			field += 2;
		}

		// mode C code and confidence indicator
		if (*fspec & FSPEC17_I001_100)
		{
			track.ModeC = new RDFTargetMode ();
			track.ModeC->Valid = (field[0] & 0x80) ? false : true;
			track.ModeC->Garbled = (field[0] & 0x40) ? true : false;
			track.ModeC->Code = field2unsigned (field, 12);
			track.ModeC->ConfidenceCode = field2unsigned (field + 2, 12);
			//cms_info ("modec %d qxi %d", track.ModeC.Code, track.ModeC.ConfidenceCode);
			field += 4;
		}

		// mode-2 code confidence indicator
		if (*fspec & FSPEC18_I001_060 && track.Mode2)
		{
			track.Mode2->ConfidenceCode = field2unsigned (field, 12);
			field += 2;
		}

		// warning/error conditions
		if (*fspec & FSPEC19_I001_030)
		{
			if (field[0] & 1)
			{
				field += 1;
			}
			field += 1;
		}

		// SP
		if (*fspec & FSPEC20_I001_SP)
		{
			if (config_alenia)
			{
				unsigned char * subfspec = field + 1;
				unsigned char * subfield = subfspec;
				while (*subfield & 1)
				{
					subfield++;
				}
				subfield++;

				// track speed stereo components
				if (*subfspec & 0x80)
				{
					double vx, vy;
					signed short int xy;
					xy = subfield[1]<<8;
					xy += subfield[0];
//					vx = (double) field2unsigned (subfield, 15) * 1852 * 100 / (1<<22);
					vx = (double) xy * 1852. * 100. / 4194304.;

					xy = subfield[3]<<8;
					xy += subfield[2];
					vy = (double) xy * 1852 * 100 / (1<<22);
					track.GroundSpeed = new RDFTargetGroundSpeed ();
					rdf_xy2rt (vx, vy, track.GroundSpeed->Speed, track.GroundSpeed->Heading);
					subfield += 4;
				}

				// track weights
				if (*subfspec & 0x40)
				{
					subfield += 4;
				}

				// acceleration components
				if (*subfspec & 0x20)
				{
					subfield += 2;
				}

				// extrapolated height
				if (*subfspec & 0x10)
				{
					subfield += 2;
				}

				// rate of climb/descent
				if (*subfspec & 0x08)
				{
					track.ClimbSpeed = new RDFTargetClimbSpeed ();
					track.ClimbSpeed->Speed = ((char) field[0]) * 0.3048;
					if (track.ClimbSpeed->Speed > 0)
					{
//						track.TrackClimb = TRACK_CLIMB_CLIMBING;
					}
					else if (track.ClimbSpeed->Speed < 0)
					{
//						track.TrackClimb = TRACK_CLIMB_DESCENDING;
					}
					else
					{
//						track.TrackClimb = TRACK_CLIMB_MAINTAINING;
					}
					subfield += 1;
				}

				// track characteristics
				if (*subfspec & 0x04)
				{
					if ((*subfield & 0x06) == 0)
					{
						track.TrackClimb = TRACK_CLIMB_MAINTAINING;
					}
					else if ((*subfield & 0x06) == 0x02)
					{
						track.TrackClimb = TRACK_CLIMB_CLIMBING;
					}
					else if ((*subfield & 0x06) == 0x04)
					{
						track.TrackClimb = TRACK_CLIMB_DESCENDING;
					}
					while (*subfield & 1)
					{
						subfield++;
					}
					subfield++;
				}

				// ununsed
				if (*subfspec & 0x02)
				{
					cms_error ("unable to handle bit");
					goto error;
				}

				// field extension indicator
				if (*subfspec & 0x01)
				{
					cms_error ("unable to handle bit");
					goto error;
				}

				field += field[0];
				if (field != subfield)
				{
					cms_error ("wrong sp length");
					goto error;
				}
			}

			else
			{
				if (sp_len==-1) field += field[0]; else field += sp_len;
			}
		}

		// RS
		if (*fspec & FSPEC21_I001_RS)
		{
			if (rs_len==-1)
			{
				cms_error ("unable to handle spare bit");
				goto error;
			} else field += rs_len;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// presence of x-pulse
		if (*fspec & FSPEC22_I001_150)
		{
			field += 1;
		}

		// nestandartni polozka pouzita v systemech INDRA
		if (*fspec & FSPEC23_I001_INDRA)
		{
			field += 2;
		}

		// spare
		if (*fspec & FSPEC24_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC25_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC26_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC27_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC28_I001_SPA)
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
	cms_data (size, msg);
	track_list.clear ();
	return false;
}

int TargetTrack_to_Asterix001 (deque<RDFTargetTrack> & track_list, 
	void * data, int maxsize, int scaling_factor)
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
	msg[0] = 1; // kategorie
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

		// data source identifier
		{
			*fspec |= FSPEC01_I001_010;
			field[0] = track.SAC;
			field[1] = track.SIC;
			field += 2;
		}

		// target report description
		{
			*fspec |= FSPEC02_I001_020;
			field[0] = 0x80;

			switch (track.TargetType)
			{
				case TARGET_TYPE_UNKNOWN:
  				case TARGET_TYPE_ADSB: // chova se jako neznamy
				{
					field[0] |= 0 << 4;
					break;
				}
				case TARGET_TYPE_PRIMARY:
				{
					field[0] |= 1 << 4;
					break;
				}
				case TARGET_TYPE_SECONDARY:
				case TARGET_TYPE_MODES_ALL_CALL:
				case TARGET_TYPE_MODES_ROLL_CALL:
				{
					field[0] |= 2 << 4;
					break;
				}
				case TARGET_TYPE_COMBINED:
				case TARGET_TYPE_MODES_ALL_CALL_PSR:
				case TARGET_TYPE_MODES_ROLL_CALL_PSR:
				{
					field[0] |= 3 << 4;
					break;
				}
			}

			if (track.Simulated)
			{
				field[0] |= 0x40;
			}
 			if (track.SPI)
 			{
 				field[0] |= 0x04;
 			}

		 	field[1] = track.Tested ? 0x80 : 0;

			if (field[1])
			{
				field [0] |= 0x01;
				field += 2;
			}
			else
			{
				field += 1;
			}
		}

		// track number
		if (!track.TrackNumber.Present)
		{
			cms_error ("track number must be defined");
		}
		else
		{
			*fspec |= FSPEC03_I001_161;
			field[0] = (track.TrackNumber.Number >> 8) & 0xff;
			field[1] = track.TrackNumber.Number & 0xff;
			field += 2;
		}

		// measured position in polar co-ordinates
		if (track.Polar)
		{
			*fspec |= FSPEC04_I001_040;
			long rho = lrint (track.Polar->Rho * 128 / 1852);
			long theta = lrint (track.Polar->Theta * 0x10000 / (2 * M_PI));
			if (rho < 0 || rho > USHRT_MAX)
			{
				cms_warning ("rho out of limit");
				goto skip;
			}
			field[0] = (rho >> 8) & 0xff;
			field[1] = rho & 0xff;
			field[2] = (theta >> 8) & 0xff;
			field[3] = theta & 0xff;
			field += 4;
		}

		// calculated position in cartesian coordinates
		if (track.Cartesian)
		{
			*fspec |= FSPEC05_I001_042;
			long x = lrint (track.Cartesian->X / exp2 (scaling_factor - 6) / 1852);
			long y = lrint (track.Cartesian->Y / exp2 (scaling_factor - 6) / 1852);
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

		// calculated track velocity in polar coordinates
		if (track.GroundSpeed)
		{
			*fspec |= FSPEC06_I001_200;
			unsigned speed = (unsigned) rint (track.GroundSpeed->Speed * (1<<14) / 1852);
			unsigned heading = (unsigned) rint (track.GroundSpeed->Heading * (1<<16) / (2 * M_PI));
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field[2] = (heading >> 8) & 0xff;
			field[3] = heading & 0xff;
			field += 4;
		}

		// mode 3/A code in octal representation
		if (track.Mode3A)
		{
			*fspec |= FSPEC07_I001_070;
			field[0] = track.Mode3A->Valid ? 0 : 0x80;
			field[0] |= track.Mode3A->Garbled ? 0x40 : 0;
			field[0] |= track.Mode3A->Tracked ? 0x20 : 0;
			field[0] |= (track.Mode3A->Code >> 8) & 0x0f;
			field[1] = track.Mode3A->Code & 0xff;
			field += 2;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// flight level
		if (track.FlightLevel)
		{
			*fspec |= FSPEC08_I001_090;
			int height = (int) rint (track.FlightLevel->Height / 0.3048 / 25);
			field[0] = track.FlightLevel->Valid ? 0 : 0x80;
			field[0] |= track.FlightLevel->Garbled ? 0x40 : 0;
			field[0] |= (height >> 8) & 0x3f;
			field[1] = height & 0xff;
			field += 2;
		}

		// trancated time of day
		if (track.Time.Present)
		{
			*fspec |= FSPEC09_I001_141;
			unsigned time = RDF_time_to_asterix_time(track.Time.Time);
			field[0] = (time >> 8) & 0xff;
			field[1] = time & 0xff;
			field += 2;
		}

		// radar track charactreistics
		if (0)
		{
			*fspec |= FSPEC10_I001_130;
		}

		// received power 
		if (0)
		{
			*fspec |= FSPEC11_I001_131;
		}

		// measured radial doppler speed
		if (0)
		{
			*fspec |= FSPEC12_I001_120;
		}

		// track status
		if (track.TrackNumber.Present)
		{
			*fspec |= FSPEC13_I001_170;
			field[0] = track.Tentative ? 0x80 : 0;
			field[0] |=  track.SSR ? 0x40 : 0;
			field[1] = track.Cancel ? 0x80 : 0;

			if (field[1])
			{
				field[0] |= 0x01;
				field += 1;
			}

			field += 1;
		}

		// track quality
		if (0)
		{
			*fspec |= FSPEC14_I001_210;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// mode-2 code in octal representation
		if (track.Mode2)
		{
			*fspec |= FSPEC15_I001_050;
			field[0] = track.Mode2->Valid ? 0 : 0x80;
			field[0] |= track.Mode2->Garbled ? 0x40 : 0;
			field[0] |= track.Mode2->Tracked ? 0x20 : 0;
			field[0] |= (track.Mode2->Code >> 8) & 0x0f;
			field[1] = track.Mode2->Code & 0xff;
			field += 2;
		}

		// mode 3/A code dconfidence indicator
		if (track.Mode3A && track.Mode3A->ConfidenceCode)
		{
			*fspec |= FSPEC16_I001_080;
			field[0] = (track.Mode3A->ConfidenceCode >> 8) & 0x0f;
			field[1] = track.Mode3A->ConfidenceCode & 0xff;
			field += 2;
		}

		// mode C code and confidence indicator
		if (track.ModeC)
		{
			*fspec |= FSPEC17_I001_100;
			field[0] = track.ModeC->Valid ? 0 : 0x80;
			field[0] |= track.ModeC->Garbled ? 0x40 : 0;
			field[0] |= (track.ModeC->Code >> 8) & 0x0f;
			field[1] = track.ModeC->Code & 0xff;
			field[2] = (track.ModeC->ConfidenceCode >> 8) & 0x0f;
			field[3] = track.ModeC->ConfidenceCode & 0xff;
			field += 4;
		}

		// mode-2 code confidence indicator
		if (track.Mode2 && track.Mode2->ConfidenceCode)
		{
			*fspec |= FSPEC18_I001_060;
			field[0] = (track.Mode2->ConfidenceCode >> 8) & 0xf;
			field[1] = track.Mode2->ConfidenceCode & 0xff;
			field += 2;
		}

		// warning/error conditions
		if (0)
		{
			*fspec |= FSPEC22_I001_150;
		}

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

