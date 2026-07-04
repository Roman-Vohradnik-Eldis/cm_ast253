#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I065_010 0x80
#define FSPEC02_I065_000 0x40
#define FSPEC03_I065_015 0x20
#define FSPEC04_I065_030 0x10
#define FSPEC05_I065_020 0x08
#define FSPEC06_I065_040 0x04
#define FSPEC07_I065_050 0x02

#define FSPEC08_I065_SPA 0x80
#define FSPEC09_I065_SPA 0x40
#define FSPEC10_I065_SPA 0x20
#define FSPEC11_I065_SPA 0x10
#define FSPEC12_I065_SPA 0x08
#define FSPEC13_I065_RE  0x04
#define FSPEC14_I065_SP  0x02

bool Asterix065_to_unknown (const void * data, int size)
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

	if (cat != 65)
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

		// Data Source Identifier
		if (*fspec & FSPEC01_I065_010)
		{
			//track.SAC = field[0];
			//track.SIC = field[1];
			field += 2;
		}

		// Message Type
		if (*fspec & FSPEC02_I065_000)
		{
			field += 1;
		}

		// Service Identification
		if (*fspec & FSPEC03_I065_015)
		{
			field += 1;
		}

		// Time Of Service
		if (*fspec & FSPEC04_I065_030)
		{
			field += 3;
		}

		// Batch Number
		if (*fspec & FSPEC05_I065_020)
		{
			field += 1;
		}

		// SDPS Configuration and Status
		if (*fspec & FSPEC06_I065_040)
		{
			field += 1;
		}

		// Service Status Report
		if (*fspec & FSPEC07_I065_050)
		{
			field += 1;
		}
		
		// fx bit
		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// spare
		if (*fspec & FSPEC08_I065_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC09_I065_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}
		
		// spare
		if (*fspec & FSPEC10_I065_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC11_I065_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC12_I065_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// Reserved Expansion Field
		if (*fspec & FSPEC13_I065_RE)
		{
			field += field[0];
		}

		// Sprecial Purpose Field
		if (*fspec & FSPEC14_I065_SP)
		{
			field += field[0];
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

