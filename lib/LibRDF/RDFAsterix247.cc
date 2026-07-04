#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I247_010 0x80
#define FSPEC02_I247_015 0x40
#define FSPEC03_I247_140 0x20
#define FSPEC04_I247_550 0x10
#define FSPEC05_I247_SPA 0x08
#define FSPEC06_I247_SP  0x04
#define FSPEC07_I247_RE  0x02

bool Asterix247_to_VersionNumber (const void *data, int size,
	deque < RDFVersionNumber > &version_list)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	version_list.clear ();

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

	if (cat != 247)
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
		RDFVersionNumber version;

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

		// I247/010 Data Source Identifier
		if (*fspec & FSPEC01_I247_010)
		{
			version.SAC = field[0];
			version.SIC = field[1];
			field += 2;
		}

		// I247/015 Service Identification
		if (*fspec & FSPEC02_I247_015)
		{
			field += 1;
		}

		// I247/140 Time of Day
		if (*fspec & FSPEC03_I247_140)
		{
			version.Time.Present = true;
			version.Time.Time = (long long) field2unsigned (field, 24) * 1000 / 128;
			field += 3;
		}

		// I247/550 Category Version Number Report
		if (*fspec & FSPEC04_I247_550)
		{
			unsigned rep = field[0];
			field += 1;
			for (unsigned i = 0; i < rep; i++)
			{
				RDFCategoryVersion cv;
				cv.Category = field[0];
				cv.MainVersion = field[1];
				cv.SubVersion = field[2];
				version.CategoryVersion.push_back (cv);
				field += 3;
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
		version_list.push_back (version);

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
	version_list.clear ();
	return false;
}

int VersionNumber_to_Asterix247 (deque<RDFVersionNumber> & version_list, 
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
	msg[0] = 247; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;	

	// seznam je prazdny, neni co zpracovavat
	if (version_list.empty ())
	{
		return 0;
	}
	
	size = 3;
	while (version_list.size () > 0)
	{
		RDFVersionNumber & version = version_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;

		// Data Source Identifier
		{
			*fspec |= FSPEC01_I247_010;
			field[0] = version.SAC;
			field[1] = version.SIC;
			field += 2;
		}

		// Service Identification

		// Time of Day
		{
			*fspec |= FSPEC03_I247_140;
			int time = RDF_time_to_asterix_time(version.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Version number report
		if (version.CategoryVersion.size ())
		{
			*fspec |= FSPEC04_I247_550;
			field[0] = version.CategoryVersion.size ();
			field += 1;
			for (unsigned i = 0; i < version.CategoryVersion.size (); i++)
			{	
				const RDFCategoryVersion & cv = version.CategoryVersion[i];
				field[0] = cv.Category;
				field[1] = cv.MainVersion;
				field[2] = cv.SubVersion;
				field += 3;
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
				version_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;
		
		// odstranime track z fronty
		version_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}

