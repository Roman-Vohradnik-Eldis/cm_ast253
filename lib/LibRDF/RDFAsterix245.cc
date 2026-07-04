#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I245_010 0x80
#define FSPEC02_I245_020 0x40
#define FSPEC03_I245_SPA 0x20
#define FSPEC04_I245_SPA 0x10
#define FSPEC05_I245_SPA 0x08
#define FSPEC06_I245_SPA 0x04
#define FSPEC07_I245_SPA 0x02

bool Asterix245_to_WarningAlert (const void * data, int size, 
	deque<RDFWarningAlert> & alert_list)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	alert_list.clear ();
	
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

	if (cat != 245)
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
		// vytvorime novy alert
		RDFWarningAlert alert;
		
		// najdeme zacatek datovych poli
		fspec = field = record;
		while ((*field & FSPEC_FX) && field < msg + len) field++;
		field++;

		if (*fspec & FSPEC01_I245_010)
		{
			switch (field[0])
			{
				case 0:
				{
					alert.AlertType = ALERT_TYPE_NONE;
					break;
				}
				case 1:
				{
					alert.AlertType = ALERT_TYPE_STCA;
					break;
				}
				case 2:
				{
					alert.AlertType = ALERT_TYPE_MSAW;
					break;
				}
				case 3:
				{
					alert.AlertType = ALERT_TYPE_DAIW;
					break;
				}
				default:
				{
					cms_warning ("unknown alert %d", field[0]);
					alert.AlertType = ALERT_TYPE_NONE;
					break;
				}
			}
			field += 1;
		}

		if (*fspec & FSPEC02_I245_020)
		{
			alert.TrackNumber.Present = true;
			alert.TrackNumber.Number = field2unsigned (field, 32);
			field += 4;
		}

		if (*fspec & FSPEC03_I245_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC04_I245_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC05_I245_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC06_I245_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC07_I245_SPA)
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
		// pridame polozku do seznamu
		alert_list.push_back (alert);
		
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
	alert_list.clear ();
	return false;
}
