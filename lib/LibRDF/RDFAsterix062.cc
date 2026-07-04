#include "RDFAsterix.hh"
#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

#include <iostream>
#include <iomanip>

#define FSPEC_FX 0x01

#define FSPEC01_I062_010 0x80
#define FSPEC02_I062_SPA 0x40
#define FSPEC03_I062_015 0x20
#define FSPEC04_I062_070 0x10
#define FSPEC05_I062_105 0x08
#define FSPEC06_I062_100 0x04
#define FSPEC07_I062_185 0x02

#define FSPEC08_I062_210 0x80
#define FSPEC09_I062_060 0x40
#define FSPEC10_I062_245 0x20
#define FSPEC11_I062_380 0x10
#define FSPEC12_I062_040 0x08
#define FSPEC13_I062_080 0x04
#define FSPEC14_I062_290 0x02

#define FSPEC15_I062_200 0x80
#define FSPEC16_I062_295 0x40
#define FSPEC17_I062_136 0x20
#define FSPEC18_I062_130 0x10
#define FSPEC19_I062_135 0x08
#define FSPEC20_I062_220 0x04
#define FSPEC21_I062_390 0x02

#define FSPEC22_I062_270 0x80
#define FSPEC23_I062_300 0x40
#define FSPEC24_I062_110 0x20
#define FSPEC25_I062_120 0x10
#define FSPEC26_I062_510 0x08
#define FSPEC27_I062_500 0x04
#define FSPEC28_I062_340 0x02

#define FSPEC29_I062_XX  0x80
#define FSPEC30_I062_XX  0x40
#define FSPEC31_I062_XX  0x20
#define FSPEC32_I062_XX  0x10
#define FSPEC33_I062_XX  0x08
#define FSPEC34_I062_RE  0x04
#define FSPEC35_I062_SP  0x02

/**
 * funkce spocita pocet bitu s hodnotou val v poli
 * bytu data o velikosti data_siz, pricemz na kazdy
 * byt aplikuje masku mask
 */
uint32_t
bit_count(const uint8_t *data, const size_t data_siz,
      const bool val, uint8_t mask ) {

   uint32_t bit_count = 0;
   size_t pos = 0;
   const uint8_t *data_pos = data;

   while(pos < data_siz) {
      uint8_t pos_in_byte = 0;

      while(pos_in_byte < 8) {

         if(((*data_pos & mask) >> pos_in_byte) & 0x1) {
            bit_count++;
         }

         pos_in_byte++;
      }

      data_pos++;
      pos+=8;
   }

   return bit_count;
}

/**
 * Funkce vepise do stringstreamu latitude a longitude ve stupnich
 * minutach - lat, lon jsou v radianech
 */
void
geo_to_sstr(const double &lat, const double &lon, std::stringstream &out) {

   out << "lat=";
   int min, way, deg;
   double sec;
   char flag = '\0';

   rdf_rad2dms (lat, way, deg, min, sec);

   if (way == 1)
      flag = 'N';
   else
      flag = 'S';

   out << setfill ('0') << std::setw (3) << deg << " "
   << std::setw (2) << min << " " << std::setw (2)
   << (int) trunc (sec) << "." << std::setprecision(2) << std::fixed <<
   (int)trunc (sec * 100) % 100 << " " << flag << " lon=";

   rdf_rad2dms(lon, way, deg, min, sec);

   if (way == 1)
      flag = 'E';
   else
      flag = 'W';

    out << setfill ('0') << std::setw (3) << deg << " "
      << std::setw (2) << min << " " << std::setw (2)
      << (int) trunc (sec) << "." << std::setprecision(2) << std::fixed
      << (int)trunc (sec * 100) % 100 << " " << flag;
}

bool Asterix062_to_TargetTrack (const void * data, int size,
	deque<RDFTargetTrack> & track_list, std::stringstream *out_str, bool one_line)
{
	bool config_nosp = false;
	bool config_bangalore = false;
	const char * config;
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int cat;
	int len;

	if ((config = g_getenv ("ASTERIX062")))
	{
		if (!strcmp (config, "NOSP"))
		{
			config_nosp = true;
		}
		else if (!strcmp (config, "BANGALORE"))
		{
			config_bangalore = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX062 value");
		}
	}

	track_list.clear ();

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

	if (cat != 62)
	{
		cms_error ("unknown category %d", cat);
		return false;
	}

	if (len < size)
	{
		//cms_warning ("wrong length %d %d", len, size);
		size = len;
	}
	else if (len > size)
	{
		cms_error ("wrong length %d %d", len, size);
		goto error;
	}

	while (record < msg + len)
	{
		// najdeme zacatek datovych poli
		fspec = field = record;
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

		// vytvorime novou polozku
		RDFTargetTrack track;
		track.CAT = 62;

		// data source identifier
		if (*fspec & FSPEC01_I062_010)
		{
			track.SAC = field[0];
			track.SIC = field[1];

         if(out_str) {
            *out_str << "010 [sac=" << static_cast<int>(field[0])
               << " sic=" << static_cast<int>(field[1]) << "]";

            NEW_LINE(*out_str, 0, '\0');
         }

			//INFO ("SAC %d SIC %d", track.SAC, track.SIC);
			field += 2;

		}

		// spare
		if (*fspec & FSPEC02_I062_SPA)
		{
			cms_error ("FSPEC1_I062_sp1");
			goto error;
		}

		// service identification
		if (*fspec & FSPEC03_I062_015)
		{
			if(out_str) {
            *out_str << std::dec << "015 [service=" <<
               static_cast<int>(field[0]) <<
                  std::hex << std::showbase
                  << " (" << static_cast<int>(field[0]) << ")]";

            *out_str << std::dec;
            NEW_LINE(*out_str, 0, '\0');
         }//INFO ("SID %d", field[0]);
			field += 1;
		}

		// time of track information
		if (*fspec & FSPEC04_I062_070)
		{
			track.Time.Present = true;
			track.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
         if(out_str) {
            uint64_t time = field2unsigned (field, 24);

            uint64_t mod = time % 128;
            time = time / 128;

            *out_str << "070 [time=" << time << " ";
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

		// calculated track position (WGS-84)
		if (*fspec & FSPEC05_I062_105)
		{
			//INFO ("WGS");
			track.Geographic = new RDFCoorGeographic ();
			track.Geographic->Latitude = (double) field2signed (field, 32) * M_PI / 0x2000000;
			track.Geographic->Longitude = (double) field2signed (field + 4, 32) * M_PI / 0x2000000;
         if(out_str) {
		      *out_str << "105 [";

            geo_to_sstr(track.Geographic->Latitude,
                  track.Geographic->Latitude, *out_str);

      		*out_str << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
			field += 8;
		}

		// calculated track position (cartesian)
		if (*fspec & FSPEC06_I062_100)
		{
			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = (double) field2signed (field, 24) / 2;
			track.Cartesian->Y = (double) field2signed (field + 3, 24) / 2;

         if(out_str) {
            *out_str << std::setprecision(1) << "100 [x=" <<
               track.Cartesian->X << "m (" << std::setprecision(3) <<
               track.Cartesian->X /1852.0 << " NM) y="
               << std::setprecision(1) << track.Cartesian->Y <<
               std::setprecision(3) << "(" <<
               track.Cartesian->Y /1852.0 << " NM)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			//INFO("cartesian %d %d", track.Cartesian.X, track.Cartesian.Y);
			field += 6;
		}

		// calculated track velocity (cartesian)
		if (*fspec & FSPEC07_I062_185)
		{
			double vx, vy;
			vx = field2signed (field, 16) / 4.0;
			vy = field2signed (field + 2, 16) / 4.0;
			track.GroundSpeed = new RDFTargetGroundSpeed ();
			rdf_xy2rt (vx, vy, track.GroundSpeed->Speed, track.GroundSpeed->Heading);

         if(out_str) {
            *out_str << "185 [vx=" << std::setprecision(3) <<
               vx << "m/s (" << (vx/1852.0) * 3600.0 << " kt) vy=" <<
               vy << "m/s (" << (vy/1852.0) * 3600.0 << " kt)]";

            NEW_LINE(*out_str, 0, '\0');
         }

			field += 4;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// calculated acceleration (cartesian)
		if (*fspec & FSPEC08_I062_210)
		{
			//double ax, ay;
			//ax = (double)((char)field[0]) / 4;
			//ay = (double)((char)field[0]) / 4;

			track.Acceleration = new RDFCoorCartesian();
			track.Acceleration->X=(double)((char)field[0]) / 4;
			field+=1;
			track.Acceleration->Y=(double)((char)field[0]) / 4;
			field += 1;

         if(out_str) {
            (*out_str).precision(3);
            *out_str << "210 [ax=" << track.Acceleration->X
               << "ay=" << track.Acceleration->Y  << "]";

            NEW_LINE(*out_str, 0, '\0');
         }
		}

		// track mode 3/A code
		if (*fspec & FSPEC09_I062_060)
		{
			track.Mode3A = new RDFTargetMode ();
			track.Mode3A->Valid = true;
			track.Mode3A->Code = field2unsigned (field, 12);
			//INFO ("Mode3A %d", Mode3 (track.Mode3A.Code));
         if(out_str) {
            *out_str << "060 [";

            if(field[0] & 0x20)
               *out_str << "Y";
            else
               *out_str << "-";

            *out_str << " m-3/A code=" << std::oct <<
               track.Mode3A->Code << std::dec << "]";

//            NEW_LINE(*out_str, 0, '\0');
         }

			field += 2;
		}

		// target identification
		if (*fspec & FSPEC10_I062_245)
		{
			asterix_decode_callsign (field + 1, track.AircraftIdentification);
         if(out_str) {
            switch ((field[0] >> 6) & 0x3) {
               case 0: *out_str <<  "245 [sti=callsign_or_reg"; break;
               case 1: *out_str <<  "245 [sti=not_callsign"; break;
               case 2: *out_str <<  "245 [sti=not_registration"; break;
               case 3: *out_str <<  "245 [sti=invalid"; break;
            }

            *out_str <<  " callsign=\"" << track.AircraftIdentification <<"\"]";
            NEW_LINE(*out_str, 0, '\0');
         }

			field += 7;
		}

		// aircraft derived data
		if (*fspec & FSPEC11_I062_380)
		{
			unsigned char * subfield = field;
			while (subfield[0] & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}

         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
            *out_str << "380 [present_subfields=" <<
               bit_count(field, 8*(subfield - field), 0x1, 0xfe)
               << "]";
         }
			subfield += 1;
			// subfield 1: target address
			if (field[0] & 0x80)
			{
				track.AircraftAddress.Present = true;
				track.AircraftAddress.Address = field2unsigned (subfield, 24);

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ADR adress=" << std::hex <<
                  std::showbase <<
                  track.AircraftAddress.Address << "]";
            }
				subfield += 3;
			}
			// subfield 2: target identification
			if (field[0] & 0x40)
			{
				asterix_decode_callsign (subfield, track.AircraftIdentification);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ID callsig=\"" << track.AircraftIdentification
                  << "\"]";
            }
				subfield += 6;
			}
			// subfield 3: magnetic heading
			if (field[0] & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               out_str->precision(3);
               *out_str << "[MHG heading=" << std::fixed <<
                  field2unsigned (subfield, 16)/0x10000 << " deg]";
            }
				subfield += 2;
			}
			// subfield 4: indicated airspeed
			if (field[0] & 0x10)
			{
            //IAS
				if(subfield[0] & 0x80) {
               if(out_str) {
                  NEW_LINE(*out_str, 4, ' ');
                  double air_speed =
                     static_cast<double>(field2unsigned(subfield, 15)) /0x4000;

                  *out_str << "[IAS speed=" << air_speed * 1852.0 <<
                     " m/s (" << air_speed * 3600.0 << " kt)]";
               }
            //MACH
				} else {
               if(out_str) {
                  NEW_LINE(*out_str, 4, ' ');
					   double mach =
                     static_cast<double>(field2unsigned(subfield, 15)) / 0x3e8;

					   *out_str <<  "[IAS speed=" << mach <<"Mach";
               }
				}
				subfield += 2;
			}

			// subfield 5: true airspeed
			if (field[0] & 0x08)
			{
            if(out_str) {
               int kt = field2unsigned(subfield, 16);
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TAS speed" << kt * 1852.0 / 3600.0 <<
                  " m/s ( " << kt << "kt)]";
            }
				subfield += 2;
			}
			// subfield 6: selected altitude
			if (field[0] & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if(subfield[0] & 0x80)
                  *out_str << "[SAL sas=1 ";
               else
                  *out_str << "[SAL sas=0 ";

               switch((subfield[0] >> 5) & 0x3) {
                  case 0:
                     *out_str << "source=? "; break;
                  case 1:
                     *out_str << "source=aircraft "; break;
                  case 2:
                     *out_str << "source=FCU/MCP ";break;
                  case 3:
                     *out_str << "source=FMS "; break;
               }

               uint32_t alt = field2unsigned(subfield, 13);

               *out_str << "altitude=" << alt << "ft (" <<
                  std::setprecision(3) << std::fixed <<
                  alt * 0.3048 << "m)]";

            }

				subfield += 2;
			}
			// subfield 7: final state selected altitude
			if (field[0] & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4,  ' ');

               if (subfield[0] & 0x80)
                  *out_str << "[FSS mv=1 ";
               else
                  *out_str << "[FSS mv=0 ";

               if (subfield[0] & 0x40)
                  *out_str << "ah=1 ";
               else
                  *out_str << "ah=0 ";

               if (subfield[0] & 0x20)
                  *out_str << "am=1 ";
               else
                  *out_str << "am=0 ";

               int alt = field2signed(subfield, 13) * 25;
               *out_str << "altitude= " << alt << " ft (" <<
                  std::setprecision(3) << std::fixed << alt * 0.3048
                  << "m)]";
            }
            subfield += 2;
			}

         if (!(field[0] & 1))
			{
            if(out_str) {
//               NEW_LINE(*out_str, 0,  '\0');
            }
				goto end380;
			}

			// subfield 8: trajectory intent status
			if (field[1] & 0x80)
		   {
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (subfield[0] & 0x80)
                  *out_str << "[TIS nav=0 ";
               else
                  *out_str << "[TIS nav=1 ";

               if (subfield[0] & 0x40)
                  *out_str << "nvb=INV";
               else
                  *out_str << "nvb=VAL";

               *out_str << "]";
            }
				//cms_info ("!!!!!! trajectory status");
				subfield += 1;
			}
			// subfield 9: trajectory intent data
			if (field[1] & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str , 4, ' ');
               uint32_t rep = subfield[0];
               *out_str << "[TID rep=" << rep << "]";

               for(uint32_t i = 0; i < rep; i++) {

                  NEW_LINE(*out_str, 4, ' ');

                  if(subfield[0] & 0x80)
                     *out_str << "[tca=0 ";
                  else
                     *out_str << "[tca=1 ";

                  if(subfield[0] & 0x40)
                     *out_str << "nc=non_compliance ";
                  else
                     *out_str << "nc=compliance ";

                  *out_str << " tcp=" <<  (subfield[0] & 0x3f) << " ";

                  subfield += 1;

                  int alt = field2signed(subfield, 16) * 10;
                  *out_str << "altitude=" << alt << " ft (" <<
                     std::setprecision(3) << std::fixed << alt * 0x3048
                     << " m)";

                  subfield += 2;

                  double lat = static_cast<double>(
                           field2signed(subfield, 24) * M_PI / 0x800000
                        );

                  subfield += 3;

                  double lon = static_cast<double>(
                           field2signed(subfield, 24) * M_PI / 0x800000
                         );
                  subfield += 3;

                  geo_to_sstr(lat, lon, *out_str);
                  *out_str << "]";

                  NEW_LINE(*out_str, 4, ' ');

                  switch((subfield[0] >> 4) & 0xf) {
                  	case 0: *out_str << "[ptype=?"; break;
                     case 1: *out_str << "[ptype=fly_by (lt)"; break;
                     case 2: *out_str << "[ptype=fly_over (lt)"; break;
                     case 3: *out_str << "[ptype=hold_patt (lt)"; break;
                     case 4: *out_str << "[ptype=proc_hold (lt)"; break;
                     case 5: *out_str << "[ptype=proc_turn (lt)"; break;
                     case 6: *out_str << "[ptype=RF_leg (lt)"; break;
                     case 7: *out_str << "[ptype=climb_top (vt)"; break;
                     case 8: *out_str << "[ptype=desc_top (vt)"; break;
                     case 9: *out_str << "[ptype=level_start (vt)"; break;
                     case 10: *out_str << "[ptype=cross-over_alt (vt)"; break;
                     case 11: *out_str << "[ptype=trans_alt (vt)"; break;
                  }

                  switch ((subfield[0] >> 2) & 0x3) {
                     case 0: *out_str << " td=n/a"; break;
                     case 1: *out_str << " td=right"; break;
                     case 2: *out_str << " td=left"; break;
                     case 3: *out_str << " td=-"; break;
                  }

                  if(subfield[0] & 0x2) {
                     subfield += 2;
                     double wit = static_cast<double>(
                           field2unsigned(subfield, 16) / 100
                           );

                     *out_str << "tra=1 ttr=" << wit << " NM (" <<
                        wit * 100 << "m)";
                  } else {
                     *out_str << "tra=0";
                     subfield += 2;
                  }

                  if(subfield[0] & 0x01){
                     *out_str << "toa=0]";
                  } else  {
                     subfield += 3;
                     int tov = field2unsigned(field, 24);
                     *out_str << "toa=1 tov=" << tov << "]";
                     //NEW_LINE(*out_str, 0, '\0');
                  }
               }/* for each replies */
            /* if(out_str) */
            } else {
               subfield += 1 + subfield[0] * 15;
            }
			}
			// subfield 10: communications / ACAS capability and flight status
			if (field[1] & 0x20)
			{
				track.CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus ();
				switch ((subfield[0] >> 5) & 7)
				{
					case 0: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_NONE; break;
					case 1: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B; break;
					case 2: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B_ELMU; break;
					case 3: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B_ELMU_ELMD; break;
					case 4: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_LEVEL_5; break;
					default: track.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_NONE; break;		// 5 to 7 not assigned
				}
				switch ((subfield[0] >> 2) & 7)
				{
					case 0: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE; break;
					case 1: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ON_GROUND; break;
					case 2: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE_ALERT; break;
					case 3: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ON_GROUND_ALERT; break;
					case 4: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ALERT_SPI; break;
					case 5: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_SPI; break;
					default: track.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE; break;		// 6 to 7 not assigned
				}
				track.CommCapabilityAndFlightStatus->ModeSSpecificService = (subfield[1] & 0x80) ? false : true;
				track.CommCapabilityAndFlightStatus->AltitudeCapability = (subfield[1] & 0x40) ? true : false;
				track.CommCapabilityAndFlightStatus->IdentificationCapability = (subfield[1] & 0x20) ? true : false;
				track.CommCapabilityAndFlightStatus->B1A = (subfield[1] & 0x10) ? true : false;
				track.CommCapabilityAndFlightStatus->B1A = subfield[1] & 0x0f;

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

            	switch ((subfield[0] >> 5) & 7) {
                  case 0: *out_str << "[COM com=NO"; break;
                  case 1: *out_str << "[COM com=A+B"; break;
                  case 2: *out_str << "[COM com=A+B+Uplink"; break;
                  case 3: *out_str << "[COM com=A+B+Up+Downlink"; break;
                  case 4: *out_str << "[COM com=Level5"; break;
   				}

               switch ((subfield[0] >> 2) & 7) {
                  case 0: *out_str << " stat=airborne"; break;
                  case 1: *out_str << " stat=on_ground"; break;
                  case 2: *out_str << " stat=airborne_alert"; break;
                  case 3: *out_str << " stat=on_ground_alert"; break;
                  case 4: *out_str << " stat=alert_spi"; break;
                  case 5: *out_str << " stat=spi"; break;
               }

               if (subfield[1] & 0x80)
                  *out_str << " ssc=1";
               else
                  *out_str << " ssc=0";

               if (subfield[1] & 0x40)
                  *out_str << " arc=25ft";
               else
                  *out_str << " arc=100ft";

               if (subfield[1] & 0x20)
                  *out_str << " aic=1";
               else
                  *out_str << " aic=0";

               *out_str << "b1a=" << ((subfield[1] >> 4) & 0x1) <<
                  "b1b=" <<  (subfield[1] & 0x0f) << "]";
            }
				subfield += 2;
			}
			// subfield 11: status reported ADS-B
			if (field[1] & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               switch ((subfield[0] >> 6) & 0x3) {
                  case 0: *out_str << "[SAB ac=?"; break;
                  case 1: *out_str << "[SAB ac=-"; break;
                  case 2: *out_str << "[SAB ac=Yes"; break;
                  case 3: *out_str << "[SAB ac=inv"; break;
               }

               switch ((subfield[0] >> 4) & 0x3) {
                  case 0: *out_str << " mn=?"; break;
                  case 1: *out_str << " mn=-"; break;
                  case 2: *out_str << " mn=Yes"; break;
                  case 3: *out_str << " mn=inv"; break;
               }

               switch ((subfield[0] >> 2) & 0x3) {
                  case 0: *out_str << " dc=?"; break;
                  case 1: *out_str << " dc=Yes"; break;
                  case 2: *out_str << " dc=-"; break;
                  case 3: *out_str << " dc=inv"; break;
               }

               if (subfield[0] & 0x2)
                  *out_str << " gbs=Yes";
               else
                  *out_str << " gbs=-";

               switch (subfield[1] & 0x7)	{
                  case 0: *out_str << " stat=-"; break;
                  case 1: *out_str << " stat=emergency"; break;
                  case 2: *out_str << " stat=lifeguard/medical"; break;
                  case 3: *out_str << " stat=minimum_fuel"; break;
                  case 4: *out_str << " stat=no_communications"; break;
                  case 5: *out_str << " stat=hijack"; break;
                  case 6: *out_str << " stat=?"; break;
               }
            }
				subfield += 2;
			}
			// subfield 12: ACAS resolution advisory report
			if (field[1] & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ACS data=0x";
               *out_str << std::hex << std::setw(2) << std::setfill('0');

               for(uint8_t i = 0; i < 7; ++i)
                  *out_str << subfield[i] << ((i < 6) ? " " : "]\n");

               *out_str << std::dec << std::setfill(' ');
            }
				subfield += 7;
			}
			// subfield 13: barometric vertical rate
			if (field[1] & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double rate = field2signed(subfield, 16) * 6.25;
               *out_str << "[BVR rate=" << rate << "ft/min (" <<
                     rate * 0.3048 / 60 <<  "m/s)]";
            }
				subfield += 2;
			}
			// subfield 14: geometric vertical rate
			if (field[1] & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double rate = field2signed(subfield, 16) * 6.25;
               *out_str << "[GVR rate=" << rate << "ft/min (" <<
                     rate * 0.3048 / 60 <<  "m/s)]";

            }
				subfield += 2;
			}

			if (!(field[1] & 1))
			{
            if(out_str) {
               NEW_LINE(*out_str, 0,  '\0');
            }

				goto end380;
			}

			// subfield 15: roll angle
			if (field[2] & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
   				*out_str << "[RAN rollangle=" << std::setprecision(3)
               << std::fixed << field2signed(subfield, 16)/100 << "deg]";

            }
				subfield += 2;
			}
			// subfield 16: track angle rate
			if (field[2] & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               switch (subfield[0] >> 6 & 0x3) {
                  case 0: *out_str << "[TAR ti=n/a"; break;
                  case 1: *out_str << "[TAR ti=left"; break;
                  case 2: *out_str << "[TAR ti=right"; break;
                  case 3: *out_str << "[TAR ti=straight"; break;
				   }

               *out_str << " rate=" << (char)subfield[1] / 8.0
                  << " deg/s]";
            }
				subfield += 2;
			}
			// subfield 17: track angle
			if (field[2] & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

				   *out_str << "[TAN angle=" <<
               field2unsigned(subfield, 16)/ 0x10000 * 360  << "deg]";
            }

				subfield += 2;
			}
			// subfield 18: ground speed
			if (field[2] & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
   				double speed = (double) field2unsigned(subfield, 16)/0x4000;

				   *out_str << "[GSP speed=" << speed * 1852 << " m/s ("
                  << speed * 3600 << " kt)]";
            }

				subfield += 2;
			}
			// subfield 19: velocity uncertainty
			if (field[2] & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
   				*out_str << "[VUN category=" << subfield[0] <<"]";
            }
				subfield += 1;
			}
			// subfield 20: meteorological data
			if (field[2] & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (subfield[0] & 0x80)
                  *out_str << "[MET ws=VAL";
               else
                  *out_str << "[MET ws=INV";

               if (subfield[0] & 0x40)
                  *out_str << " wd=VAL";
               else
                  *out_str << " wd=INV";

               if (subfield[0] & 0x20)
                  *out_str << " tmp=VAL";
               else
                  *out_str << " tmp=INV";

               if (subfield[0] & 0x10)
                  *out_str << " trb=VAL";
               else
                  *out_str << " trb=INV";

               subfield++;

               int kt = field2unsigned(subfield, 16);

               *out_str << " wspeed=" << (int)kt <<" kt (" <<
                  kt * 1852.0 / 3600.0 <<" m/s)]";

               NEW_LINE(*out_str, 4, ' ');

               subfield += 2;

               *out_str << "[wdir=" << (double)field2unsigned(subfield, 16)
                  << "deg";
               subfield += 2;

               *out_str << " temp="<< std::setprecision(2)
                  << std::fixed << (double)field2unsigned(subfield,16)/4.0
                  << "deg";

               subfield += 2;

               *out_str << " turb=" << static_cast<int>(subfield[0])
                  << "]";

               subfield++;
            } else {
               subfield += 8;
            }
			}
			// subfield 21: emitter category
			if (field[2] & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               switch (subfield[0]) {
                  case 1: *out_str << "[EMC light aircraft]"; break;
                  case 3: *out_str << "[EMC medium aircraft]"; break;
                  case 5: *out_str << "[EMC heavy aircraft]"; break;
                  case 6: *out_str << "[EMC highly manoeuvrable and high speed]"; break;
                  case 10: *out_str << "[EMC rotocraft]"; break;
                  case 11: *out_str << "[EMC glider/sailplane]"; break;
                  case 12: *out_str << "[EMC lighter-than-air]"; break;
                  case 13: *out_str << "[EMC unmanned aerial vehicle]"; break;
                  case 14: *out_str << "[EMC space/transatmospheric vehicle]"; break;
                  case 15: *out_str << "[EMC ultralight/handglider/paraglider]"; break;
                  case 16: *out_str << "[EMC parachutist/skydiver]"; break;
                  case 20: *out_str << "[EMC surface emergency vehicle]"; break;
                  case 21: *out_str << "[EMC surface service vehicle]"; break;
                  case 22: *out_str << "[EMC fixed ground or tethered obstruction]"; break;
               }
            }
				subfield += 1;
			}

			if (!(field[2] & 1))
			{
            if(out_str) {
               NEW_LINE(*out_str, 0,  '\0');
            }

				goto end380;
			}

			// subfield 22: position data
			if (field[3] & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double latitude = (double)field2signed(subfield,24) * M_PI / 0x800000;
               subfield += 3;

               double longitude = (double)field2signed(subfield, 24)* M_PI / 0x800000;

               subfield += 3;

               *out_str << "[POS ";
               geo_to_sstr(latitude, longitude, *out_str);
               *out_str << "]";
            } else {
   				subfield += 6;
            }
			}
			// subfield 23: geometric altitude data
			if (field[3] & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double alt = field2signed(subfield, 16) * 6.25;
               *out_str <<  "[GAL altitude="<<  std::setprecision(2)
                  << std::fixed << alt << "ft (" << alt * 0.3048 <<" m)]";
            }

				subfield += 2;
			}
			// subfield 24: position uncertainty data
			if (field[3] & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
              	*out_str <<  "[PUN pun=" <<
                  static_cast<int>(subfield[0] & 0xf)<< "]";
            }

				subfield += 1;
			}
			// subfield 25: mode S MB data
			if (field[3] & 0x10)
			{
				int rep = subfield[0];
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[PUN pun=" << rep << "]";
            }
				subfield += 1;

            for(int i = 0; i < rep; i++) {

               RDFTargetModeS modes;
					memcpy (modes.MessageData, subfield, 7);
					modes.Address = subfield[7];
					track.ModeS.push_back (modes);

               if(out_str) {
                  NEW_LINE(*out_str, 4, ' ');
                  *out_str << "[mb_data=0x" << std::setfill ('0') << std::hex;

                  for(int j = 0; j< 7; j++) {
                     *out_str << std::setw (2) <<
                     static_cast<int>(subfield[j]) << " ";
                     subfield += 1;
                  }

                  *out_str << std::showbase << std::setw (2) << "addr=" <<
                     static_cast<int>(subfield[0]) << "]";

                  *out_str << std::setfill(' ');

                  subfield++;
               } else {
                  subfield += 8;
               }
            }//for each reply
			}
			// subfield 26: indicated airspeed
			if (field[3] & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               int kt = field2unsigned(subfield, 16);
               *out_str << "[IAR speed=" << kt  <<"kt (" <<
                   kt * 1852.0 / 3600.0 << "m/s)]";
            }
				subfield += 2;
			}
			// subfield 27: mach number
			if (field[3] & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MAC speed="<< field2unsigned(subfield, 16) *.0008
               << "Mach]";
            }
				subfield += 2;
			}
			// subfield 28: barometric pressure setting
			if (field[3] & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
   				*out_str << "[BPS presssure=" << field2unsigned(subfield, 12)/10.0
               << "]";
            }

				subfield += 2;
			}

			if (!(field[3] & 1))
			{
            if(out_str) {
         //      NEW_LINE(*out_str, 4, ' ');
            }
				goto end380;
			}

			cms_error ("unable to handle more subfields");
			goto error;

end380:
			field = subfield;
		}

		// track number
		if (*fspec & FSPEC12_I062_040)
		{
			track.TrackNumber.Present = true;
			track.TrackNumber.Number = field2unsigned (field, 16);

         if(out_str) {
            NEW_LINE(*out_str, 0, ' ');
            *out_str << "040 [track=" << std::dec <<
               track.TrackNumber.Number << "]";
         }
			field += 2;
		}

		// track status
		if (*fspec & FSPEC13_I062_080)
		{
			track.Monosensor = (field[0] & 0x80) ? true : false;
			track.SPI = (field[0] & 0x40) ? true : false;
			track.Tentative = (field[0] & 0x02) ? true : false;

			if(out_str) {
            NEW_LINE(*out_str, 0, '\0');

    			if (field[0] & 0x80)
               *out_str << "080 [mon=monosensor";
            else
               *out_str << "080 [mon=multisensor";

            if (field[0] & 0x40)
               *out_str << " spi=Yes";
            else
               *out_str << " spi=-";

            if (field[0] & 0x20)
               *out_str << " mrh=geometric";
            else
               *out_str << " mrh=barometric";

            switch (field[0] >> 2 & 0x7)
            {
               case 0: *out_str << " src=-"; break;
               case 1: *out_str << " src=GNSS"; break;
               case 2: *out_str << " src=3d_radar"; break;
               case 3: *out_str << " src=triangulation"; break;
               case 4: *out_str << " src=height_from_coverage"; break;
               case 5: *out_str << " src=speed_table"; break;
               case 6: *out_str << " src=default_height"; break;
               case 7: *out_str << " src=multilateration"; break;
            }

            if (field[0] & 0x2)
               *out_str << " cnf=TENT]";
            else
               *out_str << " cnf=FIRM]";
         }//out_str

			// first extent
			if (field[0] & 1)
			{
				field += 1;
				track.Simulated = (field[0] & 0x80) ? true : false;
				track.Cancel = (field[0] & 0x40) ? true : false;
				track.Init = (field[0] & 0x20) ? true : false;
				track.AFF = (field[0] & 0x08) ? true : false;
				track.Correlated = (field[0] & 0x10) ? true : false;

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (field[0] & 0x80)
                  *out_str << "[sim=SIM";
               else
                  *out_str << "[sim=ACT";

               if (field[0] & 0x40)
                  *out_str << " tse=cancel";
               else
                  *out_str << " tse=alive";

               if (field[0] & 0x20)
                  *out_str << " tsb=first_msg";
               else
                  *out_str << " tsb=-";

               if (field[0] & 0x10)
                  *out_str << " fpc=correlated";
               else
                  *out_str << " fpc=not_correlated";

               if (field[0] & 0x8)
                  *out_str << " aff=inconsistent";
               else
                  *out_str << " aff=-";

               if (field[0] & 0x4)
                  *out_str << "stp=promotion";
               else
                  *out_str << " stp=-";

               if (field[0] & 0x2)
                  *out_str << " kos=background]";
               else
                  *out_str << " kos=complementary]";

            }
			}

			// second extent
			if (field[0] & 1)
			{
				field += 1;
				track.TargetFoeFri = (enum TARGET_FOEFRI) ((field[0] >> 5) & 0x3);

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (field[0] & 0x80)
                  *out_str << "[ama=amalgam";
               else
                  *out_str << "[ama=not_amalgam";

               switch ((field[0] >> 5) & 0x3)
               {
                  case 0: *out_str << " md4=no_interrogation"; break;
                  case 1: *out_str << " md4=friend"; break;
                  case 2: *out_str << " md4=?"; break;
                  case 3: *out_str << " md4=no_reply"; break;
               }

               if (field[0] & 0x10)
                  *out_str << " me=Yes";
               else
                  *out_str << " me=-";

               if (field[0] & 0x8)
                  *out_str << " mi=Yes";
               else
                  *out_str << " mi=-";

               switch (field[0] >> 1 & 0x3)
               {
                  case 0: *out_str << " md5=no_interrogation]"; break;
                  case 1: *out_str << " md5=friend]"; break;
                  case 2: *out_str << " md5=?]"; break;
                  case 3: *out_str << " md5=no_reply]"; break;
               }
            }//out_str
			}

			// third extent
			if (field[0] & 1)
			{
				field += 1;
				track.CST = (field[0] & 0x80) ? true : false;
				track.PSR = (field[0] & 0x40) ? false : true;
				track.SSR = (field[0] & 0x20) ? false : true;
				track.MDS = (field[0] & 0x10) ? false : true;
				track.ADSB = (field[0] & 0x08) ? false : true;

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (field[0] & 0x80)
                  *out_str << "[CST";
               else
                  *out_str << "[!CST";

               if (field[0] & 0x40)
                  *out_str << " !PSR";
               else
                  *out_str << " PSR";

               if (field[0] & 0x20)
                  *out_str << " !SSR";
               else
                  *out_str << " SSR";

               if (field[0] & 0x10)
                  *out_str << " !MDS";
               else
                  *out_str << " MDS";

               if (field[0] & 0x8)
                  *out_str << " !ADS";
               else
                  *out_str << " ADS";

               if (field[0] & 0x4)
                  *out_str << " SUC";
               else
                  *out_str << " !SUC";

               if (field[0] & 0x2)
                  *out_str << " AAC]";
               else
                  *out_str << " !AAC]";
            }//out_str

			}

      // fourth extent
      if (field[0] & 1)
      {
        field += 1;
        // read items
        //  ...
      }

      // fifth extent
      if (field[0] & 1)
      {
        field += 1;
        // read items
        //  ...
      }

      if (field[0] & 1)
      {
        cms_error ("unable to read more extents");
      }

			field += 1;
		}

		// system track update ages
		if (*fspec & FSPEC14_I062_290)
		{
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');

            *out_str << "290 [present:";
            if (field[0] & 0x80) *out_str << " TRK";
            if (field[0] & 0x40) *out_str << " PSR";
            if (field[0] & 0x20) *out_str << " SSR";
            if (field[0] & 0x10) *out_str << " MDS";
            if (field[0] & 0x08) *out_str << " ADS";
            if (field[0] & 0x04) *out_str << " ES";
            if (field[0] & 0x02) *out_str << " VDL";
            if (field[0] & 0x1)
            {
               if (field[1] & 0x80) *out_str << " UAT";
               if (field[1] & 0x40) *out_str << " LOP";
               if (field[1] & 0x20) *out_str << " MLT";
            }
            *out_str << "]";
         }

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

			// subfield 1: track age
			if (*field & 0x80)
			{
				//cms_info ("track age %d %d", subfield[0], track.Cancel);
				track.Age.Present = true;
				track.Age.Time = (unsigned) subfield[0] * 250;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TRK age=" << subfield[0] / 4.0
                  <<"s]";
            }

				subfield += 1;
			}
			// subfield 2: PSR age
			if (*field & 0x40)
			{
				track.AgePSR.Present = true;
				track.AgePSR.Time = (unsigned) subfield[0] * 250;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[PSR age=" << subfield[0] / 4.0
                  <<"s]";
            }


				subfield += 1;
			}
			// subfield 3: SSR age
			if (*field & 0x20)
			{
				track.AgeSSR.Present = true;
				track.AgeSSR.Time = (unsigned) subfield[0] * 250;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[SSR age=" << subfield[0] / 4.0
                  <<"s]";
            }


				subfield += 1;
			}
			// subfield 4: mode S age
			if (*field & 0x10)
			{
				track.AgeMDS.Present = true;
				track.AgeMDS.Time = (unsigned) subfield[0] * 250;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MDS age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 5: ADS-C age
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ADS age=" <<
                  field2unsigned(subfield, 16) / 4.0 <<"s]";
            }
				subfield += 2;
			}
			// subfield 6: ADS-B extended squitter age
			if (*field & 0x04)
			{
				track.AgeADSB.Present = true;
				track.AgeADSB.Time = (unsigned) subfield[0] * 250;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ES age=" << subfield[0] / 4.0
                  <<"s]";
            }


				subfield += 1;
			}
			// subfield 7: ADS-B VDL mode 4 age
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[VDL age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}

			if (!(*field & 0x01))
			{
				goto end290;
			}

			field++;

			// subfield 8: ADS-B UAT age
			if (*field & 0x80)
			{
            if(out_str) {
            NEW_LINE(*out_str, 4, ' ');
               *out_str << "[UAT age=" << subfield[0] / 4.0
                  <<"s]";
            }


				subfield += 1;
			}
			// subfield 9: loop age
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[LOP age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 10: mutilateration age
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MLT age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}

			if (*field & 0x1E)
			{
				cms_error ("unable to handle spare bit");
				goto error;
			}

			if (!(*field & 0x01))
			{
				goto end290;
			}

			cms_error ("unable to read more extents");
			goto error;

end290:
			field = subfield;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// mode of movement
		if (*fspec & FSPEC15_I062_200)
		{
			switch ((field[0] >> 6) & 3)
			{
				case 0: /* constant course */ break;
				case 1: /* right turn */ break;
				case 2: /* left turn */ break;
				case 3: /* undetermined */ break;
			}
			switch ((field[0] >> 4) & 3)
			{
				case 0: /* constant groundspeed */ break;
				case 1: /* increasing groundspeed */ break;
				case 2: /* decreasing groundspeed */ break;
				case 3: /* undetermined */ break;
			}
			//cms_info ("!!!!! %d", (field[0] >> 2) & 3);
			switch ((field[0] >> 2) & 3)
			{
				case 0: track.TrackClimb = TRACK_CLIMB_MAINTAINING; break;
				case 1: track.TrackClimb = TRACK_CLIMB_CLIMBING; break;
				case 2: track.TrackClimb = TRACK_CLIMB_DESCENDING; break;
				case 3: track.TrackClimb = TRACK_CLIMB_UNKNOWN; break;
			}
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
            switch ((field[0] >> 6) & 3) {
               case 0: *out_str << "200 [trans=constant_course"; break;
               case 1: *out_str << "200 [trans=right_turn"; break;
               case 2: *out_str << "200 [trans=left_turn"; break;
               case 3: *out_str << "200 [trans=undetermined"; break;
            }
            switch ((field[0] >> 4) & 3) {
               case 0: *out_str << " speed=constant"; break;
               case 1: *out_str << " speed=increasing"; break;
               case 2: *out_str << " speed=decreasing"; break;
               case 3: *out_str << " speed=undetermined"; break;
            }
            switch ((field[0] >> 2) & 3) {
               case 0: *out_str << " vert=level]"; break;
               case 1: *out_str << " vert=climb]"; break;
               case 2: *out_str << " vert=descent]"; break;
               case 3: *out_str << " vert=undetermined]"; break;
            }
         }

			field += 1;
		}

		// track data ages
		if (*fspec & FSPEC16_I062_295)
		{
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

         uint8_t diff = 8 * (subfield - field);
         if(diff == 40)  diff = 35;

         if(out_str) {
            NEW_LINE(*out_str, 0, ' ');
            *out_str << "295 [Present " <<
               bit_count(field, diff, 1, 0xfe) <<
               " subfields]";
         }

			// subfield 1: measure flight level age
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MFL age=" << subfield[0] / 4.0
                  <<"s]";
            }

				subfield += 1;
			}
			// subfield 2: mode 1 age
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MD1 age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 3: mode 2 age
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MD2 age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 4: mode 3/A age
			if (*field & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MDA age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 5: mode 4 age
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MD4 age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 6: mode 5 age
			if (*field & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MD6 age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 7: magnetic heading age
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MHG age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}

			if (!(*field & 0x01))
			{
				goto end295;
			}

			field++;

			// subfield 8: indicated airspeed/mach Nb age
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[IAS age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 9: true airspeed age
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TAS age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 10: selected altitude age
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[SAL age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 11: final state selected altitude age
			if (*field & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[FSS age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 12: trajectory intent data age
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TID age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 13: communications / ACAS capability and flight status age
			if (*field & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[COM age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 14: status reported by ADS-B age
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[SAB age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}

			if (!(*field & 1))
			{
				goto end295;
			}

			field++;

			// subfield 15: ACAS resolution advisory report age
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ACS age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 16: barometric vertical rate age
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[BVR age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 17: geomertic vertical rate age
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[GVR age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 18: roll ange age
			if (*field & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[RAN age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 19: track angle rate age
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TAR age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 20: track angle age
			if (*field & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TAN age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 21: sround speed age
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[GSP age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}

			if (!(*field & 0x01))
			{
				goto end295;
			}

			field++;

			// subfield 22: velocity uncertainty age
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[VUN age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 23: meteorological data age
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MET age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 24: emitter category age
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[EMC age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 25: position data age
			if (*field & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[POS age=" << subfield[0] / 4.0
                  <<"s]";
            }


				subfield += 1;
			}
			// subfield 26: geometric altitude data age
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[GAL age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 27: position uncertainly data age
			if (*field & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[PUN age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 28: mode S MB data age
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MB age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}

			if (!(*field & 0x01))
			{
				goto end295;
			}

			field++;

			// subfield 29: indicated airspeed data age
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[IAR age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 30: mach number data age
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[MAC age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			// subfield 31: barometric pressure setting data age
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[BPS age=" << subfield[0] / 4.0
                  <<"s]";
            }
				subfield += 1;
			}
			if (*field & 0x1e)
			{
				cms_error ("unable to handle more bits");
				goto error;
			}

			if (!(*field & 1))
			{
				goto end295;
			}

			field++;

			cms_error ("");
			goto error;

end295:
			field = subfield;
		}

		// measured flight level
		if (*fspec & FSPEC17_I062_136)
		{
			track.FlightLevel = new RDFTargetFlightLevel ();
			track.FlightLevel->Valid = true;
			track.FlightLevel->Garbled = false;
			track.FlightLevel->Height = (double) field2signed (field, 16) * 0.3048 * 25;
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
            double fl = field2signed (field, 16) / 4.0;
            *out_str << "136 [measured fl=" <<  fl <<
               " (" << fl * 100 * 0.3048 <<  " m)]";
         }
			field += 2;
		}

		// calculated track geometric altitude
		if (*fspec & FSPEC18_I062_130)
		{
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');

            double alt = field2signed(field, 16) * 6.25;
            *out_str << "130 [altitude=" << alt << "ft ("
               << alt * 0.3048 << "m)]";
         }

         field += 2;
		}

		// calculated track barometric altitude
		if (*fspec & FSPEC19_I062_135)
		{
		 	track.BarometricHeight = new RDFTargetHeight();
			track.BarometricHeight->QNH = (field[0] & 0x80) ? true : false;
			track.BarometricHeight->Height = (int) rint (field2signed (field, 15) * 0.3048 * 25);

         if(out_str) {

            NEW_LINE(*out_str, 0, ' ');
            if (field[0] & 0x80)
				   *out_str <<  "135 [qnh=Yes";
            else
               *out_str << "135 [qnh=-";

            double fl = field2signed(field, 12) / 4.0;

            *out_str << " fl=" << fl << " (" <<
               fl * 100 * 0.3048 << "m)]";
         }

			field += 2;
		}

		// calculated rate of climb/descent
		if (*fspec & FSPEC20_I062_220)
		{
			track.ClimbSpeed = new RDFTargetClimbSpeed ();
			track.ClimbSpeed->Speed = field2signed (field, 16) * 0.3048 * 6.25 / 60;
         if(out_str) {
            NEW_LINE(*out_str, 0, ' ');
         	double speed = field2signed(field, 16) * 6.25;

			   *out_str << "220 [climb_rate=" <<
               std::setprecision(3) << std::fixed <<
               speed << "ft/min ("
               << speed * 0.3048 / 60 <<"m/s)]";
         }
         field += 2;
		}

		// flight plan related data
		if (*fspec & FSPEC21_I062_390)
		{
			track.FlightPlan = new RDFTargetFlightPlan ();

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

         if(out_str) {
            uint16_t diff = 8 * (subfield - field);
            if (diff == 24) diff = 20;

            NEW_LINE(*out_str, 0, '\0');

            *out_str << "390 [Present " <<
               bit_count(field, diff, true, 0xfe) <<
               " subfields";
         }

			// subfield 1: FPPS indentification tag
			if (*field & 0x80)
         {
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TAG sac=" <<
                  static_cast<int>(field[0]) << "sic="<<
                  static_cast<int>(field[1]) <<  "]";
            }
				subfield += 2;
			}

			// subfield 2: callsign
			if (*field & 0x40)
			{
				track.FlightPlan->Callsign.assign ((char *) subfield, 7);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ID callsig=\"" << track.FlightPlan->Callsign
                  << "\"]";
            }
				subfield += 7;
			}
			// subfield 3: IFPS_FLIGHT_ID
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               switch (subfield[0] >> 6 & 0x3) {
                  case 0: *out_str << "[IFI typ=plan_number"; break;
                  case 1: *out_str << "[IFI typ=unit1_number"; break;
                  case 2: *out_str << "[IFI typ=unit2_number"; break;
                  case 3: *out_str << "[IFI typ=unit3_number"; break;
               }

               *out_str << " nbr=" <<
                  field2unsigned(subfield, 29) << "]";
            }
				subfield += 4;
			}
			// subfield 4: flight category
			if (*field & 0x10)
			{
				switch ((subfield[0] >> 4) & 3)
				{
					case 0: track.FlightPlan->FlightRules = FLIGHT_RULES_IFR; break;
					case 1: track.FlightPlan->FlightRules = FLIGHT_RULES_VFR; break;
					default: track.FlightPlan->FlightRules = FLIGHT_RULES_UNKNOWN;
				}

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               switch (subfield[0] >> 6 & 0x3)	{
                  case 0: *out_str << "[FCT gat/oat=?"; break;
                  case 1: *out_str << "[FCT gat/oat=GAT"; break;
                  case 2: *out_str << "[FCT gat/oat=OAT"; break;
                  case 3: *out_str << "[FCT gat/oat=N/A"; break;
               }

               switch (subfield[0] >> 4 & 0x3) {
                  case 0: *out_str << " fr1/fr2=instrument"; break;
                  case 1: *out_str << " fr1/fr2=visual"; break;
                  case 2: *out_str << " fr1/fr2=n/a"; break;
                  case 3: *out_str << " fr1/fr2=controlled_visual"; break;
               }

               switch (subfield[0] >> 2 & 0x3) {
                  case 0: *out_str << " rvsm=?"; break;
                  case 1: *out_str << " rvsm=approved"; break;
                  case 2: *out_str << " rvsm=exempt"; break;
                  case 3: *out_str << " rvsm=n/a"; break;
               }

               if (subfield[0] & 0x2)
                  *out_str << " hpr=high]";
               else
                  *out_str << " hpr=normal]";

            }

				subfield += 1;
			}
			// subfield 5: type of aircraft
			if (*field & 0x08)
			{
				track.FlightPlan->TypeOfAircraft.assign ((char *) subfield, 4);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[TAC type=\"" <<
                  track.FlightPlan->TypeOfAircraft <<
                  "\"]";
            }
				subfield += 4;
			}
			// subfield 6: wake turbulence category
			if (*field & 0x04)
			{
				track.FlightPlan->WakeTurbulenceCategory = subfield[0];
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               switch (subfield[0]) {
                  case 'L': *out_str << "[WTC category=light]"; break;
                  case 'M': *out_str << "[WTC category=medium]"; break;
                  case 'H': *out_str << "[WTC category=heavy]"; break;
                  case 'J': *out_str << "[WTC category=super]"; break;
               }
            }

				subfield += 1;
			}
			// subfield 7: departure airport
			if (*field & 0x02)
			{
				//cms_info ("!!!!!! departure");
				track.FlightPlan->DepartureAirport.assign ((char *) subfield, 4);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[DEP name=\"" <<
                  track.FlightPlan->DepartureAirport <<
                  "\"]";
            }
				subfield += 4;
			}

			if (!(*field & 0x01))
			{
				goto end390;
			}

			field++;

			// subfield 8: destination airport
			if (*field & 0x80)
			{
				//cms_info ("!!!!!! destination");
				track.FlightPlan->DestinationAirport.assign ((char *) subfield, 4);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[DST name=\"" <<
                  track.FlightPlan->DestinationAirport <<
                  "\"]";
            }

				subfield += 4;
			}
			// subfield 9: runway designation
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[RDS nu1=" << subfield <<
                  " nu2=" << subfield[1] << "ltr=" <<
                  subfield[2] << "]";
            }

				subfield += 3;
			}
			// subfield 10: current cleared flight level
			if (*field & 0x20)
			{
				//cms_info ("!!!!!!cleared flight level");
				track.FlightPlan->ClearedFlightLevelPresent = true;
				track.FlightPlan->ClearedFlightLevel = (int) rint (field2signed (subfield, 16) * 0.3048 * 25);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double fl = field2signed(subfield, 16) / 4.0;
				   *out_str << "[CFL fl=" << fl << "("  <<
                  int (fl * 100 * 0.3048) << "m)]";
            }

				subfield += 2;
			}
			// subfield 11: current control position
			if (*field & 0x10)
			{
				//cms_info ("!!!!! control");
				track.FlightPlan->CurrentControlPresent = true;
				track.FlightPlan->CurrentControlCenter = subfield[0];
				track.FlightPlan->CurrentControlPosition = subfield[1];
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[CTL centre=" <<
                  static_cast<int>(subfield[0]) <<
                  "position=" << static_cast<int>(subfield[1]) <<
                  "]";
            }

            subfield += 2;
			}
			// subfield 12: time of departure / arrival
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               uint16_t rep = subfield[0];
               *out_str << "[TOD rep=" << rep <<"]";
               subfield += 1;

               for(uint16_t i = 0; i < rep; i++) {

                  NEW_LINE(*out_str, 4, ' ');

                  switch (subfield[0] >> 3 & 0x1f)  {
                     case 0: *out_str << "[scheduled off-block tine:"; break;
                     case 1: *out_str << "[estimated off-block tine:"; break;
                     case 2: *out_str << "[estimated take-off time:"; break;
                     case 3: *out_str << "[actual off-block time:"; break;
                     case 4: *out_str << "[predicted time at runway hold:"; break;
                     case 5: *out_str << "[actual time at runway hold:"; break;
                     case 6: *out_str << "[actual line-up time:"; break;
                     case 7: *out_str << "[actual take-off time:"; break;
                     case 8: *out_str << "[estimated time of arrival:"; break;
                     case 9: *out_str << "[predicted landing time:"; break;
                     case 10: *out_str << "[actual landing time:"; break;
                     case 11: *out_str << "[actual time off runway:"; break;
                     case 12: *out_str << "[predicted time to gate:"; break;
                     case 13: *out_str << "[actual on-block time:"; break;
                     default: *out_str << "[unknown type " << (subfield[0] >> 3 & 0x1f); break;
                  }

                  switch (subfield[0] >> 1 & 0x3) {
                     case 0: *out_str << " TODAY"; break;
                     case 1: *out_str << " YESTERDAY"; break;
                     case 2: *out_str << " TOMORROW"; break;
                     case 3: *out_str << " INVALID"; break;
                  }

                  subfield += 1;

                  *out_str << (subfield[0] & 0x1f);

                  subfield += 1;
                  *out_str << (subfield[0] & 0x3f);

                  subfield += 1;
                  if(subfield[0] & 0x80)
                     *out_str << "]";
                  else
                     *out_str << ":" << (subfield[0] & 0x3f)
                        << "]";

                  subfield += 1;

               }/* for each subfield */

            } else {
   				subfield += 1 + subfield[0] * 4;
            }
			}
			// subfield 13: aricraft stand
			if (*field & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               char name[7];
   				name[6] = '\0';
	   			memcpy(name, subfield, 6);

               *out_str << "[AST name=\"" <<
                   name << "\"]";
            }
				subfield += 6;
			}
			// subfield 14: stand status
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               switch (subfield[0] >> 6 & 0x3) {
                  case 0: *out_str << "[STS emp=empty"; break;
                  case 1: *out_str << "[STS emp=occupied"; break;
                  case 2: *out_str << "[STS emp=unknown"; break;
                  case 3: *out_str << "[STS emp=invalid"; break;
               }

               switch (subfield[0] >> 4 & 0x3) {
                  case 0: *out_str << " avl=available]"; break;
                  case 1: *out_str << " avl=not_available]"; break;
                  case 2: *out_str << " avl=unknown]"; break;
                  case 3: *out_str << " avl=invalid]"; break;
               }

            }

				subfield += 1;
			}

			if (!(*field & 0x01))
			{
				goto end390;
			}

			field++;

			// subfield 15: standard instrument departure
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               char name[8];
               name[7] = '\0';
               memcpy (name, subfield, 7);
               *out_str << "[STD sid=\"" << name
                  << "\"]";
            }
				subfield += 7;
			}
			// subfield 16: standart instrument arrival
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               char name[8];
               name[7] = '\0';
               memcpy (name, subfield, 7);
               *out_str <<  "[STA star=\"" << name
                  << "\"]";
            }
				subfield += 7;
			}
			// subfield 17: pre-emergency mode 3/A code
			if (*field & 0x20)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               if(subfield[0] & 0x10)

                  *out_str << "[PEM va=VAL";
               else
                  *out_str << "[PEM va=INV";

               *out_str << " code=" << std::oct <<
                  field2unsigned(subfield, 12)
                  << std::dec << "]";
            }

				subfield += 2;
			}
			// subfield 18: pre-emergency callsign
			if (*field & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               char name[8];
               name[7] = '\0';
               memcpy (name, subfield, 7);
               *out_str << "[PEC star=\""
                  << name <<  "\"]";
            }
				subfield += 7;
			}
			if (*field & 0x08)
			{
				cms_error ("unable to handle bits");
				goto error;
			}
			if (*field & 0x04)
			{
				cms_error ("unable to handle bits");
				goto error;
			}
			if (config_bangalore)
			{
				// specialni subfield: trat
				if (*field & 0x02)
				{
               if(out_str) {
                  NEW_LINE(*out_str, 4, ' ');
                     *out_str << "[ROUTE: ";
               }

					while (subfield[0] & 1)
					{
						track.FlightPlan->Route.push_back (subfield[0] >> 1);
                  if(out_str) {
                     *out_str << (subfield[0] >> 1);
                  }
						subfield += 1;
					}
					track.FlightPlan->Route.push_back (subfield[0] >> 1);
               if(out_str) {
                  *out_str << (subfield[0] >> 1) << "]";
               }

					subfield += 1;
				}
			}
			else
			{
				if (*field & 0x02)
				{
					cms_error ("unable to handle bits");
					goto error;
				}
			}

			if (!(*field & 0x01))
			{
				goto end390;
			}

			cms_error ("unable to handle more subfields");
			goto error;

end390:
			field = subfield;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		// target size & orientation
		if (*fspec & FSPEC22_I062_270)
		{
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
         	*out_str << "270 [length=" <<
               (field[0] >> 1);
         }

         uint32_t i =0;

			while (field[0] & 1)
			{
            if(out_str) {
               switch(i++) {
                  case 0:
                     *out_str << " orientation=" <<
                        (field[0] >> 1) * 360 / 128.0
                        << "deg";
                     break;
                  case 1:
                     *out_str << " width=" <<
                        (field[0] >> 1);
                     break;
               }
            }

				field += 1;
			}

         if(out_str) {
            *out_str << "]";
         }

			field += 1;
		}

		// vehicle fleet identification
		if (*fspec & FSPEC23_I062_300)
		{
         if(out_str) {
            NEW_LINE(*out_str, 0, ' ');
            switch (field[0])	{
               case 0: *out_str << "300 [vfi=unknown]"; break;
               case 1: *out_str << "300 [vfi=atc equipment maintenance]"; break;
               case 2: *out_str << "300 [vfi=airport maintenance]"; break;
               case 3: *out_str << "300 [vfi=fire]"; break;
               case 4: *out_str << "300 [vfi=bird scarer]"; break;
               case 5: *out_str << "300 [vfi=snow plough]"; break;
               case 6: *out_str << "300 [vfi=runway sweeper]"; break;
               case 7: *out_str << "300 [vfi=emergency]"; break;
               case 8: *out_str << "300 [vfi=police]"; break;
               case 9: *out_str << "300 [vfi=bus]"; break;
               case 10: *out_str << "300 [vfi=tug (push/tow)]"; break;
               case 11: *out_str << "300 [vfi=grass cutter]"; break;
               case 12: *out_str << "300 [vfi=fuel]"; break;
               case 13: *out_str << "300 [vfi=baggage]"; break;
               case 14: *out_str << "300 [vfi=catering]"; break;
               case 15: *out_str << "300 [vfi=aircraft maintenance]"; break;
               case 16: *out_str << "300 [vfi=flyco (follow me)]"; break;
            }
         }

			field += 1;
		}

		// mode 5 data reports & extended mode 1 code
		if (*fspec & FSPEC24_I062_110)
		{
			unsigned char * subfield = field;
         if(out_str) {
            NEW_LINE(*out_str, 0, ' ');
            *out_str << "110 [Present:";
            if (field[0] & 0x80) *out_str << " SUM";
            if (field[0] & 0x40) *out_str << " PMN";
            if (field[0] & 0x20) *out_str << " POS";
            if (field[0] & 0x10) *out_str << " GA";
            if (field[0] & 0x08) *out_str << " EM1";
            if (field[0] & 0x04) *out_str << " TOS";
            if (field[0] & 0x02) *out_str << " XP";

            *out_str << "]";
         }

			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			// subfield 1: mode 5 summary
         if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               if (subfield[0] & 0x80)
                  *out_str << "[SUM m5=Yes";
               else
                  *out_str << "[SUM m5=-";

               if (subfield[0] & 0x40)
                  *out_str << " id=auth";
               else
                  *out_str << " id=no_auth";

               if (subfield[0] & 0x20)
                  *out_str << " da=auth";
               else
                  *out_str << " da=no_auth";

               if (subfield[0] & 0x10)
                  *out_str << " m1=Yes]";
               else
                  *out_str << " m1=-]";

               NEW_LINE(*out_str, 4, ' ');
               if (subfield[0] & 0x8)
                  *out_str << "[m2=Yes";
               else
                  *out_str << "[m2=-";

               if (subfield[0] & 0x4)
                  *out_str << " m3=Yes";
               else
                  *out_str << " m3=-";

               if (subfield[0] & 0x2)
                  *out_str << " mc=Yes";
               else
                  *out_str << " mc=-";

               if (subfield[0] & 0x1)
                  *out_str << " x=Yes]";
               else
                  *out_str << " x=-]";
            }

				subfield += 1;
			}
			// subfield 2: mode 5 PIN / national origin / mission code
			if (*field & 0x40)
			{

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[PMN pin=" <<
                  field2unsigned(subfield, 10) <<
				      " nat="  << (subfield[2] & 0x1f) <<
				      " mis=" <<  (subfield[3] & 0x1f) <<
                  "]";
            }

				subfield += 4;
			}
			// subfield 3: mode 5 reported position
			if (*field & 0x20)
			{
            if(out_str) {

               NEW_LINE(*out_str, 4, ' ');

               *out_str << "[POS";
               double lat = static_cast<double>(
                  field2signed(subfield, 24) *
                     M_PI / 0x800000
                  );

               subfield += 3;

               double lon = static_cast<double>(
               field2signed(subfield, 24) *
                  M_PI / 0x800000
                );

               subfield += 3;

               geo_to_sstr(lat, lon, *out_str);

               *out_str << "]";

            }

				subfield += 6;
			}
			// subfield 4: mode 5 GNSS-derived altitude
			if (*field & 0x10)
			{
            if(out_str) {

               NEW_LINE(*out_str, 4, ' ');

               if (subfield[0] & 0x40)
                  *out_str << "[GA res=25 ft";
               else
                  *out_str << "[GA res=100 ft";

               int ft = field2signed(subfield, 10) * 25;
               *out_str << " ga=" << ft * 0.3048 <<
                  " m (" << ft << " ft)]";
            }

				subfield += 2;
			}
			// subfield 5: extended mode 1 code in octal reprezentation
			if (*field & 0x08)
			{
				track.Mode1 = new RDFTargetMode ();
				track.Mode1->Valid = true;
				track.Mode1->Code = field2unsigned (subfield, 12);
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << std::showbase << std::oct << "[EM1 code=" <<
                  track.Mode1->Code << std::hex <<
                  " (" << track.Mode1->Code << ")]";
            }
				subfield += 2;
			}
			// subfield 6: time offset for POS and GA
			if (*field & 0x04)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double time = char (subfield[0]) / 128.0;
               *out_str << "[TOS tos=" << time
                  << "sec]";
            }

				subfield += 1;
			}
			// subfield 7: X pulse presence
			if (*field & 0x02)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (subfield[0] & 0x10)
                  *out_str << "[XP x5=Yes";
               else
                  *out_str << "[XP x5=-";

               if (subfield[0] & 0x8)
                  *out_str << " xc=Yes";
               else
                  *out_str << " xc=-";

               if (subfield[0] & 0x4)
                  *out_str << " x3=Yes";
               else
                  *out_str << " x3=-";

               if (subfield[0] & 0x2)
                  *out_str << " x2=Yes";
               else
                  *out_str << " x2=-";

               if (subfield[0] & 0x1)
                  *out_str << " x1=Yes";
               else
                  *out_str << " x1=-";

            }
				subfield += 1;
			}

			if (*field & 0x01)
			{
				cms_error ("unable to handle more subfields");
				goto error;
			}

			field = subfield;
		}

		// track mode 2 code
		if (*fspec & FSPEC25_I062_120)
		{
			//cms_info ("---- mode2");
			track.Mode2 = new RDFTargetMode ();
			track.Mode2->Valid = true;
			track.Mode2->Code = field2unsigned (field, 12);
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');
            *out_str << "120 [code=" << track.Mode2->Code << "]";
         }

			field += 2;
		}

		// composed track number
		if (*fspec & FSPEC26_I062_510)
		{
         if(out_str) {
            NEW_LINE(*out_str, 0, '0');
            *out_str <<  "510 [Master system_unit=" <<
               static_cast<int>(field[0]) <<
               "system_track=" << field2unsigned(field, 15) <<
               "]";
         }

			while (field[2] & 1)
			{
				field += 3;

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               *out_str <<  "[slave system_unit=" <<
               static_cast<int>(field[0]) <<
               "system_track=" << field2unsigned(field, 15) <<
               "]";
            }
			}
			field += 3;
		}

		// estimated accuracies
		if (*fspec & FSPEC27_I062_500)
		{
			unsigned char * subfield = field;
         if(out_str) {
            *out_str << "500 [Present:";
            if (field[0] & 0x80) *out_str << " APC";
            if (field[0] & 0x40) *out_str << " COV";
            if (field[0] & 0x20) *out_str << " APW";
            if (field[0] & 0x10) *out_str << " AGA";
            if (field[0] & 0x08) *out_str << " ABA";
            if (field[0] & 0x04) *out_str << " ATV";
            if (field[0] & 0x02) *out_str << " AA";
            if ((field[0] & 0x01) && (field[1] & 0x80))
            *out_str << " ARC";

            *out_str << "]\n";
         }

			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			// subfield 1: estimated accuracy of track position (cartesian)
			if (*field & 0x80)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->Cartesian = new RDFCoorCartesian ();
				track.Accuracy->Cartesian->X = field2unsigned (subfield, 16) * 0.5;
				subfield += 2;
				track.Accuracy->Cartesian->Y = field2unsigned (subfield, 16) * 0.5;
				subfield += 2;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "APC apc_x=" << track.Accuracy->Cartesian->X <<
                  "m apc_y=" << track.Accuracy->Cartesian->Y << "m]";
            }

			}

			// subfield 2: XY covariance
			if (*field & 0x40)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->CovXY = new RDFTargetCartesianCovariance ();
				track.Accuracy->CovXY->Cov = field2signed (subfield, 16) * 0.5;

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[COV cov_xy=" << std::setprecision(3) <<
                  std::fixed << track.Accuracy->CovXY->Cov << " m]";
            }

				subfield += 2;
			}

			// subfield 3: estimated accuracy of track position (WGS-84)
			if (*field & 0x20)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->Geographic = new RDFCoorGeographic ();
				track.Accuracy->Geographic->Latitude = (field2unsigned (subfield, 16) >> 25 ) * M_PI;
				subfield += 2;
				track.Accuracy->Geographic->Longitude = (field2unsigned (subfield, 16) >> 25 ) * M_PI;
				subfield += 2;

            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[APW";
               geo_to_sstr(track.Accuracy->Geographic->Latitude,
                  track.Accuracy->Geographic->Latitude, *out_str);
               *out_str << "]";
            }

			}
			// subfield 4: estimated accuracy of calculated track geometric altitude
			if (*field & 0x10)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->GeometricHeight = new RDFTargetHeight ();
				track.Accuracy->GeometricHeight->Height = ((unsigned) subfield[0]) * 6.25 * 0.3048;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[AGA accuracy=" <<
                  track.Accuracy->GeometricHeight->Height /0.3048 <<
                  " fl( " << track.Accuracy->GeometricHeight->Height <<
                  " m )]";
            }

				subfield += 1;

			}

			// subfield 5: estimated accuracy of calculated track barometric altitude
			if (*field & 0x08)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->BarometricHeight = new RDFTargetHeight ();
				track.Accuracy->BarometricHeight->Height = ((unsigned) subfield[0]) * 25. * 0.3048;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
				   *out_str << "[ABA accuracy=" <<
                  track.Accuracy->BarometricHeight->Height/30.48 <<
                  " fl" <<	track.Accuracy->BarometricHeight->Height <<
                  " m)]";
            }
				subfield += 1;
			}

			// subfield 6: estimated accuracy of track velocity (cartesian)
			if (*field & 0x04)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->CartesianSpeed = new RDFCoorCartesian ();
				track.Accuracy->CartesianSpeed->X = ((unsigned) subfield[0]) * 0.25;
				track.Accuracy->CartesianSpeed->Y = ((unsigned) subfield[1]) * 0.25;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ATV atv_x=" <<
                  track.Accuracy->CartesianSpeed->X <<
                  " m/s atv_y=" << track.Accuracy->CartesianSpeed->Y
                  << " m/s]";
            }

				subfield += 2;
			}

			// subfield 7: estimated accuracy of acceleration
			if (*field & 0x02)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->Acceleration = new RDFCoorCartesian ();
				track.Accuracy->Acceleration->X = ((unsigned) subfield[0]) * 0.25;
				track.Accuracy->Acceleration->Y = ((unsigned) subfield[1]) * 0.25;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[AA aa_x=" <<
                  track.Accuracy->Acceleration->X <<
                  " m/s aa_y=" << track.Accuracy->Acceleration->Y
                  << " m/s]";
            }
				subfield += 2;
			}

			if (!(*field & 0x01))
			{
				goto end500;
			}

			field++;

			// subfield 8: estimated accuracy of rate of climb/descent
			if (*field & 0x80)
			{
				if (!track.Accuracy)
				{
					track.Accuracy = new RDFTargetAccuracy ();
				}
				track.Accuracy->ClimbSpeed = new RDFTargetClimbSpeed ();
				track.Accuracy->ClimbSpeed->Speed = ((unsigned) subfield[0]) * 6.25 * 0.3048 / 60;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[ARC accuracy=" << " ft/min" <<
                  track.Accuracy->ClimbSpeed->Speed *60.0/0.3048 <<
                  " (=" << track.Accuracy->ClimbSpeed->Speed
                  << " m/s)]";
            }

				subfield += 1;
			}

			if (*field & 0x7e)
			{
				cms_error ("unable to handle bits");
				goto error;
			}

			if (!(*field & 0x01))
			{
				goto end500;
			}

			cms_error ("error unable to handle more subfields");
			goto error;

end500:
			field = subfield;
		}

		// measured information
		if (*fspec & FSPEC28_I062_340)
		{
			unsigned char * subfield = field;
			if(out_str) {
            NEW_LINE(*out_str, 0, '\0');

            *out_str << "340 [Present:";
            if (field[0] & 0x80) *out_str << " SID";
            if (field[0] & 0x40) *out_str << " POS";
            if (field[0] & 0x20) *out_str << " HEI";
            if (field[0] & 0x10) *out_str << " MDC";
            if (field[0] & 0x08) *out_str << " MDA";
            if (field[0] & 0x04) *out_str << " TYP";

            *out_str << "]";
         }
			while (*subfield & 0x01)
			{
				if (++subfield >= msg + len)
				{
					cms_error ("end of message");
					goto error;
				}
			}
			subfield += 1;

			// subfield 1: sensor identification
			if (*field & 0x80)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               *out_str << "[SID sac=" << static_cast<int>(subfield[0])
                  << " sic=" << static_cast<int>(subfield[1]) << "\n";
            }

				subfield += 2;
			}
			// subfield 2: measured position
			if (*field & 0x40)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double rho = field2unsigned(subfield, 16) * 1852 / 256;
               double theta = ((double)
                     field2unsigned(subfield + 2,16) *
                     (2 * M_PI) / 0x10000);

               *out_str << "[POS rho=" << int(rho) << " m (" <<
                    rho/1852 << " NM) theta=" << theta <<
                    " (" <<  theta / (2 * M_PI) * 360;
            }

				subfield += 4;
			}
			// subfield 3: measured 3D height
			if (*field & 0x20)
			{
				track.MeasuredHeight = new RDFTargetHeight ();
				track.MeasuredHeight->Height = field2unsigned(subfield, 16) * 25 * 0.3048;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               double fl = field2unsigned(subfield, 16) / 4.0;
               *out_str << "[HEI height=" << fl << "fl (" <<
                     fl * 100 * 0.3048 << "m)]";
            }

				subfield += 2;
			}
			// subfield 4: last measured mode C code
			if (*field & 0x10)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (subfield[0] & 0x80)
                  *out_str << "[MDC v=INV";
               else
                  *out_str << "[MDC v=VAL";

               if (subfield[0] & 0x40)
                  *out_str << " g=GARBLED";
               else
                  *out_str << " g=-";
               double fl = field2signed (subfield, 14) / 4.0;
               *out_str << " fl=" << fl << "( " << fl * 100.0 * 0.3048
                  << "m)]";
            }

				subfield += 2;
			}
			// subfield 5: last measured mode 3/A code
			if (*field & 0x08)
			{
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');

               if (subfield[0] & 0x80)
                  *out_str << "[MDA v=INV";
               else
                  *out_str << "[MDA v=VAL";

               if (subfield[0] & 0x40)
                  *out_str << " g=GARBLED";
               else
                  *out_str << " g=-";

               if (subfield[0] & 0x20)
                  *out_str << " l=TRACKED";
               else
                  *out_str << " l=-";

               *out_str << " code=" << std::oct <<
                  field2unsigned(subfield, 16) << "]" << std::dec;
            }

				subfield += 2;
			}
			// subfield 6: report type
			if (*field & 0x04)
			{
				switch (subfield[0] >> 5)
				{
					case 0:
					{
						track.TargetType = TARGET_TYPE_UNKNOWN;
						break;
					}
					case 1:
					{
						track.TargetType = TARGET_TYPE_PRIMARY;
						break;
					}
					case 2:
					{
						track.TargetType = TARGET_TYPE_SECONDARY;
						break;
					}
					case 3:
					{
						track.TargetType = TARGET_TYPE_COMBINED;
						break;
					}
					case 4:
					{
						track.TargetType = TARGET_TYPE_MODES_ALL_CALL;
						break;
					}
					case 5:
					{
						track.TargetType = TARGET_TYPE_MODES_ROLL_CALL;
						break;
					}
					case 6:
					{
						track.TargetType = TARGET_TYPE_MODES_ALL_CALL_PSR;
						break;
					}
					case 7:
					{
						track.TargetType = TARGET_TYPE_MODES_ROLL_CALL_PSR;
						break;
					}
				}
				//track.Simulated = (subfield[0] & 0x10) ? true : false;
				track.RAB = (subfield[0] & 0x08) ? true : false;
				track.Tested = (subfield[0] & 0x04) ? true : false;
            if(out_str) {
               NEW_LINE(*out_str, 4, ' ');
               switch ((subfield[0] >> 5) & 0x7)
               {
                  case 0: *out_str << "[TYP typ=?"; break;
                  case 1: *out_str << "[TYP typ=PSR"; break;
                  case 2: *out_str << "[TYP typ=SSR"; break;
                  case 3: *out_str << "[TYP typ=Combined"; break;
                  case 4: *out_str << "[TYP typ=ALL-CALL"; break;
                  case 5: *out_str << "[TYP typ=ROLL-CALL"; break;
                  case 6: *out_str << "[TYP typ=ALL-CALL + PSR"; break;
                  case 7: *out_str << "[TYP typ=ROLL-CALL + PSR"; break;
               }

               if (subfield[0] & 0x10)
                  *out_str << " sim=SIM";
               else
                  *out_str << " sim=ACT";

               if (subfield[0] & 0x8)
                  *out_str << " rab=FIX";
               else
                  *out_str << " rab=AIR";

               if (subfield[0] & 0x4)
                  *out_str << " tst=TEST]";
               else
                  *out_str << " tst=REAL]";
            }

				subfield += 1;
			}

			if (*field & 0x02)
			{
				cms_error ("unable to handle bits");
				goto error;
			}

			if (*field & 0x01)
			{
				cms_error ("unable to handle more subfields");
				goto error;
			}

			field = subfield;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

		if (*fspec & FSPEC29_I062_XX)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC30_I062_XX)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC31_I062_XX)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC32_I062_XX)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC33_I062_XX)
		{
			cms_error ("unable to handle spare bit");
			goto error;
		}

		if (*fspec & FSPEC34_I062_RE)
		{
         if(out_str) {
            NEW_LINE(*out_str, 0, '\0');

            int rep = field[0];
            *out_str << "RE  [bytes=" << rep-1 << "]";

            field += 1;
            NEW_LINE(*out_str, 0, '\0');

            *out_str << "[0x" << std::hex;

            for (int i = 1; i < rep; ++i) {
               if (i == rep - 1) {
                  *out_str <<  static_cast<int>(field[0]);
               } else if (i % 10) {
                  *out_str <<  static_cast<int>(field[0]) << " ";
               } else {
                  *out_str << static_cast<int>(field[0]);
                  if(one_line == false)
                     *out_str << "\n";
               }
               field += 1;
            }

            *out_str << "]";
         } else {
            field += field[0];
         }
		}
      //TODO:
		if (*fspec & FSPEC35_I062_SP)
		{
			if (config_nosp)
			{
				field += field[0];
			}

			else
			{
				unsigned char * subfspec = field + 1;
				unsigned char * subfield = subfspec;

				while (*subfield & 1)
				{
					subfield++;
				}

				subfield++;

				// user text
				if (*subfspec & 0x80)
				{
					track.UserText.assign ((char *) subfield + 1, (unsigned) subfield[0]);
					subfield += subfield[0] + 1;

				}

				field += field[0];
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
		// zkontrolujeme track number
		if (!track.TrackNumber.Present)
		{
			cms_error ("track number is not present");
			goto error;
		}

		// pridame polozku do seznamu
		track_list.push_back (track);

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error ("wrong packet length (%d > %d)", record - msg, size);
		goto error;
	}

   if(out_str) {
      NEW_LINE(*out_str, 0, '\0');
   }

	return true;

error:
	cms_data (size, msg);
	track_list.clear();
	return false;
}

int TargetTrack_to_Asterix062 (deque<RDFTargetTrack> & track_list,
	void * data, int maxsize)
{
	bool config_nosp = false;
	const char * config;
	unsigned char buffer[4096];
	unsigned char buffer2[4096];
	unsigned char * msg;
	unsigned char * record;
	unsigned char * fspec;
	unsigned char * field;
	int fspec_size;
	int field_size;
	int size;

	if ((config = g_getenv ("ASTERIX062")))
	{
		if (!strcmp (config, "NOSP"))
		{
			config_nosp = true;
		}
		else
		{
			cms_warning ("unknown ASTERIX062 value");
		}
	}

	memset (data, 0, maxsize);
	msg = (unsigned char *) data;
	msg[0] = 62; // kategorie
	msg[1] = 0; // delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if (track_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (track_list.size () > 0)
	{
		RDFTargetTrack & track = track_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// data source identifier
		if (1)
		{
			//INFO ("SIC %d, SAC %d", track.SIC, track.SAC);
			*fspec |= FSPEC01_I062_010;
			field[0] = track.SAC;
			field[1] = track.SIC;
			field += 2;
		}

		// spare
		// FSPEC02_I062_SPA

		// service identification
		// FSPEC03_I062_015

		// time of day
		if (track.Time.Present)
		{
			*fspec |= FSPEC04_I062_070;
			int time = RDF_time_to_asterix_time(track.Time.Time);
			field[0] = (time >> 16) & 0xff;
			field[1] = (time >> 8) & 0xff;
			field[2] = time & 0xff;
			field += 3;
		}

		// calculated track position (WGS-84)
		if (track.Geographic)
		{
			*fspec |= FSPEC05_I062_105;
			long long lat = llrint (track.Geographic->Latitude * 0x2000000 / M_PI);
			long long lon = llrint (track.Geographic->Longitude * 0x2000000 / M_PI);
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

		// calculated track position (cartesian)
		if (track.Cartesian)
		{
			*fspec |= FSPEC06_I062_100;
			long x = lrint (track.Cartesian->X * 2);
			long y = lrint (track.Cartesian->Y * 2);
			if (x > 0x7FFFFF || x < -0x800000)
			{
				cms_warning ("x out of limit");
				goto skip;
			}
			if (y > 0x7FFFFF || y < -0x800000)
			{
				cms_warning ("y out of limit");
				goto skip;
			}
			field[0] = (x >> 16) & 0xff;
			field[1] = (x >> 8) & 0xff;
			field[2] = x & 0xff;
			field[3] = (y >> 16) & 0xff;
			field[4] = (y >> 8) & 0xff;
			field[5] = y & 0xff;
			field += 6;
		}

		// calculated track velocity (cartesian)
		if (track.GroundSpeed)
		{
			*fspec |= FSPEC07_I062_185;
			int vx = (int) rint (cos (M_PI_2 - track.GroundSpeed->Heading)
				* track.GroundSpeed->Speed * 4);
			field[0] = (vx >> 8) & 0xff;
			field[1] = vx & 0xff;
			int vy = (int) rint (sin (M_PI_2 - track.GroundSpeed->Heading)
				* track.GroundSpeed->Speed * 4);
			field[2] = (vy >> 8) & 0xff;
			field[3] = vy & 0xff;
			field += 4;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// calculated acceleration (cartesian)
		// FSPEC08_I062_210
		if (track.Acceleration)
		{
			*fspec |= FSPEC08_I062_210;
			field[0] = track.Acceleration->X < 63.75 ? (char) track.Acceleration->X << 2 : 0xff;
			field[1] = track.Acceleration->Y < 63.75 ? (char) track.Acceleration->Y << 2 : 0xff;
			field += 2;
		}

		// track mode 3/A code
		if (track.Mode3A)
		{
			*fspec |= FSPEC09_I062_060;
			field[0] = (track.Mode3A->Code >> 8) & 0x0f;
			field[1] = track.Mode3A->Code & 0xff;
			field += 2;
		}

		// target identification
		if (track.AircraftIdentification.size ())
		{
			*fspec |= FSPEC10_I062_245;
			field[0] = 0;
			asterix_encode_callsign (track.AircraftIdentification, field + 1);
			field += 7;
		}

		// aircraft derived data
		if (1)
		{
		  	static const unsigned char c_maxsize380 = 99;
 			unsigned char buffer380[c_maxsize380];		// buffer na vsechna data v 380
			unsigned char * subfield = buffer380;     	// ukazatel na aktualni pozici v bufferu
			unsigned char * primary_subfield = field; 	// ukazatel na aktualni pozici v hlavicce
			memset(buffer380, 0, sizeof(buffer380));

			// subfield 1: target address
			if (track.AircraftAddress.Present)
			{
				*primary_subfield |= 0x80;
				subfield[0] = (track.AircraftAddress.Address >> 16) & 0xff;
				subfield[1] = (track.AircraftAddress.Address >> 8) & 0xff;
				subfield[2] = track.AircraftAddress.Address & 0xff;
				subfield += 3;
			}

			// subfield 2: target identification
			if (track.AircraftIdentification.size ())
			{
				*primary_subfield |= 0x40;
				asterix_encode_callsign (track.AircraftIdentification, subfield);
				subfield += 6;
			}

			// subfield 3: magnetic heading
			// subfield 4: indicated airspeed
			// subfield 5: true airspeed
			// subfield 6: selected altitude
			// subfield 7: final state selected altitude

			*primary_subfield |= FSPEC_FX;
			primary_subfield++;

			// subfield 8: trajectory intent status
			// subfield 9: trajectory intent data
			// subfield 10: communications / ACAS capability and flight status
			if (track.CommCapabilityAndFlightStatus)
			{
				char i = 0;
				*primary_subfield |= 0x20;
				switch (track.CommCapabilityAndFlightStatus->CommCapability)
				{
					case COMM_CAPABILITY_NONE: i = 0; break;
					case COMM_CAPABILITY_A_B: i = 1; break;
					case COMM_CAPABILITY_A_B_ELMU: i = 2; break;
					case COMM_CAPABILITY_A_B_ELMU_ELMD: i = 3; break;
					case COMM_CAPABILITY_LEVEL_5: i = 4; break;
				}
				subfield[0] |= i << 5;
				switch (track.CommCapabilityAndFlightStatus->FlightStatus)
				{
					case FLIGHT_STATUS_AIRBORNE: i = 0; break;
					case FLIGHT_STATUS_ON_GROUND: i = 1; break;
					case FLIGHT_STATUS_AIRBORNE_ALERT: i = 2; break;
					case FLIGHT_STATUS_ON_GROUND_ALERT: i = 3; break;
					case FLIGHT_STATUS_ALERT_SPI: i = 4; break;
					case FLIGHT_STATUS_SPI: i = 5; break;
					case FLIGHT_STATUS_GENERAL_EMERGENCY:
					case FLIGHT_STATUS_LIFEGUARD:
					case FLIGHT_STATUS_MINIMUM_FUEL:
					case FLIGHT_STATUS_NO_COMMUNICATIONS:
					case FLIGHT_STATUS_UNLAWFUL_INTERFERENCE: i = 0; break;
				}
				subfield[0] |= i << 2;
				subfield[1] = track.CommCapabilityAndFlightStatus->ModeSSpecificService ? 0 : 0x80;
				subfield[1] |= track.CommCapabilityAndFlightStatus->AltitudeCapability ? 0x40 : 0;
				subfield[1] |= track.CommCapabilityAndFlightStatus->IdentificationCapability ? 0x20 : 0;
				subfield[1] |= track.CommCapabilityAndFlightStatus->B1A ? 0x10 : 0;
				subfield[1] |= track.CommCapabilityAndFlightStatus->B1B & 0x0f;
				subfield += 2;
			}

			// subfield 11: status reported ADS-B
			// subfield 12: ACAS resolution advisory report
			// subfield 13: barometric vertical rate
			// subfield 14: geometric vertical rate

			*primary_subfield |= FSPEC_FX;
			primary_subfield++;

		        // subfield 15: roll angle
			// subfield 16: track angle rate
			// subfield 17: track angle
			// subfield 18: ground speed
			// subfield 19: velocity uncertainty
			// subfield 20: meteorological data
			// subfield 21: emitter category

			*primary_subfield |= FSPEC_FX;
			primary_subfield++;

			// subfield 22: position data
			// subfield 23: geometric altitude data
			// subfield 24: position uncertainty data
			// subfield 25: mode S MB data
			if (track.ModeS.size ())
			{
				*primary_subfield |= 0x10;
				unsigned char * subfield1 = subfield;
				unsigned char rep = *subfield1 = 0;
				subfield += 1;
				for (unsigned i = 0; i < track.ModeS.size (); i++)
				{
					memcpy (subfield, track.ModeS[i].MessageData, 7);
					subfield[7] = track.ModeS[i].Address;
					subfield += 8;
					rep += 1;
				}
				*subfield1 = rep;
			}

			// subfield 26: indicated airspeed
			// subfield 27: mach number
			// subfield 28: barometric pressure setting

 			// vymazeme nevyuzite polozky primary_subfield
			while (!(*primary_subfield & ~FSPEC_FX) && primary_subfield > field)
			{
				primary_subfield--;
			}
			*primary_subfield &= ~FSPEC_FX;
			int subfield_size = subfield - buffer380;
			if (subfield_size > c_maxsize380)
			{
				cms_error("item 380 exceeded buffer size");
				break;
			}

			if (subfield_size) // aspon jedna nenulova polozka
			{
				*fspec |= FSPEC11_I062_380;
  			 	// konec polozky primary subfield a ukazujeme na zacatek dat
				primary_subfield++;
				memcpy (primary_subfield,buffer380,subfield_size);
				field = primary_subfield + subfield_size;
			}
		}

		// track number
		if (!track.TrackNumber.Present)
		{
			cms_error ("track number must be defined");
			return 0;
		}
		else
		{
			*fspec |= FSPEC12_I062_040;
			field[0] = (track.TrackNumber.Number >> 8) & 0xff;
			field[1] = track.TrackNumber.Number & 0xff;
			field += 2;
		}

		// track status
		if (track.TrackNumber.Present)
		{
			*fspec |= FSPEC13_I062_080;

			field[0]  = track.Monosensor ? 0x80 : 0;
			field[0] |= track.SPI ? 0x40 : 0;
			field[0] |= track.Tentative ? 0x02 : 0;
			field[0] |= 1;
			field++;

			field[0]  = track.Simulated ? 0x80 : 0;
			field[0] |= track.Cancel ? 0x40 : 0;
			field[0] |= track.Init ? 0x20 : 0;
			field[0] |= track.AFF ? 0x08 : 0;
			field[0] |= track.Correlated ? 0x10 : 0;
			field[0] |= 1;
			field++;

			field[0] = (track.TargetFoeFri & 0x3) << 5;
			field[0] |= 1;
			field++;

			field[0]  = track.CST ? 0x80 : 0;
			field[0] |= track.PSR ? 0 : 0x40;
			field[0] |= track.SSR ? 0 : 0x20;
			field[0] |= track.MDS ? 0 : 0x10;
			field[0] |= track.ADSB ? 0 : 0x08;
			field++;
		}

		// system track update ages
		if (track.Age.Present
			|| track.AgePSR.Present
			|| track.AgeSSR.Present
			|| track.AgeMDS.Present
			|| track.AgeADSB.Present)
		{
			*fspec |= FSPEC14_I062_290;
			unsigned char * subfield = field;
			*subfield = 0;
			field += 1;

			if (track.Age.Present)
			{
				*subfield |= 0x80;
				field[0] = track.Age.Time > 63500 ? 0xff : track.Age.Time / 250;
				field++;
			}
			if (track.PSR)
			{
				*subfield |= 0x40;
				field[0] = track.AgePSR.Time > 63500 ? 0xff : track.AgePSR.Time / 250;
				field++;
			}
			if (track.SSR)
			{
				*subfield |= 0x20;
				field[0] = track.AgeSSR.Time > 63500 ? 0xff : track.AgeSSR.Time / 250;
				field++;
			}
			if (track.MDS)
			{
				*subfield |= 0x10;
				field[0] = track.AgeMDS.Time > 63500 ? 0xff : track.AgeMDS.Time / 250;
				field++;
			}
			if (track.ADSB)
			{
				*subfield |= 0x04;
				field[0] = track.AgeADSB.Time > 63500 ? 0xff : track.AgeADSB.Time / 250;
				field++;
			}
		}

		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// mode of movement
		if (track.TrackClimb != TRACK_CLIMB_UNKNOWN)
		{
			*fspec |= FSPEC15_I062_200;
			field[0]  = 3 << 6;
			field[0] |= 3 << 4;

			switch (track.TrackClimb)
			{
				case TRACK_CLIMB_MAINTAINING:
				{
					field[0] |= 0 << 2;
					break;
				}
				case TRACK_CLIMB_CLIMBING:
				{
					field[0] |= 1 << 2;
					break;
				}
				case TRACK_CLIMB_DESCENDING:
				{
					field[0] |= 2 << 2;
					break;
				}
				case TRACK_CLIMB_UNKNOWN:
				{
					field[0] |= 3 << 2;
					break;
				}
			}

			field += 1;
		}

		// track data ages
		// FSPEC16_I062_295

		// measured flight level
		if (track.FlightLevel)
		{
			*fspec |= FSPEC17_I062_136;
			int height = (int) rint (track.FlightLevel->Height / 0.3048 / 25);
			field[0] = (height >> 8) & 0xff;
			field[1] = height & 0xff;
			field += 2;
		}

		// calculated track geometric altitude
		// FSPEC18_I062_130

		// calculated track barometric altitude
		// FSPEC19_I062_135
		if (track.BarometricHeight)
		{
			*fspec |= FSPEC19_I062_135;
			int height = (int) rint (track.BarometricHeight->Height / 0.3048 / 25);
			field[0]|= track.BarometricHeight->QNH ? 0x80 : 0x00;
			field[0] = (height >> 8) & 0x7f;
			field[1] = height & 0xff;
			field += 2;
		}

		// calculated rate of climb/descent
		if (track.ClimbSpeed)
		{
			*fspec |= FSPEC20_I062_220;
			int speed = (int) rint (track.ClimbSpeed->Speed * 60 / 6.25 / 0.3048);
			field[0] = (speed >> 8) & 0xff;
			field[1] = speed & 0xff;
			field += 2;
		}

		// flight plan related data
		if (track.FlightPlan)
		{
			*fspec |= FSPEC21_I062_390;
			unsigned char * subfield = buffer2;
			memset (buffer2, 0, sizeof (buffer2));

			// subfield 2: callsign
			if (track.FlightPlan->Callsign.size ())
			{
				*field |= 0x40;
				strncpy ((char *) subfield, track.FlightPlan->Callsign.c_str (), 7);
				subfield += 7;
			}

			// subfield 4: flight category
			switch (track.FlightPlan->FlightRules)
			{
				case FLIGHT_RULES_IFR:
				{
					*field |= 0x10;
					subfield[0] |= 0 << 4;
					subfield += 1;
					break;
				}
				case FLIGHT_RULES_VFR:
				{
					*field |= 0x10;
					subfield[0] |= 1 << 4;
					subfield += 1;
					break;
				}
				case FLIGHT_RULES_UNKNOWN:
				{
					break;
				}
			}

			// subfield 5: type of aircraft
			if (track.FlightPlan->TypeOfAircraft.size ())
			{
				field[0] |= 0x08;
				strncpy ((char *) subfield, track.FlightPlan->TypeOfAircraft.c_str (), 4);
				subfield += 4;
			}

			// subfield 6: wake turbulence category
			if (track.FlightPlan->WakeTurbulenceCategory)
			{
				field[0] |= 0x04;
				subfield[0] = track.FlightPlan->WakeTurbulenceCategory;
				subfield += 1;
			}

			// subfield 7: departure airport
			if (track.FlightPlan->DepartureAirport.size ())
			{
				field[0] |= 0x02;
				strncpy ((char *) subfield, track.FlightPlan->DepartureAirport.c_str (), 4);
				subfield += 4;
			}

			*field |= 1;
			field += 1;

			// subfield 8: destination airport
			if (track.FlightPlan->DestinationAirport.size ())
			{
				*field |= 0x80;
				strncpy ((char *) subfield, track.FlightPlan->DestinationAirport.c_str (), 4);
				subfield += 4;
			}

			// subfield 10: current cleared flight level
			if (track.FlightPlan->ClearedFlightLevelPresent)
			{
				*field |= 0x20;
				int fl = (int) rint (track.FlightPlan->ClearedFlightLevel / 0.3048 / 25);
				subfield[0] = (fl >> 8) & 0xff;
				subfield[1] = fl & 0xff;
				subfield += 2;
			}

			// subfield 11: current control position
			if (track.FlightPlan->CurrentControlPresent)
			{
				*field |= 0x10;
				subfield[0] = track.FlightPlan->CurrentControlCenter;
				subfield[1] = track.FlightPlan->CurrentControlPosition;
				subfield += 2;
			}

			if (*field == 0)
			{
				field--;
				*field &= ~1;
			}
			field++;

			int subsize = subfield - buffer2;
			memcpy (field, buffer2, subsize);
			field += subsize;
		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// target size & orientation
		// FSPEC22_I062_270

		// vehicle fleet identification
		// FSPEC23_I062_300

		// mode 5 data reports & extended mode 1 code
		if (track.Mode1)
		{
			*fspec |= FSPEC24_I062_110;
			unsigned char * subfield = buffer2;
			memset (buffer2, 0, sizeof (buffer2));

			// subfield 5: extended mode 1 code in octal reprezentation
			if (track.Mode1)
			{
				*field |= 0x08;
				subfield[0] = (track.Mode1->Code >> 8) & 0x0f;
				subfield[1] = track.Mode1->Code & 0xff;
				subfield += 2;
			}

			field++;

			int subsize = subfield - buffer2;
			memcpy (field, buffer2, subsize);
			field += subsize;
		}

		// track mode 2 code
		if (track.Mode2)
		{
			*fspec |= FSPEC25_I062_120;
			field[0] = (track.Mode2->Code >> 8) & 0x0f;
			field[1] = track.Mode2->Code & 0xff;
			field += 2;
		}

		// composed track number
		// FSPEC26_I062_510

		// estimated accuracies
		// FSPEC27_I062_500
		if (track.Accuracy)
		{
		  	static const unsigned char c_maxsize500 = 17;
 			unsigned char buffer500[c_maxsize500];		// buffer na vsechna data v 500
			unsigned char * subfield = buffer500;     	// ukazatel na aktualni pozici v bufferu
			unsigned char * primary_subfield = field; 	// ukazatel na aktualni pozici v hlavicce
			memset(buffer500, 0, sizeof(buffer500));

			// subfield 1: estimated accuracy of track position (cartesian)
			if (track.Accuracy->Cartesian)
			{
				*primary_subfield |= 0x80;
				const double c_max_val = ((1 << 16) - 1) / 2.;
				if (track.Accuracy->Cartesian->X > c_max_val)
				{
					track.Accuracy->Cartesian->X = c_max_val;
				}
				if (track.Accuracy->Cartesian->Y > c_max_val)
				{
					track.Accuracy->Cartesian->Y = c_max_val;
				}
				int x = (int) rint (track.Accuracy->Cartesian->X * 2);
				subfield[0] = (x >> 8) & 0xff;
				subfield[1] = x & 0xff;
				int y = (int) rint (track.Accuracy->Cartesian->Y * 2);
				subfield[2] = (y >> 8) & 0xff;
				subfield[3] = y & 0xff;
				subfield += 4;
			}

			// subfield 2: XY covariance
			if (track.Accuracy->CovXY)
			{
				*primary_subfield |= 0x40;
				const double c_max_val = ((1 << 15) - 1) / 2.;
				if (fabs(track.Accuracy->CovXY->Cov) > c_max_val)
				{
					track.Accuracy->CovXY->Cov = signbit(track.Accuracy->CovXY->Cov) ? -c_max_val : c_max_val;
				}
				int cov = (int) rint (track.Accuracy->CovXY->Cov * 2);
				subfield[0] = (cov >> 8) & 0xff;
				subfield[1] = cov & 0xff;
				subfield += 2;
			}

			// subfield 3: estimated accuracy of track position (WGS-84)
			if (track.Accuracy->Geographic)
			{
				*primary_subfield |= 0x20;
				const double c_max_val = M_PI * (double) ((1 << 16) - 1) / (double) (1 << 25);
				if (track.Accuracy->Geographic->Latitude > c_max_val)
				{
					track.Accuracy->Geographic->Latitude = c_max_val;
				}
				if (track.Accuracy->Geographic->Longitude > c_max_val)
				{
					track.Accuracy->Geographic->Longitude = c_max_val;
				}
				int lat = (int) floor (track.Accuracy->Geographic->Latitude / M_PI * (1 << 25));
				subfield[0] = (lat >> 8) & 0xff;
				subfield[1] = lat & 0xff;
				int lon = (int) floor (track.Accuracy->Geographic->Longitude / M_PI * (1 << 25));
				subfield[2] = (lon >> 8) & 0xff;
				subfield[3] = lon & 0xff;
				subfield += 4;
			}
			// subfield 4: estimated accuracy of calculated track geometric altitude
			if (track.Accuracy->GeometricHeight)
			{
				*primary_subfield |= 0x10;
				const double c_max_val = 6.25 * 0.3048 * ((1 << 8) - 1);
				if (track.Accuracy->GeometricHeight->Height > c_max_val)
				{
					track.Accuracy->GeometricHeight->Height = c_max_val;
				}
				subfield[0] = (int) rint (track.Accuracy->GeometricHeight->Height / 6.25 / 0.3048) & 0xff;
				subfield += 1;
			}

			// subfield 5: estimated accuracy of calculated track barometric altitude
			if (track.Accuracy->BarometricHeight)
			{
				*primary_subfield |= 0x08;
				const double c_max_val = 25 * 0.3048 * ((1 << 8) - 1);
				if (track.Accuracy->BarometricHeight->Height > c_max_val)
				{
					track.Accuracy->BarometricHeight->Height = c_max_val;
				}
				subfield[0] = (int) rint (track.Accuracy->BarometricHeight->Height / 25 / 0.3048) & 0xff;
				subfield += 1;
			}

			// subfield 6: estimated accuracy of track velocity (cartesian)
			if (track.Accuracy->CartesianSpeed)
			{
				*primary_subfield |= 0x04;
				const double c_max_val = ((1 << 8) - 1) / 4.;
				if (track.Accuracy->CartesianSpeed->X > c_max_val)
				{
					track.Accuracy->CartesianSpeed->X = c_max_val;
				}
				if (track.Accuracy->CartesianSpeed->Y > c_max_val)
				{
					track.Accuracy->CartesianSpeed->Y = c_max_val;
				}
				subfield[0] = (int) rint (track.Accuracy->CartesianSpeed->X * 4) & 0xff;
				subfield[1] = (int) rint (track.Accuracy->CartesianSpeed->Y * 4) & 0xff;
				subfield += 2;
			}

			// subfield 7: estimated accuracy of acceleration
			if (track.Accuracy->Acceleration)
			{
				*primary_subfield |= 0x02;
				const double c_max_val = ((1 << 8) - 1) / 4.;
				if (track.Accuracy->Acceleration->X > c_max_val)
				{
					track.Accuracy->Acceleration->X = c_max_val;
				}
				if (track.Accuracy->Acceleration->Y > c_max_val)
				{
					track.Accuracy->Acceleration->Y = c_max_val;
				}
				subfield[0] = (int) rint (track.Accuracy->Acceleration->X * 4) & 0xff;
				subfield[1] = (int) rint (track.Accuracy->Acceleration->Y * 4) & 0xff;
				subfield += 2;
			}

			*primary_subfield |= FSPEC_FX;
			primary_subfield++;

			// subfield 8: estimated accuracy of rate of climb/descent
			if (track.Accuracy->ClimbSpeed)
			{
				*primary_subfield |= 0x80;
				const double c_max_val = 6.25 * 0.3048 / 60.* ((1 << 8) - 1);
				if (track.Accuracy->ClimbSpeed->Speed > c_max_val)
				{
					track.Accuracy->ClimbSpeed->Speed = c_max_val;
				}
				subfield[0] = (int) rint (track.Accuracy->ClimbSpeed->Speed / 6.25 / 0.3048 * 60) & 0xff;
				subfield += 1;
			}
 			// vymazeme nevyuzite polozky primary_subfield
			while (!(*primary_subfield & ~FSPEC_FX) && primary_subfield > field)
			{
				primary_subfield--;
			}
			*primary_subfield &= ~FSPEC_FX;
			int subfield_size = subfield - buffer500;
			if (subfield_size > c_maxsize500)
			{
				cms_error("item 500 exceeded buffer size");
				break;
			}
			if (subfield_size) // aspon jedna nenulova polozka
			{
				*fspec |= FSPEC27_I062_500;
  			 	// konec polozky primary subfield a ukazujeme na zacatek dat
				primary_subfield++;
				memcpy(primary_subfield,buffer500,subfield_size);
				field = primary_subfield + subfield_size;
			}

		}

		// measured information - pozn.: je implementovano jen castecne 2021-11-26
		if (1)
		{
			*fspec |= FSPEC28_I062_340;

			unsigned char* primary_subfield = field;   // ukazatel na aktualni pozici v hlavicce

			*primary_subfield = 0x00; // absence/presence jsou nastavovany nize dle subfieldu
			field++;

			// subfield 3: Measured 3-D Height in Two's Complement
			if (track.MeasuredHeight)
			{
			  *primary_subfield |= 0x20; // HEI - subfield #3 (Measured 3-D Height) present

			  long height = lrint (track.MeasuredHeight->Height / 0.3048 / 25);
				field[0] = (height >> 8) & 0xff;
				field[1] = height & 0xff;

				field += 2;
			}

			// subfield 6: Report Type
			*primary_subfield |= 0x04;  // TYP - subfield #6 (Report Type) present

			switch (track.TargetType)
      {
        case TARGET_TYPE_ADSB: // chova se jako neznamy
        case TARGET_TYPE_UNKNOWN:
        {
          break;
        }
        case TARGET_TYPE_PRIMARY:
        {
          field[0] |= 1 << 5;
          break;
        }
        case TARGET_TYPE_SECONDARY:
        {
          field[0] |= 2 << 5;
          break;
        }
        case TARGET_TYPE_COMBINED:
        {
          field[0] |= 3 << 5;
          break;
        }
        case TARGET_TYPE_MODES_ALL_CALL:
        {
          field[0] |= 4 << 5;
          break;
        }
        case TARGET_TYPE_MODES_ROLL_CALL:
        {
          field[0] |= 5 << 5;
          break;
        }
        case TARGET_TYPE_MODES_ALL_CALL_PSR:
        {
          field[0] |= 6 << 5;
          break;
        }
        case TARGET_TYPE_MODES_ROLL_CALL_PSR:
        {
          field[0] |= 7 << 5;
          break;
        }
      }
      //field[0] |= track.Simulated ? 0x10 : 0;
      field[0] |= track.RAB ? 0x08 : 0;
      field[0] |= track.Tested ? 0x04 : 0;

      field++;

		}

		// konec polozky fspec
		*fspec++ |= FSPEC_FX;
		*fspec = 0;

		// spare
		// FSPEC29_I062_SPA

		// spare
		// FSPEC30_I062_SPA

		// spare
		// FSPEC31_I062_SPA

		// spare
		// FSPEC32_I062_SPA

		// spare
		// FSPEC33_I062_SPA

		// reserved expansion field
		// FSPEC34_I062_RE

		// reserved for special purpose indicator
		// FSPEC35_I062_SP
		if (!config_nosp)
		{
			unsigned char * subfspec = field + 1;
			unsigned char * subfield = subfspec + 1;

			if (track.UserText.size ())
			{
				if (track.UserText.size () >= 253)
				{
					cms_error ("SP size error (%d)", track.UserText.size ());
				}
				else
				{
					*subfspec |= 0x80;
					subfield[0] = track.UserText.size ();
					subfield += 1;
					memcpy (subfield, track.UserText.data (), track.UserText.size ());
					subfield += track.UserText.size ();
				}
			}

			if (*subfspec)
			{
				*fspec |= FSPEC35_I062_SP;
				field[0] = subfield - field;
				field = subfield;
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
				track_list.pop_front ();
				continue;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime track z fronty
		track_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;
}

