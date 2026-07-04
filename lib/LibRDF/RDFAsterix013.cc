#include "RDFAsterix.hh"
#include "CMSDebug.hh"
#include "RDFHeader.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I013_010 0x80
#define FSPEC02_I013_000 0x40
#define FSPEC03_I013_030 0x20
#define FSPEC04_I013_020 0x10
#define FSPEC05_I013_040 0x08
#define FSPEC06_I013_041 0x04
#define FSPEC07_I013_042 0x02

#define FSPEC08_I013_050 0x80
#define FSPEC09_I013_120 0x40
#define FSPEC10_I013_043 0x20
#define FSPEC11_I013_044 0x10
#define FSPEC12_I013_045 0x08
#define FSPEC13_I013_046 0x04
#define FSPEC14_I013_047 0x02

#define FSPEC15_I013_SPA 0x80
#define FSPEC16_I013_SPA 0x40
#define FSPEC17_I013_SPA 0x20
#define FSPEC18_I013_SPA 0x10
#define FSPEC19_I013_SPA 0x08
#define FSPEC20_I013_RE  0x04
#define FSPEC21_I013_SP  0x02

bool __Asterix013_to_ParService (const void *data, int size,
	deque < RDFParService > &service_list)
{
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
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

	if (cat != 13)
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
		RDFParService service;

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
		if (*fspec & FSPEC01_I013_010)
		{
			service.SAC = field[0];
			service.SIC = field[1];
			field += 2;
		}

		// Message Type
		if (*fspec & FSPEC02_I013_000)
		{
			service.MessageType = (PAR_SERVICE_MESSAGE_TYPE) field[0];
			field += 1;
		}

		// Time of Day
		if (*fspec & FSPEC03_I013_030)
		{
			service.Time.Present = true;
			service.Time.Time = ((long long) field2unsigned (field, 24)
				* 1000 + 64) / 128;
			field += 3;
		}

		// Sector Number
		if (*fspec & FSPEC04_I013_020)
		{
			service.SectorAnglePresent = true;
			service.SectorAngle = (float) (char) field[0] * M_PI / 256;
			field += 1;
		}

		// Azimuthal/Elevation Angle
		if (*fspec & FSPEC05_I013_040)
		{
			service.AzimuthalElevationAnglePresent = true;
			service.AzimuthalElevationAngle = (float) field2unsigned (field, 16)
				* M_TWOPI / (1 << 16);
			field += 2;
		}

		// Antenna Scanning Period
		if (*fspec & FSPEC06_I013_041)
		{
			service.AntennaScanningPeriodPresent = true;
			service.AntennaScanningPeriod = (field2unsigned (field, 16)
				* 1000 + 64) / 128;
			field += 2;
		}

		// Antenna Pedestal Direction
		if (*fspec & FSPEC07_I013_042)
		{
			service.AntennaPedestalDirectionPresent = true;
			service.AntennaPedestalDirection =
				(float) field2unsigned (field, 16) * M_TWOPI / (1 << 16);
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// System Configuration & Status
		if (*fspec & FSPEC08_I013_050)
		{
			service.ConfigurationStatusPresent = true;

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

			// #1: COM
			if (*field & 0x80)
			{
				service.ConfigurationStatus.COMPresent = true;
				service.ConfigurationStatus.COM.NOGO = (subfield[0] >> 7) & 1;
				service.ConfigurationStatus.COM.RDPC = (subfield[0] >> 6) & 1;
				service.ConfigurationStatus.COM.RDPR = (subfield[0] >> 5) & 1;
				service.ConfigurationStatus.COM.OVLRDP = (subfield[0] >> 4) & 1;
				service.ConfigurationStatus.COM.OVLXMT = (subfield[0] >> 3) & 1;
				service.ConfigurationStatus.COM.MSC = (subfield[0] >> 2) & 1;
				service.ConfigurationStatus.COM.TSV = (subfield[0] >> 1) & 1;
				subfield += 1;
			}

			if (*field & 0x40)
			{
				cms_error ("unable to handle spare bit");
				goto error;
			}

			// 3: WND
			if (*field & 0x20)
			{
				service.ConfigurationStatus.WNDPresent = true;
				service.ConfigurationStatus.WND.SPEED_10s = (float) field2unsigned (subfield, 16) * 200 / (1 << 16);
				service.ConfigurationStatus.WND.DIRECTION_10s = (float) field2unsigned (subfield + 2, 16) * M_TWOPI / (1 << 16);
				subfield += 4;
				service.ConfigurationStatus.WND.SPEED_1min = (float) field2unsigned (subfield, 16) * 200 / (1 << 16);
				service.ConfigurationStatus.WND.DIRECTION_1min = (float) field2unsigned (subfield + 2, 16) *  M_TWOPI / (1 << 16);
				subfield += 4;
			}

			// PAR
			if (*field & 0x10)
			{
				service.ConfigurationStatus.PARPresent = true;
				service.ConfigurationStatus.PAR.ANT = (subfield[0] >> 7) & 1;
				service.ConfigurationStatus.PAR.CHAB = (subfield[0] >> 5) & 3;
				service.ConfigurationStatus.PAR.OVL = (subfield[0] >> 4) & 1;
				service.ConfigurationStatus.PAR.MRC = (subfield[0] >> 3) & 1;
				service.ConfigurationStatus.PAR.ARR = (subfield[0] >> 2) & 1;
				service.ConfigurationStatus.PAR.OPR = (subfield[0] >> 1) & 1;
				service.ConfigurationStatus.PAR.CAL = subfield[0] & 1;
				subfield += 1;
			}

			// CTR
			if (*field & 0x08)
			{
				service.ConfigurationStatus.CTRPresent = true;
				service.ConfigurationStatus.CTR.RNG = (subfield[0] >> 7) & 1;
				service.ConfigurationStatus.CTR.FREQ = (subfield[0] >> 6) & 1;
        service.ConfigurationStatus.CTR.PSDM = (subfield[0] >> 5) & 1;
				subfield += 1;
			}

			// POL
			if (*field & 0x04)
			{
				service.ConfigurationStatus.POLPresent = true;
        service.ConfigurationStatus.POL.CRA = (subfield[0] >> 7) & 1;
				service.ConfigurationStatus.POL.CLA = (subfield[0] >> 6) & 1;
				service.ConfigurationStatus.POL.CRE = (subfield[0] >> 5) & 1;
        service.ConfigurationStatus.POL.CLE = (subfield[0] >> 4) & 1;
				service.ConfigurationStatus.POL.LVA = (subfield[0] >> 3) & 1;
				service.ConfigurationStatus.POL.LHA = (subfield[0] >> 2) & 1;
				service.ConfigurationStatus.POL.LVE = (subfield[0] >> 1) & 1;
				service.ConfigurationStatus.POL.LHE = (subfield[0] >> 0) & 1;
				subfield += 1;
			}

			// PSV
			if (*field & 0x02)
			{
				service.ConfigurationStatus.PSVPresent = true;

			  service.ConfigurationStatus.PSV.AngleElevation = (float) field2signed (subfield, 16)
				  * M_PI_2 / (1 << 15);
			  subfield += 2;

        service.ConfigurationStatus.PSV.AngleAzimuthal = (float) field2signed (subfield, 16)
				  * M_PI_2 / (1 << 15);
			  subfield += 2;

        service.ConfigurationStatus.PSV.AspectRatioElevation = (float) field2unsigned (subfield, 16);
			  subfield += 2;

        service.ConfigurationStatus.PSV.AspectRatioAzimuthal = (float) field2unsigned (subfield, 16);
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

		// 3D-Position of Data Source
		if (*fspec & FSPEC09_I013_120)
		{
			service.Position3DPresent = true;
			service.Position3D.Height = (double) field2signed (field, 16);
			service.Position3D.Latitude =
				(double) field2signed (field + 2, 24) * M_PI / (1 << 23);
			service.Position3D.Longitude =
				(double) field2signed (field + 5, 24) * M_PI / (1 << 23);
			field += 8;
		}

		// Azimuthal Antenna Tilt
		if (*fspec & FSPEC10_I013_043)
		{
			service.AzimuthalAntennaTiltPresent = true;
			service.AzimuthalAntennaTilt = (double) field2signed (field, 16)
				* M_PI_2 / (1 << 15);
			field += 2;
		}

		// Elevation Antenna Tilt
		if (*fspec & FSPEC11_I013_044)
		{
			service.ElevationAntennaTiltPresent = true;
			service.ElevationAntennaTilt.X = (float) field2signed (field, 16)
				* M_PI_2 / (1 << 15);
			service.ElevationAntennaTilt.Y = (float) field2signed (field + 2, 16)
				* M_PI_2 / (1 << 15);
			field += 4;
		}

		// Elevation Antenna Correction
		if (*fspec & FSPEC12_I013_045)
		{
			service.ElevationCorrectionPresent = true;
			service.ElevationCorrection = (float) field2signed (field, 16)
				* M_PI_2 / (1 << 15);
			field += 2;
		}

		// Runway in Use
		if (*fspec & FSPEC13_I013_046)
		{
			service.RunwayInUsePresent = true;

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

			// Subfield #1: ID
			// Runway Identification
			if (*field & 0x80)
			{
				service.RunwayInUse.IDPresent = true;
				asterix_decode_callsign (subfield, service.RunwayInUse.ID);
				subfield += 6;
			}

			// Subfield #2: RWD
			// Runway direction of the antenna pedestal
			if (*field & 0x40)
			{
				service.RunwayInUse.RWDPresent = true;
				service.RunwayInUse.RWD = (float) field2signed (subfield, 16)  // TODO unsigned ???
					* M_TWOPI / (1 << 16);
				subfield += 2;
			}

			// Subfield #3: TDD
			// Touch Down Point distance from center of Azimuth Antenna
			if (*field & 0x20)
			{
				service.RunwayInUse.TDDPresent = true;
				service.RunwayInUse.TDD = (float) field2unsigned (subfield, 16);
				subfield += 2;
			}

			// Subfield #4: LDR
			// Lateral distance from center of Azimuth Antenna to center of Runway
			if (*field & 0x10)
			{
				service.RunwayInUse.LDRPresent = true;
				service.RunwayInUse.LDR = (float) field2unsigned (subfield, 16);
				subfield += 2;
			}

			// Subfield #5: TDE
			// Elevation of the Touch Down Point with the center of Elevation Antenna
			if (*field & 0x08)
			{
				service.RunwayInUse.TDEPresent = true;
				service.RunwayInUse.TDE = (float) field2signed (subfield, 16) / 100.;
				subfield += 2;
			}

			if (*field & 0x04)
			{
				ERROR ("librdf", "unable to handle spare bit");
				throw fspec;
			}

			// Subfield #7: RWN
			// Runway Number from RFGEN
			if (*field & 0x02)
			{
				service.RunwayInUse.RWNPresent = true;
				service.RunwayInUse.RWN = subfield[0];
				subfield += 1;
			}

			if (*field & FSPEC_FX)
			{
				ERROR ("librdf", "unable to handle more extents");
				throw fspec;
			}
		    field = subfield;
		}

		// Radar Base Tilt
    if (*fspec & FSPEC14_I013_047)
    {
      service.RadarBaseTiltPresent = true;
      service.RadarBaseTilt.X = (float) field2signed (field, 16)
        * M_PI_2 / (1 << 15);
      service.RadarBaseTilt.Y = (float) field2signed (field + 2, 16)
        * M_PI_2 / (1 << 15);
      field += 4;
    }

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		if (*fspec & FSPEC15_I013_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC16_I013_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC17_I013_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC18_I013_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC19_I013_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Reserved Expansion Field
		if (*fspec & FSPEC20_I013_RE)
		{
			field += field[0];
		}

		// Special Purpose Field
		if (*fspec & FSPEC21_I013_SP)
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

bool Asterix013_to_ParService (const void * data, int size,
	deque<RDFParService> & service_list, std::stringstream * out_str,
	bool one_line)
{
	return __Asterix013_to_ParService (data, size, service_list);
}

int ParService_to_Asterix013 (deque < RDFParService > &service_list,
	void *data, int maxsize)
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
	msg[0] = 13;				// kategorie
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
		RDFParService & service = service_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// Data Source Identifier
		{
			*fspec |= FSPEC01_I013_010;
			field[0] = service.SAC;
			field[1] = service.SIC;
			field += 2;
		}

		// Message Type
		{
			*fspec |= FSPEC02_I013_000;
			field[0] = service.MessageType;
			field += 1;
		}

		// Time of Day
		if (service.Time.Present)
		{
			*fspec |= FSPEC03_I013_030;
			int time = RDF_time_to_asterix_time(service.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Sector Number
		if (service.SectorAnglePresent)
		{
			*fspec |= FSPEC04_I013_020;
			field[0] = lrint (service.SectorAngle * 256 / M_PI);
			field += 1;
		}

		// Azimuthal/Elevation Angle
		if (service.AzimuthalElevationAnglePresent)
		{
			*fspec |= FSPEC05_I013_040;
			int32_t direction = lrint (service.AzimuthalElevationAngle
				* (1 << 16) / M_TWOPI);
			field[0] = (direction >> 8) & 0xff;
			field[1] = direction & 0xff;
			field += 2;
		}

		// Antenna Scanning Period
		if (service.AntennaScanningPeriodPresent)
		{
			*fspec |= FSPEC06_I013_041;
			int period = (service.AntennaScanningPeriod * 128 + 500) / 1000;
			field[0] = (period >> 8) & 0xff;
			field[1] = period & 0xff;
			field += 2;
		}

		// Antenna Pedestal Direction
		if (service.AntennaPedestalDirectionPresent)
		{
			*fspec |= FSPEC07_I013_042;
			int32_t direction = lrint (service.AntennaPedestalDirection
				* (1 << 16) / M_TWOPI);
			field[0] = (direction >> 8) & 0xff;
			field[1] = direction & 0xff;
			field += 2;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// System Configuration & Status
		if (service.ConfigurationStatusPresent)
		{
			*fspec |= FSPEC08_I013_050;
			uint8_t * fspec_50 = field;
			field += 1;

			// Subfield #1: COM
			if (service.ConfigurationStatus.COMPresent)
			{
				*fspec_50 |= 0x80;
				field[0] = (service.ConfigurationStatus.COM.NOGO << 7);
				field[0] |= service.ConfigurationStatus.COM.RDPC << 6;
				field[0] |= service.ConfigurationStatus.COM.RDPR << 5;
				field[0] |= service.ConfigurationStatus.COM.OVLRDP << 4;
				field[0] |= service.ConfigurationStatus.COM.OVLXMT << 3;
				field[0] |= service.ConfigurationStatus.COM.MSC << 2;
				field[0] |= service.ConfigurationStatus.COM.TSV << 1;
				field += 1;
			}

			// Subfield 3: WND
			if (service.ConfigurationStatus.WNDPresent)
			{
				*fspec_50 |= 0x20;
				long speed = lrint (service.ConfigurationStatus.WND.SPEED_10s * (1 << 16) / 200);
				long heading = lrint (service.ConfigurationStatus.WND.DIRECTION_10s * (1 << 16) / M_TWOPI);
				field[0] = (speed >> 8) & 0xff;
				field[1] = speed & 0xff;
				field[2] = (heading >> 8) & 0xff;
				field[3] = heading & 0xff;
				field += 4;

				speed = lrint (service.ConfigurationStatus.WND.SPEED_1min * (1 << 16) / 200);
				heading = lrint (service.ConfigurationStatus.WND.DIRECTION_1min * (1 << 16) / M_TWOPI);
				field[0] = (speed >> 8) & 0xff;
				field[1] = speed & 0xff;
				field[2] = (heading >> 8) & 0xff;
				field[3] = heading & 0xff;
				field += 4;
			}

			// Subfield #4: PAR
			if (service.ConfigurationStatus.PARPresent)
			{
				*fspec_50 |= 0x10;
				field[0] = service.ConfigurationStatus.PAR.ANT << 7;
				field[0] |= service.ConfigurationStatus.PAR.CHAB << 5;
				field[0] |= service.ConfigurationStatus.PAR.OVL << 4;
				field[0] |= service.ConfigurationStatus.PAR.MRC << 3;
				field[0] |= service.ConfigurationStatus.PAR.ARR << 2;
				field[0] |= service.ConfigurationStatus.PAR.OPR << 1;
				field[0] |= service.ConfigurationStatus.PAR.CAL;
				field += 1;
			}

			// Subfield 5: CTR
			if (service.ConfigurationStatus.CTRPresent)
			{
				*fspec_50 |= 0x08;
				field[0] = 0;
				field[0] |= service.ConfigurationStatus.CTR.RNG << 7;
				field[0] |= service.ConfigurationStatus.CTR.FREQ << 6;
        field[0] |= service.ConfigurationStatus.CTR.PSDM << 5;
				field += 1;
			}

			// Subfield #6: POL
			if (service.ConfigurationStatus.POLPresent)
			{
				*fspec_50 |= 0x04;
				field[0] = 0;
        field[0] |= service.ConfigurationStatus.POL.CRA << 7;
				field[0] |= service.ConfigurationStatus.POL.CLA << 6;
        field[0] |= service.ConfigurationStatus.POL.CRE << 5;
				field[0] |= service.ConfigurationStatus.POL.CLE << 4;
				field[0] |= service.ConfigurationStatus.POL.LVA << 3;
				field[0] |= service.ConfigurationStatus.POL.LHA << 2;
				field[0] |= service.ConfigurationStatus.POL.LVE << 1;
				field[0] |= service.ConfigurationStatus.POL.LHE << 0;
				field += 1;
			}

			// Subfield 7: PSV
			if (service.ConfigurationStatus.PSVPresent)
			{
				*fspec_50 |= 0x02;
			  int32_t value = lrint (service.ConfigurationStatus.PSV.AngleElevation
				  * (1 << 15) / M_PI_2);
			  field[0] = (value >> 8) & 0xff;
			  field[1] = value & 0xff;
			  field += 2;

        value = lrint (service.ConfigurationStatus.PSV.AngleAzimuthal
				  * (1 << 15) / M_PI_2);
			  field[0] = (value >> 8) & 0xff;
			  field[1] = value & 0xff;
			  field += 2;

        value = lrint (service.ConfigurationStatus.PSV.AspectRatioElevation);
			  field[0] = (value >> 8) & 0xff;
			  field[1] = value & 0xff;
			  field += 2;

        value = lrint (service.ConfigurationStatus.PSV.AspectRatioAzimuthal);
			  field[0] = (value >> 8) & 0xff;
			  field[1] = value & 0xff;
			  field += 2;
			}

			if (!(*fspec_50))
			{
				field--;
			}
		}

		// 3D-Position of Data Source
		if (service.Position3DPresent)
		{
			*fspec |= FSPEC09_I013_120;
			long height = lrint (service.Position3D.Height);
			long lat = lrint (service.Position3D.Latitude / M_PI * (1 << 23));
			long lon = lrint (service.Position3D.Longitude / M_PI * (1 << 23));
			if (lat > 0x7FFFFF || lat < -0x800000)
			{
				cms_warning ("lat out of limit");
				goto skip;
			}
			if (lon > 0x7FFFFF || lon < -0x800000)
			{
				cms_warning ("lon out of limit");
				goto skip;
			}
			field[0] = (height >> 8) & 0xff;
			field[1] = height & 0xff;
			field[2] = (lat >> 16) & 0xff;
			field[3] = (lat >> 8) & 0xff;
			field[4] = lat & 0xff;
			field[5] = (lon >> 16) & 0xff;
			field[6] = (lon >> 8) & 0xff;
			field[7] = lon & 0xff;
			field += 8;
		}

		// Azimuthal Antenna Tilt
		if (service.AzimuthalAntennaTiltPresent)
		{
			*fspec |= FSPEC10_I013_043;
			int32_t tilt = lrint (service.AzimuthalAntennaTilt
				* (1 << 15) / M_PI_2);
			field[0] = (tilt >> 8) & 0xff;
			field[1] = tilt & 0xff;
			field += 2;
		}

		// Elevation Antenna Tilt
		if (service.ElevationAntennaTiltPresent)
		{
			*fspec |= FSPEC11_I013_044;
			int32_t x = lrint (service.ElevationAntennaTilt.X
				* (1 << 15) / M_PI_2);
			int32_t y = lrint (service.ElevationAntennaTilt.Y
				* (1 << 15) / M_PI_2);
			field[0] = (x >> 8) & 0xff;
			field[1] = x & 0xff;
			field[2] = (y >> 8) & 0xff;
			field[3] = y & 0xff;
			field += 4;
		}

		// Elevation Antenna Correction
		if (service.ElevationCorrectionPresent)
		{
			*fspec |= FSPEC12_I013_045;;
			int32_t correction = lrint (service.ElevationCorrection
				* (1 << 15) / M_PI_2);
			field[0] = (correction >> 8) & 0xff;
			field[1] = correction & 0xff;
			field += 2;
		}

		// Runway in Use
		if (service.RunwayInUsePresent)
		{
			*fspec |= FSPEC13_I013_046;

			uint8_t * fspec = field;
			field += 1;

			// Subfield #1: ID
			// Runway Identification
			if (service.RunwayInUse.IDPresent)
			{
				*fspec |= 0x80;
				asterix_encode_callsign (service.RunwayInUse.ID, field);
				field += 6;
			}

			// Subfield #2: RWD
			// Runway direction of the antenna pedestal
			if (service.RunwayInUse.RWDPresent)
			{
				*fspec |= 0x40;
				int16_t rwd = lrint (service.RunwayInUse.RWD
					* (1 << 16) / M_TWOPI);
				field[0] = (rwd >> 8) & 0xff;
				field[1] = rwd & 0xff;
				field += 2;
			}

			// Subfield #3: TDD
			// Touch Down Point distance from center of Azimuth Antenna
			if (service.RunwayInUse.TDDPresent)
			{
				*fspec |= 0x20;
				uint16_t tdd = lrint (service.RunwayInUse.TDD);
				field[0] = (tdd >> 8) & 0xff;
				field[1] = tdd & 0xff;
				field += 2;
			}

			// Subfield #4: LDR
			// Lateral distance from center of Azimuth Antenna to center of Runway
			if (service.RunwayInUse.LDRPresent)
			{
				*fspec |= 0x10;
				uint16_t ldr = lrint (service.RunwayInUse.LDR);
				field[0] = (ldr >> 8) & 0xff;
				field[1] = ldr & 0xff;
				field += 2;
			}

			// Subfield #5: TDE
			// Elevation of the Touch Down Point with the center of Elevation Antenna
			if (service.RunwayInUse.TDEPresent)
			{
				*fspec |= 0x08;
				int16_t tde = lrint (service.RunwayInUse.TDE * 100.);
				field[0] = (tde >> 8) & 0xff;
				field[1] = tde & 0xff;
				field += 2;
			}

			// Subfield #7: RWN
			// Runway Number from RFGEN
			if (service.RunwayInUse.RWNPresent)
			{
				*fspec |= 0x02;
				field[0] = service.RunwayInUse.RWN;
				field += 1;
			}
		}
		
		// Radar Base Tilt
    if (service.RadarBaseTiltPresent)
    {
      *fspec |= FSPEC14_I013_047;
      int32_t x = lrint (service.RadarBaseTilt.X
        * (1 << 15) / M_PI_2);
      int32_t y = lrint (service.RadarBaseTilt.Y
        * (1 << 15) / M_PI_2);
      field[0] = (x >> 8) & 0xff;
      field[1] = x & 0xff;
      field[2] = (y >> 8) & 0xff;
      field[3] = y & 0xff;
      field += 4;
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
				service_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
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
