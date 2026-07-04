#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I004_010 0x80
#define FSPEC02_I004_000 0x40
#define FSPEC03_I004_015 0x20
#define FSPEC04_I004_020 0x10
#define FSPEC05_I004_040 0x08
#define FSPEC06_I004_045 0x04
#define FSPEC07_I004_060 0x02

#define FSPEC08_I004_030 0x80
#define FSPEC09_I004_170 0x40
#define FSPEC10_I004_120 0x20
#define FSPEC11_I004_070 0x10
#define FSPEC12_I004_076 0x08
#define FSPEC13_I004_074 0x04
#define FSPEC14_I004_075 0x02

#define FSPEC15_I004_100 0x80
#define FSPEC16_I004_035 0x40
#define FSPEC17_I004_171 0x20
#define FSPEC18_I004_110 0x10
#define FSPEC19_I004_SPA 0x08
#define FSPEC20_I048_RE  0x04
#define FSPEC21_I048_SP  0x02

bool Asterix004_to_WarningAlert (const void * data, int size, 
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

	if (cat != 4)
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
		cms_data (size, data);
		return false;
	}
	
	while (record < msg + len)
	{
		// vytvorime novou polozku
		RDFWarningAlert alert;

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

		// data source identifier (2)
		if (*fspec & FSPEC01_I004_010)
		{
			alert.SAC = field[0];
			alert.SIC = field[1];
			field += 2;
		}

		// message type (1)
		if (*fspec & FSPEC02_I004_000)
		{
			alert.AlertType = (ALERT_TYPE) field[0];
			field += 1;
		}

		// SDPS identifier (1 + 2 * n)
		if (*fspec & FSPEC03_I004_015)
		{
			int rep = field[0];
			field += 1 + 2 * rep;
		}

		// time of message (3)
		if (*fspec & FSPEC04_I004_020)
		{
			alert.Time.Present = true;
			alert.Time.Time = (long long) field2unsigned (field, 24) * 1000 / 128;
			field += 3;
		}

		// alert identifier (1)
		if (*fspec & FSPEC05_I004_040)
		{
			alert.AlertID = field2unsigned (field, 16);
			field += 2;
		}

		// alert status (1)
		if (*fspec & FSPEC06_I004_045)
		{
			alert.AlertStatus = (field[0] >> 1) & 7;
			field += 1;
		}

		// safety net function status (1 + 1)
		if (*fspec & FSPEC07_I004_060)
		{
			// RAMLD - 0x40
			// RAMHD - 0x20
			// MSAW - 0x10
			// APW - 0x08
			// CLAM - 0x04
			// STCA - 0x02

			if (*field & 1)
			{
				field += 1;
				// AFDA - 0x80
				// RIMCA - 0x40
			}

			field += 1;
	}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// track number 1 (2)
		if (*fspec & FSPEC08_I004_030)
		{
			alert.TrackNumber.Present = true;
			alert.TrackNumber.Number = field2unsigned (field, 16);
			field += 2;
		}

		// aircraft identification & charakteristics
		if (*fspec & FSPEC09_I004_170)
		{
			unsigned char * subfspec = field;
			unsigned char * subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			// aircraft identifier 1
			if (*subfspec & 0x80)
			{
				subfield += 7;
			}

			// mode 3/A code aircraft 1
			if (*subfspec & 0x40)
			{
				subfield += 2;
			}

			// predicted conflict position 1 (WGS84)
			if (*subfspec & 0x20)
			{
				subfield += 10;
			}

			// predicted conflict position 1 (Cartesian)
			if (*subfspec & 0x10)
			{
				subfield += 8;
			}

			// time to threshold aircraft 1
			if (*subfspec & 0x08)
			{
				subfield += 3;
			}

			// distance to threshold aircraft 1
			if (*subfspec & 0x04)
			{
				subfield += 2;
			}

			// aircraft characteristics aircraft 1
			if (*subfspec & 0x02)
			{
				if (*subfield & 1)
				{
					subfield += 1;
				}
				if (*subfield & 1)
				{
					cms_error ("unable to handle bits");
					goto error;
				}
				subfield += 1;
			}

			// extension
			if (!(*subfspec & 1))
			{
				goto end170;
			}

			subfspec++;

			// mode S identifier aircraft 1
			if (*subfspec & 0x80)
			{
				subfield += 6;
			}

			// flight plan number aircraft 1
			if (*subfspec & 0x40)
			{
				subfield += 4;
			}

			// cleared flight level aircraft 1
			if (*subfspec & 0x20)
			{
				subfield += 2;
			}

			// spare bits
			if (*subfspec & 0x1f)
			{
				cms_error ("unable to handle bits (0x%X)", *field);
				goto error;
			}
end170:
			field = subfield;
		}

		// conflict charakteristics
		if (*fspec & FSPEC10_I004_120)
		{
			unsigned char * subfspec = field;
			unsigned char * subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			// conflict nature
			if (*subfspec & 0x80)
			{
				if (*subfield & 1)
				{
					subfield += 1;
				}
				if (*subfield & 1)
				{
					cms_error ("unable to handle bits");
					goto error;
				}
				subfield += 1;
			}

			// conflict classification
			if (*subfspec & 0x40)
			{
				if (!alert.Conflict)
					alert.Conflict = new RDFConflictCharacteristics ();
				alert.Conflict->ConflictClassificationPresent = true;
				alert.Conflict->ConflictClassification.TableId = (field[0] >> 4) & 0xf;
				alert.Conflict->ConflictClassification.ConflictProperties = (field[0] >> 1) & 0x7;
				alert.Conflict->ConflictClassification.ConflictSeverity = (field[0] >> 0) & 0x1;
				subfield += 1;
			}

			// conflict probability
			if (*subfspec & 0x20)
			{
				subfield += 1;
			}

			// conflict duration
			if (*subfspec & 0x10)
			{
				if (!alert.Conflict)
					alert.Conflict = new RDFConflictCharacteristics ();
				alert.Conflict->DurationPresent = true;
				alert.Conflict->Duration = (long long)
					field2unsigned (subfield, 24) * 1000 / 128;
				subfield += 3;
			}

			// spare bits
			if (*subfspec & 0x0f)
			{
				cms_error ("unable to handle bits");
				goto error;
			}

			field = subfield;
		}

		// conflict timing and separation
		if (*fspec & FSPEC11_I004_070)
		{
			unsigned char * subfspec = field;
			unsigned char * subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			alert.Separation = new RDFAlertSeparation ();
			
			// time to conflict
			if (*subfspec & 0x80)
			{
				alert.Separation->TimeToConflictPresent = true;
				alert.Separation->TimeToConflict = (long long) 
					field2unsigned (subfield, 24) * 1000 / 128;
				subfield += 3;
			}

			// time to closest approach
			if (*subfspec & 0x40)
			{
				alert.Separation->TimeToClosestApproachPresent = true;
				alert.Separation->TimeToClosestApproach = (long long) 
					field2unsigned (subfield, 24) * 1000 / 128;
				subfield += 3;
			}

			// current horizontal separation
			if (*subfspec & 0x20)
			{
				alert.Separation->CurrentHSeparationPresent = true;
				alert.Separation->CurrentHSeparation = 
					field2unsigned (subfield, 24) / 2;
				subfield += 3;
			}

			// estimated minimum horizontal separation
			if (*subfspec & 0x10)
			{
				alert.Separation->EstimatedHSeparationPresent = true;
				alert.Separation->EstimatedHSeparation = 
					field2unsigned (subfield, 16) / 2;
				subfield += 2;
			}

			// current vertical separation
			if (*subfspec & 0x08)
			{
				alert.Separation->CurrentVSeparationPresent = true; 
				alert.Separation->CurrentVSeparation = (int) 
					rint (field2unsigned (subfield, 16) * 25 * 0.3048);
				subfield += 2;
			}

			// estimated vertical minimum separation
			if (*subfspec & 0x04)
			{
				alert.Separation->EstimatedVSeparationPresent = true;
				alert.Separation->EstimatedVSeparation = (int)
					rint (field2unsigned (subfield, 16)  * 25 * 0.3048);
				subfield += 2;
			}

			// spare bits
			if (*subfspec & 0x03)
			{
				cms_error ("unable to handle bits");
				goto error;
			}

			field = subfield;
		}

		// vertical deviation
		if (*fspec & FSPEC12_I004_076)
		{
			alert.VerticalDeviationPresent = true;
			alert.VerticalDeviation = (int)	rint (field2signed (field, 16)  * 25 * 0.3048);
			field += 2;
		}

		// longitudinal deviation
		if (*fspec & FSPEC13_I004_074)
		{
			field += 2;
		}

		// transverzal distance deviation
		if (*fspec & FSPEC14_I004_075)
		{
			field += 3;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// area definition
		if (*fspec & FSPEC15_I004_100)
		{
			unsigned char * subfspec = field;
			unsigned char * subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			alert.Area = new RDFAlertArea ();

			// area name
			if (*subfspec & 0x80)
			{
				asterix_decode_callsign (subfield, alert.Area->Name);
				subfield += 6;
			}

			// crossing area name
			if (*subfspec & 0x40)
			{
				subfield += 7;
			}

			// runway/taxiway designator 1
			if (*subfspec & 0x20)
			{
				subfield += 7;
			}

			// runway/taxiway designator 2
			if (*subfspec & 0x10)
			{
				subfield += 7;
			}

			// stop bar designator
			if (*subfspec & 0x08)
			{
				subfield += 7;
			}

			// gate designator
			if (*subfspec & 0x04)
			{
				subfield += 7;
			}

			// spare bits
			if (*subfspec & 0x03)
			{
				cms_error ("unable to handle bits");
				goto error;
			}

			field = subfield;
		}

		// track number 2
		if (*fspec & FSPEC16_I004_035)
		{
			alert.TrackNumber2.Present = true;
			alert.TrackNumber2.Number = field2unsigned (field, 16);
			field += 2;
		}

		// aircraft identification and charakteristics 2
		if (*fspec & FSPEC17_I004_171)
		{
			unsigned char * subfspec = field;
			unsigned char * subfield = field;
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			// aircraft identifier 2
			if (*subfspec & 0x80)
			{
				subfield += 7;
			}

			// mode 3/A code aircraft 2
			if (*subfspec & 0x40)
			{
				subfield += 2;
			}

			// predicted conflict position 2 (WGS84)
			if (*subfspec & 0x20)
			{
				subfield += 10;
			}

			// predicted conflict position 2 (Cartesian)
			if (*subfspec & 0x10)
			{
				subfield += 8;
			}

			// time to threshold aircraft 2
			if (*subfspec & 0x08)
			{
				subfield += 3;
			}

			// distance to threshold aircraft 2
			if (*subfspec & 0x04)
			{
				subfield += 2;
			}

			// aircraft characteristics aircraft 2
			if (*subfspec & 0x02)
			{
				if (*subfield & 1)
				{
					subfield += 1;
				}
				if (*subfield & 1)
				{
					cms_error ("unable to handle bits");
					goto error;
				}
				subfield += 1;
			}

			// extension
			if (!(*subfspec & 1))
			{
				goto end171;
			}

			subfspec++;

			// mode S identifier aircraft 2
			if (*subfspec & 0x80)
			{
				subfield += 6;
			}

			// flight plan number aircraft 2
			if (*subfspec & 0x40)
			{
				subfield += 4;
			}

			// cleared flight level aircraft 2
			if (*subfspec & 0x20)
			{
				subfield += 2;
			}

			// spare bits
			if (*subfspec & 0x1f)
			{
				cms_error ("unable to handle bits (0x%X)", *field);
				goto error;
			}
end171:
			field = subfield;
		}

		// FDPS sector control identifier
		if (*fspec & FSPEC18_I004_110)
		{
			int rep = field[0];
			field += 1 + 2 * rep;
		}

		// spare
		if (*fspec & FSPEC19_I004_SPA)
		{
			cms_error ("unable to handle bit");
			goto error;
		}

		// reserved for expansion field
		if (*fspec & FSPEC20_I048_RE)
		{
			field += field[0];
		}

		// reserved for special purpose field
		if (*fspec & FSPEC21_I048_SP)
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

int WarningAlert_to_Asterix004 (deque<RDFWarningAlert> & alert_list, 
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
	msg[0] = 4; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;	

	// seznam je prazdny, neni co zpracovavat
	if (alert_list.empty ())
	{
		return 0;
	}
	
	size = 3;
	while (alert_list.size () > 0)
	{
		RDFWarningAlert & alert = alert_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;

		// data source identifier (2)
		{
			*fspec |= FSPEC01_I004_010;
			field[0] = alert.SAC;
			field[1] = alert.SIC;
			field += 2;
		}

		// message type (1)
		{
			*fspec |= FSPEC02_I004_000;
			field[0] = alert.AlertType;
			field += 1;
		}

		// SDPS identifier (1 + 2 * n)
		// *fspec |= FSPEC03_I004_015;

		// time of message (3)
		if (alert.Time.Present)
		{
			*fspec |= FSPEC04_I004_020;
			int time = RDF_time_to_asterix_time(alert.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// alert identifier (1)
		{
			*fspec |= FSPEC05_I004_040;
			field[0] = (alert.AlertID >> 8) & 0xff;
			field[1] = alert.AlertID & 0xff;
			field += 2;
		}

		// alert status (1)
		{
			*fspec |= FSPEC06_I004_045;
			field[0] = (alert.AlertStatus & 7) << 1;
			field += 1;
		}

		// safety net function status (1 + 1)
		// *fspec |= FSPEC107_I004_060

		// konec polozky fspec
		*fspec |= FSPEC_FX;
		fspec++;

		// track number
		if (alert.TrackNumber.Present)
		{
			*fspec |= FSPEC08_I004_030;
			field[0] = (alert.TrackNumber.Number >> 8) & 0xff;
			field[1] = alert.TrackNumber.Number & 0xff;
			field += 2;
		}

		// aircraft identification & charakteristics
		// *fspec |= FSPEC09_I004_170

		// conflict charakteristics
		if (alert.Conflict)
		{
			*fspec |= FSPEC10_I004_120;
			unsigned char * subfield = field + 1;

			// conflcit classification
			if (alert.Conflict->ConflictClassificationPresent)
			{
				*field |= 0x40;
				subfield[0] = 0;
				subfield[0] |= alert.Conflict->ConflictClassification.TableId << 4;
				subfield[0] |= alert.Conflict->ConflictClassification.ConflictProperties << 1;
				subfield[0] |= alert.Conflict->ConflictClassification.ConflictSeverity;
				subfield += 1;
			}

			// conflict duration
			if (alert.Conflict->Duration)
			{
				*field |= 0x10;
				int duration = (long long) alert.Conflict->Duration * 128 / 1000;
				subfield[0] = (duration >> 16) & 0xff;
				subfield[1] = (duration >> 8) & 0xff;
				subfield[2] = duration & 0xff;
				subfield += 3;
			}

			field = subfield;
		}

		// conflict timing and separation
		if (alert.Separation)
		{
			*fspec |= FSPEC11_I004_070;
			unsigned char * subfield = field + 1;

			// time to conflict
			if (alert.Separation->TimeToConflictPresent)
			{
				*field |= 0x80;
				int time = (long long) alert.Separation->TimeToConflict * 128 / 1000;
				subfield[0] = (time >> 16) & 0xff;
				subfield[1] = (time >> 8) & 0xff;
				subfield[2] = time & 0xff;
				subfield += 3;
			}

			// time to closest approach
			if (alert.Separation->TimeToClosestApproachPresent)
			{
				*field |= 0x40;
				int time = (long long) alert.Separation->TimeToClosestApproach * 128 / 1000;
				subfield[0] = (time >> 16) & 0xff;
				subfield[1] = (time >> 8) & 0xff;
				subfield[2] = time & 0xff;
				subfield += 3;
			}

			// current horizontal separation
			if (alert.Separation->CurrentHSeparationPresent)
			{
				*field |= 0x20;
				int separation = alert.Separation->CurrentHSeparation * 2;
				subfield[0] = (separation >> 16) & 0xff;
				subfield[1] = (separation >> 8) & 0xff;
				subfield[2] = separation & 0xff;
				subfield += 3;
			}

			// estimated minimum horizontal separation
			if (alert.Separation->EstimatedHSeparationPresent)
			{
				*field |= 0x10;
				int separation = alert.Separation->EstimatedHSeparation * 2;
				subfield[0] = (separation >> 8) & 0xff;
				subfield[1] = separation & 0xff;
				subfield += 2;
			}

			// current vertical separation
			if (alert.Separation->CurrentVSeparationPresent)
			{
				*field |= 0x08;
				int separation = (int) rint (alert.Separation->CurrentVSeparation / 0.3048 / 25);
				subfield[0] = (separation >> 8) & 0xff;
				subfield[1] = separation & 0xff;
				subfield += 2;
			}

			// estimated vertical minimum separation
			if (alert.Separation->EstimatedVSeparationPresent)
			{
				*field |= 0x04;
				int separation = (int) rint (alert.Separation->EstimatedVSeparation / 0.3048 / 25);
				subfield[0] = (separation >> 8) & 0xff;
				subfield[1] = separation & 0xff;
				subfield += 2;
			}

			field = subfield;
		}

		// vertical deviation
		if (alert.VerticalDeviationPresent)
		{
			*fspec |= FSPEC12_I004_076;
			int deviation = (int) rint (alert.VerticalDeviation / 0.3048 / 25);
			field[0] = (deviation >> 8) & 0xff;
			field[1] = deviation & 0xff;
			field += 2;
		}

		// longitudinal deviation
		// *fspec |= FSPEC13_I004_074

		// transverzal distance deviation
		// *fspec |= FSPEC14_I004_075

		// konec polozky fspec
		*fspec |= FSPEC_FX;
		fspec++;

		// area definition
		if (alert.Area)
		{
			*fspec |= FSPEC15_I004_100;
			unsigned char * subfield = field + 1;

			// area name
			//if (alert.Area.NamePresent)
			{
				*field |= 0x80;
				asterix_encode_callsign (alert.Area->Name, subfield);
				subfield += 6;
			}

			// crossing area name
			// *field |= 0x40

			// runway/taxiway designator 1
			// *field |= 0x20

			// runway/taxiway designator 2
			// *field |= 0x10

			// stop bar designator
			// *field |= 0x08

			// gate designator
			// *field |= 0x04

			field = subfield;
		}

		// track number 2
		if (alert.TrackNumber2.Present)
		{
			*fspec |= FSPEC16_I004_035;
			field[0] = (alert.TrackNumber2.Number >> 8) & 0xff;
			field[1] = alert.TrackNumber2.Number & 0xff;
			field += 2;
		}

		// aircraft identification and charakteristics 2
		/*{
			*fspec |= FSPEC3_I004_171;
			unsigned char * subfield = buffer2;
			memset (buffer2, 0, sizeof (buffer2));

			// aircraft identifier 2
			// *field |= 0x80

			// mode 3/A code aircraft 2
			// *field |= 0x40

			// predicted conflict position 2 (WGS84)
			// *field |= 0x20

			// predicted conflict position 2 (Cartesian)
			// *field |= 0x10

			// time to threshold aircraft 2
			// *field |= 0x08

			// distance to threshold aircraft 2
			// *field |= 0x04

			// aircraft characteristics aircraft 2
			// *field |=0x02

			// extension
			*field |= 1;
			field += 1;

			// mode S identifier aircraft 2
			// *field |= 0x80

			// flight plan number aircraft 2
			// *field |= 0x40

			// cleared flight level aircraft 2
			// *field |= 0x20

			if (!*field)
			{
				field--;
				*field &= ~1;
			}
			field++;

			int subsize = (int) subfield - (int) buffer2;
			memcpy (field, buffer2, subsize);
			field += subsize;
		}*/

		// FDPS sector control identifier
		/*{
			*fspec |= FSPEC3_I004_110;
		}*/


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
				alert_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;
		
		// odstranime track z fronty
		alert_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}

