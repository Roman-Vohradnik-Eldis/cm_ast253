#include "RDFAsterix.hh"
#include "RDFAsterix048Private.hh"
#include "CMSDebug.hh"

#include <sstream>

#define FSPEC_FX 0x01

#define FSPEC01_I048_010 0x80
#define FSPEC02_I048_140 0x40
#define FSPEC03_I048_020 0x20
#define FSPEC04_I048_040 0x10
#define FSPEC05_I048_070 0x08
#define FSPEC06_I048_090 0x04
#define FSPEC07_I048_130 0x02

#define FSPEC08_I048_220 0x80
#define FSPEC09_I048_240 0x40
#define FSPEC10_I048_250 0x20
#define FSPEC11_I048_161 0x10
#define FSPEC12_I048_042 0x08
#define FSPEC13_I048_200 0x04
#define FSPEC14_I048_170 0x02

#define FSPEC15_I048_210 0x80
#define FSPEC16_I048_030 0x40
#define FSPEC17_I048_080 0x20
#define FSPEC18_I048_100 0x10
#define FSPEC19_I048_110 0x08
#define FSPEC20_I048_120 0x04
#define FSPEC21_I048_230 0x02

#define FSPEC22_I048_260 0x80
#define FSPEC23_I048_055 0x40
#define FSPEC24_I048_050 0x20
#define FSPEC25_I048_065 0x10
#define FSPEC26_I048_060 0x08
#define FSPEC27_I048_SP  0x04
#define FSPEC28_I048_RE  0x02

/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                       Asterix048_to_Target                                */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/

bool Asterix048_to_Target (const void *data, int size,
	deque < RDFTargetTrack > &track_list, std::stringstream * outstr, bool one_line)
{
	bool config_nosp = false;
	bool config_kuantan = false;
	const char *config;
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int cat;
	int len;

	if ((config = g_getenv ("ASTERIX048")))
	{
		if (!strcmp (config, "NOSP"))
		{
			config_nosp = true;
		}
		else if (!strcmp (config, "KUANTAN"))
		{
			config_kuantan = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX048 value");
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

	if (cat != 48)
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
		track.CAT = 48;

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
		if (*fspec & FSPEC01_I048_010)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_010 (field, outstr, one_line);
			}
			asterix_048_read_010 (track, field);
		}

		// Time of Day
		if (*fspec & FSPEC02_I048_140)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_140 (field, outstr, one_line);
			}
			asterix_048_read_140 (track, field);
		}

		// Target Report Description
		if (*fspec & FSPEC03_I048_020)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_020(field, outstr, one_line);
			}
			if (!asterix_048_read_020 (track, field))
			{
				goto error;
			}
		}

		// Measured Position in Slant Polar Coordinates
		if (*fspec & FSPEC04_I048_040)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_040 (field, outstr, one_line);
			}
			asterix_048_read_040 (track, field);
		}

		// Mode-3/A Code in Octal Representation
		if (*fspec & FSPEC05_I048_070)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_070 (field, outstr, one_line);
			}
			asterix_048_read_070 (track, field);
		}

		// Flight Level in Binary Representation
		if (*fspec & FSPEC06_I048_090)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_090 (field, outstr, one_line);
			}
			asterix_048_read_090 (track, field);
		}

		// Radar Plot Characteristics
		if (*fspec & FSPEC07_I048_130)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_130 (field, outstr, one_line);
			}
			if (!asterix_048_read_130 (track, field))
			{
				goto error;
			}
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// Aircraft Address
		if (*fspec & FSPEC08_I048_220)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_220 (field, outstr, one_line);
			}
			asterix_048_read_220 (track, field);
		}

		// Aircraft Identification
		if (*fspec & FSPEC09_I048_240)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_240 (field, outstr, one_line);
			}
			asterix_048_read_240 (track, field);
		}

		// Mode S MB Data
		if (*fspec & FSPEC10_I048_250)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_250 (field, outstr, one_line);
			}
			asterix_048_read_250 (track, field);
		}

		// Track Number
		if (*fspec & FSPEC11_I048_161)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_161 (field, outstr, one_line);
			}
			asterix_048_read_161 (track, field);
		}

		// Calculated Position in Cartesian Coordinates
		if (*fspec & FSPEC12_I048_042)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_042 (field, outstr, one_line);
			}
			asterix_048_read_042 (track, field);
		}

		// calculated track velocity
		if (*fspec & FSPEC13_I048_200)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_200 (field, outstr, one_line);
			}
			asterix_048_read_200 (track, field);
		}

		// Track Status
		if (*fspec & FSPEC14_I048_170)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_170 (field, outstr, one_line);
			}
			asterix_048_read_170 (track, field);
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// Track Quality
		if (*fspec & FSPEC15_I048_210)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_210 (field, outstr, one_line);
			}
			asterix_048_read_210 (track, field);
		}

		// Warning/Error Conditions
		if (*fspec & FSPEC16_I048_030)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_030 (field, outstr, one_line);
			}
			asterix_048_read_030 (track, field);
		}

		//  Mode-3/A Code Confidence Indicator
		if (*fspec & FSPEC17_I048_080)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_080 (field, outstr, one_line);
			}
			if(!track.Mode3A) {
				field += 2;
				cms_warning("Mode3A is not present");
			} else {
				asterix_048_read_080 (track, field);
			}
		}

		// Mode-C Code and Confidence Indicator
		if (*fspec & FSPEC18_I048_100)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_100 (field, outstr, one_line);
			}
			asterix_048_read_100 (track, field);
		}

		// Height Measured by 3D Radar
		if (*fspec & FSPEC19_I048_110)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_110 (field, outstr, one_line);
			}
			asterix_048_read_110 (track, field);
		}

		// Radial Doppler Speed
		if (*fspec & FSPEC20_I048_120)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_120 (field, outstr, one_line);
			}
			if (!asterix_048_read_120 (track, field))
			{
				goto error;
			}
		}

		// Communications / ACAS Capability and Flight Status
		if (*fspec & FSPEC21_I048_230)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_230 (field, outstr, one_line);
			}
			asterix_048_read_230 (track, field);
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// ACAS Resolution Advisory Report
		if (*fspec & FSPEC22_I048_260)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_260 (field, outstr, one_line);
			}
			asterix_048_read_260 (track, field);
		}


		// Mode-1 Code in Octal Representation
		if (*fspec & FSPEC23_I048_055)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_055 (field, outstr, one_line);
			}
			asterix_048_read_055 (track, field);
		}

		// Mode-2 Code in Octal Representation
		if (*fspec & FSPEC24_I048_050)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_050 (field, outstr, one_line);
			}
			asterix_048_read_050 (track, field);
		}

		// Mode-1 Code Confidence Indicator
		if (*fspec & FSPEC25_I048_065)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_065 (field, outstr, one_line);
			}
			asterix_048_read_065 (track, field);
		}

		// Mode-2 Code Confidence Indicator
		if (*fspec & FSPEC26_I048_060)
		{
			if (outstr != NULL)
			{
				asterix_048_tobuffer_060 (field, outstr, one_line);
			}
			asterix_048_read_060 (track, field);
		}

		if (*fspec & FSPEC27_I048_SP)
		{
			if (config_nosp)
			{
				field += field[0];
			}

			else if (config_kuantan)
			{
				// takhle jsme dekodovali data v kuantanu
				// bylo to blbe :-( tak to radeji nepouzivejte
				unsigned char *subfield = field;
				while (*subfield & 0x01)
				{
					if (++subfield >= msg + len)
					{
						cms_error ("end of message");
						goto error;
					}
				}
				subfield += 1;

				// Smoothed Mode3A Code Octet no. 1 and 2
				if (*field & 0xC0)
				{
					track.SmoothedMode3A = new RDFTargetMode ();
					track.SmoothedMode3A->Valid = (subfield[0] & 0x80) ? false : true;
					track.SmoothedMode3A->Garbled = (subfield[0] & 0x40) ? true : false;
					track.SmoothedMode3A->Tracked = (subfield[0] & 0x20) ? true : false;
					track.SmoothedMode3A->Code = field2unsigned (subfield, 12);
					//INFO ("SmoothedMode3A %d", mode3 (track.SmoothedMode3A.Code));
					subfield += 2;
				}

				field = subfield;
			}

			else
			{
				unsigned char *subfspec = field + 1;
				unsigned char *subfield = subfspec;

				while (*subfield & 1)
				{
					subfield++;
				}
				subfield++;

				// User Text
				if (*subfspec & 0x80)
				{
					if (outstr != NULL)
					{
						asterix_048_tobuffer_user_text (subfield, outstr, one_line);
					}
					asterix_048_read_user_text (track, subfield);
				}

				// Calculated Rate of Climb Descent
				if (*subfspec & 0x40)
				{
					if (outstr != NULL)
					{
						asterix_048_tobuffer_rate_of_climb (subfield, outstr, one_line);
					}
					asterix_048_read_rate_of_climb (track, subfield);
				}

				// Calculated Acceleration
				if (*subfspec & 0x20)
				{
					if (outstr != NULL)
					{
						asterix_048_tobuffer_acceleration (subfield, outstr, one_line);
					}
					asterix_048_read_acceleration (track, subfield);
				}

				// Mode of Movement
				if (*subfspec & 0x10)
				{
					if (outstr != NULL)
					{
						asterix_048_tobuffer_mode_of_movement (subfield, outstr,
							one_line);
					}
					asterix_048_read_mode_of_movement (track, subfield);
				}
                if(*subfspec & 0x8)
                {
                    if(outstr != NULL)
                    {
                        asterix_048_tobuffer_transponder_capability(subfield, outstr,
                                one_line);
                    }
                    asterix_048_read_transponder_capability(track, subfield);
                }


				// Mode S MB Data with Age
				if (*subfspec & 0x04)
				{
					if (outstr != NULL)
					{
						asterix_048_tobuffer_250_with_age (subfield, outstr, one_line);
					}
					asterix_048_read_250_with_age (track, subfield);
				}

				// Target Composition
				if (*subfspec & 0x02)
				{
					if (outstr != NULL)
					{
						asterix_048_tobuffer_target_composition (subfield,
							outstr, one_line);
					}
					asterix_048_read_target_composition (track, subfield);
				}


				// skocime na konec SP
				field += field[0];
			}
		}

		if (*fspec & FSPEC28_I048_RE)
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
		if (outstr)
		{
			*outstr << std::endl;
		}
		/*hexdump */
		//outpu_block(track.out, data, data + size);
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

bool Asterix048_to_TargetTrack (const void *data, int size,
	deque < RDFTargetTrack > &track_list)
{
	deque < RDFTargetTrack > target_list;

	track_list.clear ();

	if (!Asterix048_to_Target (data, size, target_list))
	{
		//cms_info ("cat 48 target failed");
		return false;
	}

	while (!target_list.empty ())
	{
		if (target_list.front ().TrackNumber.Present)
		{
			track_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}

bool Asterix048_to_TargetPlot (const void *data, int size,
	deque < RDFTargetPlot > &plot_list)
{
	deque < RDFTargetTrack > target_list;

	plot_list.clear ();

	if (!Asterix048_to_Target (data, size, target_list))
	{
		//cms_info ("cat 48 target failed");
		return false;
	}

	while (!target_list.empty ())
	{
		if (!target_list.front ().TrackNumber.Present)
		{
			plot_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}

/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                       Target_to_Asterix048                                */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/

int Target_to_Asterix048 (deque < RDFTargetTrack > &track_list, void *data, int maxsize)
{
	bool config_nosp = false;
	bool config_kuantan = false;
	const char *config;
	unsigned char buffer[MSGMAX];
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int fspec_size;
	int field_size;
	int size;

	if ((config = g_getenv ("ASTERIX048")))
	{
		if (!strcmp (config, "NOSP"))
		{
			config_nosp = true;
		}
		else if (!strcmp (config, "KUANTAN"))
		{
			config_kuantan = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX048 value");
		}
	}

	memset (data, 0, maxsize);
	msg = (unsigned char *) data;
	msg[0] = 48;
	msg[1] = 0;
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
		
		// Data Source Identifier
		{
			//INFO ("SIC %d, SAC %d", track.SIC, track.SAC);
			*fspec |= FSPEC01_I048_010;
			asterix_048_write_010 (track, field);
		}

		// Time of Day
		if (track.Time.Present)
		{
			*fspec |= FSPEC02_I048_140;
			asterix_048_write_140 (track, field);
		}

		// Target Report Description
		{
			*fspec |= FSPEC03_I048_020;
			asterix_048_write_020 (track, field);
		}

		// Measured Position in Slant Polar Coordinates
		if (track.Polar)
		{
			*fspec |= FSPEC04_I048_040;
			if (!asterix_048_write_040 (track, field))
			{
				goto skip;
			}
		}

		// Mode-3/A Code in Octal Representation
		if (track.Mode3A)
		{
			*fspec |= FSPEC05_I048_070;
			asterix_048_write_070 (track, field);
		}

		// Flight Level in Binary Representation
		if (track.FlightLevel)
		{
			*fspec |= FSPEC06_I048_090;
			asterix_048_write_090 (track, field);
		}

		// Radar Plot Characteristics
		if (track.TargetCharacteristics)
		{
			*fspec |= FSPEC07_I048_130;
			asterix_048_write_130 (track, field);
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;
		
		// Aircraft Address
		if (track.AircraftAddress.Present)
		{
			*fspec |= FSPEC08_I048_220;
			asterix_048_write_220 (track, field);
		}

		// Aircraft Identification
		if (track.AircraftIdentification.size ())
		{
			*fspec |= FSPEC09_I048_240;
			asterix_048_write_240 (track, field);
		}

		// Mode S MB Data
		if (track.ModeS.size ())
		{
			*fspec |= FSPEC10_I048_250;
			asterix_048_write_250 (track, field);
		}

		// Track Number
		if (track.TrackNumber.Present)
		{
			*fspec |= FSPEC11_I048_161;
			asterix_048_write_161 (track, field);
		}

		// Calculated Position in Cartesian Coordinates
		if (track.Cartesian)
		{
			*fspec |= FSPEC12_I048_042;
			if (!asterix_048_write_042 (track, field))
			{
				goto skip;
			}
		}

		// Calculated Track Velocity in Polar Representation
		if (track.GroundSpeed)
		{
			*fspec |= FSPEC13_I048_200;
			asterix_048_write_200 (track, field);
		}

		// Track Status
		if (track.TrackNumber.Present)
		{
			*fspec |= FSPEC14_I048_170;
			asterix_048_write_170 (track, field);
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Track Quality
		if (track.Accuracy)
		{
			*fspec |= FSPEC15_I048_210;
			asterix_048_write_210 (track, field);
		}

		// Warning/Error Conditions
		if (track.WarningError.size ())
		{
			*fspec |= FSPEC16_I048_030;
			asterix_048_write_030 (track, field);
		}

		//  Mode-3/A Code Confidence Indicator
		if (track.Mode3A && track.Mode3A->ConfidencePresent)
		{
			*fspec |= FSPEC17_I048_080;
			asterix_048_write_080 (track, field);
		}

		// Mode-C Code and Confidence Indicator
		if (track.ModeC)
		{
			*fspec |= FSPEC18_I048_100;
			asterix_048_write_100 (track, field);
		}

		// Height Measured by 3D Radar
		if (track.MeasuredHeight)
		{
			*fspec |= FSPEC19_I048_110;
			asterix_048_write_110 (track, field);
		}

		// Radial Doppler Speed
		if (track.DopplerSpeed.CalculatedPresent || track.DopplerSpeed.RawPresent)
		{
			*fspec |= FSPEC20_I048_120;
			asterix_048_write_120 (track, field);
		}

		// Communications / ACAS Capability and Flight Status
		if (track.CommCapabilityAndFlightStatus)
		{
			*fspec |= FSPEC21_I048_230;
			asterix_048_write_230 (track, field);
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// ACAS Resolution Advisory Report
		if (track.ResolutionAdvisory.Present)
		{
			*fspec |= FSPEC22_I048_260;
			asterix_048_write_260 (track, field);
		}

		// Mode-1 Code in Octal Representation
		if (track.Mode1)
		{
			*fspec |= FSPEC23_I048_055;
			asterix_048_write_055 (track, field);
		}

		// Mode-2 Code in Octal Representation
		if (track.Mode2)
		{
			*fspec |= FSPEC24_I048_050;
			asterix_048_write_050 (track, field);
		}

		// Mode-1 Code Confidence Indicator
		if (track.Mode1 && track.Mode1->ConfidencePresent)
		{
			*fspec |= FSPEC25_I048_065;
			asterix_048_write_065 (track, field);
		}

		// Mode-2 Code Confidence Indicator
		if (track.Mode2 && track.Mode2->ConfidencePresent)
		{
			*fspec |= FSPEC26_I048_060;
			asterix_048_write_060 (track, field);
		}

		if (config_kuantan)
		{
			if (track.SmoothedMode3A)
			{
				*fspec |= FSPEC27_I048_SP;
				unsigned char *subfield = field;
				*subfield = 0;
				field += 1;

				// Smoothed Mode3A Code Octet no. 1 and 2
				if (track.SmoothedMode3A)
				{
					*subfield |= 0xC0;
					field[0] = track.SmoothedMode3A->Valid ? 0 : 0x80;
					field[0] |= track.SmoothedMode3A->Garbled ? 0x40 : 0;
					field[0] |= track.SmoothedMode3A->Tracked ? 0x20 : 0;
					field[0] |= (track.SmoothedMode3A->Code >> 8) & 0x0f;
					field[1] = track.SmoothedMode3A->Code & 0xff;
					field += 2;
				}
			}
		}

		else if (!config_nosp)
		{
			unsigned char subbuffer[MSGMAX];
			unsigned char *subfspec = field + 1;
			unsigned char *subfield = subbuffer;
			memset (subbuffer, 0, sizeof (subbuffer));

			// User Text
			if (track.UserText.size ())
			{
				*subfspec |= 0x80;
				asterix_048_write_user_text (track, subfield);
			}

			// Calculated Rate of Climb Descent
			if (track.ClimbSpeed)
			{
				*subfspec |= 0x40;
				asterix_048_write_rate_of_climb (track, subfield);
			}

			// Calculated Acceleration
			if (track.Acceleration)
			{
				*subfspec |= 0x20;
				asterix_048_write_acceleration (track, subfield);
			}

			// Mode of Movement
			if (track.TransversalAcceleration != TRANS_ACCEL_UNDETERMINED
				|| track.LongitudinalAcceleration != LONG_ACCEL_UNDETERMINED
				|| track.TrackClimb != TRACK_CLIMB_UNKNOWN)
			{
				*subfspec |= 0x10;
				asterix_048_write_mode_of_movement (track, subfield);
			}

            if(track.TransponderCapabilityPresent) {
                *subfspec |= 0x8;
                asterix_048_write_transponder_capability(track, subfield);
            }

			// Mode S MB Data with Age
			if (track.ModeSAge.size () > 0)
			{
				*subfspec |= 0x04;
				asterix_048_write_250_with_age (track, subfield);
			}

			// Target Composition
			if (track.TargetComposition.Mode3A != CODE_SOURCE_NONE
				|| track.TargetComposition.ModeC != CODE_SOURCE_NONE
				|| track.TargetComposition.Position != POSITION_SOURCE_NONE
				|| track.TargetComposition.Detection != DETECTION_SOURCE_NONE)
			{
				*subfspec |= 0x02;
				asterix_048_write_target_composition (track, subfield);
			}


			if (*subfspec)
			{
				long subsize = (long) subfield - (long) subbuffer;
				if (subsize > 253)
				{
					cms_error ("SP is too long (%ld)", subsize);
				}
				else
				{
					memcpy (subfspec + 1, subbuffer, (subfield - subbuffer));
					*fspec |= FSPEC27_I048_SP;
					field[0] = subsize + 2;
					field += subsize + 2;
				}
			}
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

int TargetTrack_to_Asterix048 (deque < RDFTargetTrack > &track_list,
	void *data, int maxsize)
{
	deque < RDFTargetTrack > tmp;

	while (!track_list.empty ())
	{
		if (track_list.front ().TrackNumber.Present)
		{
			tmp.push_back (track_list.front ());
		}
		track_list.pop_front ();
	}

	int send_size = Target_to_Asterix048 (tmp, data, maxsize);

	if (tmp.size ())
	{
		track_list.resize (tmp.size ());
		copy (tmp.begin (), tmp.end (), track_list.begin ());
	}

	return send_size;
}

int TargetPlot_to_Asterix048 (deque < RDFTargetPlot > &plot_list, void *data,
	int maxsize)
{
	deque < RDFTargetTrack > tmp;

	while (!plot_list.empty ())
	{
		tmp.push_back (plot_list.front ());
		plot_list.pop_front ();
	}

	int send_size = Target_to_Asterix048 (tmp, data, maxsize);

	if (tmp.size ())
	{
		plot_list.resize (tmp.size ());
		copy (tmp.begin (), tmp.end (), plot_list.begin ());
	}

	return send_size;
}
