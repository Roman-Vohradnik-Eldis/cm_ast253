#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

#include <algorithm>			/* std::copy */
#include <sstream>
#include <iomanip>

enum CAT021_EDITION
{
	CAT021_EDITION_023,
	CAT021_EDITION_026,
	CAT021_EDITION_10P,
};

/*makro pro odsazovani *out_stru*/
#ifndef NEW_LINE
#define NEW_LINE(st,cnt,c) \
   if((void *)&st != NULL) {\
      if(one_line == false) { \
         st << "\n";          \
         int i = 0;           \
         while(i++ < (int)cnt)\
            st << (c);        \
                              \
       } else {               \
         st << ';';           \
       }\
   }
#endif


#define FSPEC_FX 0x01

#define FSPEC01_I021_ED0_010 0x80
#define FSPEC02_I021_ED0_040 0x40
#define FSPEC03_I021_ED0_030 0x20
#define FSPEC04_I021_ED0_130 0x10
#define FSPEC05_I021_ED0_080 0x08
#define FSPEC06_I021_ED0_140 0x04
#define FSPEC07_I021_ED0_090 0x02

#define FSPEC08_I021_ED0_210 0x80
#define FSPEC09_I021_ED0_230 0x40
#define FSPEC10_I021_ED0_145 0x20
#define FSPEC11_I021_ED0_150 0x10
#define FSPEC12_I021_ED0_151 0x08
#define FSPEC13_I021_ED0_152 0x04
#define FSPEC14_I021_ED0_155 0x02

#define FSPEC15_I021_ED0_157 0x80
#define FSPEC16_I021_ED0_160 0x40
#define FSPEC17_I021_ED0_165 0x20
#define FSPEC18_I021_ED0_170 0x10
#define FSPEC19_I021_ED0_095 0x08
#define FSPEC20_I021_ED0_032 0x04
#define FSPEC21_I021_ED0_200 0x02

#define FSPEC22_I021_ED0_020 0x80
#define FSPEC23_I021_ED0_220 0x40
#define FSPEC24_I021_ED0_146 0x20
#define FSPEC25_I021_ED0_148 0x10
#define FSPEC26_I021_ED0_110 0x08
#define FSPEC27_I021_ED0_070 0x04
#define FSPEC28_I021_ED0_131 0x02

#define FSPEC29_I021_ED0_XX  0x80
#define FSPEC30_I021_ED0_XX  0x40
#define FSPEC31_I021_ED0_XX  0x20
#define FSPEC32_I021_ED0_XX  0x10
#define FSPEC33_I021_ED0_XX  0x08
#define FSPEC34_I021_ED0_RE  0x04
#define FSPEC35_I021_ED0_SP  0x02

bool test_bit(uint8_t &val, uint8_t mask) {
   return val & mask ? true : false;
}

bool __Asterix021ed0_to_TargetAdsb (const void * data, int size,
	enum CAT021_EDITION edition, deque<RDFTargetAdsb> & adsb_list,
   std::stringstream * out_str = NULL, bool one_line= false)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int len;

	adsb_list.clear ();
	
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
	if (msg[0] != 21)
	{
		cms_error ("unknown category %u", msg[0]);
		return false;
	}

	len = (((unsigned)msg[1]) << 8) | msg[2];
	if (len < size)
	{
		cms_warning ("wrong length %u %u", len, size);
		size = len;
	}
	else if (len > size)
	{
		cms_error ("wrong length %u %u", len, size);
		goto error;
	}
	
	record = msg + 3;
	while (record < msg + len)
	{
		// vytvorime novou polozku
		RDFTargetAdsb adsb;
		
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
		if (*fspec & FSPEC01_I021_ED0_010)
		{
         if(out_str) {
            *out_str <<  "010 [sac=" << static_cast<int>(field[0])
                     << " sic=" << static_cast<int>(field[1]) << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			adsb.SAC = field[0];
			adsb.SIC = field[1];
			field += 2;
		}

		// Target Report Descriptor
		if (*fspec & FSPEC02_I021_ED0_040)
		{
         if(out_str) {

            switch (field[1] >> 5) {
               case 0: *out_str << "040 [atp=ICAO"; break;
               case 1: *out_str << "040 [atp=DUP"; break;
               case 2: *out_str << "040 [atp=SURF"; break;
               case 3: *out_str << "040 [atp=ANON"; break;
               default: *out_str << "040 [atp=reserved"; break;
    			}

				*out_str <<  static_cast<enum ALTITUDE_CAPABILITY>((field[1] >> 3) & 3)
               << "]";

            NEW_LINE(*out_str, 4, ' ');

            *out_str << "[DCR=" << test_bit(field[0], 0x80)
               << " GBS=" << test_bit(field[0], 0x40)
               << " SIM=" << test_bit(field[0], 0x20)
               << " TST=" << test_bit(field[0], 0x10)
               << " FXD=" << test_bit(field[0], 0x08)
               << " SAA=" << test_bit(field[0], 0x04)
               << " SPI=" << test_bit(field[0], 0x02)
               << "]";

               NEW_LINE(*out_str, 0, ' ');

         }

			adsb.DCR = (field[0] & 0x80) ? true : false;
			adsb.GBS = (field[0] & 0x40) ? true : false;
			adsb.Simulated = (field[0] & 0x20) ? true : false;
			adsb.Tested = (field[0] & 0x10) ? true : false;
			adsb.Fixed = (field[0] & 0x08) ? true : false;
			adsb.SAA = (field[0] & 0x04) ? true : false;
			adsb.SPI = (field[0] & 0x02) ? true : false;

			switch (field[1] >> 5)
			{
				case 0:
					adsb.AddressType = ADDRESS_TYPE_DUPLICATE;
					break;

				case 1:
					adsb.AddressType = ADDRESS_TYPE_ICAO;
					break;

				case 2:
					adsb.AddressType = ADDRESS_TYPE_SURFACE;
					break;

				case 3:
					adsb.AddressType = ADDRESS_TYPE_ANONYMOUS;
					break;

				default:
					adsb.AddressType = static_cast<enum ADDRESS_TYPE>(field[1] >> 5);
					break;
			}

			adsb.AltitudeCapability =
				static_cast<enum ALTITUDE_CAPABILITY>((field[1] >> 3) & 3);

			field += 2;
		}

		// Time of Day
		if (*fspec & FSPEC03_I021_ED0_030)
		{
         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;
            *out_str << "030 [time=" << time << " ";
            *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
               << ":" << std::setw(2)  << std::setfill('0')
               << ((time  % 3600) / 60) << ":" << std::setw(2) //m
               << std::setfill('0') << ((time % 3600) % 60) //s
               << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
               << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			adsb.TOAp.Present = true;
			adsb.TOAp.Time = (static_cast<long long>(field2unsigned (field, 24)) * 1000 + 64) / 128;
			field += 3;
		}

		// Position in WGS-84 co-ordinates
		if (*fspec & FSPEC04_I021_ED0_130)
		{
			if (edition == CAT021_EDITION_023)
			{
        if (!adsb.Geographic) {
				  adsb.Geographic = new RDFCoorGeographic ();
        }
				adsb.Geographic->Latitude = static_cast<double>(field2signed (field, 24))
						* M_PI / (1 << 23);
				adsb.Geographic->Longitude = static_cast<double>(field2signed (field + 3, 24))
						* M_PI / (1 << 23);
            				field += 6;
			}
			else
			{
        if (!adsb.Geographic) {
          adsb.Geographic = new RDFCoorGeographic ();
        }
				adsb.Geographic->Latitude = static_cast<double>(field2signed (field, 32))
						* M_PI / (1 << 25);
				adsb.Geographic->Longitude = static_cast<double>(field2signed (field + 4, 32))
						* M_PI / (1 << 25);
				field += 8;
			}

         if(out_str) {
               *out_str << "130 [WGS ";
               geo_to_sstr(adsb.Geographic->Latitude, adsb.Geographic->Longitude, *out_str);
               *out_str << "]";

               NEW_LINE(*out_str, 0, '\0');
         }
		}

		// Target Address
		if (*fspec & FSPEC05_I021_ED0_080)
		{
			adsb.AircraftAddress.Present = true;
			adsb.AircraftAddress.Address = field2unsigned (field, 24);
			field += 3;
		}

		// Geometric Altitude
		if (*fspec & FSPEC06_I021_ED0_140)
		{
			adsb.GeometricHeight = new RDFTargetHeight ();
			adsb.GeometricHeight->Height = field2signed (field, 16) * 6.25 * 0.3048;
			field += 2;
		}

		// Figure of Merit
		if (*fspec & FSPEC07_I021_ED0_090)
		{
			adsb.NUCp_NIC = field[1] & 0x0F;
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Link technology
		if (*fspec & FSPEC08_I021_ED0_210)
		{
			adsb.CDTI = (field[0] & 0x10) ? true : false;

			if (field[0] & 0x01) adsb.LinkTechnology = LINK_TYPE_OTHER;
			if (field[0] & 0x02) adsb.LinkTechnology = LINK_TYPE_VDL4;
			if (field[0] & 0x04) adsb.LinkTechnology = LINK_TYPE_UAT;
			if (field[0] & 0x08) adsb.LinkTechnology = LINK_TYPE_ES;

			field += 1;
		}

		// Roll Angle
		if (*fspec & FSPEC09_I021_ED0_230)
		{
			field += 2;
		}

		// Flight Level
		if (*fspec & FSPEC10_I021_ED0_145)
		{
			adsb.FlightLevel = new RDFTargetFlightLevel ();
			adsb.FlightLevel->Valid = true;
			adsb.FlightLevel->Height = field2signed (field, 16) * 25 * 0.3048;
			field += 2;
		}

		// Air Speed
		if (*fspec & FSPEC11_I021_ED0_150)
		{
			adsb.AirSpeed = new RDFTargetAirSpeed ();
			adsb.AirSpeed->Mach = (field[0] & 0x80) ? true : false;
			if (adsb.AirSpeed->Mach)
			{
				adsb.AirSpeed->Speed = field2unsigned (field, 15) * 0.001 * 340.3;
			}
			else
			{
				adsb.AirSpeed->Speed = static_cast<double>(field2unsigned (field, 15))
					* 1852 / (1 << 14);
			}
			field += 2;
		}

		// True Airspeed
		if (*fspec & FSPEC12_I021_ED0_151)
		{
			adsb.TrueAirspeed = new RDFTargetAirSpeed ();
			adsb.TrueAirspeed->Speed = field2unsigned (field, 16) * KNOT_TO_SPEED;
			field += 2;
		}

		// Magnetic Heading
		if (*fspec & FSPEC13_I021_ED0_152)
		{
			field += 2;
		}

		// Barometric Vertical Rate
		if (*fspec & FSPEC14_I021_ED0_155)
		{
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Geometric Vertical Rate
		if (*fspec & FSPEC15_I021_ED0_157)
		{
			field += 2;
		}

		// Ground Vector
		if (*fspec & FSPEC16_I021_ED0_160)
		{
			adsb.GroundSpeed = new RDFTargetGroundSpeed ();
			adsb.GroundSpeed->Speed = static_cast<double>(field2unsigned (field, 16))
					* 1852 / (1 << 14);
			adsb.GroundSpeed->Heading = static_cast<double>(field2unsigned (field + 2, 16))
					* (2 * M_PI) / (1 << 16);
			field += 4;
		}

		// Rate of Tum
		if (*fspec & FSPEC17_I021_ED0_165)
		{
			while (field[0] & 1)
			{
				field += 1;
			}
			field += 1;
		}

		// Target Indetification
		if (*fspec & FSPEC18_I021_ED0_170)
		{
			asterix_decode_callsign (field, adsb.AircraftIdentification);
			field += 6;
		}

		// Velocity Accuracy
		if (*fspec & FSPEC19_I021_ED0_095)
		{
			adsb.NUCr_NACv = field[0];
			field += 1;
		}

		// Time of day accuracy
		if (*fspec & FSPEC20_I021_ED0_032)
		{
			field += 1;
		}

		// Target Status
		if (*fspec & FSPEC21_I021_ED0_200)
		{
			// 0.23 defines emergency states up to 5. When higher number is
			// received, we use 'General emergency' and issue a warning
			if (field[0] > 5)
			{
				adsb.PriorityStatus = PRIORITY_STATUS_GENERAL_EMERGENCY;
				cms_error ("Decoded invalid I021/200 value %d (ed 0.23 defines values up to 5)",
					field[0]);
			}
			else
			{
				adsb.PriorityStatus =
					static_cast<enum PRIORITY_STATUS>(field[0]);
			}
			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Emitter Category
		if (*fspec & FSPEC22_I021_ED0_020)
		{
			field += 1;
		}

		// Met report
		if (*fspec & FSPEC23_I021_ED0_220)
		{
			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
				goto error;
			}

			unsigned char *subfield = field + 1;

			// wind speed subfield
			if (field[0] & 0x80)
				subfield += 2;

			// wind direction
			if (field[0] & 0x40)
				subfield += 2;

			// temperature
			if (field[0] & 0x20)
				subfield += 2;

			// turbulence
			if (field[0] & 0x10)
				subfield += 1;

			field = subfield;
		}

		// Intermediate State Selected Altitude
		if (*fspec & FSPEC24_I021_ED0_146)
		{
			field += 2;
		}

		// Final State Selected Altitude
		if (*fspec & FSPEC25_I021_ED0_148)
		{
			field += 2;
		}

		// Trajectory Intent
		if (*fspec & FSPEC26_I021_ED0_110)
		{
			unsigned char *subfield = field;

			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
				goto error;
			}

			subfield += 1;

			// skip subfield 1
			if (field[0] & 0x80)
			{
				if (subfield[0] & 1)
				{
					cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
					goto error;
				}

				subfield += 1;
			}

			// skip subfield 2
			if (field[0] & 0x40)
			{
				subfield += 1 + subfield[0] * 15;
			}

			field = subfield;
		}

		// Mode 3/A Code in Octal Representation
		if (*fspec & FSPEC27_I021_ED0_070)
		{
			if (edition == CAT021_EDITION_023)
			{
				cms_error ("unable to handle spare bits (sac %u, sic %u)",
					adsb.SAC, adsb.SIC);
				goto error;
			}
			else
			{
				field += 2;
			}
		}

		// Signal Amplitude
		if (*fspec & FSPEC28_I021_ED0_131)
		{
			if (edition == CAT021_EDITION_023)
			{
				cms_error ("unable to handle spare bits (sac %u, sic %u)",
					adsb.SAC, adsb.SIC);
				goto error;
			}
			else
			{
				field += 1;
			}
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// spare
		if (*fspec & FSPEC29_I021_ED0_XX)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC30_I021_ED0_XX)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC31_I021_ED0_XX)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC32_I021_ED0_XX)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC33_I021_ED0_XX)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// Reserved Expansion Field
		if (*fspec & FSPEC34_I021_ED0_RE)
		{
			adsb.RE_data.assign (field + 1, field + field[0]);
			field += field[0];
		}

		// Special Purpose Field
		if (*fspec & FSPEC35_I021_ED0_SP)
		{
			adsb.SP_data.assign (field + 1, field + field[0]);
			field += field[0];
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		cms_error ("unable to handle more fspec (sac %u, sic %u)",
			adsb.SAC, adsb.SIC);
		goto error;

end:
		// pridame polozku do seznamu
		adsb_list.push_back (adsb);

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error ("wrong packet length (%u > %u) (sac %u, sic %u)",
			record - msg, size,
			adsb_list.back().SAC, adsb_list.back().SIC);
		goto error;
	}

	return true;

error:
	cms_data (size, data);
	adsb_list.clear ();
	return false;
}

#define FSPEC_FX 0x01

#define FSPEC01_I021_ED1_010 0x80
#define FSPEC02_I021_ED1_040 0x40
#define FSPEC03_I021_ED1_161 0x20
#define FSPEC04_I021_ED1_015 0x10
#define FSPEC05_I021_ED1_071 0x08
#define FSPEC06_I021_ED1_130 0x04
#define FSPEC07_I021_ED1_131 0x02

#define FSPEC08_I021_ED1_072 0x80
#define FSPEC09_I021_ED1_150 0x40
#define FSPEC10_I021_ED1_151 0x20
#define FSPEC11_I021_ED1_080 0x10
#define FSPEC12_I021_ED1_073 0x08
#define FSPEC13_I021_ED1_074 0x04
#define FSPEC14_I021_ED1_075 0x02

#define FSPEC15_I021_ED1_076 0x80
#define FSPEC16_I021_ED1_140 0x40
#define FSPEC17_I021_ED1_090 0x20
#define FSPEC18_I021_ED1_210 0x10
#define FSPEC19_I021_ED1_070 0x08
#define FSPEC20_I021_ED1_230 0x04
#define FSPEC21_I021_ED1_145 0x02

#define FSPEC22_I021_ED1_152 0x80
#define FSPEC23_I021_ED1_200 0x40
#define FSPEC24_I021_ED1_155 0x20
#define FSPEC25_I021_ED1_157 0x10
#define FSPEC26_I021_ED1_160 0x08
#define FSPEC27_I021_ED1_165 0x04
#define FSPEC28_I021_ED1_077 0x02

#define FSPEC29_I021_ED1_170 0x80
#define FSPEC30_I021_ED1_020 0x40
#define FSPEC31_I021_ED1_220 0x20
#define FSPEC32_I021_ED1_146 0x10
#define FSPEC33_I021_ED1_148 0x08
#define FSPEC34_I021_ED1_110 0x04
#define FSPEC35_I021_ED1_016 0x02

#define FSPEC36_I021_ED1_008 0x80
#define FSPEC37_I021_ED1_271 0x40
#define FSPEC38_I021_ED1_132 0x20
#define FSPEC39_I021_ED1_250 0x10
#define FSPEC40_I021_ED1_260 0x08
#define FSPEC41_I021_ED1_400 0x04
#define FSPEC42_I021_ED1_295 0x02

#define FSPEC43_I021_ED1_SPA 0x80
#define FSPEC44_I021_ED1_SPA 0x40
#define FSPEC45_I021_ED1_SPA 0x20
#define FSPEC46_I021_ED1_SPA 0x10
#define FSPEC47_I021_ED1_SPA 0x08
#define FSPEC48_I021_ED1_RE  0x04
#define FSPEC49_I021_ED1_SP  0x02

static uint8_t popcount (uint32_t x)
{
	uint8_t count;
	for (count = 0; x; ++count)
		x &= x - 1;
	
	return count;
}
static const char *PS_to_text[] = {"no emergency",
							"general emergency",
							"lifeguard/medical emergency",
							"minimum fuel",
							"no communications",
							"unlawful interference",
							"\"downed\" aircraft"};

static const char *SS_to_text[] = {
   "no condition reported",
   "permanent alert",
   "temporary alert",
   "SPI set"
};
static const char *TC_to_text[] = {
   "none",
   "TC+0",
   "multiple",
   "reserved"
};


static const char *VN_to_text[] = {
   "ED102/DO-260",
	"DO-260A",
	"ED102A/DO-260B"
};
static const char *LT_to_text[] = {
   "other",
   "UAT",
   "1090 ES",
   "VDL 4",
   "not assigned"
};
static const char *ecat_to_text[] = {	
   "no information",
   "light aircraft",
   "small aircraft",
   "medium aircraft",
   "high vortex large",
   "heavy aircraft",
   "highly manoeuvrable and high speed",
   "reserved",
   "reserved",
   "reserved",
   "rotocraft",
   "glider / sailplane",
   "lighter-than-air",
   "unmanned aerial vehicle",
   "space / transatmospheric vehicle",
   "ultralight / handglider / paraglider",
   "parachutist / skydiver",
   "reserved",
   "reserved",
   "reserved",
   "surface emergency vehicle",
   "surface service vehicle",
   "fixed ground or tethered obstruction",
   "cluster obstacle",
   "line obstacle",
   "unknown"
};


bool __Asterix021ed1_to_TargetAdsb (const void * data, int size,
	enum CAT021_EDITION edition, deque<RDFTargetAdsb> & adsb_list
   ,std::stringstream *out_str, bool one_line)
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int len;

	adsb_list.clear ();
	
	if (!data || size > 4096)
	{
		cms_error ("wrong packet %x %u", data, size);
		return false;
	}

	msg = (unsigned char *) data;
	if (msg[0] != 21)
	{
		cms_error ("unknown category %u", msg[0]);
		return false;
	}

	len = (((unsigned) msg[1]) << 8) | msg[2];
	if (len < size)
	{
		cms_warning ("wrong length %u %u", len, size);
		size = len;
	}
	else if (len > size)
	{
		cms_error ("wrong length %u %u", len, size);
		goto error;
	}
	
	record = msg + 3;
	while (record < msg + len)
	{
		// vytvorime novou polozku
		RDFTargetAdsb adsb;

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
		if (*fspec & FSPEC01_I021_ED1_010)
		{
         if(out_str) {
            *out_str <<  "010 [sac=" << static_cast<int>(field[0])
                     << " sic=" << static_cast<int>(field[1]) << "]";

            NEW_LINE(*out_str, 0, '\0');
         }


			adsb.SAC = field[0];
			adsb.SIC = field[1];
			field += 2;
		}

		// Target Report Descriptor
		if (*fspec & FSPEC02_I021_ED1_040)
		{
         if(out_str) {
            switch ((field[0] >> 5) & 0x7) {
               case 0: *out_str << "040 [ICAO "; break;
               case 1: *out_str << "040 [DUP "; break;
               case 2: *out_str << "040 [SURF "; break;
               case 3: *out_str << "040 [ANON "; break;
               default: *out_str << "040 [RESERV "; break;
    			}

            switch ((field[0] >> 3) & 0x3) {
               case 0: *out_str << " 25 "; break;
               case 1: *out_str << " 100 "; break;
               case 2: *out_str << " UNK "; break;
               case 3: *out_str << " INV "; break;
            }
            *out_str << ((field[0] & 0x04) ? " RC " : " - ")
               << ((field[0] & 0x02) ? " FIX" : " AIR") << "]";
         }

			adsb.AddressType = static_cast<enum ADDRESS_TYPE>(field[0] >> 5);
			adsb.AltitudeCapability = static_cast<enum ALTITUDE_CAPABILITY>((field[0] >> 3) & 0x3);
			adsb.RC  = (field[0] & 0x4) ? true : false;
			adsb.Fixed = (field[0] & 0x2) ? true : false;

			if (field[0] & 1)
			{
				field += 1;
            if(out_str) {

             NEW_LINE(*out_str, 4, ' ');

             *out_str << "[DCR=" << test_bit(field[0], 0x80)
               << " GBS=" << test_bit(field[0], 0x40)
               << " SIM=" << test_bit(field[0], 0x20)
               << " TST=" << test_bit(field[0], 0x10)
               << " FXD=" << test_bit(field[0], 0x08)
               << " SAA=" << test_bit(field[0], 0x04)
               << " SPI=" << test_bit(field[0], 0x02)
               << "]";

          		switch ((field[0] >> 1) & 0x3) {
					   case 0: *out_str << " cl=valid]"; break;
					   case 1: *out_str << " cl=suspect]"; break;
					   case 2: *out_str << " cl=no info]"; break;
					   case 3: *out_str << " cl=reserved]"; break;
				   }

            }

				adsb.DCR = (field[0] & 0x80) ? true : false;
				adsb.GBS = (field[0] & 0x40) ? true : false;
				adsb.Simulated = (field[0] & 0x20) ? true : false;
				adsb.Tested = (field[0] & 0x10) ? true : false;
				adsb.SAA = (field[0] & 0x8) ? true : false;
				adsb.ConfidenceLevel = static_cast<enum CONFIDENCE_LEVEL>((field[0] >> 1) & 0x3);
			}

			if (field[0] & 1)
			{
				field += 1;

            if(out_str) {

               NEW_LINE(*out_str, 4, ' ');

               *out_str << " IPC=" << ((field[0] & 0x20 )? "FAIL" : "OK")
                  << ((field[0] & 0x10 )? " NOGO" : "")
                  << " CPR=" << ((field[0] & 0x08 )? "FAIL" : "OK")
                  << ((field[0] & 0x04 )? " LDPJ" : "")
                  << " RCF=" << ((field[0] & 0x02 )? "FAIL" : "OK")
                  << "]";

            }

				adsb.IPC = (field[0] & 0x20) ? true : false;
				adsb.NOGO = (field[0] & 0x10) ? true : false;
				adsb.CPR = (field[0] & 0x8) ? true : false;
				adsb.LDPJ = (field[0] & 0x4) ? true : false;
				adsb.RCF = (field[0] & 0x2) ? true : false;
			}

         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
         }

			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)",
					adsb.SAC, adsb.SIC);
				goto error;
			}

			field += 1;
		}

		// Target ID
		if (*fspec & FSPEC03_I021_ED1_161)
		{
         if(out_str) {
            *out_str <<  "161 [track=" << field2unsigned(field, 12) << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			adsb.TrackNumber.Present = true;
			adsb.TrackNumber.Number = field2unsigned (field, 12);
			field += 2;
		}

		// Service Identification
		if (*fspec & FSPEC04_I021_ED1_015)
		{
         if(out_str) {
            *out_str << "015 [sid=" << static_cast<int>(field[0]) << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		// Time of Applicability for Position
		if (*fspec & FSPEC05_I021_ED1_071)
		{
         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;
            *out_str << "071 [time=" << time << " ";
            *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
               << ":" << std::setw(2)  << std::setfill('0')
               << ((time  % 3600) / 60) << ":" << std::setw(2) //m
               << std::setfill('0') << ((time % 3600) % 60) //s
               << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
               << "]";

            NEW_LINE(*out_str, 0, '\0');
         }


			adsb.TOAp.Present = true;
			adsb.TOAp.Time = (static_cast<long long>(field2unsigned (field, 24)) * 1000 + 64) / 128;
			field += 3;
		}

		// Position in WGS-84 coordinates
		if (*fspec & FSPEC06_I021_ED1_130)
		{
      if (!adsb.Geographic) {
			 adsb.Geographic = new RDFCoorGeographic ();
			}
			adsb.Geographic->Latitude = static_cast<double>(field2signed (field, 24))
					* M_PI / (1 << 23);
			adsb.Geographic->Longitude = static_cast<double>(field2signed (field + 3, 24))
					* M_PI / (1 << 23);
			adsb.HRPosition = false;

         if(out_str) {

            *out_str << "130 [WGS";

            geo_to_sstr(adsb.Geographic->Latitude, adsb.Geographic->Longitude, *out_str);
            *out_str << "]";
            NEW_LINE(*out_str, 0, '\0');
         }

			field += 6;
		}

		// Position in WGS-84 coordinates, high res.
		if (*fspec & FSPEC07_I021_ED1_131)
		{
			if (!adsb.Geographic) {
        adsb.Geographic = new RDFCoorGeographic ();
      }
			adsb.Geographic->Latitude = static_cast<double>(field2signed (field, 32))
					* M_PI / (1 << 30);
			adsb.Geographic->Longitude = static_cast<double>(field2signed (field + 4, 32))
					* M_PI / (1 << 30);
			adsb.HRPosition = true;

         if(out_str) {
             *out_str << "131 [HP WGS";

            geo_to_sstr(adsb.Geographic->Latitude, adsb.Geographic->Longitude, *out_str);
            *out_str << "]";
            NEW_LINE(*out_str, 0, '\0');

         }

			field += 8;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Time of Applicability for Velocity
		if (*fspec & FSPEC08_I021_ED1_072)
		{
			adsb.TOAv.Present = true;
			adsb.TOAv.Time = (static_cast<long long>(field2unsigned (field, 24)) * 1000 + 64) / 128;

         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;
            *out_str << "072 [time=" << time << " ";
            *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
               << ":" << std::setw(2)  << std::setfill('0')
               << ((time  % 3600) / 60) << ":" << std::setw(2) //m
               << std::setfill('0') << ((time % 3600) % 60) //s
               << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
               << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 3;
		}

		// Air Speed
		if (*fspec & FSPEC09_I021_ED1_150)
		{
			adsb.AirSpeed = new RDFTargetAirSpeed ();
			adsb.AirSpeed->Mach = (field[0] & 0x80) ? true : false;

         int speed = field2unsigned(field, 15);

			if (adsb.AirSpeed->Mach)
         {
            if(out_str) {
              *out_str << "150 [air speed=" << speed * 0.0001 << "M]";

              NEW_LINE(*out_str, 0, '\0');
            }

				adsb.AirSpeed->Speed = field2unsigned (field, 15) * 0.001 * 340.3;
			}
			else
			{
            if(out_str) {
               *out_str << "150 [air speed= "
					<< (double) speed / 0x4000 * 3600 << "kt ("
					<< (double) speed / 0x4000 * 1852 << "m/s)]";

               NEW_LINE(*out_str, 0, '\0');
            }

				adsb.AirSpeed->Speed = static_cast<double>(field2unsigned (field, 15))
					* 1852 / (1 << 14);
			}
			field += 2;
		}

		// True Air Speed
		if (*fspec & FSPEC10_I021_ED1_151)
		{
         if(out_str) {
            int speed = field2unsigned(field,15);

            *out_str << "151 [true air speed=" << speed << "kt ("
				<< speed * 1852.0 / 3600 << " m/s)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			adsb.TrueAirspeed = new RDFTargetAirSpeed ();
			adsb.TrueAirspeed->Speed = field2unsigned (field, 15) * KNOT_TO_SPEED;
			adsb.TrueAirspeed->RE = (field[0] & 0x80) ? true : false;
			field += 2;
		}

		// Target Address
		if (*fspec & FSPEC11_I021_ED1_080)
		{
         if(out_str) {
            *out_str << "080 [addr_hex=" << std::hex << std::noshowbase << std::setw(6)
   				<<field2unsigned(field, 24) << std::dec << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			adsb.AircraftAddress.Present = true;
			adsb.AircraftAddress.Address = field2unsigned (field, 24);
			field += 3;
		}

		// Time of Message Reception of Position
		if (*fspec & FSPEC12_I021_ED1_073)
		{
         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;
            *out_str << "073 [time=" << time << " ";
            *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
               << ":" << std::setw(2)  << std::setfill('0')
               << ((time  % 3600) / 60) << ":" << std::setw(2) //m
               << std::setfill('0') << ((time % 3600) % 60) //s
               << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
               << "]";

            NEW_LINE(*out_str, 0, '\0');
         }


			adsb.TOMRp.Present = true;
			adsb.TOMRp.Time = (static_cast<long long>(field2unsigned (field, 24)) * 1000 + 64) / 128;
			field += 3;
		}

		// Time of Message Reception of Position - High Precision
		if (*fspec & FSPEC13_I021_ED1_074)
		{
         if(out_str) {
     			switch (field[0] >> 6) {
               case 0: *out_str << "074 [FSI=0 "; break;
               case 1: *out_str << "074 [FSI=1 "; break;
               case 2: *out_str << "074 [FSI=-1 "; break;
               case 3: *out_str << "074 [FSI=? "; break;
   			}

   			double frac = field2unsigned(field, 26) * 1e9 / 0x40000000;
   			*out_str << frac << " ns]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 4;
		}

		// Time of Message Reception of Velocity
		if (*fspec & FSPEC14_I021_ED1_075)
		{
			adsb.TOMRv.Present = true;
			adsb.TOMRv.Time = (static_cast<long long>(field2unsigned (field, 24)) * 1000 + 64) / 128;

         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;
            *out_str << "073 [time=" << time << " ";
            *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
               << ":" << std::setw(2)  << std::setfill('0')
               << ((time  % 3600) / 60) << ":" << std::setw(2) //m
               << std::setfill('0') << ((time % 3600) % 60) //s
               << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
               << "]";

            NEW_LINE(*out_str, 0, '\0');
         }


			field += 3;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Time of Message Reception of Velocity - High Precision
		if (*fspec & FSPEC15_I021_ED1_076)
		{
         if(out_str) {
     			switch (field[0] >> 6) {
               case 0: *out_str << "076 [FSI=0 "; break;
               case 1: *out_str << "076 [FSI=1 "; break;
               case 2: *out_str << "076 [FSI=-1 "; break;
               case 3: *out_str << "076 [FSI=? "; break;
   			}

   			double frac = field2unsigned(field, 26) * 1e9 / 0x40000000;
   			*out_str << frac << " ns]";

            NEW_LINE(*out_str, 0, '\0');
         }


			field += 4;
		}

		// Geometric Height
		if (*fspec & FSPEC16_I021_ED1_140)
		{
			adsb.GeometricHeight = new RDFTargetHeight ();
			adsb.GeometricHeight->Height = field2signed (field, 16) * 6.25 * 0.3048;

         if(out_str) {
            int16_t h = field2signed(field, 16);

            *out_str << "140 [height " << std::setprecision (2) << h * 6.25
               << " ft (" << std::setprecision (3) << h * 6.25 * 0.3048
               << " m)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 2;
		}

		// Quality Indicators
		if (*fspec & FSPEC17_I021_ED1_090)
		{
         if(out_str) {
            *out_str << "090 [NUCr/NACv=" << static_cast<int>(field[0] >> 5)
               << " NUCp/NIC=" << static_cast<int>((field[0] >> 1) & 0xF);
         }

			adsb.NUCr_NACv = field[0] >> 5;
			adsb.NUCp_NIC = (field[0] >> 1) & 0x0f;

			if (field[0] & 1)
			{
				field += 1;

            if(out_str) {
            	*out_str << " NICbaro=" << ((field[0] & 0x80) == 0x80)
                  << " SIL=" << static_cast<int>((field[0] >> 5) & 0x3)
                  <<	" NACp=" << static_cast<int>((field[0] >> 1) & 0xF)
                  << "]";

            }

				adsb.NICbaro = (field[0] >> 7);
				adsb.SIL = (field[0] >> 5) & 0x3;
				adsb.NACp = (field[0] >> 1) & 0x0f;
			}

			if (field[0] & 1)
			{
				field += 1;

            if(out_str) {

               NEW_LINE(*out_str, 4, ' ');

               *out_str << "[ SILs="
                  << (((field[0] & 0x20) == 0x20) ? "per sample" : "per flight-hour")
               << " SDA=" << static_cast<int>((field[0] >> 3) & 0x3)
               << " GVA=" << static_cast<int>((field[0] >> 1) & 0x3)
               << "]";
            }

				adsb.SILs = (field[0] >> 5) & 0x1;
				adsb.SDA = (field[0] >> 3) & 0x3;
				adsb.GVA = (field[0] >> 1) & 0x3;
			}

			if (field[0] & 1)
			{
				field += 1;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

        			*out_str << "[PIC=" << static_cast<int>((field[0] >> 4) & 0xF)
               << "]";
            }

				adsb.PIC = (field[0] >> 4) & 0x0f;
			}

         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
         }

			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
				goto error;
			}

			field += 1;
		}

		// MOPS Version
		if (*fspec & FSPEC18_I021_ED1_210)
		{
			adsb.VNS = (field[0] & 0x40) ? true : false;
			adsb.VersionNumber = (field[0] >> 3) & 0x7;
			adsb.LinkTechnology = static_cast<enum TECHNOLOGY_TYPE>(field[0] & 0x7);
         if(out_str) {
            uint8_t ver = (field[0] >> 3) & 0x7;
			   if (ver > 1) ver = 2;

			   uint8_t lt = (field[0] & 0x7);
			   if (lt > 3) lt = 4;

			   *out_str << "210 [ver " << VN_to_text[ver] << ":"
				<< ((field[0] & 0x40) ? " not supported" : " supported")
				<< ", technology " << LT_to_text[lt] << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		// Mode 3/A Code
		if (*fspec & FSPEC19_I021_ED1_070)
		{
			adsb.Mode3A = new RDFTargetMode ();
			adsb.Mode3A->Valid = true;
			adsb.Mode3A->Code = field2unsigned (field, 12);

         if(out_str) {

            *out_str <<  "070 [M3A_oct=" << std::oct << std::setw(4)
                     << std::fixed << std::setfill('0') << field2unsigned (field, 12)
                     << std::dec << std::setfill(' ') << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 2;
		}

		// Roll Angle
		if (*fspec & FSPEC20_I021_ED1_230)
		{
         if(out_str) {
   			double angle = field2signed(field, 16) * 0.01;
	   		const char *wing = (angle < 0) ? "left" : "right";

   			*out_str << "230 [angle=" << angle << " deg (" << wing
               << " wing down)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 2;
		}

		// Flight Level
		if (*fspec & FSPEC21_I021_ED1_145)
		{
			adsb.FlightLevel = new RDFTargetFlightLevel ();
			adsb.FlightLevel->Valid = true;
			adsb.FlightLevel->Height = field2signed (field, 16) * 25 * 0.3048;

         if(out_str) {
            int16_t fl = field2signed(field, 16);
            *out_str << "145 [fl " << std::setprecision (2)
                     << fl * 0.25 << " (" << std::setprecision (3)
                     << fl * 25 * 0.3048 << " m)]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Magnetic Heading
		if (*fspec & FSPEC22_I021_ED1_152)
		{
         if(out_str) {
            double heading = (double)field2unsigned (field, 16) / 0x10000 * 360;
   			*out_str << "152 [heading=" << heading << " deg]";

            NEW_LINE(*out_str, 0, '\0');
         }

			// heading = (double) field2unsigned (field, 16) * M_PI / 180 / 0x10000;
			field += 2;
		}

		// Target Status
		if (*fspec & FSPEC23_I021_ED1_200)
		{
         if(out_str) {
            *out_str << "200 [";

            *out_str << ((field[0] & 0x80) ? "ICF ": " - ")
                     << ((field[0] & 0x40) ? "LNAV" : " - ");

            uint8_t ps = (field[0] >> 2) & 0x7;
            uint8_t ss = (field[0] & 0x3);

			   *out_str << "PS=\'" << (ps > 6 ? "invalid" : PS_to_text[ps])
               << "\', SS=\'" << (ss > 3 ? "invalid" : SS_to_text[ss])
               << "\']";

            NEW_LINE(*out_str, 0, '\0');
         }

			adsb.ICF = (field[0] & 0x80) ? true : false;
			adsb.LNAV = (field[0] & 0x40) ? true : false;
			adsb.PriorityStatus =
				static_cast<enum PRIORITY_STATUS>((field[0] >> 2) & 0x7);
			adsb.SurveillanceStatus =
				static_cast<enum SURVEILLANCE_STATUS>(field[0] & 0x3);
			field += 1;
		}

		// Barometric Vertical Rate
		if (*fspec & FSPEC24_I021_ED1_155)
		{
         if(out_str) {
            *out_str << ((field[0] & 0x80 )? "155 [RE" : "155 [");

            double rate = field2signed (field, 15) * 6.25;
   			*out_str << " rate=" << rate << " ft/m (" << rate * 0.3048 / 60.
               << " ms)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 2;
		}

		// Geometric Vertical Rate
		if (*fspec & FSPEC25_I021_ED1_157)
		{
         if(out_str) {
            *out_str << ((field[0] & 0x80) ? "157 [RE" : "157 [ -");

            double rate = field2signed (field, 15) * 6.25;
    			*out_str << " rate=" << rate << " ft/m (" << rate * 0.3048 / 60.
                     << " ms)]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 2;
		}

		// Ground Vector
		if (*fspec & FSPEC26_I021_ED1_160)
		{
         if(out_str) {
            *out_str << ((field[0] & 0x80) ? "160 [RE" : "160 [ - ");
			
            int speed = field2unsigned(field, 15);
            double heading = (double)field2unsigned(field + 2, 16)/ 0x10000 * 360;

   			*out_str << " speed="
	         			<< (double) speed / 0x4000 * 3600 << " kt ("
				         << (double) speed / 0x4000 * 1852 << " m/s)"
         				<< " heading=" << heading << " deg]";

            NEW_LINE(*out_str, 0, '\0');
         }

			adsb.GroundSpeed = new RDFTargetGroundSpeed ();
			adsb.GroundSpeed->Speed = static_cast<double>(field2unsigned (field, 15))
					* 1852 / (1<<14);
			adsb.GroundSpeed->Heading = static_cast<double>(field2unsigned (field + 2, 16))
					* (2 * M_PI) / (1<<16);
			field += 4;
		}

		// Track Angle Rate
		if (*fspec & FSPEC27_I021_ED1_165)
		{
         if(out_str) {
           	*out_str << "165 [rate=" << (field2signed (field, 10) / 32.)
                     << " deg/s]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 2;
		}

		// Time of Message Transmission
		if (*fspec & FSPEC28_I021_ED1_077)
		{
			adsb.Time.Present = true;
			adsb.Time.Time = (static_cast<long long>(field2unsigned (field, 24)) * 1000 + 64) / 128;

         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;
            *out_str << "077 [time=" << time << " ";
            *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
               << ":" << std::setw(2)  << std::setfill('0')
               << ((time  % 3600) / 60) << ":" << std::setw(2) //m
               << std::setfill('0') << ((time % 3600) % 60) //s
               << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
               << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 3;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Target Indentification
		if (*fspec & FSPEC29_I021_ED1_170)
		{
         if(out_str) {
            std::string callsign("");

            asterix_decode_callsign (field, callsign);

            *out_str << "170 [callsign=\"" << callsign << "\"]";

            NEW_LINE(*out_str, 0, '\0');
         }

			asterix_decode_callsign (field, adsb.AircraftIdentification);
			field += 6;
		}

		// Emitter Category
		if (*fspec & FSPEC30_I021_ED1_020)
		{
         if(out_str) {
            if (field[0] > 24)
				   *out_str << "020 [emitter category '" << ecat_to_text[25] << "' ("
					       << static_cast<int>(field[0]) << ")]";
   			else
	   			*out_str << "020 [emitter category '" << ecat_to_text[field[0]] << "']";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		// Met Information
		if (*fspec & FSPEC31_I021_ED1_220)
		{
			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
				goto error;
			}

			unsigned char *subfield = field + 1;

         if(out_str) {
         	*out_str << "220 [present";

   			if (field[0] & 0x80)
               *out_str << " WS";
            else
               *out_str << " - ";

	   		if (field[0] & 0x40)
               *out_str << " WD";
            else
               *out_str << " - ";

		   	if (field[0] & 0x20)
               *out_str << " TMP";
            else
               *out_str << " - ";

			   if (field[0] & 0x10)
               *out_str << " TRB]";
            else
               *out_str << " -] ";
         }

			// wind speed subfield
			if (field[0] & 0x80) {
            if(out_str) {
               NEW_LINE(*out_str, 4,  ' ');

         		int speed = field2unsigned(field, 16);
   				*out_str << "[wind speed=" << speed << " kt ("
					         << std::setprecision (1) << (double) speed * 1852. / 3600
          					<< " ms)]";
            }

				subfield += 2;
         }
			// wind direction
			if (field[0] & 0x40) {
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

            	int dir = field2unsigned(field, 16);
   				*out_str << "[wind direction=" << dir << " deg]";
            }

				subfield += 2;
         }
			// temperature
			if (field[0] & 0x20) {
            if(out_str) {
               NEW_LINE(*out_str, 4,  ' ');

               double temp = field2signed(field, 16) / 4.0;
   				*out_str << "[temperature=" << std::setprecision (2)
                  << temp << " deg]";
            }
				subfield += 2;
         }
			// turbulence
			if (field[0] & 0x10) {
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               *out_str << "[turbulence=" << static_cast<int>(subfield[0])
               << "]";
            }
				subfield += 1;
         }

         NEW_LINE(*out_str, 0, '\0');

			field = subfield;
		}

		// Intermediate State Selected Altitude
		if (*fspec & FSPEC32_I021_ED1_146)
		{
         if(out_str) {
           *out_str << ((field[0] & 0x80 )? "146 [SAS=" : "146 [ -");

            switch ((field[0] >> 5) & 0x3) {
               case 0: *out_str << " UNK"; break;
               case 1: *out_str << " AIR"; break;
               case 2: *out_str << " MCP/FCU"; break;
               case 3: *out_str << " FMS"; break;
            }

            uint32_t alt = field2signed (field, 13) * 25;
            *out_str << " altitude=" << alt << " ft (" << alt * 0.3048
                     << " m)]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 2;
		}

		// Final State Selected Altitude
		if (*fspec & FSPEC33_I021_ED1_148)
		{
         if(out_str) {
            *out_str << ((field[0] & 0x80) ? "148 [MV" : "148 [ - ")
                     << ((field[0] & 0x40) ? " AH" : " - ")
                     << ((field[0] & 0x20) ? " AM" : " - ");

			   uint32_t alt = field2signed (field, 13) * 25;
   			*out_str << " altitude=" << alt << " ft (" << alt * 0.3048
         				<< " m)]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 2;
		}

		// Trajectory Intent
		if (*fspec & FSPEC34_I021_ED1_110)
		{
			unsigned char *subfield = field;

			if (field[0] & 1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
				goto error;
			}

			subfield += 1;

			// skip subfield 1
			if (field[0] & 0x80)
			{
				if (subfield[0] & 1)
				{
					cms_error ("unable to handle more extents (sac %u, sic %u)", adsb.SAC, adsb.SIC);
					goto error;
				}

				subfield += 1;
			}

			// skip subfield 2
			if (field[0] & 0x40)
			{
				subfield += 1 + subfield[0] * 15;
			}

			field = subfield;
		}

		// Service Management
		if (*fspec & FSPEC35_I021_ED1_016)
		{
         if(out_str) {
         	switch (field[0]) {
   				case 0:
		            *out_str << "016 [data driven mode]"; break;
	   			case 0xFF:
            		 *out_str << "016 [period >= 127.5 s]"; break;
		   		default:
            		 *out_str << "016 [period=" << static_cast<int>(field[0] / 2.)
                          << " s]";
			   }

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// Aircraft Operational Status
		if (*fspec & FSPEC36_I021_ED1_008)
		{
         if(out_str) {
         	uint8_t tc = (field[0] >> 5) & 0x3;

   			*out_str << " TC=" << TC_to_text[tc];

            *out_str << ((field[0] & 0x10) ? " TS" : " - ")
                     << ((field[0] & 0x08) ? " ARV" : " - ")
                     << ((field[0] & 0x04) ? " CDTI/1" : " - ")
                     << ((field[0] & 0x02) ? " !TCAS" : " - ")
                     << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		// Surface Capabilities and Characteristics
		if (*fspec & FSPEC37_I021_ED1_271)
		{
         if(out_str) {
            *out_str << ((field[0] & 0x02) ? " POA" : " - ")
                     << ((field[0] & 0x10) ? " CDTI/S" : " - ")
                     << ((field[0] & 0x08) ? " B2low" : " - ")
                     << ((field[0] & 0x04) ? " RAW" : " - ")
                     << ((field[0] & 0x02) ? " IDENT" : " - ");
         }

			if (*field & 1)
			{
				field += 1;
			}

         if(out_str) {
            *out_str << " L+W=" << (field[0]  & 0xf) << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		// Message Amplitude
		if (*fspec & FSPEC38_I021_ED1_132)
		{
         if(out_str) {

     			int tmp = static_cast<int8_t>(field[0]);
	   		*out_str << "132 [mam=" << tmp << " dBm]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 1;
		}

		// Mode S MB Data
		if (*fspec & FSPEC39_I021_ED1_250)
		{
			int rep = field[0];
         if(out_str) {
           *out_str << "250 [MODE_S rep=" << rep << "]";
         }

			field += 1;
			for (int i = 0; i < rep; i++)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

            	*out_str << "[mb_data_hex=" << std::setfill ('0')
                        << std::hex << std::noshowbase;

               for(int j = 0; j< 7; j++) {
                  *out_str << std::setw (2) << static_cast<int>(field[j])
                           << " ";
               }

		         *out_str << std::setw (2) << std::hex << " addr_hex="
                        << static_cast<int>(field[7]) << "]";
            }

				// vytvorime novou polozku
				RDFTargetModeS modes;
				memcpy (modes.MessageData, field, 7);
				modes.Address = field[7];
				adsb.ModeS.push_back (modes);
				field += 8;
			}

         if(out_str) {
             *out_str << std::setfill(' ');

            NEW_LINE(*out_str, 0, '\0');
         }
		}

		// ACAS Resolution Advisory Report
		if (*fspec & FSPEC40_I021_ED1_260)
		{
         if(out_str) {
           	*out_str << "260 [acas=" << std::hex;

   			for (int i = 0; i < 6; ++i)
   	      	*out_str << std::setw (2) << field[i] << " ";

   			*out_str << std::setw (2) << field[6] << std::dec << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 7;
		}

		// Receiver ID
		if (*fspec & FSPEC41_I021_ED1_400)
		{
         if(out_str) {
            *out_str << "400 [receiver id=" << static_cast<int>(field[0])
	          			<< "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 1;
		}

		// Data Ages
		if (*fspec & FSPEC42_I021_ED1_295)
		{
			uint8_t count = popcount (field[0] & 0xfe);
			if (field[0] & 0x1)
			{
				field += 1;
				count += popcount (field[0] & 0xfe);
			}

			if (field[0] & 0x1)
			{
				field += 1;
				count += popcount (field[0] & 0xfe);
			}

			if (field[0] & 0x1)
			{
				field += 1;
				count += popcount (field[0] & 0xc0);
			}

			if (field[0] & 0x1)
			{
				cms_error ("unable to handle more extents (sac %u, sic %u)",
					adsb.SAC, adsb.SIC);
				goto error;
			}

			field += 1 + count;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		// spare
		if (*fspec & FSPEC43_I021_ED1_SPA)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC44_I021_ED1_SPA)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC45_I021_ED1_SPA)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC46_I021_ED1_SPA)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}

		// spare
		if (*fspec & FSPEC47_I021_ED1_SPA)
		{
			cms_error ("unable to handle spare bits (sac %u, sic %u)",
				adsb.SAC, adsb.SIC);
			goto error;
		}


		// Reserved Expansion Field
		if (*fspec & FSPEC48_I021_ED1_RE)
		{
			adsb.RE_data.assign (field + 1, field + field[0]);

         if(out_str) {
            *out_str << "    [re_data=";
            uint16_t i = 0;

            while(i < adsb.RE_data.size()) {
               *out_str << std::hex << std::setw(2) << std::fixed <<
                  std::setfill('0') << adsb.RE_data[i++];
             }

            *out_str <<  "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += field[0];
		}

		// Special Purpose Field
		if (*fspec & FSPEC49_I021_ED1_SP)
		{
			adsb.SP_data.assign (field + 1, field + field[0]);

         if(out_str) {
            *out_str << "    [sp_data=";
            uint16_t i = 0;

            while(i < adsb.SP_data.size()) {
               *out_str << std::hex << std::setw(2) << std::fixed <<
                  std::setfill('0') << adsb.SP_data[i++];
            }

            *out_str <<  std::setfill(' ') << "]";


            NEW_LINE(*out_str, 0, '\0');
         }

			field += field[0];
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}
		
		fspec++;

		cms_error ("unable to handle more fspec (sac %u, sic %u)", adsb.SAC, adsb.SIC);
		goto error;

end:
		// pridame polozku do seznamu
		adsb_list.push_back (adsb);

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error ("wrong packet length (%u > %u)  (sac %u, sic %u)",
			record - msg, size,
			adsb_list.back().SAC, adsb_list.back().SIC);
		goto error;
	}

	return true;

error:
	cms_data (size, msg);
	adsb_list.clear ();
	return false;
}

bool Asterix021_to_TargetAdsb (const void * data, int size,
	deque<RDFTargetAdsb> & adsb_list, std::stringstream *out_str,
   bool one_line)
{
	enum CAT021_EDITION edition = CAT021_EDITION_10P;
	unsigned char sac, sic;
	const char * config;
	char text[64];

	if (!AsterixGetSACSIC (data, size, 21, sac, sic))
	{
		return false;
	}

	sprintf (text, "ASTERIX021_%u_%u", sac, sic);
	if ((config = g_getenv (text)))
	{
		if (!strcmp (config, "0.23"))
		{
			edition = CAT021_EDITION_023;
		}
		else if (!strcmp (config, "0.26"))
		{
			edition = CAT021_EDITION_026;
		}
		else if (!strcmp (config, "1.0P"))
		{
			edition = CAT021_EDITION_10P;
		}
		else
		{
			cms_warning ("unknown %s value %s", text, config);
		}
	}

	if (edition == CAT021_EDITION_10P)
	{

		if (!__Asterix021ed1_to_TargetAdsb (data, size, edition, adsb_list, out_str, one_line))
		{
			cms_warning ("cat21ed1 target failed");
			return false;
		}	
	}
	else
	{
		if (!__Asterix021ed0_to_TargetAdsb (data, size, edition, adsb_list, out_str, one_line))
		{
			cms_warning ("cat21ed0 target failed");
			return false;
		}		
	}

	return true;
}

int __TargetAdsb_to_Asterix021ed0 (deque<RDFTargetAdsb> & adsb_list,
	enum CAT021_EDITION edition, void * data, int maxsize)
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
	msg[0] = 21; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;	

	// seznam je prazdny, neni co zpracovavat
	if (adsb_list.empty ())
	{
		return 0;
	}
	
	size = 3;
	while (adsb_list.size () > 0)
	{
		RDFTargetAdsb & adsb = adsb_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// Data Source Identifier
		{
			*fspec |= FSPEC01_I021_ED0_010;
			field[0] = adsb.SAC;
			field[1] = adsb.SIC;
			field += 2;
		}

		// Target Report Descriptor
		{
			*fspec |= FSPEC02_I021_ED0_040;
			field[0] = 0;

			if (adsb.DCR) field[0] |= 0x80;
			if (adsb.GBS) field[0] |= 0x40;
			if (adsb.Simulated) field[0] |= 0x20;
			if (adsb.Tested) field[0] |= 0x10;
			if (adsb.Fixed) field[0] |= 0x08;
			if (adsb.SAA) field[0] |= 0x04;
			if (adsb.SPI) field[0] |= 0x02;

			switch (adsb.AddressType)
			{
				case ADDRESS_TYPE_DUPLICATE:
					field[1] = 0 << 5;
					break;

				case ADDRESS_TYPE_ICAO:
					field[1] = 1 << 5;
					break;

				case ADDRESS_TYPE_SURFACE:
					field[1] = 2 << 5;
					break;

				case ADDRESS_TYPE_ANONYMOUS:
					field[1] = 3 << 5;
					break;

				default:
					field[1] = (static_cast<unsigned>(adsb.AddressType) & 0x7) << 5;
					break;
			}

			field[1] |= (static_cast<unsigned>(adsb.AltitudeCapability & 0x3)) << 3;

			field += 2;
		}

		// Time of Day
		/* Warning: Conversion from ed 0.23 to 1.0+ (and vice versa) may cause
		problems with I021/030

		This item is defined in ASTERIX 021 ed. 0.23 as:
		"Time of applicability (measurement) of the reported position"

		However EUROCAE ED-129 compliant ground station will send in this data
		item either: time of applicability of position, time of message
		reception of the position message or time of message reception of
		velocity message */
		if (adsb.TOAp.Present)
		{
			*fspec |= FSPEC03_I021_ED0_030;
			int time = RDF_time_to_asterix_time(adsb.TOAp.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		if (adsb.Geographic)
		{
			*fspec |= FSPEC04_I021_ED0_130;

			if (edition == CAT021_EDITION_023)
			{
				long lat = lrint (adsb.Geographic->Latitude * 0x800000 / M_PI);
				long lon = lrint (adsb.Geographic->Longitude * 0x800000 / M_PI);
				if (lat > 0x7FFFFF || lat < -0x800000)
				{
					cms_warning ("x out of limit");
					goto skip;
				}
				if (lon > 0x7FFFFF || lon < -0x800000)
				{
					cms_warning ("y out of limit");
					goto skip;
				}
				field[0] = (lat >> 16) & 0xff;
				field[1] = (lat >> 8) & 0xff;
				field[2] = lat & 0xff;
				field[3] = (lon >> 16) & 0xff;
				field[4] = (lon >> 8) & 0xff;
				field[5] = lon & 0xff;
				field += 6;
			}
			else
			{
				long long lat = llrint (adsb.Geographic->Latitude * 0x2000000 / M_PI);
				long long lon = llrint (adsb.Geographic->Longitude * 0x2000000 / M_PI);
				if (lat > INT_MAX || lat < INT_MIN)
				{
					cms_warning ("lat out of limit");
					goto skip;
				}
				if (lon > INT_MAX || lon < INT_MIN)
				{
					cms_warning ("lon out of limit");
					goto skip;
				}
				field[0] = (lat >> 24) & 0xff;
				field[1] = (lat >> 16) & 0xff;
				field[2] = (lat >> 8) & 0xff;
				field[3] = lat & 0xff;
				field[4] = (lon >> 24) & 0xff;
				field[5] = (lon >> 16) & 0xff;
				field[6] = (lon >> 8) & 0xff;
				field[7] = lon & 0xff;
				field += 8;
			}
		}

		// Target Address
		if (adsb.AircraftAddress.Present)
		{
			*fspec |= FSPEC05_I021_ED0_080;
			field[0] = (adsb.AircraftAddress.Address >> 16) & 0xff;
			field[1] = (adsb.AircraftAddress.Address >> 8) & 0xff;
			field[2] = adsb.AircraftAddress.Address & 0xff;
			field += 3;
		}

		// Geometric Height
		if (adsb.GeometricHeight)
		{
			*fspec |= FSPEC06_I021_ED0_140;
			int height = static_cast<int>(rint (adsb.GeometricHeight->Height / 6.25 / 0.3048));
			field[0] = (height >> 8) & 0xff;
			field[1] = height & 0xff;
			field += 2;
		}

		// Figure of Merit
		{
			*fspec |= FSPEC07_I021_ED0_090;
			field[0] = 0;
			field[1] = adsb.NUCp_NIC & 0x0f;
			field += 2;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Link technology
		{
			*fspec |= FSPEC08_I021_ED0_210;
			field[0] = 0;
			if (adsb.CDTI)
			{
				field[0] |= 0x10;
			}

			switch (adsb.LinkTechnology)
			{
				case LINK_TYPE_VDL4:
					field[0] |= 0x02;
					break;

				case LINK_TYPE_UAT:
					field[0] |= 0x04;
					break;

				case LINK_TYPE_ES:
					field[0] |= 0x08;
					break;

				case LINK_TYPE_OTHER:
				default:
					field[0] |= 0x01;
					break;
			}

			field += 1;
		}

		// Flight level
		if (adsb.FlightLevel)
		{
			*fspec |= FSPEC10_I021_ED0_145;
			int fl = static_cast<int>(rint (adsb.FlightLevel->Height / 0.3048 / 25));
			field[0] = (fl >> 8) & 0xff;
			field[1] = fl & 0xff;
			field += 2;
		}

		// Air Speed
		if (adsb.AirSpeed)
		{
			*fspec |= FSPEC11_I021_ED0_150;
			field[0] = 0;
			int speed;

			if (adsb.AirSpeed->Mach)
			{
				field[0] |= 0x80;
				speed = static_cast<int>(rint (adsb.AirSpeed->Speed * 1000 / 340.3));
			}
			else
			{
				speed = static_cast<int>(rint (adsb.AirSpeed->Speed / 1852 * 0x4000));
			}

			field[0] |= (speed >> 8) & 0x7f;
			field[1] = speed & 0xff;
			field += 2;
		}

		// True Airspeed
		if (adsb.TrueAirspeed)
		{
			*fspec |= FSPEC12_I021_ED0_151;
			int speed = static_cast<int>(rint (adsb.TrueAirspeed->Speed * SPEED_TO_KNOT));
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field += 2;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Ground Vector
		if (adsb.GroundSpeed)
		{
			*fspec |= FSPEC16_I021_ED0_160;
			unsigned short speed = static_cast<int>(rint (adsb.GroundSpeed->Speed * (1<<14) / 1852));
			unsigned short heading = static_cast<int>(rint (adsb.GroundSpeed->Heading * (1<<16) / (2 * M_PI)));

			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field[2] = (heading >> 8) & 0xff;
			field[3] = heading & 0xff;
			field += 4;
		}

		// Target Idenfification
		if (adsb.AircraftIdentification.size ())
		{
			*fspec |= FSPEC18_I021_ED0_170;
			asterix_encode_callsign (adsb.AircraftIdentification, field);
			field += 6;
		}

		// Velocity Accuracy
		{
			*fspec |= FSPEC19_I021_ED0_095;
			field[0] = adsb.NUCr_NACv;
			field += 1;
		}

		// Target Status
		{
			*fspec |= FSPEC21_I021_ED0_200;
			field[0] = static_cast<unsigned>(adsb.PriorityStatus);
			field += 1;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// RE data
		if (adsb.RE_data.size ())
		{
			if (adsb.RE_data.size () >= 254)
			{
				cms_error ("RE size error (%d)", adsb.RE_data.size ());
			}
			else
			{
				*fspec |= FSPEC48_I021_ED1_RE;
				field[0] = adsb.RE_data.size () + 1;
				std::copy (adsb.RE_data.begin (), adsb.RE_data.end (), field + 1);
				field += adsb.RE_data.size () + 1;
			}
		}

		// SP data
		if (adsb.SP_data.size ())
		{
			if (adsb.SP_data.size () >= 254)
			{
				cms_error ("SP size error (%d)", adsb.SP_data.size ());
			}
			else
			{
				*fspec |= FSPEC49_I021_ED1_SP;
				field[0] = adsb.SP_data.size () + 1;
				std::copy (adsb.SP_data.begin (), adsb.SP_data.end (), field + 1);
				field += adsb.SP_data.size () + 1;
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
				adsb_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime track z fronty
		adsb_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}

int __TargetAdsb_to_Asterix021ed1 (deque<RDFTargetAdsb> & adsb_list,
	enum CAT021_EDITION edition, void * data, int maxsize)
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
	msg[0] = 21; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;	

	// seznam je prazdny, neni co zpracovavat
	if (adsb_list.empty ())
	{
		return 0;
	}
	
	size = 3;
	while (adsb_list.size () > 0)
	{
		RDFTargetAdsb & adsb = adsb_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// Data Source Identifier
		{
			*fspec |= FSPEC01_I021_ED1_010;
			field[0] = adsb.SAC;
			field[1] = adsb.SIC;
			field += 2;
		}

		// Target Report Description
		{
			*fspec |= FSPEC02_I021_ED1_040;
			field[0] = static_cast<unsigned>(adsb.AddressType) << 5;
			field[0] |= (static_cast<unsigned>(adsb.AltitudeCapability) << 3);
			if (adsb.RC) field[0] |= 0x4;
			if (adsb.Fixed) field[0] |= 0x2;

			field[1] = (static_cast<unsigned>(adsb.ConfidenceLevel) << 1);
			if (adsb.DCR) field[1] |= 0x80;
			if (adsb.GBS) field[1] |= 0x40;
			if (adsb.Simulated) field[1] |= 0x20;
			if (adsb.Tested) field[1]    |= 0x10;
			if (adsb.SAA) field[1] |= 0x8;

			field[2] = 0;
			if (adsb.IPC) field[2] |= 0x20;
			if (adsb.NOGO) field[2] |= 0x10;
			if (adsb.CPR) field[2] |= 0x8;
			if (adsb.LDPJ) field[2] |= 0x4;
			if (adsb.RCF) field[2] |= 0x2;

			/* at least one bit set to '1' */
			if (field[1] || field[2])
			{
				field[0] |= 0x1;
				field += 1;
			}

			if (field[2])
			{
				field[1] |= 0x1;
				field += 1;
			}

			field += 1;
		}

		// Target ID
		if (adsb.TrackNumber.Present)
		{
			*fspec |= FSPEC03_I021_ED1_161;
			field[0] = (adsb.TrackNumber.Number >> 8) & 0x0f;
			field[1] = adsb.TrackNumber.Number & 0xff;
			field += 2;
		}

		// Time of Applicability for Position
		if (adsb.TOAp.Present)
		{
			*fspec |= FSPEC05_I021_ED1_071;
			int time = RDF_time_to_asterix_time(adsb.TOAp.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Position in WGS-84 coordinates
		if (adsb.Geographic && !adsb.HRPosition)
		{
			*fspec |= FSPEC06_I021_ED1_130;
			long lat = lrint (adsb.Geographic->Latitude / M_PI * (1 << 23));
			long lon = lrint (adsb.Geographic->Longitude / M_PI * (1 << 23));
			if (lat > 0x7FFFFF || lat < -0x800000)
			{
				cms_warning ("x out of limit");
				goto skip;
			}
			if (lon > 0x7FFFFF || lon < -0x800000)
			{
				cms_warning ("y out of limit");
				goto skip;
			}
			field[0] = (lat >> 16) & 0xff;
			field[1] = (lat >> 8) & 0xff;
			field[2] = lat & 0xff;
			field[3] = (lon >> 16) & 0xff;
			field[4] = (lon >> 8) & 0xff;
			field[5] = lon & 0xff;
			field += 6;
		}

		// Position in WGS-84 coordinates, high res.
		if (adsb.Geographic && adsb.HRPosition)
		{
			*fspec |= FSPEC07_I021_ED1_131;
			long long lat = llrint (adsb.Geographic->Latitude / M_PI * (1 << 30));
			long long lon = llrint (adsb.Geographic->Longitude / M_PI * (1 << 30));
			if (lat > INT_MAX || lat < INT_MIN)
			{
				cms_warning ("lat out of limit");
				goto skip;
			}
			if (lon > INT_MAX || lon < INT_MIN)
			{
				cms_warning ("lon out of limit");
				goto skip;
			}
			field[0] = (lat >> 24) & 0xff;
			field[1] = (lat >> 16) & 0xff;
			field[2] = (lat >> 8) & 0xff;
			field[3] = lat & 0xff;
			field[4] = (lon >> 24) & 0xff;
			field[5] = (lon >> 16) & 0xff;
			field[6] = (lon >> 8) & 0xff;
			field[7] = lon & 0xff;
			field += 8;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Time of Applicability for Velocity
		if (adsb.TOAv.Present)
		{
			*fspec |= FSPEC08_I021_ED1_072;
			int time = RDF_time_to_asterix_time(adsb.TOAv.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Air Speed
		if (adsb.AirSpeed)
		{
			*fspec |= FSPEC09_I021_ED1_150;
			field[0] = 0;
			int speed;

			if (adsb.AirSpeed->Mach)
			{
				field[0] |= 0x80;
				speed = static_cast<int>(rint (adsb.AirSpeed->Speed * 1000 / 340.3));
			}
			else
			{
				speed = static_cast<int>(rint (adsb.AirSpeed->Speed / 1852 * 0x4000));
			}

			field[0] |= (speed >> 8) & 0x7f;
			field[1] = speed & 0xff;
			field += 2;
		}

		// True Airspeed
		if (adsb.TrueAirspeed)
		{
			*fspec |= FSPEC10_I021_ED1_151;
			field[0] = 0;
			if (adsb.TrueAirspeed->RE)
			{
				field[0] |= 0x80;
			}
			int speed = static_cast<int>(rint (adsb.TrueAirspeed->Speed * SPEED_TO_KNOT));
			field[0] = (speed >> 8) & 0x7f;
			field[1] = speed & 0xff;
			field += 2;
		}

		// Target Address
		if (adsb.AircraftAddress.Present)
		{
			*fspec |= FSPEC11_I021_ED1_080;
			field[0] = (adsb.AircraftAddress.Address >> 16) & 0xff;
			field[1] = (adsb.AircraftAddress.Address >> 8) & 0xff;
			field[2] = adsb.AircraftAddress.Address & 0xff;
			field += 3;
		}

		// Time of Message Reception of Position
		if(adsb.TOMRp.Present)
		{
			*fspec |= FSPEC12_I021_ED1_073;
			int time = RDF_time_to_asterix_time(adsb.TOMRp.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// Time of Message Reception of Velocity
		if(adsb.TOMRv.Present)
		{
			*fspec |= FSPEC14_I021_ED1_075;
			int time = RDF_time_to_asterix_time(adsb.TOMRv.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Geometric Height
		if (adsb.GeometricHeight)
		{
			*fspec |= FSPEC16_I021_ED1_140;
			int height = static_cast<int>(rint (adsb.GeometricHeight->Height / 6.25 / 0.3048));
			field[0] = (height >> 8) & 0xff;
			field[1] = height & 0xff;
			field += 2;
		}

		// Quality Indicators
		{
			*fspec |= FSPEC17_I021_ED1_090;
			field[0] = (adsb.NUCr_NACv << 5);
			field[0] |= (adsb.NUCp_NIC << 1);

			field[1] = (adsb.NICbaro << 7);
			field[1] |= (adsb.SIL << 5);
			field[1] |= (adsb.NACp << 1);

			field[2] = (adsb.SILs << 5);
			field[2] |= (adsb.SDA << 3);
			field[2] |= (adsb.GVA << 1);

			field[3] = (adsb.PIC << 4);

			/* at least one bit set to '1' */
			for (size_t i = 3; i > 0; --i)
			{
				if (field[i])
				{
					field[i - 1] |= 0x1;
				}
			}

			while (field[0] & 0x1)
			{
				field += 1;
			}
			field += 1;
		}

		// MOPS Version
		{
			*fspec |= FSPEC18_I021_ED1_210;
			field[0] = static_cast<unsigned>(adsb.LinkTechnology);
			field[0] |= (adsb.VersionNumber << 3);
			if (adsb.VNS)
			{
				field[0] |= 0x40;
			}
			field += 1;
		}

		// Mode 3/A Code
		if (adsb.Mode3A)
		{
			*fspec |= FSPEC19_I021_ED1_070;
			field[0] = (adsb.Mode3A->Code >> 8) & 0x0f;
			field[1] = adsb.Mode3A->Code & 0xff;
			field += 2;
		}

		// Flight Level
		if (adsb.FlightLevel)
		{
			*fspec |= FSPEC21_I021_ED1_145;
			int fl = static_cast<int>(rint (adsb.FlightLevel->Height / 0.3048 / 25));
			field[0] = (fl >> 8) & 0xff;
			field[1] = fl & 0xff;
			field += 2;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// Target Status
		{
			*fspec |= FSPEC23_I021_ED1_200;
			field[0] = static_cast<unsigned>(adsb.SurveillanceStatus);
			field[0] |= static_cast<unsigned>(adsb.PriorityStatus) << 2;
			if (adsb.LNAV)
			{
				field[0] |= 0x40;
			}
			if (adsb.ICF)
			{
				field[0] |= 0x80;
			}
			field += 1;
		}

		// Ground Vector
		if (adsb.GroundSpeed)
		{
			*fspec |= FSPEC26_I021_ED1_160;
			unsigned short speed = static_cast<int>(rint (adsb.GroundSpeed->Speed * (1<<14) / 1852));
			unsigned short heading = static_cast<int>(rint (adsb.GroundSpeed->Heading * (1<<16) / (2 * M_PI)));
			field[0] = (speed >> 8) & 0x7f;
			field[1] = speed & 0xff;
			field[2] = (heading >> 8) & 0xff;
			field[3] = heading & 0xff;
			field += 4;
		}

		// Time of Message Transmission
		if (adsb.Time.Present)
		{
			*fspec |= FSPEC28_I021_ED1_077;
			int time = RDF_time_to_asterix_time(adsb.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		*fspec++ |= FSPEC_FX;
 		*fspec = 0;

		// Target Indentification
		if (adsb.AircraftIdentification.size ())
		{
			*fspec |= FSPEC29_I021_ED1_170;
			asterix_encode_callsign (adsb.AircraftIdentification, field);
			field += 6;
		}

		*fspec++ |= FSPEC_FX;
 		*fspec = 0;

		if (adsb.ModeS.size ())
		{
			*fspec |= FSPEC39_I021_ED1_250;
			unsigned char * subfield = field;
			unsigned char rep = *subfield = 0;
			field += 1;
			for (unsigned i = 0; i < adsb.ModeS.size (); i++)
			{
				memcpy (field, adsb.ModeS[i].MessageData, 7);
				field[7] = adsb.ModeS[i].Address;
				field += 8;
				rep += 1;
			}
			*subfield = rep;
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// RE data
		if (adsb.RE_data.size ())
		{
			if (adsb.RE_data.size () >= 254)
			{
				cms_error ("RE size error (%d)", adsb.RE_data.size ());
			}
			else
			{
				*fspec |= FSPEC48_I021_ED1_RE;
				field[0] = adsb.RE_data.size () + 1;
				std::copy (adsb.RE_data.begin (), adsb.RE_data.end (), field + 1);
				field += adsb.RE_data.size () + 1;
			}
		}

		// SP data
		if (adsb.SP_data.size ())
		{
			if (adsb.SP_data.size () >= 254)
			{
				cms_error ("SP size error (%d)", adsb.SP_data.size ());
			}
			else
			{
				*fspec |= FSPEC49_I021_ED1_SP;
				field[0] = adsb.SP_data.size () + 1;
				std::copy (adsb.SP_data.begin (), adsb.SP_data.end (), field + 1);
				field += adsb.SP_data.size () + 1;
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
				adsb_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime track z fronty
		adsb_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}
	
	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}


int TargetAdsb_to_Asterix021 (deque<RDFTargetAdsb> & adsb_list,
	void * data, int maxsize)
{
	enum CAT021_EDITION edition = CAT021_EDITION_10P;
	unsigned char sac, sic;
	const char * config;
	char text[64];

	if (adsb_list.empty ())
	{
		return 0;
	}

	sac = adsb_list.front ().SAC;
	sic = adsb_list.front ().SIC;

	sprintf (text, "ASTERIX021_%u_%u", sac, sic);

	if ((config = g_getenv (text)))
	{
		if (!strcmp (config, "0.23"))
		{
			edition = CAT021_EDITION_023;
		}
		else if (!strcmp (config, "0.26"))
		{
			edition = CAT021_EDITION_026;
		}
		else if (!strcmp (config, "1.0P"))
		{
			edition = CAT021_EDITION_10P;
		}
		else
		{
			cms_warning ("unknown %s value %s", text, config);
		}
	}

	if (edition == CAT021_EDITION_10P)
	{

		return (__TargetAdsb_to_Asterix021ed1 (adsb_list, edition, data, maxsize));
	}
	
	return (__TargetAdsb_to_Asterix021ed0 (adsb_list, edition, data, maxsize));
}

