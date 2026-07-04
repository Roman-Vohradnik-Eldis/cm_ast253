#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I252_010 0x80
#define FSPEC02_I252_015 0x40
#define FSPEC03_I252_020 0x20
#define FSPEC04_I252_035 0x10
#define FSPEC05_I252_110 0x08
#define FSPEC06_I252_330 0x04
#define FSPEC07_I252_210 0x02

#define FSPEC08_I252_220 0x80
#define FSPEC09_I252_200 0x40
#define FSPEC10_I252_130 0x20
#define FSPEC11_I252_140 0x10
#define FSPEC12_I252_120 0x08
#define FSPEC13_I252_150 0x04
#define FSPEC14_I252_160 0x02

#define FSPEC15_I252_060 0x80
#define FSPEC16_I252_170 0x40
#define FSPEC17_I252_190 0x20
#define FSPEC18_I252_230 0x10
#define FSPEC19_I252_320 0x08
#define FSPEC20_I252_310 0x04
#define FSPEC21_I252_240 0x02

#define FSPEC22_I252_270 0x80
#define FSPEC23_I252_280 0x40
#define FSPEC24_I252_290 0x20
#define FSPEC25_I252_260 0x10
#define FSPEC26_I252_250 0x08
#define FSPEC27_I252_300 0x04
#define FSPEC28_I252_255 0x02

#define FSPEC29_I252_265 0x80
#define FSPEC30_I252_070 0x40
#define FSPEC31_I252_080 0x20
#define FSPEC32_I252_135 0x10
#define FSPEC33_I252_137 0x08
#define FSPEC34_I252_350 0x04
#define FSPEC35_I252_360 0x02

#define FSPEC36_I252_370 0x80
#define FSPEC37_I252_400 0x40
#define FSPEC38_I252_410 0x20
#define FSPEC39_I252_420 0x10
#define FSPEC40_I252_430 0x08
#define FSPEC41_I252_440 0x04
#define FSPEC42_I252_450 0x02

#define FSPEC43_I252_460 0x80
#define FSPEC44_I252_470 0x40
#define FSPEC45_I252_480 0x20
#define FSPEC46_I252_SPA 0x10
#define FSPEC47_I252_SPA 0x08
#define FSPEC48_I252_SPA 0x04
#define FSPEC49_I252_SPA 0x02

bool Asterix252_to_unknown (const void * data, int size)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

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

	if (cat != 252)
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
		//STargetTrack track;

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

		// Server Identification Tag
		if (*fspec & FSPEC01_I252_010)
		{
			field += 2;
		}

		// User Number
		if (*fspec & FSPEC02_I252_015)
		{
			field += 2;
		}

		// Time Of message
		if (*fspec & FSPEC03_I252_020)
		{
			field += 3;
		}

		// Type Of Message
		if (*fspec & FSPEC04_I252_035)
		{
			field += 1;
		}

		// Service Identification
		if (*fspec & FSPEC05_I252_110)
		{
			while (field[0] & 1) field += 1;
			field += 1;
		}

		// Service Related Report
		if (*fspec & FSPEC06_I252_330)
		{
			field += 1 + field[0] * 2;
		}

		// Item Selector
		if (*fspec & FSPEC07_I252_210)
		{
			field += 1 + field[0] * 2;
		}

		// fx bit
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Cyclical Update Characteristics
		if (*fspec & FSPEC08_I252_220)
		{
			field += 5;
		}

		// Track/Flight Categories Selector
		if (*fspec & FSPEC09_I252_200)
		{
			field += 4;
		}

		// Track Number Selector
		if (*fspec & FSPEC10_I252_130)
		{
			field += 1 + field[0] * 2;
		}

		// Code Family Selector
		if (*fspec & FSPEC11_I252_140)
		{
			field += 1 + field[0] * 2;
		}

		// Callsign Selector
		if (*fspec & FSPEC12_I252_120)
		{
			field += 1 + field[0] * 7;
		}

		// Departure Airport Selector
		if (*fspec & FSPEC13_I252_150)
		{
			field += 1 + field[0] * 4;
		}

		// Destination Airport Selector
		if (*fspec & FSPEC14_I252_160)
		{
			field += 1 + field[0] * 4;
		}
		
		// fx bit
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Geographical Area
		if (*fspec & FSPEC15_I252_060)
		{
			field += 1 + field[0] * 6;
		}

		// Aircraft Type Selector
		if (*fspec & FSPEC16_I252_170)
		{
			field += 1 + field[0] * 4;
		}

		// Current Control Position Selector
		if (*fspec & FSPEC17_I252_190)
		{
			field += 2;
		}

		// Radar Synchronisation Characteristics
		if (*fspec & FSPEC18_I252_230)
		{
			while (field[2] & 1) field += 3;
			field += 3;
		}

		// Minimal Period
		if (*fspec & FSPEC19_I252_320)
		{
			field += 1;
		}

		// Refreshment Period
		if (*fspec & FSPEC20_I252_310)
		{
			field += 1;
		}

		// Discrete Parameters Flags
		if (*fspec & FSPEC21_I252_240)
		{
			while (field[1] & 1) field += 2;
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Heading Criterion
		if (*fspec & FSPEC22_I252_270)
		{
			field += 1;
		}

		// Speed Module Criterion
		if (*fspec & FSPEC23_I252_280)
		{
			field += 1;
		}

		// Altitude Criterion
		if (*fspec & FSPEC24_I252_290)
		{
			field += 1;
		}

		// Position Criterion
		if (*fspec & FSPEC25_I252_260)
		{
			field += 1;
		}

		// Mof Probability Criterion
		if (*fspec & FSPEC26_I252_250)
		{
			field += 1;
		}

		// Factor K
		if (*fspec & FSPEC27_I252_300)
		{
			field += 1;
		}

		// Rate Of Climb/Descent Criterion
		if (*fspec & FSPEC28_I252_255)
		{
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Rate Of Turn Criterion
		if (*fspec & FSPEC29_I252_265)
		{
			field += 1;
		}

		// Lower Limit
		if (*fspec & FSPEC30_I252_070)
		{
			field += 2;
		}

		// Upper Limit
		if (*fspec & FSPEC31_I252_080)
		{
			field += 2;
		}

		// Aircraft Address Selector
		if (*fspec & FSPEC32_I252_135)
		{
			field += 1 + field[0] * 3;
		}

		// Aircraft Identification Selector
		if (*fspec & FSPEC33_I252_137)
		{
			field += 1 + field[0] * 6;
		}

		// Aircraft Selector
		if (*fspec & FSPEC34_I252_350)
		{
			field += 1 + field[0] * 2;
		}

		// Sensor Item Selector
		if (*fspec & FSPEC35_I252_360)
		{
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Periodical Characteristics Of Sensor Information Service
		if (*fspec & FSPEC36_I252_370)
		{
			field += 2;
		}

		// Refreshment Period Of Sensor Informatin Service
		if (*fspec & FSPEC37_I252_400)
		{
			field += 2;
		}

		// Discrete Parameters of Sensor Information Service
		if (*fspec & FSPEC38_I252_410)
		{
			field += 1;
		}

		// Time Stamping Criterion Of Sensor Information Service
		if (*fspec & FSPEC39_I252_420)
		{
			field += 2;
		}

		// SSR Range Bias Criterion Of Sensor Information Service
		if (*fspec & FSPEC40_I252_430)
		{
			field += 2;
		}

		// SSR Range Gain Criterion Of Sensor Information Service
		if (*fspec & FSPEC41_I252_440)
		{
			field += 3;
		}

		// SSR Azimuth BIas Criterion Of Sensor Information Service
		if (*fspec & FSPEC42_I252_450)
		{
			field += 2;
		}
		
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// PR Range Bias Criterion Of Sensor Information Service
		if (*fspec & FSPEC43_I252_460)
		{
			field += 2;
		}

		// PR Range Gain Criterion Of Sensor Information Service
		if (*fspec & FSPEC44_I252_470)
		{
			field += 3;
		}

		// PR Azimuth Bias Criterion Of Sensor Information Service
		if (*fspec & FSPEC45_I252_480)
		{
			field += 2;
		}

		// spare
		if (*fspec & FSPEC46_I252_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC47_I252_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC48_I252_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC49_I252_SPA)
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
	return false;
}

