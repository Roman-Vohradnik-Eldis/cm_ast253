#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I034_010 0x80
#define FSPEC02_I034_000 0x40
#define FSPEC03_I034_030 0x20
#define FSPEC04_I034_020 0x10
#define FSPEC05_I034_041 0x08
#define FSPEC06_I034_050 0x04
#define FSPEC07_I034_060 0x02

#define FSPEC08_I034_070 0x80
#define FSPEC09_I034_100 0x40
#define FSPEC10_I034_110 0x20
#define FSPEC11_I034_120 0x10
#define FSPEC12_I034_090 0x08
#define FSPEC13_I034_RE  0x04
#define FSPEC14_I034_SP  0x02

bool Asterix034_to_RadarService (const void *data, int size,
	deque < RDFRadarService > &service_list)
{
	bool config_nosp = false;
	const char * config;
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int cat;
	int len;

	if ((config = g_getenv ("ASTERIX034")))
	{
		if (!strcmp (config, "NOSP"))
		{
			config_nosp = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX034 value");
		}
	}

	service_list.clear ();

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

	if (cat != 34)
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
		RDFRadarService service;

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

		// data source identifier
		if (*fspec & FSPEC01_I034_010)
		{
			service.SAC = field[0];
			service.SIC = field[1];
			field += 2;
		}

		// message type
		if (*fspec & FSPEC02_I034_000)
		{
			switch (field[0])
			{
				case 1:
				{
					service.Type = RADAR_SERVICE_TYPE_NORTH;
					break;
				}
				case 2:
				{
					service.Type = RADAR_SERVICE_TYPE_SECTOR;
					break;
				}
				case 3:
				{
					service.Type = RADAR_SERVICE_GEO_FILTER;
					break;
				}
				case 4:
				{
					service.Type = RADAR_SERVICE_JAMMING_STROBE;
					break;
				}
				default:
				{
					service.Type = RADAR_SERVICE_TYPE_UNKNOWN;
					break;
				}
			}
			field += 1;
		}

		// time of day
		if (*fspec & FSPEC03_I034_030)
		{
			service.Time.Present = true;
			service.Time.Time =
				((long long) field2unsigned (field, 24) * 1000 + 64) / 128;

			field += 3;
		}

		// sector number
		if (*fspec & FSPEC04_I034_020)
		{
			service.Sector = ((double) field[0] * (2 * M_PI)) / 256.0;
			field += 1;
		}

		// antenna rotation speed
		if (*fspec & FSPEC05_I034_041)
		{
			service.AntennaRotation = new RDFAntennaRotation ();
			service.AntennaRotation->Speed = (double) field2unsigned (field, 16) / 128;
			field += 2;
		}

		/* system configuration and status */
		if (*fspec & FSPEC06_I034_050)
		{
			service.SystemConfigurationAndStatus =
				new RDFSystemConfigurationAndStatus ();
			unsigned char *subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message (%d >= %d)", subfield - msg, len);
					goto error;
				}
			}
			subfield += 1;

			// com
			if (*field & 0x80)
			{
				service.SystemConfigurationAndStatus->COMPresent = true;
				service.SystemConfigurationAndStatus->COM = (char) subfield[0];
				subfield += 1;
			}

			// psr
			if (*field & 0x10)
			{
				service.SystemConfigurationAndStatus->PSRPresent = true;
				service.SystemConfigurationAndStatus->PSR = (char) subfield[0];
				subfield += 1;
			}

			// ssr
			if (*field & 0x08)
			{
				service.SystemConfigurationAndStatus->SSRPresent = true;
				service.SystemConfigurationAndStatus->SSR = (char) subfield[0];
				subfield += 1;
			}
			// mds
			if (*field & 0x04)
			{
				service.SystemConfigurationAndStatus->MDSPresent = true;
				service.SystemConfigurationAndStatus->MDS = (char) subfield[0];
				service.SystemConfigurationAndStatus->MDS_1 = (char) subfield[1];
				subfield += 2;
			}

			// fx
			if (*field & 0x01)
			{
				cms_error ("unble to handle  more subfields");
				goto error;
			}

			field = subfield;
		}

		/* system processing mode */
		if (*fspec & FSPEC07_I034_060)
		{
			service.SystemProcessingMode = new RDFSystemProcessingMode ();

			unsigned char *subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message (%d >= %d)", subfield - msg, len);
					goto error;
				}
			}
			subfield += 1;

			// com
			if (*field & 0x80)
			{
				service.SystemProcessingMode->COMPresent = true;
				service.SystemProcessingMode->COM = *subfield;
				subfield += 1;
			}
			// psr
			if (*field & 0x10)
			{
				service.SystemProcessingMode->PSRPresent = true;
				service.SystemProcessingMode->PSR = *subfield;
				subfield += 1;
			}
			// ssr
			if (*field & 0x08)
			{
				service.SystemProcessingMode->SSRPresent = true;
				service.SystemProcessingMode->SSR = *subfield;
				subfield += 1;
			}
			// mds
			if (*field & 0x04)
			{
				service.SystemProcessingMode->MDSPresent = true;
				service.SystemProcessingMode->MDS = *subfield;
				subfield += 1;
			}
			// fx
			if (*field & 0x01)
			{
				cms_error ("unable to handle more subfields");
				goto error;
			}

			field = subfield;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		/* Message count values - begin */
		if (*fspec & FSPEC08_I034_070)
		{
			int replies;
			replies = field[0];

         if(service.CountValues == NULL)
            service.CountValues = new RDFCountValues();

			service.CountValues->Rep = replies;

			field += 1;

			for (int i = 0; i < replies; i++)
			{
				unsigned int typ, counter;
				typ = field[0] >> 3;
				//tri bity z tech osmi + shift
				counter = field2unsigned (field, 11);
				switch (typ)
				{
					case 0:
					{
						service.CountValues->NoDetection = counter;
						break;
					}
					case 1:
					{
						service.CountValues->PSR = counter;
						break;
					}
					case 2:
					{
						service.CountValues->SSR = counter;
						break;
					}
					case 3:
					{
						service.CountValues->SSR_PSR = counter;
						break;
					}
					case 4:
					{
						service.CountValues->AllCall = counter;
						break;
					}
					case 5:
					{
						service.CountValues->RollCall = counter;
						break;
					}
					case 6:
					{
						service.CountValues->AllCall_PSR = counter;
						break;
					}
					case 7:
					{
						service.CountValues->RollCall_PSR = counter;
						break;
					}
					case 8:
					{
						service.CountValues->FilterWeather = counter;
						break;
					}
					case 9:
					{
						service.CountValues->FilterJamming = counter;
						break;
					}
					case 10:
					{
						service.CountValues->FilterPSR = counter;
						break;
					}
					case 11:
					{
						service.CountValues->FilterSSR = counter;
						break;
					}
					case 12:
					{
						service.CountValues->FilterSSR_PSR = counter;
						break;
					}
					case 13:
					{
						service.CountValues->FilterEnh = counter;
						break;
					}
					case 14:
					{
						service.CountValues->FilterEnhPSR = counter;
						break;
					}
					case 15:
					{
						service.CountValues->FilterNotAPI = counter;
						break;
					}
					case 16:
					{
						service.CountValues->FilterEnhPSR_SSR = counter;
						break;
					}
					default:	//mozna pridat chybove hlaseni
					{
						break;
					}
				}
				field += 2;
			}					/*for */
		}
		/* Message count values - end */

		/* generic polar window */
		if (*fspec & FSPEC09_I034_100)
		{
			//int rho_start, rho_end;
			//double theta_start, theta_end;
			//rho_start = field2unsigned (field, 16) * 1852 / 256;
			//rho_end = field2unsigned (field + 2, 16) * 1852 / 256;
			//theta_start = (double) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000;
			//theta_end = (double) field2unsigned (field + 6, 16) * (2 * M_PI) / 0x10000;
			//cms_info ("polar %d %d %f %f", rho_start, rho_end, theta_start, theta_end);
			field += 8;
		}

		/* data filter */
		if (*fspec & FSPEC10_I034_110)
		{
			service.DataFilter = field[0];
			field += 1;
		}
		
		/* 3d-position of data source */
		if (*fspec & FSPEC11_I034_120)
		{
			service.Position3D = new RDFPosition3D ();
			service.Position3D->Height = field2signed (field, 16);
			service.Position3D->Latitude =
				(double) field2signed (field + 2, 24) * M_PI / 0x800000;
			service.Position3D->Longitude =
				(double) field2signed (field + 5, 24) * M_PI / 0x800000;
			field += 8;
		}

		/* collimation error */
		if (*fspec & FSPEC12_I034_090)
		{

			service.CollimationError = new RDFCollimationError ();
			service.CollimationError->Range = field2signed (field, 8) * 1852 / 128;

			service.CollimationError->Azimuth =
				field2signed (field + 1, 8) * (2 * M_PI) / 0x4000;

			field += 2;
		}

		// reserved expansion field
		if (*fspec & FSPEC13_I034_RE)
		{
			field += field[0];
		}

		// special purpose field
		if (*fspec & FSPEC14_I034_SP)
		{
			if (config_nosp)
			{
				field += field[0];
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
					service.UserText.assign ((char *) subfield + 1, (unsigned) subfield[0]);
					subfield += subfield[0] + 1;
				}

				// skocime na konec SP
				field += field[0];
			}
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
		service_list.push_back (service);

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
	service_list.clear ();
	return false;
}

int RadarService_to_Asterix034 (deque < RDFRadarService > &service_list,
	void *data, int maxsize)
{
	bool config_nosp = false;
	const char * config;
	unsigned char buffer[4096];
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	int fspec_size;
	int field_size;
	int size;

	if ((config = g_getenv ("ASTERIX034")))
	{
		if (!strcmp (config, "NOSP"))
		{
			config_nosp = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX034 value");
		}
	}

	memset (data, 0, maxsize);
	msg = (unsigned char *) data;
	msg[0] = 34;				// kategorie
	msg[1] = 0;					// delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if (service_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (service_list.size () > 0)
	{
		RDFRadarService & service = service_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;

		// data source identifier
		{
			*fspec |= FSPEC01_I034_010;
			field[0] = service.SAC;
			field[1] = service.SIC;
			field += 2;
		}

		// message type
		{
			*fspec |= FSPEC02_I034_000;
			switch (service.Type)
			{
				case RADAR_SERVICE_TYPE_NORTH:
				{
					field[0] = 1;
					break;
				}
				case RADAR_SERVICE_TYPE_SECTOR:
				{
					field[0] = 2;
					break;
				}
				case RADAR_SERVICE_GEO_FILTER:
				{
					field[0] = 3;
					break;
				}
				case RADAR_SERVICE_JAMMING_STROBE:
				{
					field[0] = 4;
					break;
				}
				case RADAR_SERVICE_TYPE_UNKNOWN:
				{
					cms_error ("");
					break;
				}
			}
			field += 1;
		}

		// time of day
		if (service.Time.Present)
		{
			*fspec |= FSPEC03_I034_030;
			int time = RDF_time_to_asterix_time(service.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// sector number
		if (service.Type == RADAR_SERVICE_TYPE_SECTOR)
		{
			*fspec |= FSPEC04_I034_020;
			field[0] = (int) rint (service.Sector * 256 / (2 * M_PI));
			field += 1;
		}

		// antenna rotation speed
		if (service.AntennaRotation)
		{
			*fspec |= FSPEC05_I034_041;
			int speed = (int) rint (service.AntennaRotation->Speed * 128);
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field += 2;
		}

		/* system configuration and status */
		if (service.SystemConfigurationAndStatus)
		{
			*fspec |= FSPEC06_I034_050;
			unsigned char *subfield = field;
			subfield += 1;
			// COM
			if (service.SystemConfigurationAndStatus->COMPresent)
			{
				*field |= 0x80;
				subfield[0] = (unsigned char) service.SystemConfigurationAndStatus->COM;
				subfield += 1;
			}
			// PSR
			if (service.SystemConfigurationAndStatus->PSRPresent)
			{
				*field |= 0x10;
				subfield[0] = (unsigned char) service.SystemConfigurationAndStatus->PSR;
				subfield += 1;
			}
			// SSR
			if (service.SystemConfigurationAndStatus->SSRPresent)
			{
				*field |= 0x08;
				subfield[0] = (unsigned char) service.SystemConfigurationAndStatus->SSR;
				subfield += 1;
			}
			// MDS
			if (service.SystemConfigurationAndStatus->MDSPresent)
			{
				*field |= 0x04;
				subfield[0] = (unsigned char) service.SystemConfigurationAndStatus->MDS;

				subfield[1] = (unsigned char) service.SystemConfigurationAndStatus->MDS_1;

				subfield += 2;
			}

			field = subfield;
		}

		// system processing mode
		if (0)
		{
			*fspec |= FSPEC07_I034_060;
		};

		// konec polozky fspec
		*fspec |= FSPEC_FX;
		fspec++;

		// message count values
		if (0)
		{
			*fspec |= FSPEC08_I034_070;
		}

		// generic polar window
		if (0)
		{
			*fspec |= FSPEC09_I034_100;
		}

		// data filter
		if (service.DataFilter)
		{
			*fspec |= FSPEC10_I034_110;

         field[0] = service.DataFilter;

         field += 1;
		}

		// 3D Position of Data Source
		if (service.Position3D)
		{
			*fspec |= FSPEC11_I034_120;
			field[0] = ((int) service.Position3D->Height >> 8) & 0xff;
			field[1] = (int) service.Position3D->Height & 0xff;
			int latitude = (int) rint (service.Position3D->Latitude * 0x800000 / M_PI);
			field[2] = (latitude >> 16) & 0xff;
			field[3] = (latitude >> 8) & 0xff;
			field[4] = latitude & 0xff;
			int longitude =
				(int) rint (service.Position3D->Longitude * 0x800000 / M_PI);
			field[5] = (longitude >> 16) & 0xff;
			field[6] = (longitude >> 8) & 0xff;
			field[7] = longitude & 0xff;
			field += 8;
		}

		if (!config_nosp)
		{
			unsigned char subbuffer[MSGMAX];
			unsigned char *subfspec = field + 1;
			unsigned char *subfield = subbuffer;
			memset (subbuffer, 0, sizeof (subbuffer));

			// User Text
			if (service.UserText.size ())
			{
				*subfspec |= 0x80;

				subfield[0] = service.UserText.size ();
				subfield += 1;
				memcpy (subfield, service.UserText.data (), service.UserText.size ());
				subfield += service.UserText.size ();
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
					*fspec |= FSPEC14_I034_SP;
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

		*fspec &= ~FSPEC_FX;

		// konec polozky fspec
		fspec++;

		// zkontrolujeme, jestli se polozka vejde do zaznamu
		fspec_size = fspec - record;
		field_size = field - buffer;
		if (size + fspec_size + field_size > maxsize)
		{
			if (size == 3)
			{
				cms_error ("not enough space in output buffer");
				service_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

		// odstranime track z fronty
		service_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}
