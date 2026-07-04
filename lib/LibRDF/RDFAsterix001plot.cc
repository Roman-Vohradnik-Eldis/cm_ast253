#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I001_010 0x80
#define FSPEC02_I001_020 0x40
#define FSPEC03_I001_040 0x20
#define FSPEC04_I001_070 0x10
#define FSPEC05_I001_090 0x08
#define FSPEC06_I001_130 0x04
#define FSPEC07_I001_141 0x02

#define FSPEC08_I001_050 0x80
#define FSPEC09_I001_120 0x40
#define FSPEC10_I001_131 0x20
#define FSPEC11_I001_080 0x10
#define FSPEC12_I001_100 0x08
#define FSPEC13_I001_060 0x04
#define FSPEC14_I001_030 0x02

#define FSPEC15_I001_150 0x80
#define FSPEC16_I001_SPA 0x40
#define FSPEC17_I001_SPA 0x20
#define FSPEC18_I001_SPA 0x10
#define FSPEC19_I001_SPA 0x08
#define FSPEC20_I001_SP  0x04
#define FSPEC21_I001_RS  0x02

#define FSPEC22_I001_SPA 0x80
#define FSPEC23_I001_INDRA 0x40
#define FSPEC24_I001_SPA 0x20
#define FSPEC25_I001_SPA 0x10
#define FSPEC26_I001_SPA 0x08
#define FSPEC27_I001_SPA 0x04
#define FSPEC28_I001_SPA 0x02

bool Asterix001_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list, std::stringstream *out_str, bool one_line) 
{
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	const char *config;
	int sp_len = -1;
	if ((config = g_getenv ("ASTERIX001_SP_LEN")))
	{
		sp_len = atoi(config);
	}
	int rs_len = -1;
	if ((config = g_getenv ("ASTERIX001_RS_LEN")))
	{
		rs_len = atoi(config);
	}

	plot_list.clear ();

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

	if (cat != 1)
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
		RDFTargetPlot plot;
		plot.CAT = 1;

		// najdeme zacatek datovych poli
		fspec = field = record;
		while ((*field & FSPEC_FX) && field < msg + len)
		{
			field++;
		}
		field++;

		// data source identifier
		if (*fspec & FSPEC01_I001_010)
		{
         if(out_str) {
             *out_str << "010 [sac=" << static_cast<int>(field[0])
                      << " sic=" << static_cast<int>(field[1]) << "]";

              NEW_LINE(*out_str, 0, '\0');
         }

			plot.SAC = field[0];
			plot.SIC = field[1];

			//cms_info ("SIC %d, SAC %d", plot.SIC, plot.SAC);
			field += 2;
		}

		// target report description
		if (*fspec & FSPEC02_I001_020)
		{
         if(out_str) {

            if (field[0] & 0x80)
               *out_str << "020 [TRACK ";
            else
               *out_str << "020 [PLOT ";

            if (field[0] & 0x40)
               *out_str << "SIM";
            else
               *out_str << "ACT";

            switch ((field[0] >> 4) & 3) {
               case 1: *out_str << " PSR"; break;
               case 2: *out_str << " SSR"; break;
               case 3: *out_str << " COMBINED"; break;
               default: *out_str << " NO_DETECTION"; break;
            }

            *out_str << " ANT_" << 1 + ((field[0] >> 3) & 0x1);

            if (field[0] & 0x04)
               *out_str << " SPI";
            else
               *out_str << "  - ";

            if (field[0] & 0x2)
               *out_str << " FIX]";
            else
               *out_str << " AIR]";

            if (field[0] & 1) {
               NEW_LINE(*out_str, 4, ' ');

               if (field[0] & 0x80)
                  *out_str << "[TEST ";
               else
                  *out_str << "[REAL ";

               if (field[0] & 0x10)
                  *out_str << "ME ";
               else
                  *out_str << " - ";

               if (field[0] & 0x8)
                  *out_str << "MI ";
               else
                  *out_str << " - ";

               switch (field[0] & 0x60) {
                  case 0: *out_str << "-]"; break;
                  case 0x20: *out_str << "HIJACK (7500)]"; break;
                  case 0x40: *out_str << "RADIO_FAIL (7600)]"; break;
                  case 0x60: *out_str << "EMERGENCY (7700)]"; break;
               }
            }
            NEW_LINE(*out_str, 0, '\0');
         }

			// kontrola priznaku plot/track
			if (field[0] & 0x80)
			{
				return false;
			}

			//cms_info("target %02x", field[0]);
			switch ((field[0] >> 4) & 0x3)
			{
				case 0:
				{
					plot.TargetType = TARGET_TYPE_UNKNOWN;
				}
				case 1:
				{
					plot.TargetType = TARGET_TYPE_PRIMARY;
					break;
				}
				case 2:
				{
					plot.TargetType = TARGET_TYPE_SECONDARY;
					break;
				}
				case 3:
				{
					plot.TargetType = TARGET_TYPE_COMBINED;
					break;
				}
			}

			plot.Simulated = (field[0] & 0x40) ? true : false;
 			plot.SPI = (field[0] & 0x04) ? true : false;

			if (field[0] & 1)
			{
				field += 1;
				//cms_info("target2 %02x", field[0]);
				plot.Tested = (field[0] & 0x80) ? true : false; 
			}

			field += 1;
		}

		// measured position in polar co-ordinates
		if (*fspec & FSPEC03_I001_040)
		{
         if(out_str) {
            double rho = (double)field2unsigned (field, 16) * 1852 / 128.0;
				double theta = (double) field2unsigned (field + 2, 16) * 360 / 0x10000;

				*out_str << "040 [rho=" << static_cast<int>(rho) <<" m (" 
               << std::setprecision(2) << std::fixed
					<< rho / 1852.0 << " NM) theta=" << theta << " deg]";

            NEW_LINE(*out_str, 0, '\0');
         }

			plot.Polar = new RDFCoorPolar ();
			plot.Polar->Rho = (double) field2unsigned (field, 16) * 1852 / 128;
			plot.Polar->Theta = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
			//cms_info ("polar %d %f", plot.Rho, plot.Theta);
			field += 4;
		}

		// mode 3/A code in octal representation
		if (*fspec & FSPEC04_I001_070)
		{
         if(out_str) {
            *out_str << "070 [M3A_oct=" << std::oct << 
               field2unsigned(field, 12) << std::dec;

            if (field[0] & 0x80)
					*out_str << " INVALID";
				else
					*out_str << " VALID";

				if (field[0] & 0x40)
					*out_str << " GARBLED";
				else
					*out_str << " -";

				if (field[0] & 0x20)
					*out_str << " TRACKED]";
				else
					*out_str << " -]";

            NEW_LINE(*out_str, 0, '\0');
         }

			//cms_info ("Mode3A");
			plot.Mode3A = new RDFTargetMode ();
			plot.Mode3A->Valid = (field[0] & 0x80) ? false : true;
			plot.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
			plot.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
			plot.Mode3A->Code = field2unsigned (field, 12);
			field += 2;
		}

		// flight level
		if (*fspec & FSPEC05_I001_090)
		{
         if(out_str) {
         	if (field[0] & 0x80)
					*out_str << "090 [INVALID";
				else
					*out_str << "090 [VALID";

				if (field[0] & 0x40)
					*out_str << " GARBLED";
				else
					*out_str << " - ";

				double fl = field2signed (field, 14) / 4.0;

				*out_str << " fl=" << std::setprecision (2) << std::fixed
               << fl << " (" << std::setprecision (3) 
               << fl * 100 * 0.3048 << " m)]";

            NEW_LINE(*out_str, 0, '\0');
         }
			//cms_info ("FLIGHT LEVEL");
			plot.FlightLevel = new RDFTargetFlightLevel ();
			plot.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
			plot.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
			plot.FlightLevel->Height = (double) field2signed (field, 14) * 25 * 0.3048;
			field += 2;
		}

		// radar plot charactreistics
		if (*fspec & FSPEC06_I001_130)
		{
         if(out_str) {
         	*out_str << "130 [hex=";
         }

         uint16_t pos = 0;

			while (field[0] & 1) {
            pos++;

            if(out_str) {
               *out_str << std::hex << std::setw(2) << std::fixed 
                  << static_cast<int>(field[0] >> 1);

               if(pos % 10 == 0) {
                  *out_str << "]";
                  NEW_LINE(*out_str, 4, ' ');
               } else 
                  *out_str << " "; 
            }

				field += 1;
			}

         if(out_str) {
            if(pos % 10 != 0) {
               *out_str << "]";
               NEW_LINE(*out_str, 0, '\0');
            }
         }

			field += 1;
		}

		// trancated time of day
		if (*fspec & FSPEC07_I001_141)
		{
         if(out_str) {
			   *out_str << "141 [time=" << field2unsigned (field, 16) / 128.0
					<< " sec]";

            NEW_LINE(*out_str, 0, '0');
         }

			field += 2;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// mode-2 code in octal representation
		if (*fspec & FSPEC08_I001_050)
		{
         if(out_str) {
         	if (field[0] & 0x80)
					*out_str << "[INVALID";
				else
					*out_str << "[VALID";

				if (field[0] & 0x40)
					*out_str << " GARBLED";
				else
					*out_str << " - ";

				if (field[0] & 0x20)
					*out_str << " TRACKED";
				else
					*out_str << " - ";

				*out_str << " code=" << std::hex << field2unsigned(field, 12)
					<< std::dec << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			plot.Mode2 = new RDFTargetMode ();
			plot.Mode2->Valid = (field[0] & 0x80) ? false : true;
			plot.Mode2->Garbled = (field[0] & 0x40) ? true : false;
			plot.Mode2->Tracked = (field[0] & 0x20) ? true : false;
			plot.Mode2->Code = field2unsigned (field, 12);
			//cms_info ("mode2 %d %d", plot.Mode2Validated, mode3 (plot.Mode2));
			field += 2;
		}

		// measured radial doppler speed
		if (*fspec & FSPEC09_I001_120)
		{
         if(out_str) {
            double speed = (double) field2signed (field, 8) / 256;
				*out_str << "120 [speed" << speed * 1852 << " m/s ("
					<< speed * 3600 << " kt)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 1;
		}
		// received power 
		if (*fspec & FSPEC10_I001_131)
		{
         if(out_str) {
            *out_str << "131 [power=" << field2signed (field, 8) << " dBm]";
            NEW_LINE(*out_str, 0, '\0');
         }

			field += 1;
		}

		// mode 3/A code dconfidence indicator
		if (*fspec & FSPEC11_I001_080 && plot.Mode3A)
		{
         if(out_str) {

            int32_t conf = field2unsigned (field, 12);

            *out_str << "080 [QXi=";
             for (char i = 11; i >= 0; --i)
                  *out_str << ((conf >> i) & 0x1);
             *out_str << "]";

             NEW_LINE(*out_str, 0, '\0');
         }

			plot.Mode3A->ConfidenceCode = field2unsigned (field, 12);
			//cms_info ("mode3 %d", mode3);
			field += 2;
		}

		// mode C code and confidence indicator
		if (*fspec & FSPEC12_I001_100)
		{
         if(out_str) {
            if (field[0] & 0x80)
					*out_str << "100 [INVALID";
				else
					*out_str << "100 [VALID";

				if (field[0] & 0x40)
					*out_str << "GARBLED";
				else
					*out_str << " - ";

				*out_str << "code=" << std::hex << field2unsigned (field, 12);

				int32_t conf = field2unsigned (field + 2, 12);
				if (conf) {
					*out_str << " conf=";
					for (char i = 11; i >= 0; --i)
						*out_str << (conf >> i & 1);
				}
            NEW_LINE(*out_str, 0, '\0');

         }
			plot.ModeC = new RDFTargetMode ();
			plot.ModeC->Valid = (field[0] & 0x80) ? false : true;
			plot.ModeC->Garbled = (field[0] & 0x40) ? true : false;
			plot.ModeC->Code = field2unsigned (field, 12);
			plot.ModeC->ConfidenceCode = field2unsigned (field + 2, 12);
			//cms_info ("modec %d qxi %d", modec, qxi);
			field += 4;
		}

		// mode-2 code confidence indicator
		if (*fspec & FSPEC13_I001_060 && plot.Mode2)
		{
         if(out_str) {

            int32_t conf = field2unsigned (field, 12);

            *out_str << "060 [QXi=";
             for (char i = 11; i >= 0; --i)
                  *out_str << ((conf >> i) & 0x1);
             *out_str << "]";

             NEW_LINE(*out_str, 0, '\0');
         }

			plot.Mode2->ConfidenceCode = field2unsigned (field, 12);
			field += 2;
		}

		// warning/error conditions
		if (*fspec & FSPEC14_I001_030)
		{
         if(out_str) {
            unsigned char *tmp_field = field;
            *out_str << "030 [warning/error conditions]";
            uint8_t wec = 0;
            do {
               wec = *tmp_field >> 1;
               switch (wec) {
                  case 1:
                     *out_str << "[" << static_cast<int>(wec) << " GARBLED_REPLY]"; break;
                  case 2:
                     *out_str << "[" << static_cast<int>(wec) << " REFLECTION]"; break;
                  case 3:
                     *out_str << "[" << static_cast<int>(wec) << " SIDELOBE_REPLY]"; break;
                  case 4:
                     *out_str << "[" << static_cast<int>(wec) << " SPLIT_PLOT]"; break;
                  case 5:
                     *out_str << " [" << static_cast<int>(wec) << " SECOND_TIME_AROUND_REPLY]"; break;
                  case 6:
                     *out_str << " [" << static_cast<int>(wec) << " ANGELS]"; break;
                  case 7:
                     *out_str << " [" << static_cast<int>(wec) << " TERRESTRIAL_VEHICLES]"; break;
                  case 64:
                     *out_str << " [" << static_cast<int>(wec) << " POS_WRONG_CODE_Mode-3A]"; break;
                  case 65:
                     *out_str << " [" << static_cast<int>(wec) << " POS_WRONG_ALT]"; break;
                  case 66:
                     *out_str << " [" << static_cast<int>(wec) << " POS_PHANTOM_MSSR_PLOT]"; break;
                  case 80:
                     *out_str << " [" << static_cast<int>(wec) << " FIXED_PSR_PLOT]"; break;
                  case 81:
                     *out_str << " [" << static_cast<int>(wec) << " FIXED_PSR_PLOT]"; break;
                  case 82:
                     *out_str << " [" << static_cast<int>(wec) << " LOW_QUALITY_PSR_PLOT]"; break;
               }

               NEW_LINE(*out_str, 0, '\0');
            } while(*tmp_field++ & 0x1);
         }

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

		// presence of x-pulse
		if (*fspec & FSPEC15_I001_150)
		{
			field += 1;
		}

		// spare
		if (*fspec & FSPEC16_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC17_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC18_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC19_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// SP
		if (*fspec & FSPEC20_I001_SP)
		{
			if (sp_len==-1) field += field[0]; else field += sp_len;
		}

		// RS
		if (*fspec & FSPEC21_I001_RS)
		{
			if (rs_len==-1)
			{
				cms_error ("unable to handle spare bit");
				goto error;
			} else field += rs_len;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// spare
		if (*fspec & FSPEC22_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// nestandartni polozka pouzita v systemech INDRA
		if (*fspec & FSPEC23_I001_INDRA)
		{
			field += 2;
		}

		// spare
		if (*fspec & FSPEC24_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC25_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC26_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC27_I001_SPA)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		// spare
		if (*fspec & FSPEC28_I001_SPA)
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
      NEW_LINE(*out_str, 0, 0);
		// pridame polozku do seznamu
		plot_list.push_back (plot);

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
	plot_list.clear();
	return false;
}

int TargetPlot_to_Asterix001 (deque<RDFTargetPlot> & plot_list, 
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
	msg[0] = 1; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if (plot_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (plot_list.size () > 0)
	{
		RDFTargetPlot & plot = plot_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// data source identifier
		{
			*fspec |= FSPEC01_I001_010;
			field[0] = plot.SAC;
			field[1] = plot.SIC;
			field += 2;
		}

		// target report description
		{
			*fspec |= FSPEC02_I001_020;
			field[0] = 0;

			switch (plot.TargetType)
			{
				case TARGET_TYPE_PRIMARY:
				{
					field[0] |= 1 << 4;
					break;
				}
				case TARGET_TYPE_SECONDARY:
				case TARGET_TYPE_MODES_ALL_CALL:
				case TARGET_TYPE_MODES_ROLL_CALL:
				{
					field[0] |= 2 << 4;
					break;
				}
				case TARGET_TYPE_COMBINED:
				case TARGET_TYPE_MODES_ALL_CALL_PSR:
				case TARGET_TYPE_MODES_ROLL_CALL_PSR:
				{
					field[0] |= 3 << 4;
					break;
				}
				case TARGET_TYPE_UNKNOWN:
  				case TARGET_TYPE_ADSB: // chova se jako neznamy
				{
					break;
				}
			}

			field[0] |= plot.Simulated ? 0x40 : 0;
			field[0] |= plot.SPI ? 0x04 : 0;

			field[1] = plot.Tested ? 0x80 : 0;

			if (field[1])
			{
				field[0] |= 0x01;
				field += 2;
			}
			else
			{
				field += 1;
			}
		}

		// measured position in polar co-ordinates
		if (plot.Polar)
		{
			*fspec |= FSPEC03_I001_040;
			long rho = lrint (plot.Polar->Rho * 128 / 1852);
			long theta = lrint (plot.Polar->Theta * 0x10000 / (2 * M_PI));
			if (rho < 0 || rho > USHRT_MAX)
			{
				cms_warning ("rho out of limit");
				goto skip;
			}
			field[0] = (rho >> 8) & 0xff;
			field[1] = rho & 0xff;
			field[2] = (theta >> 8) & 0xff;
			field[3] = theta & 0xff;
			field += 4;
		}

		// mode 3/A code in octal representation
		if (plot.Mode3A)
		{
			*fspec |= FSPEC04_I001_070;
			field[0] = plot.Mode3A->Valid ? 0 : 0x80;
			field[0] |= plot.Mode3A->Garbled ? 0x40 : 0;
			field[0] |= plot.Mode3A->Tracked ? 0x20 : 0;
			field[0] |= (plot.Mode3A->Code >> 8) & 0x0f;
			field[1] = plot.Mode3A->Code & 0xff;
			field += 2;
		}

		// flight level
		if (plot.FlightLevel)
		{
			*fspec |= FSPEC05_I001_090;
			int height = (int) rint (plot.FlightLevel->Height / 0.3048 / 25);
			field[0] = plot.FlightLevel->Valid ? 0 : 0x80;
			field[0] |= plot.FlightLevel->Garbled ? 0x40 : 0;
			field[0] |= (height >> 8) & 0x3f;
			field[1] = height & 0xff;
			field += 2;
		}

		// radar plot charactreistics
		if (0)
		{
			*fspec |= FSPEC06_I001_130;
		}

		// trancated time of day
		if (0)
		{
			*fspec |= FSPEC07_I001_141;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// mode-2 code in octal representation
		if (plot.Mode2)
		{
			*fspec |= FSPEC08_I001_050;
			field[0] = plot.Mode2->Valid ? 0 : 0x80;
			field[0] |= plot.Mode2->Garbled ? 0x40 : 0;
			field[0] |= plot.Mode2->Tracked ? 0x20 : 0;
			field[0] |= (plot.Mode2->Code >> 8) & 0x0f;
			field[1] = plot.Mode2->Code & 0xff;
			field += 2;
		}

		// measured radial doppler speed
		if (0)
		{
			*fspec |= FSPEC09_I001_120;
		}

		// received power 
		if (0)
		{
			*fspec |= FSPEC10_I001_131;
		}

		// mode 3/A code dconfidence indicator
		if (plot.Mode3A && plot.Mode3A->ConfidenceCode)
		{
			*fspec |= FSPEC11_I001_080;
			field[0] = (plot.Mode3A->ConfidenceCode >> 8) & 0x0f;
			field[1] = plot.Mode3A->ConfidenceCode & 0xff;
			field += 2;
		}

		// mode C code and confidence indicator
		if (plot.ModeC)
		{
			*fspec |= FSPEC12_I001_100;
			field[0] = plot.ModeC->Valid ? 0 : 0x80;
			field[0] |= plot.ModeC->Garbled ? 0x40 : 0;
			field[0] |= (plot.ModeC->Code >> 8) & 0x0f;
			field[1] = plot.ModeC->Code & 0xff;
			field[2] = (plot.ModeC->ConfidenceCode >> 8) & 0x0f;
			field[3] = plot.ModeC->ConfidenceCode & 0xff;
			field += 4;
		}

		// mode-2 code confidence indicator
		if (plot.Mode2 && plot.Mode2->ConfidenceCode)
		{
			*fspec |= FSPEC13_I001_060;
			field[0] = (plot.Mode2->ConfidenceCode >> 8) & 0x0f;
			field[1] = plot.Mode2->ConfidenceCode & 0xff;
			field += 2;
		}

		// warning/error conditions
		if (0)
		{
			*fspec |= FSPEC14_I001_030;
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
				plot_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime plot z fronty
		plot_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}



