#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I019_010 0x80
#define FSPEC02_I019_000 0x40
#define FSPEC03_I019_140 0x20
#define FSPEC04_I019_550 0x10
#define FSPEC05_I019_551 0x08
#define FSPEC06_I019_552 0x04
#define FSPEC07_I019_553 0x02

#define FSPEC08_I019_600 0x80
#define FSPEC09_I019_610 0x40
#define FSPEC10_I019_620 0x20
#define FSPEC11_I019_SPA 0x10
#define FSPEC12_I019_SPA 0x08
#define FSPEC13_I019_RE  0x04
#define FSPEC14_I019_SP  0x02

bool Asterix019_to_unknown (const void * data, int size)
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

	if (cat != 19)
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

		// Data Source Identifier
		if (*fspec & FSPEC01_I019_010)
		{
			//track.SAC = field[0];
			//track.SIC = field[1];
			field += 2;
		}

		// Message Type
		if (*fspec & FSPEC02_I019_000)
		{
			field += 1;
		}

		// Time of Day
		if (*fspec & FSPEC03_I019_140)
		{
			//track.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
			field += 3;
		}

		// System Status
		if (*fspec & FSPEC04_I019_550)
		{
			field += 1;
		}

		// Tracking Processor Detailed Status
		if (*fspec & FSPEC05_I019_551)
		{
			field += 1;
		}

		// Remote Sensor Detailed Status
		if (*fspec & FSPEC06_I019_552)
		{
			field += field[0] * 2 + 1;
		}

		// Reference Transponder Detailed Status
		if (*fspec & FSPEC07_I019_553)
		{
			while (field[0] & 1)
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

		// Position of the MLT System Reference point
		if (*fspec & FSPEC08_I019_600)
		{
			field += 8;
		}

		// Height of the MLT System Reference point
		if (*fspec & FSPEC09_I019_610)
		{
			field += 2;
		}

		// WGS-84 Undulation
		if (*fspec & FSPEC10_I019_620)
		{
			field += 1;
		}

		// Spare
		if (*fspec & FSPEC11_I019_SPA)
		{
			cms_error ("unable to handle spare");
			goto error;
		}

		// Spare
		if (*fspec & FSPEC12_I019_SPA)
		{
			cms_error ("unable to handle spare");
			goto error;
		}

		// RE
		if (*fspec & FSPEC13_I019_RE)
		{
			field += field[0];
		}

		// SP
		if (*fspec & FSPEC14_I019_SP)
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
		//track_list.push_back (track);

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
	return false;
}

