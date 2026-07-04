#include "RDFAsterix.hh"
#include "CMSDebug.hh"
#include <sstream>
#include <iomanip>

enum CAT012_EDITION
{
	CAT012_EDITION_1_00,
	CAT012_EDITION_1_01,
	CAT012_EDITION_1_02,
	CAT012_EDITION_1_03,
	CAT012_EDITION_1_04,
	CAT012_EDITION_1_05,
	CAT012_EDITION_1_06,
	CAT012_EDITION_1_07,
	CAT012_EDITION_1_08,
};

#define FSPEC_FX 0x01

#define FSPEC01_I012_010 0x80
#define FSPEC02_I012_000 0x40
#define FSPEC03_I012_020 0x20
#define FSPEC04_I012_140 0x10
#define FSPEC05_I012_040 0x08
#define FSPEC06_I012_043 0x04
#define FSPEC07_I012_SPA 0x02

#define FSPEC08_I012_131 0x80
#define FSPEC09_I012_132 0x40
#define FSPEC10_I012_030 0x20
#define FSPEC11_I012_161 0x10
#define FSPEC12_I012_044 0x08
#define FSPEC13_I012_045 0x04
#define FSPEC14_I012_170 0x02

#define FSPEC15_I012_200 0x80
#define FSPEC16_I012_201 0x40
#define FSPEC17_I012_SPA 0x20
#define FSPEC18_I012_SPA 0x10
#define FSPEC19_I012_SPA 0x08
#define FSPEC20_I012_SP  0x04
#define FSPEC21_I012_RE  0x02

bool __Asterix012_to_ParTarget (const void *data, int size,
	enum CAT012_EDITION edition, deque < RDFParTarget > &target_list)
{
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int cat;
	int len;

	target_list.clear ();

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

	if (cat != 12)
	{
		cms_error ("unknown category %d", cat);
		goto error;
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
		RDFParTarget target;

		// najdeme zacatek datovych poli
		fspec = field = record;
		while (*field & FSPEC_FX)
		{
			if (++field >= msg + len)
			{
				cms_error ("end of message (%d >= %d)", field - msg, len);
				goto error;
			}
		}

		field++;

		// Data Source Identifier
		if (*fspec & FSPEC01_I012_010)
		{
			target.SAC = field[0];
			target.SIC = field[1];
			field += 2;
		}

		// Messsage Type
		if (*fspec & FSPEC02_I012_000)
		{
			// starsi revize byly v rozporu s ICD, kvuli starsim nahravkam
			// sem davam moznost prijmout korektne i starsi verzi
			if (field[0] == 3)
			{
				target.MessageType = PAR_TARGET_MESSAGE_TYPE_AZIMUTHAL_TARGET;
			}
			else if (field[0] == 4)
			{
				target.MessageType = PAR_TARGET_MESSAGE_TYPE_ELEVATION_TARGET;
			}
			else
			{
				target.MessageType = field[0];
			}
			field += 1;
		}

		// Target Report Descriptor
		if (*fspec & FSPEC03_I012_020)
		{
			target.Simulated = (field[0] & 0x80) ? true : false;
			target.Tested = (field[0] & 0x40) ? true : false;
			target.ParDetection = (field[0] & 0x20) ? true : false;
			target.LOP = (field[0] >> 3) & 0x3;
			target.TOT = (field[0] >> 1) & 0x3;

			if (field[0] & 1)
			{
				cms_error ("unable to handle FX");
				goto end;
			}

			field += 1;
		}

		// Time of Day
		if (*fspec & FSPEC04_I012_140)
		{
			target.Time.Present = true;
			target.Time.Time = ((long long) field2unsigned (field, 24)
				* 1000 + 64) / 128;
			field += 3;
		}

		// Measured Azimuthal Position in Slant Polar Coordinates
		if (*fspec & FSPEC05_I012_040)
		{
			target.MeasuredAzimuthalPositionPresent = true;
			target.MeasuredAzimuthalPosition.Rho = field2unsigned (field, 16);
			target.MeasuredAzimuthalPosition.Theta =
				(double) field2signed (field + 2, 16) * M_PI / 0x8000;
			field += 4;
		}

		// Measured Elevation Position in Slant Polar Coordinates
		if (*fspec & FSPEC06_I012_043)
		{
			target.MeasuredElevationPositionPresent = true;
			target.MeasuredElevationPosition.Rho = field2unsigned (field, 16);
			target.MeasuredElevationPosition.Theta =
				(double) field2signed (field + 2, 16) * M_PI / 0x8000;
			field += 4;
		}

		if (*fspec & FSPEC07_I012_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// Radar Plot Characteristics in Azimuthal Antenna
		if (*fspec & FSPEC08_I012_131)
		{
			if (edition == CAT012_EDITION_1_00)
			{
				// Amplitude of Primary Plot (PAM)
				target.PamAzimuthPresent = true;
				target.PamAzimuth = (char) field[0];
				field += 1;
			}
			else
			{
				unsigned char * subfspec = field;
				field += 1;

				// Amplitude of Primary Plot (PAM)
				if (*subfspec & 0x80)
				{
					target.PamAzimuthPresent = true;
					target.PamAzimuth = (char) field[0];
					field += 1;
				}

				// Primary Plot Runlength (PRL)
				if (*subfspec & 0x40)
				{
					target.PrlAzimuthPresent = true;
					target.PrlAzimuth = field[0];
					field += 1;
				}

				if (*subfspec & 0x3F)
				{
					cms_error ("unable to handle more subfspec bits");
					return false;
				}
			}
		}

		// Radar Plot Characteristics in Elevation Antenna
		if (*fspec & FSPEC09_I012_132)
		{
			if (edition == CAT012_EDITION_1_00)
			{
				// Amplitude of Primary Plot (PAM)
				target.PamElevationPresent = true;
				target.PamElevation = (char) field[0];
				field += 1;
			}
			else
			{
				unsigned char * subfspec = field;
				field += 1;

				// Amplitude of Primary Plot (PAM)
				if (*subfspec & 0x80)
				{
					target.PamElevationPresent = true;
					target.PamElevation = (char) field[0];
					field += 1;
				}

				// Primary Plot Runlength (PRL)
				if (*subfspec & 0x40)
				{
					target.PrlElevationPresent = true;
					target.PrlElevation = field[0];
					field += 1;
				}

				if (*subfspec & 0x3F)
				{
					cms_error ("unable to handle more subfspec bits");
					return false;
				}
			}
		}

		// Warning/Error Conditions
		if (*fspec & FSPEC10_I012_030)
		{
			while (field < msg + len)
			{
				unsigned char we = field[0] >> 1;
				target.WarningError.push_back (we);
				if (!(field[0] & 0x01))
				{
					break;
				}
				field += 1;
			}
			field += 1;
		}

		// Track Number
		if (*fspec & FSPEC11_I012_161)
		{
			target.TrackNumber.Present = true;
			target.TrackNumber.Number = field2unsigned (field, 16);
			field += 2;
		}

		// Calculated Azimuthal Position in Slant Polar Coordinates
		if (*fspec & FSPEC12_I012_044)
		{
			target.CalculatedAzimuthalPositionPresent = true;
			target.CalculatedAzimuthalPosition.Rho = field2unsigned (field, 16);
			target.CalculatedAzimuthalPosition.Theta =
				(double) field2signed (field + 2, 16) * M_PI / 0x8000;
			field += 4;
		}

		// Calculated Elevation Position in Slant Polar Coordinates
		if (*fspec & FSPEC13_I012_045)
		{
			target.CalculatedElevationPositionPresent = true;
			target.CalculatedElevationPosition.Rho = field2unsigned (field, 16);
			target.CalculatedElevationPosition.Theta =
				(double) field2signed (field + 2, 16) * M_PI / 0x8000;
			field += 4;
		}

		// Track Status
		if (*fspec & FSPEC14_I012_170)
		{
			target.Tentative = field[0] & 0x80 ? true : false;
			target.Cancel = field[0] & 0x40 ? true : false;
			target.Ghost = field[0] & 0x20 ? true : false;
			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// Calculated Azimuthal Track Velocity in Slant Polar Coordinates
		if (*fspec & FSPEC15_I012_200)
		{
			target.AzimuthalVelocityPresent = true;
			target.AzimuthalVelocity.Speed =
				(double) field2unsigned (field, 16) * 1852 / (1 << 14);
			target.AzimuthalVelocity.Heading =
				(double) field2unsigned (field + 2, 16) * (2 * M_PI) / (1 << 16);
			field += 4;
		}

		// Calculated Elevation Track Velocity in Slant Polar Coordinates
		if (*fspec & FSPEC16_I012_201)
		{
			target.ElevationVelocityPresent = true;
			target.ElevationVelocity.Speed =
				(double) field2unsigned (field, 16) * 1852 / (1 << 14);
			target.ElevationVelocity.Heading =
				(double) field2unsigned (field + 2, 16) * (2 * M_PI) / (1 << 16);
			field += 4;
		}

		if (*fspec & FSPEC17_I012_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC18_I012_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC19_I012_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Special Purpose Field
		if (*fspec & FSPEC20_I012_SP)
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
				target.UserText.assign ((char *) subfield + 1, (unsigned) subfield[0]);
				subfield += subfield[0] + 1;
			}

			// skocime na konec SP
			field += field[0];
		}

		// Reserved Expansion Field
		if (*fspec & FSPEC21_I012_RE)
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
		target_list.push_back (target);

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error ("wrong packet length %d > %d", record - msg, size);
		goto error;
	}

	return true;

error:
	cms_data (size, msg);
	target_list.clear ();
	return false;
}

bool Asterix012_to_ParTarget (const void *data, int size,
	deque < RDFParTarget > &target_list, 
	std::stringstream * out_str, bool one_line)
{
	enum CAT012_EDITION edition = CAT012_EDITION_1_08;
	unsigned char sac, sic;
	const char * config;
	char text[64];

	if (!AsterixGetSACSIC (data, size, 12, sac, sic))
	{
		return false;
	}

	sprintf (text, "ASTERIX012_%u_%u", sac, sic);
	if ((config = g_getenv (text)))
	{
		if (!strcmp (config, "1.0"))
		{
			edition = CAT012_EDITION_1_00;
		}
		else if (!strcmp (config, "1.1")
			|| !strcmp (config, "1.2")
			|| !strcmp (config, "1.3")
			|| !strcmp (config, "1.4")
			|| !strcmp (config, "1.5")
			|| !strcmp (config, "1.6")
			|| !strcmp (config, "1.7")
			|| !strcmp (config, "1.8"))
		{
			edition = CAT012_EDITION_1_08;
		}
		else
		{
			cms_warning ("unknown %s value %s", text, config);
		}
	}

	if (!__Asterix012_to_ParTarget (data, size, edition, target_list))
	{
		return false;
	}	

	return true;
}

int __ParTarget_to_Asterix012 (deque < RDFParTarget > &target_list,
	enum CAT012_EDITION edition, void *data, int maxsize)
{
	unsigned char buffer[4096];
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int fspec_size;
	int field_size;
	int size;

	memset (data, 0, maxsize);
	msg = (unsigned char *) data;
	msg[0] = 12;				// kategorie
	msg[1] = 0;					// delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if (target_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (target_list.size () > 0)
	{
		RDFParTarget & target = target_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;
		
		// Data Source Identifier
		{
			*fspec |= FSPEC01_I012_010;
			field[0] = target.SAC;
			field[1] = target.SIC;
			field += 2;
		}

		// Messsage Type
		{
			*fspec |= FSPEC02_I012_000;
			field[0] = target.MessageType;
			field += 1;
		}

		// Target Report Descriptor
		{
			*fspec |= FSPEC03_I012_020;
			field[0] = target.Simulated ? 0x80 : 0;
			field[0] |= target.Tested ? 0x40 : 0;
			field[0] |= target.ParDetection ? 0x20 : 0;
			field[0] |= (target.LOP & 0x3) << 3;
			field[0] |= (target.TOT & 0x3) << 1;
			field += 1;
		}

		// Time of Day
		if (target.Time.Present)
		{
			*fspec |= FSPEC04_I012_140;
			int time = RDF_time_to_asterix_time(target.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Measured Azimuthal Position in Slant Polar Coordinates
		if (target.MeasuredAzimuthalPositionPresent)
		{
			*fspec |= FSPEC05_I012_040;
			long rho = lrint (target.MeasuredAzimuthalPosition.Rho);
			long theta = lrint (target.MeasuredAzimuthalPosition.Theta * 0x8000 / M_PI);
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

		// Measured Elevation Position in Slant Polar Coordinates
		if (target.MeasuredElevationPositionPresent)
		{
			*fspec |= FSPEC06_I012_043;
			long rho = lrint (target.MeasuredElevationPosition.Rho);
			long theta = lrint (target.MeasuredElevationPosition.Theta * 0x8000 / M_PI);
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

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Amplitude of Primary Target Detected by Azimuthal Antenna
		if (edition == CAT012_EDITION_1_00)
		{
			if (target.PamAzimuthPresent)
			{
				*fspec |= FSPEC08_I012_131;
				field[0] = (unsigned char) target.PamAzimuth;
				field += 1;
			}
		}
		else
		{
			if (target.PamAzimuthPresent
				|| target.PrlAzimuthPresent)
			{
				*fspec |= FSPEC08_I012_131;
				unsigned char *subfspec = field;
				*subfspec = 0;
				field += 1;

				if (target.PamAzimuthPresent)
				{
					*subfspec |= 0x80;
					field[0] = (unsigned char) target.PamAzimuth;
					field += 1;
				}

				if (target.PrlAzimuthPresent)
				{
					*subfspec |= 0x40;
					field[0] = target.PrlAzimuth;
					field += 1;
				}
			}
		}

		// Amplitude of Primary Target Detected by Elevation Antenna
		if (edition == CAT012_EDITION_1_00)
		{
			if (target.PamElevationPresent)
			{
				*fspec |= FSPEC09_I012_132;
				field[0] = (unsigned char) target.PamElevation;
				field += 1;
			}
		}
		else
		{
			if (target.PamElevationPresent
				|| target.PrlElevationPresent)
			{
				*fspec |= FSPEC09_I012_132;
				unsigned char *subfspec = field;
				*subfspec = 0;
				field += 1;

				if (target.PamElevationPresent)
				{
					*subfspec |= 0x80;
					field[0] = (unsigned char) target.PamElevation;
					field += 1;
				}

				if (target.PrlElevationPresent)
				{
					*subfspec |= 0x40;
					field[0] = target.PrlElevation;
					field += 1;
				}
			}
		}

		// Warning/Error Conditions
		if (target.WarningError.size ())
		{
			*fspec |= FSPEC10_I012_030;
			for (unsigned i = 0; i < target.WarningError.size (); i++)
			{
				field[i] = target.WarningError[i];
			}
			field += target.WarningError.size ();
		}

		// Track Number
		if (target.TrackNumber.Present)
		{
			*fspec |= FSPEC11_I012_161;
			field[0] = (target.TrackNumber.Number >> 8) & 0xff;
			field[1] = target.TrackNumber.Number & 0xff;
			field += 2;
		}

		// Calculated Azimuthal Track Velocity in Slant Polar Coordinates
		if (target.CalculatedAzimuthalPositionPresent)
		{
			*fspec |= FSPEC12_I012_044;
			long rho = lrint (target.CalculatedAzimuthalPosition.Rho);
			long theta = lrint (target.CalculatedAzimuthalPosition.Theta * 0x8000 / M_PI);
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

		// Calculated Elevation Position in Slant Polar Coordinates
		if (target.CalculatedElevationPositionPresent)
		{
			*fspec |= FSPEC13_I012_045;
			long rho = lrint (target.CalculatedElevationPosition.Rho);
			long theta = lrint (target.CalculatedElevationPosition.Theta * 0x8000 / M_PI);
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

		// Track Status
		if (target.TrackNumber.Present)
		{
			*fspec |= FSPEC14_I012_170;
			field[0] |= target.Tentative ? 0x80 : 0;
			field[0] |= target.Cancel ? 0x40 : 0;
			field[0] |= target.Ghost ? 0x20 : 0;
			field += 1;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Calculated Azimuthal Track Velocity in Slant Polar Coordinates
		if (target.AzimuthalVelocityPresent)
		{
			*fspec |= FSPEC15_I012_200;
			unsigned short speed =
				(unsigned short) (target.AzimuthalVelocity.Speed * (1 << 14) / 1852);
			unsigned short heading =
				(unsigned short) (target.AzimuthalVelocity.Heading * (1 << 16) / (2 *
					M_PI));
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field[2] = (heading >> 8) & 0xff;
			field[3] = heading & 0xff;
			field += 4;
		}

		// Calculated Elevation Track Velocity in Slant Polar Coordinates
		if (target.ElevationVelocityPresent)
		{
			*fspec |= FSPEC16_I012_201;
			unsigned short speed =
				(unsigned short) (target.ElevationVelocity.Speed * (1 << 14) / 1852);
			unsigned short heading =
				(unsigned short) (target.ElevationVelocity.Heading * (1 << 16) / (2 *
					M_PI));
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field[2] = (heading >> 8) & 0xff;
			field[3] = heading & 0xff;
			field += 4;
		}

		// Special Purpose Field
		{
			unsigned char subbuffer[MSGMAX];
			unsigned char *subfspec = field + 1;
			unsigned char *subfield = subbuffer;
			memset (subbuffer, 0, sizeof (subbuffer));

			// User Text
			if (target.UserText.size ())
			{
				*subfspec |= 0x80;
				subfield[0] = target.UserText.size ();
				subfield += 1;
				memcpy (subfield, target.UserText.data (), target.UserText.size ());
				subfield += target.UserText.size ();
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
					*fspec |= FSPEC20_I012_SP;
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
				target_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime track z fronty
		target_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}

int ParTarget_to_Asterix012 (deque<RDFParTarget> & par_list,
	void * data, int maxsize)
{
	enum CAT012_EDITION edition = CAT012_EDITION_1_08;
	unsigned char sac, sic;
	const char * config;
	char text[64];

	if (par_list.empty ())
	{
		return 0;
	}

	sac = par_list.front ().SAC;
	sic = par_list.front ().SIC;

	sprintf (text, "ASTERIX012_%u_%u", sac, sic);

	if ((config = g_getenv (text)))
	{
    if (!strcmp (config, "1.0"))
    {
      edition = CAT012_EDITION_1_00;
    }
    else if (!strcmp (config, "1.1")
      || !strcmp (config, "1.2")
      || !strcmp (config, "1.3")
      || !strcmp (config, "1.4")
      || !strcmp (config, "1.5")
      || !strcmp (config, "1.6")
      || !strcmp (config, "1.7")
      || !strcmp (config, "1.8"))
    {
      edition = CAT012_EDITION_1_08;
    }
		else
		{
			cms_warning ("unknown %s value %s", text, config);
		}
	}

	return (__ParTarget_to_Asterix012 (par_list, edition, data, maxsize));
}

