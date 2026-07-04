#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I002_010 0x80
#define FSPEC02_I002_000 0x40
#define FSPEC03_I002_020 0x20
#define FSPEC04_I002_030 0x10
#define FSPEC05_I002_041 0x08
#define FSPEC06_I002_050 0x04
#define FSPEC07_I002_060 0x02

#define FSPEC08_I002_070 0x80
#define FSPEC09_I002_100 0x40
#define FSPEC10_I002_090 0x20
#define FSPEC11_I002_080 0x10
#define FSPEC12_I002_SPA 0x08
#define FSPEC13_I002_SPA 0x04
#define FSPEC14_I002_SPA 0x02

#define FSPEC15_I002_SPA 0x80
#define FSPEC16_I002_SPA 0x40
#define FSPEC17_I002_SPA 0x20
#define FSPEC18_I002_SPA 0x10
#define FSPEC19_I002_SPA 0x08
#define FSPEC20_I002_SPA 0x04
#define FSPEC21_I002_SPA 0x02

#define FSPEC22_I002_SPA 0x80
#define FSPEC23_I002_INDRA 0x40
#define FSPEC24_I002_SPA 0x20
#define FSPEC25_I002_SPA 0x10
#define FSPEC26_I002_SPA 0x08
#define FSPEC27_I002_SPA 0x04
#define FSPEC28_I002_SPA 0x02

bool Asterix002_to_RadarService (const void * data, int size, 
	deque<RDFRadarService> & service_list)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

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

	if (cat != 2)
	{ 
		cms_error("unknown category %d", cat);
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
		RDFRadarService service;

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
		if (*fspec & FSPEC01_I002_010)
		{
			service.SAC = field[0];
			service.SIC = field[1];
			field += 2;
		}

		// message type
		if (*fspec & FSPEC02_I002_000)
		{
			switch (field[0])
			{
				case 1:
				{
					//cms_info ("north marker message");
					service.Type = RADAR_SERVICE_TYPE_NORTH;
					break;
				}
				case 2:
				{
					//cms_info ("sector crossing message");
					service.Type = RADAR_SERVICE_TYPE_SECTOR;
					break;
				}
				case 3:
				{
					//cms_warning ("south marker message");
					goto error;
				}
				case 8:
				{
					//cms_warning ("activation of blind zone filtering");
					goto error;
				}
				case 9:
				{
					//cms_warning ("stop of blind zone filtering");
					goto error;
				}
				default:
				{
					if (field[0] < 128)
					{
						cms_error ("unknown message type %d", field[0]);
					}
					goto error;
				}
			}
			field += 1;
		}

		// sector number
		if (*fspec & FSPEC03_I002_020)
		{
			service.Sector = (double) field[0] * (2 * M_PI) / 256;
			field += 1;
		}

		// time of day
		if (*fspec & FSPEC04_I002_030)
		{
			service.Time.Present = true;
			service.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
			field += 3;
		}

		// antenna rotation speed
		if (*fspec & FSPEC05_I002_041)
		{
			service.AntennaRotation = new RDFAntennaRotation ();
			service.AntennaRotation->Speed = (double) field2unsigned (field, 16) / 128;
			field += 2;
		}

		// system configuration and status
		if (*fspec & FSPEC06_I002_050)
		{
			if (field[0] & 1)
			{
				field += 1;
			}
			field += 1;
		}

		// system processing mode
		if (*fspec & FSPEC07_I002_060)
		{
			if (field[0] & 1)
			{
				field += 1;
			}
			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// message count values
		if (*fspec & FSPEC08_I002_070)
		{
			unsigned rep;
			rep = field[0];
			//cms_info("rep %d", rep);
			field += 1;
			for (unsigned i = 0; i < rep; i++)
			{
				//int typ, counter;
				//typ = field[0] >> 3;
				//counter = field2unsigned (field, 10);
				field += 2;
			}
		}

		// dynamic window - type 1
		if (*fspec & FSPEC09_I002_100)
		{
			//double rho_start, rho_end;
			//double theta_start, theta_end;
			//rho_start = (double) field2unsigned (field, 16) * 1852 / 256;
			//rho_end = (double) field2unsigned (field + 2, 16) * 1852 / 256;
			//theta_start = (double) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000;
			//theta_end = (double) field2unsigned (field + 6, 16) * (2 * M_PI) / 0x10000;
			//cms_info("polar %d %d %f %f", rho_start, rho_end, theta_start, theta_end);
			field += 8;
		}

		// collimation error
		if (*fspec & FSPEC10_I002_090)
		{
			//double range_error;
			//double azimuth_error;
			//range_error = (double) field[0] * 1852 / 128;
			//azimuth_error = (double) field[1] * (2 * M_PI) / 0x4000;
			//cms_info("error %d %f", range_error, azimuth_error);
			field += 2;
		}

		// warning/error conditions
		if (*fspec & FSPEC11_I002_080)
		{
			if (field[0] & 1)
			{
				field += 1;
			}
			field += 1;
		}

		// spare
		if (*fspec & FSPEC12_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC13_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC14_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// spare
		if (*fspec & FSPEC15_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC16_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC17_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC18_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC19_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC20_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC21_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// spare
		if (*fspec & FSPEC22_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// nestandartni polozka pouzita v systemech INDRA
		if (*fspec & FSPEC23_I002_INDRA)
		{
			field += 2;
		}

		// spare
		if (*fspec & FSPEC24_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC25_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC26_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC27_I002_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC28_I002_SPA)
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

int RadarService_to_Asterix002 (deque<RDFRadarService> & service_list, 
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
	msg[0] = 2; // kategorie
	msg[1] = 0; // delka
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
			*fspec |= FSPEC01_I002_010;
			field[0] = service.SAC;
			field[1] = service.SIC;
			field += 2;
		}

		// message type
		{
			*fspec |= FSPEC02_I002_000;
			switch (service.Type)
			{
				// north marker message
				case RADAR_SERVICE_TYPE_NORTH:
				{
               field[0] = 1;
					break;
				}

				// sector crossing message
				case RADAR_SERVICE_TYPE_SECTOR:
				{
					field[0] = 2;
					break;
				}
            case RADAR_SERVICE_GEO_FILTER:
               field[0] = 3;
            break;
            case RADAR_SERVICE_JAMMING_STROBE:
               field[0] = 4;
            break;

				case RADAR_SERVICE_TYPE_UNKNOWN:
				{
					cms_error ("");
					break;
				}
			}

			field += 1;
		}


		// sector number
		if (service.Type == RADAR_SERVICE_TYPE_SECTOR)
		{
			*fspec |= FSPEC03_I002_020;
			field[0] = (unsigned) rint (service.Sector * 256 / (2 * M_PI));
			field += 1;
		}

		// time of day
		if (service.Time.Present)
		{
			*fspec |= FSPEC04_I002_030;
			unsigned time = RDF_time_to_asterix_time(service.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// antenna rotation speed
		if (service.AntennaRotation)
		{
			*fspec |= FSPEC05_I002_041;
			unsigned speed = (unsigned) rint (service.AntennaRotation->Speed * 128);
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field += 2;
		}

		// system configuration and status
		if (0)
		{
			*fspec |= FSPEC06_I002_050;
		}

		// system processing mode
		if (0)
		{
			*fspec |= FSPEC07_I002_060;
		}

		// konec polozky fspec
		*fspec |= FSPEC_FX;
		fspec++;

		// message count values
		if (0)
		{
			*fspec |= FSPEC08_I002_070;
		}

		// dynamic window - type 1
		if (0)
		{
			*fspec |= FSPEC09_I002_100;
		}

		// collimation error
		if (0)
		{
			*fspec |= FSPEC10_I002_090;
		}

		// warning/error conditions
		if (0)
		{
			*fspec |= FSPEC11_I002_080;
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

