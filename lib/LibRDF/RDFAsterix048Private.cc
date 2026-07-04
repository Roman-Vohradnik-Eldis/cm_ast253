#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#include <iomanip> //std::setprecision
#include <sstream>

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

// Data Source Identifier
void asterix_048_read_010 (RDFTargetTrack & target, unsigned char * &field)
{
    target.SAC = field[0];
    target.SIC = field[1];
    field += 2;
}

void asterix_048_tobuffer_010( const uint8_t * const field,
      std::stringstream *out_str, bool one_line, const std::string spec) {
   *out_str << spec << " [sac=" << static_cast<int>(field[0])
      << " sic=" << static_cast<int>(field[1]) << "]";

   NEW_LINE(*out_str, 0, '\0');
}

void asterix_048_write_010 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.SAC;
    field[1] = target.SIC;
    field += 2;
}

// Target Report Description
bool asterix_048_read_020 (RDFTargetTrack & target, unsigned char * &field)
{
    switch ((field[0] >> 5) & 0x7)
    {
        case 0: target.TargetType = TARGET_TYPE_UNKNOWN; break;
        case 1: target.TargetType = TARGET_TYPE_PRIMARY; break;
        case 2: target.TargetType = TARGET_TYPE_SECONDARY; break;
        case 3: target.TargetType = TARGET_TYPE_COMBINED; break;
        case 4: target.TargetType = TARGET_TYPE_MODES_ALL_CALL; break;
        case 5: target.TargetType = TARGET_TYPE_MODES_ROLL_CALL; break;
        case 6: target.TargetType = TARGET_TYPE_MODES_ALL_CALL_PSR; break;
        case 7: target.TargetType = TARGET_TYPE_MODES_ROLL_CALL_PSR; break;
    }

    target.Simulated = (field[0] & 0x10) ? true : false;
    target.SPI = (field[0] & 0x04) ? true : false;
    target.RAB = (field[0] & 0x02) ? true : false;

    if (field[0] & 0x01)
    {
        field += 1;
        target.Tested = (field[0] & 0x80) ? true : false;
        target.ME = (field[0] & 0x10) ? true : false;
        target.MI = (field[0] & 0x08) ? true : false;

        switch ((field[0] >> 1) & 0x3)
        {
            case 0: target.TargetFoeFri = TARGET_FOEFRI_NO_MODE4; break;
            case 1: target.TargetFoeFri = TARGET_FOEFRI_FRIENDLY_TARGET; break;
            case 2: target.TargetFoeFri = TARGET_FOEFRI_UNKNOWN_TARGET; break;
            case 3: target.TargetFoeFri = TARGET_FOEFRI_NO_REPLY; break;
        }

        if (field[0] & 0x01)
        {
            cms_error ("unable to handle FX bit (%x)", field[0]);
            return false;
        }
    }

    field += 1;
    return true;
}

void asterix_048_write_020 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char f = 0;

    switch (target.TargetType)
    {
        case TARGET_TYPE_ADSB: // chova se jako neznamy
        case TARGET_TYPE_UNKNOWN: f = 0; break;
        case TARGET_TYPE_PRIMARY: f = 1; break;
        case TARGET_TYPE_SECONDARY: f = 2; break;
        case TARGET_TYPE_COMBINED: f = 3; break;
        case TARGET_TYPE_MODES_ALL_CALL: f = 4; break;
        case TARGET_TYPE_MODES_ROLL_CALL: f = 5; break;
        case TARGET_TYPE_MODES_ALL_CALL_PSR: f = 6; break;
        case TARGET_TYPE_MODES_ROLL_CALL_PSR: f = 7; break;
    }
    field[0] |= f << 5;
    if (target.Simulated)
    {
        field[0] |= 0x10;
    }
    if (target.SPI)
    {
        field[0] |= 0x4;
    }
    if (target.RAB)
    {
        field[0] |= 0x2;
    }

    field[1] |= target.Tested ? 0x80 : 0;
    field[1] |= target.ME ? 0x10 : 0;
    field[1] |= target.MI ? 0x08 : 0;
    switch (target.TargetFoeFri)
    {
        case TARGET_FOEFRI_NO_MODE4: f = 0; break;
        case TARGET_FOEFRI_FRIENDLY_TARGET: f = 1; break;
        case TARGET_FOEFRI_UNKNOWN_TARGET: f = 2; break;
        case TARGET_FOEFRI_NO_REPLY: f = 3; break;
    }
    field[1] |= f << 1;

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

void
asterix_048_tobuffer_020(const uint8_t * const field, std::stringstream *out_str, const bool one_line, const std::string spec) {

   if(out_str == NULL)
      return;

   *out_str << spec;

   switch((field[0] >> 5) & 0x7)    {
                case 0: *out_str << " [NO-DET"; break;
                case 1: *out_str << " [PSR"; break;
                case 2: *out_str << " [SSR"; break;
                case 3: *out_str << " [SSR+PSR"; break;
                case 4: *out_str << " [ALL-CALL"; break;
                case 5: *out_str << " [ROLL-CALL"; break;
                case 6: *out_str << " [ALL-CALL+PSR"; break;
                case 7: *out_str << " [ROLL-CALL+PSR"; break;
   }

   if(field[0] & 0x10)
    *out_str << " SIM";
    else
        *out_str << " ACT";

    *out_str << " RDP_" << 1 + ((field[0] >> 3) & 0x1);

    if(field[0] & 0x04)
        *out_str << " SPI";
    else
        *out_str << " -";

    if(field[0] & 0x2)
        *out_str << " RAB]";
    else
        *out_str << " AIR]";

    if (field[0] & 0x01)    {

      NEW_LINE(*out_str, 4, ' ');

      *out_str << "[";
      if(field[1] & 0x80)
         *out_str << " TEST";
      else
         *out_str << " REAL";

      if(field[1] & 0x20)
         *out_str << " XPP";
      else
         *out_str << "  - ";

      if(field[1] & 0x10)
         *out_str << " ME";
      else
         *out_str << " - ";

      if(field[1] & 0x08)
         *out_str << " MI";
      else
         *out_str << " - ";


        switch ((field[1] >> 1) & 0x3) {
                case 0: *out_str << "  - "; break;
                case 1: *out_str << " FRIEND"; break;
                case 2: *out_str << " UNKNOWN"; break;
                case 3: *out_str << " NO-REP"; break;
      }

      *out_str << "]";
   }
   NEW_LINE(*out_str, 0, '\0');
}
// Warning/Error Conditions
void asterix_048_read_030 (RDFTargetTrack & target, unsigned char * &field)
{
    while (true)
    {
        target.WarningError.push_back (field[0] >> 1);
        if (!(field[0] & 0x01))
        {
            break;
        }
        field += 1;
    }
    field += 1;
}

void asterix_048_write_030 (RDFTargetTrack & target, unsigned char * &field)
{
    for (unsigned i = 0; i < target.WarningError.size (); i++)
    {
        field[0] = (target.WarningError[i] << 1);
        if (i + 1 != target.WarningError.size ())
        {
            field[0] |= 1;
        }
        field += 1;
    }
}
void asterix_048_tobuffer_030( const uint8_t * const field, std::stringstream *out_str,
      bool one_line, const std::string spec)
{
   if(out_str == NULL)
      return;

   uint8_t pos = 0;
   *out_str << spec << " [warning_err conditions]";

   while(true) {
      int err_num = field[pos] >> 1;

      NEW_LINE(*out_str, 4, ' ');

      switch(err_num) {

         case 0:
             *out_str << "[" << err_num << " NO_DEF ]"; break;
         case 1:
             *out_str << "["<< err_num << " MULTIPATH ]"; break;
         case 2:
             *out_str << "["<< err_num << " SIDELOBE ]"; break;
         case 3:
             *out_str << "["<< err_num << " SPLIT ]"; break;
         case 4:
             *out_str << "["<< err_num <<" SEC-TIME ]"; break;
         case 5:
             *out_str << "["<< err_num <<" ANGEL ]"; break;
         case 6:
             *out_str << "["<< err_num <<" VEHICLE ]"; break;
         case 7:
             *out_str << "["<< err_num <<" FIXED_PSR ]"; break;
         case 8:
             *out_str << "["<< err_num <<" SLOW_PSR ]"; break;
         case 9:
             *out_str << "["<< err_num <<" LOW_Q_PSR ]"; break;
         case 10:
             *out_str << "["<< err_num <<" PHANTOM ]"; break;
         case 11:
             *out_str << "["<< err_num <<" NON_MATCH_M3A ]"; break;
         case 12:
             *out_str << "["<< err_num <<" MCIFL_ABNORMAL ]"; break;
         case 13:
             *out_str << "["<< err_num <<" CLUTTER ]"; break;
         case 14:
             *out_str << "["<< err_num <<" MAX_DOPPLER ]"; break;
         case 15:
             *out_str << "["<< err_num <<" TRANSPONDER-ANOMALY ]"; break;
         case 16:
             *out_str << "["<< err_num <<" DUP_MODE_S ]"; break;
         case 17:
             *out_str << "["<< err_num <<" MODE_S_ERR_CORR ]"; break;
         case 18:
             *out_str << "["<< err_num <<" MCIFL_UNDECODABLE ]"; break;
         case 19:
             *out_str << "["<< err_num <<" BIRDS ]"; break;
         case 20:
             *out_str << "["<< err_num <<" FLOCK_OF_BIRDS ]"; break;
         case 21:
             *out_str << "["<< err_num <<" MODE_1 ]"; break;
         case 22:
             *out_str << "["<< err_num <<" MODE_2 ]"; break;
         case 23:
             *out_str << "["<< err_num <<" WIND_TURBINE ]"; break;
      }

      NEW_LINE(*out_str, 0, '\0');
      if (!(field[pos] & 0x01))
        break;

      pos++;

    }

}
// Measured Position in Slant Polar Coordinates
void asterix_048_read_040 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Polar = new RDFCoorPolar ();
    target.Polar->Rho = (double) field2unsigned (field, 16) * 1852 / 256;
    target.Polar->Theta = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
    field += 4;
}

bool asterix_048_write_040 (RDFTargetTrack & target, unsigned char * &field)
{
	long rho = lrint (target.Polar->Rho * 256 / 1852);
	long theta = lrint (target.Polar->Theta * 0x10000 / (2 * M_PI));
	if (rho < 0 || rho > USHRT_MAX)
	{
		cms_warning ("rho out of limit");
		return false;
	}
    field[0] = (rho >> 8) & 0xff;
    field[1] = rho & 0xff;
    field[2] = (theta >> 8) & 0xff;
    field[3] = theta & 0xff;
    field += 4;
    return true;
}

void asterix_048_tobuffer_040( const uint8_t * const field, std::stringstream *out_str,
      bool one_line,const std::string spec) {

   double rho = (double)field2unsigned (field, 16) * 1852 / 256;
   double theta = ((double)field2unsigned(field+2, 16)* (2*M_PI) / 0x10000);

   *out_str << std::setprecision(2) << spec;

   *out_str << " [rho=" << static_cast<int>(rho) <<" m ("
          << std::fixed << (rho / 1852.0) << " NM) theta=" << std::fixed <<
          theta*RAD_TO_DEG << " deg]";

   NEW_LINE(*out_str, 0, '\0');
}
// Calculated Position in Cartesian Coordinates
void asterix_048_read_042 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Cartesian = new RDFCoorCartesian ();
    target.Cartesian->X = field2signed (field, 16) * 1852 / 128.0;
    target.Cartesian->Y = field2signed (field + 2, 16) * 1852 / 128.0;
    field += 4;
}

bool asterix_048_write_042 (RDFTargetTrack & target, unsigned char * &field)
{
	long x = lrint (target.Cartesian->X * 128 / 1852);
	long y = lrint (target.Cartesian->Y * 128 / 1852);
	if (x > SHRT_MAX || x < SHRT_MIN)
	{
		cms_warning ("x out of limit");
		return false;
	}
	if (y > SHRT_MAX || y < SHRT_MIN)
	{
		cms_warning ("y out of limit");
		return false;
	}
    field[0] = (x >> 8) & 0xff;
    field[1] = x & 0xff;
    field[2] = (y >> 8) & 0xff;
    field[3] = y & 0xff;
    field += 4;
    return true;
}
void asterix_048_tobuffer_042 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   double x = field2signed (field, 16) * 1852 / 128.0;
    double y = field2signed (field + 2, 16) * 1852 / 128.0;

   *out_str << spec << " [x=" << rint (x) << " m (" << x/1852 << " NM) y="
                << rint (y) << " m (" << y/1852 << " NM)]";

   NEW_LINE(*out_str, 0, '\0');
}


// Mode-2 Code in Octal Representation
void asterix_048_read_050 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Mode2 = new RDFTargetMode ();
    target.Mode2->Valid = (field[0] & 0x80) ? false : true;
    target.Mode2->Garbled = (field[0] & 0x40) ? true : false;
    target.Mode2->Tracked = (field[0] & 0x20) ? true : false;
    target.Mode2->Code = field2unsigned (field, 12);
    field += 2;
}

void asterix_048_write_050 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.Mode2->Valid ? 0 : 0x80;
    field[0] |= target.Mode2->Garbled ? 0x40 : 0;
    field[0] |= target.Mode2->Tracked ? 0x20 : 0;
    field[0] |= (target.Mode2->Code >> 8) & 0x0f;
    field[1] = target.Mode2->Code & 0xff;
    field += 2;
}
void asterix_048_tobuffer_050 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
   {

   *out_str << spec << " [M2_hex=" << std::hex << field2unsigned(field, 12) << std::dec;

   if (field[0] & 0x80)
      *out_str << " INVALID";
   else
      *out_str << " VALID";

   if (field[0] & 0x40)
      *out_str << " GARBLED";
   else
      *out_str << " -";

   if (field[0] & 0x20)
      *out_str << " TRACKED";
   else
      *out_str << " -";

   *out_str << "]";

   NEW_LINE(*out_str, 0, '\0');
}
   // Mode-1 Code in Octal Representation
void asterix_048_read_055 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Mode1 = new RDFTargetMode ();
    target.Mode1->Valid = (field[0] & 0x80) ? false : true;
    target.Mode1->Garbled = (field[0] & 0x40) ? true : false;
    target.Mode1->Tracked = (field[0] & 0x20) ? true : false;
    target.Mode1->Code = ((field[0] & 0x1c) << 1) | (field[0] & 0x3);
    field += 1;
}

void asterix_048_write_055 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.Mode1->Valid ? 0 : 0x80;
    field[0] |= target.Mode1->Garbled ? 0x40 : 0;
    field[0] |= target.Mode1->Tracked ? 0x20 : 0;
    field[0] |= ((target.Mode1->Code >> 1) & 0x1c) | (target.Mode1->Code & 0x03);
    field += 1;
}

void asterix_048_tobuffer_055 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {

   *out_str << spec << "[ M1_hex=" << std::hex << (field[0] & 0x1f) << std::dec;

   if (field[0] & 0x80)
      *out_str << "INVALID";
   else
      *out_str << "VALID";

   if (field[0] & 0x40)
      *out_str << " GARBLED";
   else
      *out_str << " -";

   if (field[0] & 0x20)
      *out_str << " TRACKED";
   else
      *out_str << " -";

   *out_str << "]";

   NEW_LINE(*out_str, 0, '\0');
}

// Mode-2 Code Confidence Indicator
void asterix_048_read_060 (RDFTargetTrack & target, unsigned char * &field)
{
    if (target.Mode2)
    {
        target.Mode2->ConfidencePresent = true;
        target.Mode2->ConfidenceCode = field2unsigned (field, 12);
        field += 2;
    }
}

void asterix_048_write_060 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = (target.Mode2->ConfidenceCode >> 8) & 0x0f;
    field[1] = target.Mode2->ConfidenceCode & 0xff;
    field += 2;
}
void asterix_048_tobuffer_060 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   short int conf = field2unsigned(field, 12);
   *out_str << spec << " [QXi=";

   for(char i = 11; i >= 0; --i)
      *out_str << (conf >> i & 1);

   *out_str << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// Mode-1 Code Confidence Indicator
void asterix_048_read_065 (RDFTargetTrack & target, unsigned char * &field)
{
    if (target.Mode1)
    {
        target.Mode1->ConfidencePresent = true;
        target.Mode1->ConfidenceCode = ((field[0] & 0x1c) << 1) | (field[0] & 0x3);
        field += 1;
    }
}

void asterix_048_write_065 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = ((target.Mode1->ConfidenceCode >> 1) & 0x1c)
        | (target.Mode1->ConfidenceCode & 0x03);
    field += 1;
}
void asterix_048_tobuffer_065 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   char conf = field[0] & 0x1f;
   *out_str << spec << " [QXi=";

   for (char i = 4; i >= 0; --i)
      *out_str << (conf >> i & 1);

   *out_str << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// Mode-3/A Code in Octal Representation
void asterix_048_read_070 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Mode3A = new RDFTargetMode ();
    target.Mode3A->Valid = (field[0] & 0x80) ? false : true;
    target.Mode3A->Garbled = (field[0] & 0x40) ? true : false;
    target.Mode3A->Tracked = (field[0] & 0x20) ? true : false;
    target.Mode3A->Code = field2unsigned (field, 12);
    field += 2;
}

void asterix_048_write_070 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] |= target.Mode3A->Valid ? 0 : 0x80;
    field[0] |= target.Mode3A->Garbled ? 0x40 : 0;
    field[0] |= target.Mode3A->Tracked ? 0x20 : 0;
    field[0] |= (target.Mode3A->Code >> 8) & 0x0f;
    field[1] = target.Mode3A->Code & 0xff;
    field += 2;
}

void asterix_048_tobuffer_070( const uint8_t * const field, std::stringstream *out_str,
      bool one_line, const std::string spec) {

   *out_str << spec << " [M3A_oct=" << std::oct << std::setw(4)
      << std::fixed << std::setfill('0') << field2unsigned (field, 12)
      << std::dec << std::setfill(' ');

   if(field[0] & 0x80)
      *out_str << " INVALID";
   else
      *out_str << " VALID";

   if (field[0] & 0x40)
      *out_str << " GARBLED";
   else
      *out_str << " -";

   if (field[0] & 0x20)
      *out_str << " TRACKED";
   else
      *out_str << " -";

   *out_str << "]";

   NEW_LINE(*out_str, 0, ' ');
}
// Mode-3/A Code Confidence Indicator
void asterix_048_read_080 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Mode3A->ConfidencePresent = true;
    target.Mode3A->ConfidenceCode = field2unsigned (field, 12);
    field += 2;
}

void asterix_048_write_080 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = (target.Mode3A->ConfidenceCode >> 8) & 0x0f;
    field[1] = target.Mode3A->ConfidenceCode & 0xff;
    field += 2;
}

void asterix_048_tobuffer_080 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   uint16_t conf = field2unsigned(field, 12);
   *out_str << spec << " [QXi=";
   //print binary
   for(char i = 11; i >= 0; i--)
      *out_str << ((conf >> i) & 0x01);

   *out_str << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// Flight Level in Binary Representation
void asterix_048_read_090 (RDFTargetTrack & target, unsigned char * &field)
{
    target.FlightLevel = new RDFTargetFlightLevel ();
    target.FlightLevel->Valid = (field[0] & 0x80) ? false : true;
    target.FlightLevel->Garbled = (field[0] & 0x40) ? true : false;
    target.FlightLevel->Height = (double) field2signed (field, 14) * 25 * 0.3048;
    field += 2;
}

void asterix_048_write_090 (RDFTargetTrack & target, unsigned char * &field)
{
    int fl = (int) rint (target.FlightLevel->Height / 0.3048 / 25);
    field[0] |= target.FlightLevel->Valid ? 0 : 0x80;
    field[0] |= target.FlightLevel->Garbled ? 0x40 : 0;
    field[0] |= (fl >> 8) & 0x3f;
    field[1] = fl & 0xff;
    field += 2;
}
void asterix_048_tobuffer_090 ( const uint8_t * const field, std::stringstream *out_str,
      bool one_line, const std::string spec)
{
   double fl = field2signed(field, 14) / 4.0;

   *out_str << spec << " [FL=" << std::setprecision (2) << fl << " ("
      << static_cast<int>(round(fl * 100 * 0.3048)) << " m)";

   if (field[0] & 0x80)
      *out_str << " INVALID";
   else
      *out_str << " VALID";

   if (field[0] & 0x40)
      *out_str << " GARBLED";
   else
      *out_str << " -";

   *out_str << "]";
   NEW_LINE(*out_str, 0, '\0');
}
// Mode-C Code and Confidence Indicator
void asterix_048_read_100 (RDFTargetTrack & target, unsigned char * &field)
{
    target.ModeC = new RDFTargetMode ();
    target.ModeC->Valid = (field[0] & 0x80) ? false : true;
    target.ModeC->Garbled = (field[0] & 0x40) ? true : false;
    target.ModeC->Code = field2unsigned (field, 12);
    target.ModeC->ConfidencePresent = true;
    target.ModeC->ConfidenceCode = field2unsigned (field + 2, 12);
    field += 4;
}

void asterix_048_write_100 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] |= target.ModeC->Valid ? 0 : 0x80;
    field[0] |= target.ModeC->Garbled ? 0x40 : 0;
    field[0] |= (target.ModeC->Code >> 8) & 0x0f;
    field[1] = target.ModeC->Code & 0xff;
    field[2] = (target.ModeC->ConfidenceCode >> 8) & 0x0f;
    field[3] = target.ModeC->ConfidenceCode & 0xff;
    field += 4;
}

void asterix_048_tobuffer_100 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   if (field[0] & 0x80)
      *out_str << spec << " [v=INV";
   else
      *out_str << spec << " [v=VAL";

   if (field[0] & 0x40)
      *out_str << " g=GARBLED";
   else
      *out_str << " g=-";

   *out_str << " code_hex=" << std::hex << field2unsigned(field + 2, 12);

   uint16_t confid = field2unsigned(field + 2, 12);
   /*if (conf)*/
   {
      *out_str << " conf=";
      for(char i = 11; i >= 0; --i)
         *out_str << (confid >> i & 1);
   }

   *out_str << "]";
   NEW_LINE(*out_str, 0, '\0');
}
   // Height Measured by 3D Radar
void asterix_048_read_110 (RDFTargetTrack & target, unsigned char * &field)
{
    target.MeasuredHeight = new RDFTargetHeight ();
    target.MeasuredHeight->Height = (double) field2signed (field, 14) * 25 * 0.3048;
    field += 2;
}

void asterix_048_write_110 (RDFTargetTrack & target, unsigned char * &field)
{
    int height = (int) rint (target.MeasuredHeight->Height / 0.3048 / 25);
    field[0] = (height >> 8) & 0x3f;
    if (target.MeasuredHeight->QualityPresent)
    {
        field[0] |= (target.MeasuredHeight->Quality << 5) & 0xc0;
    }
    field[1] = height & 0xff;
    field += 2;
}
void asterix_048_tobuffer_110( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   double height = (double)field2signed(field, 14);

   *out_str << spec << " [height=" << rint (height * 25 * 0.3048) << " m (fl="
                << std::setprecision (2) << height / 4.0
                << std::setprecision (3) << ")]";

   NEW_LINE(*out_str, 0, '\n');
}
// Radial Doppler Speed
bool asterix_048_read_120 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char * fspec = field;
    field++;

    if (*fspec & 0x80)
    {
        target.DopplerSpeed.CalculatedPresent = true;
        target.DopplerSpeed.CalculatedValid = (field[0] & 0x80) ? false : true;
        target.DopplerSpeed.CalculatedSpeed = field2unsigned (field, 10);
        field += 2;
    }

    if (*fspec & 0x40)
    {
        target.DopplerSpeed.RawPresent = true;
        target.DopplerSpeed.RepetitonFactor = field[0];
        target.DopplerSpeed.DopplerSpeed = field2unsigned (field + 1, 16);
        target.DopplerSpeed.AmbiguityRange = field2unsigned (field + 3, 16);
        target.DopplerSpeed.TransmitterFrequency = field2unsigned (field + 5, 16);
        field += 7;
    }

    if (*fspec & 0x3F)
    {
        cms_error ("unable to handle more fspec bits");
        return false;
    }

    return true;
}

void asterix_048_write_120 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char * fspec = field;
    field++;

    if (target.DopplerSpeed.CalculatedPresent)
    {
        *fspec |= 0x80;
        field[0] |= target.DopplerSpeed.CalculatedValid ? 0 : 0x80;
        field[0] = (target.DopplerSpeed.CalculatedSpeed >> 8) & 0x0f;
        field[1] = target.DopplerSpeed.CalculatedSpeed & 0xff;
        field += 2;
    }

    if (target.DopplerSpeed.RawPresent)
    {
        *fspec |= 0x40;
        field[0] = target.DopplerSpeed.RepetitonFactor;
        field[1] = (target.DopplerSpeed.DopplerSpeed >> 8) & 0xff;
        field[2] = target.DopplerSpeed.DopplerSpeed & 0xff;
        field[3] = (target.DopplerSpeed.AmbiguityRange >> 8) & 0xff;
        field[4] = target.DopplerSpeed.AmbiguityRange & 0xff;
        field[5] = (target.DopplerSpeed.TransmitterFrequency >> 8) & 0xff;
        field[6] = target.DopplerSpeed.TransmitterFrequency & 0xff;
        field += 7;
    }
}
void asterix_048_tobuffer_120 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec){

   unsigned char * subfield = const_cast<unsigned char *>(field);

    subfield += 1;

    if(*field & 0x80) {
      if (subfield[0] & 0x80)
         *out_str << spec << " [d=DOUBT cal=" << field2signed (subfield, 10)
            << " m/s]";
      else
         *out_str << spec << " [d=VAL cal=" << field2signed (subfield, 10)
            << " m/s]";

      NEW_LINE(*out_str, 0, '\0');

      subfield += 2;
   }

   if(*field & 0x40) {
      int rep = subfield[0];
      *out_str << spec << " [raw rep=" << rep << "]";

      for(int i = 0; i < rep; ++i) {
         NEW_LINE(*out_str, 0,'\0');
         *out_str << "[dop=" << field2unsigned (subfield + 1, 16)
         << " m/s amb=" << field2unsigned (subfield + 3, 16)
         << " m/s frq=" << field2unsigned (subfield + 5, 16)
         << " Mhz]";

         subfield += 7;
      }

   }
}
// Radar Plot Characteristics
bool asterix_048_read_130 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char * fspec = field;
    field++;

    target.TargetCharacteristics = new RDFTargetCharacteristics ();

    // SRL
    if (*fspec & 0x80)
    {
        target.TargetCharacteristics->SsrRunlengthPresent = true;
        target.TargetCharacteristics->SsrRunlength = (unsigned char) field[0];
        field += 1;
    }

    // SRR
    if (*fspec & 0x40)
    {
        target.TargetCharacteristics->SsrRepliesPresent = true;
        target.TargetCharacteristics->SsrReplies = (unsigned char) field[0];
        field += 1;
    }

    // SAM
    if (*fspec & 0x20)
    {
        target.TargetCharacteristics->SsrAmplitudePresent = true;
        target.TargetCharacteristics->SsrAmplitude = (char) field[0];
        field += 1;
    }

    // PRL
    if (*fspec & 0x10)
    {
        target.TargetCharacteristics->PsrRunlengthPresent = true;
        target.TargetCharacteristics->PsrRunlength = (unsigned char) field[0];
        field += 1;
    }

    // PAM
    if (*fspec & 0x08)
    {
        target.TargetCharacteristics->PsrAmplitudePresent = true;
        target.TargetCharacteristics->PsrAmplitude = (char) field[0];
        field += 1;
    }

    // RPD
    if (*fspec & 0x04)
    {
        target.TargetCharacteristics->PsrSsrRangeDiffPresent = true;
        target.TargetCharacteristics->PsrSsrRangeDiff = (char) field[0];
        field += 1;
    }

    // APD
    if (*fspec & 0x02)
    {
        target.TargetCharacteristics->PsrSsrAzimuthDiffPresent = true;
        target.TargetCharacteristics->PsrSsrAzimuthDiff = (char) field[0];
        field += 1;
    }

    // FX
    if (*fspec & 0x01)
    {
        cms_error ("unable to handle more fspec bits");
        return false;
    }

    return true;
}

void asterix_048_write_130 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char * subfield = field;
    *subfield = 0;
    field += 1;

    // SRL
    if (target.TargetCharacteristics->SsrRunlengthPresent)
    {
        *subfield |= 0x80;
        field[0] = (unsigned char) target.TargetCharacteristics->SsrRunlength;
        field += 1;
    }

    // SRR
    if (target.TargetCharacteristics->SsrRepliesPresent)
    {
        *subfield |= 0x40;
        field[0] = (unsigned char) target.TargetCharacteristics->SsrReplies;
        field += 1;
    }

    // SAM
    if (target.TargetCharacteristics->SsrAmplitudePresent)
    {
        *subfield |= 0x20;
        field[0] = (unsigned char) target.TargetCharacteristics->SsrAmplitude;
        field += 1;
    }

    // PRL
    if (target.TargetCharacteristics->PsrRunlengthPresent)
    {
        *subfield |= 0x10;
        field[0] = (unsigned char) target.TargetCharacteristics->PsrRunlength;
        field += 1;
    }

    // PAM
    if (target.TargetCharacteristics->PsrAmplitudePresent)
    {
        *subfield |= 0x08;
        field[0] = (unsigned char) target.TargetCharacteristics->PsrAmplitude;
        field += 1;
    }

    // RPD
    if (target.TargetCharacteristics->PsrSsrRangeDiffPresent)
    {
        *subfield |= 0x04;
        field[0] = (unsigned char) target.TargetCharacteristics->PsrSsrRangeDiff;
        field += 1;
    }

    // APD
    if (target.TargetCharacteristics->PsrSsrAzimuthDiffPresent)
    {
        *subfield |= 0x02;
        field[0] = (unsigned char) target.TargetCharacteristics->PsrSsrAzimuthDiff;
        field += 1;
    }
}
void asterix_048_tobuffer_130 ( const uint8_t * const field, std::stringstream *out_str,
      bool one_line, const std::string spec = "130") {

   *out_str << spec << " [";
   if (!field[0])
      *out_str << "NONE";

   unsigned char * subfield = const_cast<unsigned char *>(field);
   subfield += 1;
   // SRL
   if(*field & 0x80) {
      *out_str << "SRL=" << subfield[0] * 360.0 / 0x2000 << " deg ";
      subfield += 1;
   }

   // SRR
   if (*field & 0x40)
   {
      *out_str << "SRR=" << static_cast<int>(subfield[0]) << " replies ";
      subfield += 1;
   }

   // SAM
   if (*field & 0x20)
   {
      *out_str << "SAM=" << static_cast<int>(subfield[0]) << " dBm ";
      subfield += 1;
   }

   // PRL
   if (*field & 0x10)
   {
      *out_str  << "PRL=" << subfield[0] * 360.0 / 0x2000 << " deg ";
      subfield += 1;
   }

   // PAM
   if (*field & 0x08)
   {
      *out_str << "PAM=" << rint (field2signed (subfield, 8)) <<  " dBm ";
      subfield += 1;
   }

   // RPD
   if (*field & 0x04)
   {
      *out_str << "RPD=";

      if (subfield[0] == 0x7f)
         *out_str << "+";
      else if (subfield[0] == 0x80)
         *out_str << "-";

      *out_str << rint (field2signed (subfield, 8) * 1852.0 / 256) << " m ";
      subfield += 1;
   }

   // APD
   if (*field & 0x02)
   {
      *out_str << "APD=";

      if (subfield[0] == 0x7f)
         *out_str << "+";
      else if (subfield[0] == 0x80)
         *out_str << "-";

      *out_str << field2signed (subfield, 8) * 360.0 / 0x4000 << " deg ";
      subfield += 1;
   }

   *out_str << "]";
   NEW_LINE(*out_str, 0, '\0');
}

// Time of Day
void asterix_048_read_140 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Time.Present = true;
    target.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
    field += 3;
}

void asterix_048_write_140 (RDFTargetTrack & target, unsigned char * &field)
{
    int time = RDF_time_to_asterix_time(target.Time.Time);
    field[0] = (time >> 16) & 0xff;
    field[1] = (time >> 8) & 0xff;
    field[2] = time & 0xff;
    field += 3;
}

void
asterix_048_tobuffer_140( const uint8_t * const field,std::stringstream *out_str, bool one_line, const std::string spec) {
   //opsano od bezdi
   uint64_t time = field2unsigned (field, 24);

   uint64_t mod = time % 128;
   time = time / 128;
   *out_str << spec << " [time=" << time << " ";
   *out_str << std::setw(2)  << std::setfill('0')  << time / 3600 //h
      << ":" << std::setw(2)  << std::setfill('0')
      << ((time  % 3600) / 60) << ":" << std::setw(2) //m
      << std::setfill('0') << ((time % 3600) % 60) //s
      << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
      << "]";

   NEW_LINE(*out_str, 0, '\0');
}

// Track Number
void asterix_048_read_161 (RDFTargetTrack & target, unsigned char * &field)
{
    target.TrackNumber.Present = true;
    target.TrackNumber.Number = field2unsigned (field, 12);
    field += 2;
}

void asterix_048_write_161 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = (target.TrackNumber.Number >> 8) & 0x0f;
    field[1] = target.TrackNumber.Number & 0xff;
    field += 2;
}
void asterix_048_tobuffer_161 (const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   *out_str << spec << " [track=" << field2unsigned(field, 12) << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// Track Status
void asterix_048_read_170 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Tentative = (field[0] & 0x80) ? true : false;
    target.PSR = (field[0] & 0x40) ? false : true;
    target.SSR = (field[0] & 0x20) ? false : true;
    target.MDS = (target.TargetType == TARGET_TYPE_MODES_ALL_CALL
        || target.TargetType == TARGET_TYPE_MODES_ROLL_CALL
        || target.TargetType == TARGET_TYPE_MODES_ALL_CALL_PSR
        || target.TargetType == TARGET_TYPE_MODES_ROLL_CALL_PSR);
    target.DOU = (field[0] & 0x10) ? true : false;
    target.MAH = (field[0] & 0x08) ? true : false;

    switch ((field[0] >> 1) & 3)
    {
        case 0: target.TrackClimb = TRACK_CLIMB_MAINTAINING; break;
        case 1: target.TrackClimb = TRACK_CLIMB_CLIMBING; break;
        case 2: target.TrackClimb = TRACK_CLIMB_DESCENDING; break;
        case 3: target.TrackClimb = TRACK_CLIMB_UNKNOWN; break;
    }

    if (field[0] & 0x01)
    {
        field += 1;
        target.Cancel = (field[0] & 0x80) ? true : false;
        target.SUP = (field[0] & 0x20) ? true : false;
    }

    field += 1;
}

void asterix_048_write_170 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned f = 0;

    field[0] |= target.Tentative ? 0x80 : 0;
    field[0] |= target.PSR ? 0 : 0x40;
    field[0] |= target.SSR ? 0 : 0x20;
    field[0] |= target.DOU ? 0x10 : 0;
    field[0] |= target.MAH ? 0x08 : 0;

    switch (target.TrackClimb)
    {
        case TRACK_CLIMB_MAINTAINING: f = 0; break;
        case TRACK_CLIMB_CLIMBING: f = 1; break;
        case TRACK_CLIMB_DESCENDING: f = 2; break;
        case TRACK_CLIMB_UNKNOWN: f = 3; break;
    }

    field[0] |= f << 1;
    field[0] |= 0x01;

    field[1] |= target.Cancel ? 0x80 : 0;
    field[1] |= target.SUP ? 0x20 : 0;
    field += 2;
}
void asterix_048_tobuffer_170 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   *out_str << spec << " [";

   if (field[0] & 0x80)
      *out_str << "TENTATIVE";
   else
      *out_str << "FIRM";

   switch ((field[0] >> 5) & 3)
   {
      case 0: *out_str << " CMB"; break;
      case 1: *out_str << " PSR"; break;
      case 2: *out_str << " SSR/MODE-S"; break;
      case 3: *out_str << " INVALID"; break;
   }

   if (field[0] & 0x10)
      *out_str << " DOU";
   else
      *out_str << "  - ";

   if (field[0] & 0x8)
      *out_str << " MAH";
   else
      *out_str << "  - ";

   switch ((field[0] >> 1) & 3)
   {
      case 0: *out_str << " MAINTAIN]"; break;
      case 1: *out_str << " CLIMB]"; break;
      case 2: *out_str << " DESCEND]"; break;
      case 3: *out_str << " INVALID]"; break;
   }


   if(field[0] & 0x01)
   {
      NEW_LINE(*out_str, 4, ' ');

      if (field[1] & 0x80)
         *out_str << "[CANCEL";
      else
         *out_str << "[ALIVE";

      if (field[1] & 0x40)
         *out_str << " GHOST";
      else
         *out_str << " TRUE";

      if (field[1] & 0x20)
         *out_str << " SUP";
      else
         *out_str << "  - ";

      if (field[1] & 0x10)
         *out_str << " TCC]";
      else
         *out_str << "  - ]";

      NEW_LINE(*out_str, 0, '\0');
   }
}

// Calculated Track Velocity in Polar Representation
void asterix_048_read_200 (RDFTargetTrack & target, unsigned char * &field)
{
    target.GroundSpeed = new RDFTargetGroundSpeed ();
    target.GroundSpeed->Speed = (double) field2unsigned (field, 16) * 1852 / (1<<14);
    target.GroundSpeed->Heading = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / (1<<16);
    field += 4;
}

void asterix_048_write_200 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned short speed = (unsigned short) (target.GroundSpeed->Speed * (1<<14) / 1852);
    unsigned short heading = (unsigned short) (target.GroundSpeed->Heading * (1<<16) / (2 * M_PI));
    field[0] = (speed >> 8) & 0xff;
    field[1] = speed & 0xff;
    field[2] = (heading >> 8) & 0xff;
    field[3] = heading & 0xff;
    field += 4;
}
void asterix_048_tobuffer_200 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
    double speed  = (double) field2unsigned (field, 16) * 1852 / (1<<14);
    double heading = (double) field2unsigned (field + 2, 16) / 0x10000 * 360;

   static const double ms2knots = 1.94384;

    *out_str << spec << " [speed=" << speed << " m/s (" << speed * ms2knots
      << " kt) heading=" << heading << " deg]";


   NEW_LINE(*out_str, 0, '\0');
}
// Track Quality
void asterix_048_read_210 (RDFTargetTrack & target, unsigned char * &field)
{
    target.Accuracy = new RDFTargetAccuracy ();

    target.Accuracy->Cartesian = new RDFCoorCartesian ();
    target.Accuracy->Cartesian->X = (double) field2unsigned (field, 8) * 1852 / 128.0;
    target.Accuracy->Cartesian->Y = (double) field2unsigned (field + 1, 8) * 1852 / 128.0;

    target.Accuracy->PolarSpeed = new RDFCoorPolar ();
    target.Accuracy->PolarSpeed->Rho = (double) field2unsigned (field + 2, 8) * 1852 / (1<<14);
    target.Accuracy->PolarSpeed->Theta = (double) field2unsigned (field + 3, 8) * (2 * M_PI) / (1<<12);

    field += 4;
}

void asterix_048_write_210 (RDFTargetTrack & target, unsigned char * &field)
{
    const double c_max_val_cart  = 255 * 1852. / 128;          // maximalni mozna hodnota std kartezskych souradnic
    const double c_max_val_rho   = 255 * 1852. / (1<<14);      // maximalni mozna hodnota std velikosti rychlosti
    const double c_max_val_theta = 255 * (2 * M_PI) / (1<<12); // maximalni mozna hodnota std smeru rychlosti

    double x = target.Accuracy->Cartesian ? target.Accuracy->Cartesian->X : DBL_MAX;
    double y = target.Accuracy->Cartesian ? target.Accuracy->Cartesian->Y : DBL_MAX;
    double v = target.Accuracy->PolarSpeed ? target.Accuracy->PolarSpeed->Rho : DBL_MAX;
    double h = target.Accuracy->PolarSpeed ? target.Accuracy->PolarSpeed->Theta : DBL_MAX;

    x = x < c_max_val_cart ? x : c_max_val_cart;
    y = y < c_max_val_cart ? y : c_max_val_cart;
    v = v < c_max_val_rho ? v : c_max_val_rho;
    h = h < c_max_val_theta ? h : c_max_val_theta;

    field[0] = (unsigned char) (x * 128 / 1852);
    field[1] = (unsigned char) (y * 128 / 1852);
    field[2] = (unsigned char) (v * (1<<14) / 1852);
    field[3] = (unsigned char) (h * (1<<12) / (2 * M_PI));

    field += 4;
}
void asterix_048_tobuffer_210 ( const uint8_t * const field
      , std::stringstream *out_str, bool one_line, const std::string spec)
{
    double x = (double) field2unsigned (field, 8) * 1852 / 128.0;
   double y = (double) field2unsigned (field + 1, 8) * 1852 / 128.0;

    double rho = (double) field2unsigned (field + 2, 8) * 1852 / (1<<14);
    double theta = (double) field2unsigned (field + 3, 8) * (2 * M_PI) / (1<<12);

   *out_str << spec << " [sigma_x=" << std::setprecision (0) << rint (x)
               << "m (" << std::setprecision (3) << x/1852 << " NM) sigma_y="
               << std::setprecision (0) << rint (y) << "m ("
               << std::setprecision (3) << y/1852 << " NM)]";

   NEW_LINE(*out_str, 4, ' ');

   *out_str << "[sigma_v=" << rho << " m/s ("
                << (rho/1852) * 3600 << " kt) sigma_h=" << theta << "deg]";
   NEW_LINE(*out_str, 0, '\0');
}
// Aircraft Address
void asterix_048_read_220 (RDFTargetTrack & target, unsigned char * &field)
{
    target.AircraftAddress.Present = true;
    target.AircraftAddress.Address = field2unsigned (field, 24);
    field += 3;
}

void asterix_048_write_220 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = (target.AircraftAddress.Address >> 16) & 0xff;
    field[1] = (target.AircraftAddress.Address >> 8) & 0xff;
    field[2] = target.AircraftAddress.Address & 0xff;
    field += 3;
}

void asterix_048_tobuffer_220 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {

   *out_str << spec << " [addr_hex=" << std::hex <<
      field2unsigned(field, 24) << std::dec << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// Communications / ACAS Capability and Flight Status
void asterix_048_read_230 (RDFTargetTrack & target, unsigned char * &field)
{
    target.CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus ();
    switch ((field[0] >> 5) & 7)
    {
        case 0: target.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_NONE; break;
        case 1: target.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B; break;
        case 2: target.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B_ELMU; break;
        case 3: target.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_A_B_ELMU_ELMD; break;
        case 4: target.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_LEVEL_5; break;
        default: target.CommCapabilityAndFlightStatus->CommCapability = COMM_CAPABILITY_NONE; break;        // 5 to 7 not assigned
    }
    switch ((field[0] >> 2) & 7)
    {
        case 0: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE; break;
        case 1: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ON_GROUND; break;
        case 2: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE_ALERT; break;
        case 3: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ON_GROUND_ALERT; break;
        case 4: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_ALERT_SPI; break;
        case 5: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_SPI; break;
        default: target.CommCapabilityAndFlightStatus->FlightStatus = FLIGHT_STATUS_AIRBORNE; break;        // 6 to 7 not assigned
    }
    target.CommCapabilityAndFlightStatus->TransponderCapability = (field[0] & 0x02) ? true : false;
    if (field[0] & 0x01)
    {
        cms_warning("spare bit not consistent");
    }
    target.CommCapabilityAndFlightStatus->ModeSSpecificService = (field[1] & 0x80) ? true : false;
    target.CommCapabilityAndFlightStatus->AltitudeCapability = (field[1] & 0x40) ? true : false;
    target.CommCapabilityAndFlightStatus->IdentificationCapability = (field[1] & 0x20) ? true : false;
    target.CommCapabilityAndFlightStatus->B1A = (field[1] & 0x10) ? true : false;
    target.CommCapabilityAndFlightStatus->B1B = field[1] & 0x0f;
    field += 2;
}


void asterix_048_write_230 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char f = 0;

    switch (target.CommCapabilityAndFlightStatus->CommCapability)
    {
        case COMM_CAPABILITY_NONE: f = 0; break;
        case COMM_CAPABILITY_A_B: f = 1; break;
        case COMM_CAPABILITY_A_B_ELMU: f = 2; break;
        case COMM_CAPABILITY_A_B_ELMU_ELMD: f = 3; break;
        case COMM_CAPABILITY_LEVEL_5: f = 4; break;
    }
    field[0] |= f << 5;
    switch (target.CommCapabilityAndFlightStatus->FlightStatus)
    {
        case FLIGHT_STATUS_AIRBORNE: f = 0; break;
        case FLIGHT_STATUS_ON_GROUND: f = 1; break;
        case FLIGHT_STATUS_AIRBORNE_ALERT: f = 2; break;
        case FLIGHT_STATUS_ON_GROUND_ALERT: f = 3; break;
        case FLIGHT_STATUS_ALERT_SPI: f = 4; break;
        case FLIGHT_STATUS_SPI: f = 5; break;
        case FLIGHT_STATUS_GENERAL_EMERGENCY:
        case FLIGHT_STATUS_LIFEGUARD:
        case FLIGHT_STATUS_MINIMUM_FUEL:
        case FLIGHT_STATUS_NO_COMMUNICATIONS:
        case FLIGHT_STATUS_UNLAWFUL_INTERFERENCE: f = 0; break;
    }
    field[0] |= f << 2;

    field[1] |= target.CommCapabilityAndFlightStatus->ModeSSpecificService ? 0x80 : 0;
    field[1] |= target.CommCapabilityAndFlightStatus->AltitudeCapability ? 0x40 : 0;
    field[1] |= target.CommCapabilityAndFlightStatus->IdentificationCapability ? 0x20 : 0;
    field[1] |= target.CommCapabilityAndFlightStatus->B1A ? 0x10 : 0;
    field[1] |= target.CommCapabilityAndFlightStatus->B1B & 0x0f;
    field += 2;
}

void asterix_048_tobuffer_230 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {

   switch ((field[0] >> 5) & 7)
   {
      case 0: *out_str << spec << " [NO_COM"; break;
      case 1: *out_str << spec << " [COM_A+B"; break;
      case 2: *out_str << spec << " [COM_A+B_UELM"; break;
      case 3: *out_str << spec << " [COM_A+B_UELM_DELM"; break;
      case 4: *out_str << spec << " [COM_LEVEL_5"; break;
      case 5: case 6: case 7: *out_str << spec << " [NOT_ASSIGNED "; break;
      default: *out_str << spec << " [COM_UNKNOWN " <<  (field[0] >> 5 & 7); break;
   }

   switch ((field[0] >> 2) & 7)
   {
      case 0: *out_str << " AIR"; break;
      case 1: *out_str << " GND"; break;
      case 2: *out_str << " AIR_ALERT"; break;
      case 3: *out_str << " GND_ALERT"; break;
      case 4: *out_str << " ALERT_SPI"; break;
      case 5: *out_str << " SPI"; break;
      case 6: case 7: *out_str << " NOT_ASSIGNED"; break;
      default: *out_str << " UNKNOWN" << (field[0] >> 2 & 7); break;
   }

   if (field[1] & 0x80)
      *out_str << " MSSC_YES";
   else
      *out_str << " MSSC_NO";

   if (field[1] & 0x40)
      *out_str << " 25FT";
   else
      *out_str << " 100FT";

   if (field[1] & 0x20)
      *out_str << " AIC";
   else
      *out_str << "  - ";

   if((field[1] >> 4 & 1))
      *out_str << " B1A";
   else
      *out_str << "  - ";

   if((field[1] & 0xf))
       *out_str << " B1B]";
   else
       *out_str << "  - ]";

  NEW_LINE(*out_str, 0, '\0');
}
// Aircraft Identification
void asterix_048_read_240 (RDFTargetTrack & target, unsigned char * &field)
{
    asterix_decode_callsign (field, target.AircraftIdentification);
    field += 6;
}

void asterix_048_write_240 (RDFTargetTrack & target, unsigned char * &field)
{
    asterix_encode_callsign (target.AircraftIdentification, field);
    field += 6;
}

void asterix_048_tobuffer_240 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   std::string callsign;
   asterix_decode_callsign(field, callsign);
   *out_str << spec << " [callsign=\"" << callsign << "\"]";

   NEW_LINE(*out_str, 0, '\0');
}

// Mode S MB Data
void asterix_048_read_250 (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned rep = field[0];
    field += 1;

    for (unsigned i = 0; i < rep; i++)
    {
        RDFTargetModeS modes;
        memcpy (modes.MessageData, field, 7);
        modes.Address = field[7];
        target.ModeS.push_back (modes);
        field += 8;
    }
}

void asterix_048_write_250 (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.ModeS.size ();
    field += 1;
    for (unsigned i = 0; i < target.ModeS.size (); i++)
    {
        memcpy (field, target.ModeS[i].MessageData, 7);
        field[7] = target.ModeS[i].Address;
        field += 8;
    }
}
void asterix_048_tobuffer_250( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   unsigned char *age_field = const_cast<unsigned char *>(field);
   int rep = age_field[0];

   *out_str << spec << " [MODE_S rep=" << rep << "]";

   age_field += 1;

   for(int i = 0; i < rep; i++) {
      NEW_LINE(*out_str, 4, ' ');

    *out_str << "[mb_data_hex=" << std::setfill ('0') << std::hex << std::noshowbase;

      for(int j = 0; j< 7; j++) {
         *out_str << std::setw (2) <<
            static_cast<int>(age_field[j]) << " ";
      }

        *out_str << std::setw (2) << std::hex <<
         " BDS_hex=" << static_cast<int>(age_field[7]) << "]" << std::dec;

      age_field += 8;
   }

   *out_str << std::setfill(' ');

   NEW_LINE(*out_str, 0, '\0');

}
// Mode S MB Data with Age
void asterix_048_read_250_with_age (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned rep = field[0];
    field += 1;

    for (unsigned i = 0; i < rep; i++)
    {
        RDFTargetModeS modes;
        memcpy (modes.MessageData, field, 7);
        modes.Address = field[7];
        modes.Age = field[8];
        target.ModeSAge.push_back (modes);
        field += 9;
    }
}

void asterix_048_tobuffer_250_with_age(const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   unsigned char *age_field = const_cast<unsigned char *>(field);
   int rep = age_field[0];

   *out_str << spec << " [MODE_S rep=" << rep << "]";

   age_field += 1;

   for(int i = 0; i < rep; i++) {
      NEW_LINE(*out_str, 4, ' ');

    *out_str << "[mb_data_hex=" << std::setfill ('0') << std::hex << std::noshowbase;

      for(int j = 0; j< 7; j++) {
         *out_str << std::setw (2) <<
            static_cast<int>(age_field[j]) << " ";
      }

        *out_str << std::setw (2) << std::hex <<
         " BDS_hex=" << static_cast<int>(age_field[7]);
    *out_str << std::dec << " age=" << std::setw(3) << std::fixed <<
         static_cast<int>(age_field[8]) << " s]";

      age_field += 9;
   }

   *out_str << std::setfill(' ');

   NEW_LINE(*out_str, 0, '\0');
}
void asterix_048_write_250_with_age (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.ModeSAge.size ();
    field += 1;

    for (unsigned i = 0; i < target.ModeSAge.size (); i++)
    {
        memcpy (field, target.ModeSAge[i].MessageData, 7);
        field[7] = target.ModeSAge[i].Address;
        field[8] = target.ModeSAge[i].Age;
        field += 9;
    }
}

// ACAS Resolution Advisory Report
void asterix_048_read_260 (RDFTargetTrack & target, unsigned char * &field)
{
    target.ResolutionAdvisory.Present = true;
    memcpy (target.ResolutionAdvisory.Data, field, 7);
    field += 7;
}

void asterix_048_write_260 (RDFTargetTrack & target, unsigned char * &field)
{
    memcpy (field, target.ResolutionAdvisory.Data, 7);
    field += 7;
}
void asterix_048_tobuffer_260 ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   *out_str << spec << " [acas_hex=" << std::hex;
    for (int i = 0; i <= 6; ++i)
      *out_str << std::setw (2) << static_cast<int>(field[i]) << " ";

   *out_str << std::dec << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// User Text
void asterix_048_read_user_text (RDFTargetTrack & target, unsigned char * &field)
{
    target.UserText.assign ((char *) field + 1, (unsigned) field[0]);
    field += field[0] + 1;
}

void asterix_048_write_user_text (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.UserText.size ();
    field += 1;
    memcpy (field, target.UserText.data (), target.UserText.size ());
    field += target.UserText.size ();
}
void asterix_048_tobuffer_user_text(const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {

   int rep = field[0];

   *out_str << spec;

   *out_str <<  " [text=\"";

   *out_str << std::string(field + 1, field + 1 + rep);

   *out_str << "\"]";

   NEW_LINE(*out_str, 0, '\0');
}
// Calculated Rate of Climb Descent
void asterix_048_read_rate_of_climb (RDFTargetTrack & target, unsigned char * &field)
{
    target.ClimbSpeed = new RDFTargetClimbSpeed ();
    target.ClimbSpeed->Speed = field2signed (field, 16) * 0.3048 * 6.25 / 60;
    field += 2;
}

void asterix_048_write_rate_of_climb (RDFTargetTrack & target, unsigned char * &field)
{
    int speed = (int) rint (target.ClimbSpeed->Speed * 60 / 6.25 / 0.3048);
    field[0] = (speed >> 8) & 0xff;
    field[1] = speed & 0xff;
    field += 2;
}
void asterix_048_tobuffer_rate_of_climb ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
    *out_str << spec << " [climb=" <<  field2signed (field, 16) * 0.3048 * 6.25 / 60;
   *out_str << "]";

   NEW_LINE(*out_str, 0,' ');
}
// Calculated Acceleration
void asterix_048_read_acceleration (RDFTargetTrack & target, unsigned char * &field)
{
    target.Acceleration = new RDFCoorCartesian();
    target.Acceleration->X = (double)((char)field[0]) / 4;
    field += 1;
   target.Acceleration->Y = (double)((char)field[0]) / 4;
    field += 1;
}

void asterix_048_write_acceleration (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = target.Acceleration->X < 63.75 ? (char) target.Acceleration->X << 2 : 0xff;
    field[1] = target.Acceleration->Y < 63.75 ? (char) target.Acceleration->Y << 2 : 0xff;
    field += 2;
}

void asterix_048_tobuffer_acceleration(const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)  {

   *out_str << spec << std::setprecision(2) << std::fixed <<
      " [acc x=" << (double)((char)field[0])/4 << " y=" <<
      (double)((char)field[1])/4 << "]";

   NEW_LINE(*out_str, 0, '\0');
}
// Mode of Movement
void asterix_048_read_mode_of_movement (RDFTargetTrack & target, unsigned char * &field)
{
    switch ((field[0] >> 6) & 3)
    {
        case 0: target.TransversalAcceleration = TRANS_ACCEL_CONSTANT_COURSE; break;
        case 1: target.TransversalAcceleration = TRANS_ACCEL_RIGHT_TURN; break;
        case 2: target.TransversalAcceleration = TRANS_ACCEL_LEFT_TURN; break;
        case 3: target.TransversalAcceleration = TRANS_ACCEL_UNDETERMINED; break;
    }
    switch ((field[0] >> 4) & 3)
    {
        case 0: target.LongitudinalAcceleration = LONG_ACCEL_CONSTANT_GROUNDSPEED; break;
        case 1: target.LongitudinalAcceleration = LONG_ACCEL_INCREASING_GROUNDSPEED; break;
        case 2: target.LongitudinalAcceleration = LONG_ACCEL_DECREASING_GROUNDSPEED; break;
        case 3: target.LongitudinalAcceleration = LONG_ACCEL_UNDETERMINED; break;
    }
    switch ((field[0] >> 2) & 3)
    {
        case 0: target.TrackClimb = TRACK_CLIMB_MAINTAINING; break;
        case 1: target.TrackClimb = TRACK_CLIMB_CLIMBING; break;
        case 2: target.TrackClimb = TRACK_CLIMB_DESCENDING; break;
        case 3: target.TrackClimb = TRACK_CLIMB_UNKNOWN; break;
    }
    field += 1;
}

void asterix_048_write_mode_of_movement (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned f = 0;

    switch (target.TransversalAcceleration)
    {
        case TRANS_ACCEL_CONSTANT_COURSE: f = 0; break;
        case TRANS_ACCEL_RIGHT_TURN: f = 1; break;
        case TRANS_ACCEL_LEFT_TURN: f = 2; break;
        case TRANS_ACCEL_UNDETERMINED: f = 3; break;
    }

    field[0] |= f << 6;

    switch (target.LongitudinalAcceleration)
    {
        case LONG_ACCEL_CONSTANT_GROUNDSPEED: f = 0; break;
        case LONG_ACCEL_INCREASING_GROUNDSPEED: f = 1; break;
        case LONG_ACCEL_DECREASING_GROUNDSPEED: f = 2; break;
        case LONG_ACCEL_UNDETERMINED: f = 3; break;
    }

    field[0] |= f << 4;

    switch (target.TrackClimb)
    {
        case TRACK_CLIMB_MAINTAINING: f = 0; break;
        case TRACK_CLIMB_CLIMBING: f = 1; break;
        case TRACK_CLIMB_DESCENDING: f = 2; break;
        case TRACK_CLIMB_UNKNOWN: f = 3; break;
    }

    field[0] |= f << 2;

    field += 1;
}

void asterix_048_tobuffer_mode_of_movement( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {

   *out_str << spec << " [movement_mode]";
   NEW_LINE(*out_str, 4, ' ');
   *out_str << "[transversal=";

   switch ((field[0] >> 6) & 3) {
        case 0: *out_str <<  "constant_course "; break;
        case 1: *out_str << "right_turn "; break;
      case 2: *out_str << "left_turn "; break;
        case 3: *out_str << "undetermined "; break;
    }

   *out_str << "]";
   NEW_LINE(*out_str, 4, ' ');

   *out_str << "[longitudinal=";
    switch ((field[0] >> 4) & 3)
    {
        case 0: *out_str << "constant_groundspeed "; break;
      case 1: *out_str << "incrasing_groundspeed "; break;
        case 2: *out_str << "decrasing_groundspeed "; break;
        case 3: *out_str << "undetermined "; break;
    }
   *out_str << "]";
   NEW_LINE(*out_str, 4, ' ');

   *out_str << "[tracklimb=";
    switch ((field[0] >> 2) & 3)
    {
        case 0: *out_str << "maintaining]"; break;
      case 1: *out_str << "climbing]"; break;
        case 2: *out_str << "descending]"; break;
        case 3: *out_str << "unknown]"; break;
    }

   NEW_LINE(*out_str, 0, '\0');
}
// Target Composition
void asterix_048_read_target_composition (RDFTargetTrack & target, unsigned char * &field)
{
    // Mode-3A Code Source
    switch ((field[0] >> 6) & 0x03)
    {
        case 0: target.TargetComposition.Mode3A = CODE_SOURCE_NONE; break;
        case 1: target.TargetComposition.Mode3A = CODE_SOURCE_SIF; break;
        case 2: target.TargetComposition.Mode3A = CODE_SOURCE_ROLL_CALL; break;
        case 3: target.TargetComposition.Mode3A = CODE_SOURCE_ADS; break;
    }

    // Mode-C Code Source
    switch ((field[0] >> 4) & 0x03)
    {
        case 0: target.TargetComposition.ModeC = CODE_SOURCE_NONE; break;
        case 1: target.TargetComposition.ModeC = CODE_SOURCE_SIF; break;
        case 2: target.TargetComposition.ModeC = CODE_SOURCE_ROLL_CALL; break;
        case 3: target.TargetComposition.ModeC = CODE_SOURCE_ADS; break;
    }

    // Position Source
    switch (field[0] & 0x0f)
    {
        case 0: target.TargetComposition.Position = POSITION_SOURCE_NONE; break;
        case 1: target.TargetComposition.Position = POSITION_SOURCE_SIF; break;
        case 2: target.TargetComposition.Position = POSITION_SOURCE_SIF_FALSE; break;
        case 3: target.TargetComposition.Position = POSITION_SOURCE_ALL_CALL; break;
        case 4: target.TargetComposition.Position = POSITION_SOURCE_ALL_CALL_FALSE; break;
        case 5: target.TargetComposition.Position = POSITION_SOURCE_ROLL_CALL; break;
        case 6: target.TargetComposition.Position = POSITION_SOURCE_ROLL_CALL_FALSE; break;
        case 7: target.TargetComposition.Position = POSITION_SOURCE_ALL_CALL_ROLL_CALL; break;
        case 8: target.TargetComposition.Position = POSITION_SOURCE_SIF_ALL_CALL; break;
        case 9: target.TargetComposition.Position = POSITION_SOURCE_SIF_ROLL_CALL; break;
        case 10: target.TargetComposition.Position = POSITION_SOURCE_SIF_ALL_CALL_ROLL_CALL; break;
        case 11: target.TargetComposition.Position = POSITION_SOURCE_ADS; break;
        default:
        {
            cms_warning ("unknown position source: %d", field[0] & 0x0f);
            target.TargetComposition.Position = POSITION_SOURCE_NONE;
        }
    }

    // Detection Source
    switch ((field[1] >> 4) & 0x0f)
    {
        case 0: target.TargetComposition.Detection = DETECTION_SOURCE_NONE; break;
        case 1: target.TargetComposition.Detection = DETECTION_SOURCE_SIF; break;
        case 2: target.TargetComposition.Detection = DETECTION_SOURCE_ALL_CALL; break;
        case 3: target.TargetComposition.Detection = DETECTION_SOURCE_ROLL_CALL; break;
        case 4: target.TargetComposition.Detection = DETECTION_SOURCE_ALL_CALL_ROLL_CALL; break;
        case 5: target.TargetComposition.Detection = DETECTION_SOURCE_ADS; break;
        case 6: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ALL_CALL; break;
        case 7: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ROLL_CALL; break;
        case 8: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ALL_CALL_ROLL_CALL; break;
        case 9: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ALL_CALL_ADS; break;
        case 10: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ROLL_CALL_ADS; break;
        case 11: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ADS; break;
        case 12: target.TargetComposition.Detection = DETECTION_SOURCE_ALL_CALL_ADS; break;
        case 13: target.TargetComposition.Detection = DETECTION_SOURCE_ROLL_CALL_ADS; break;
        case 14: target.TargetComposition.Detection = DETECTION_SOURCE_ALL_CALL_ROLL_CALL_ADS; break;
        case 15: target.TargetComposition.Detection = DETECTION_SOURCE_SIF_ALL_CALL_ROLL_CALL_ADS; break;
        default:
        {
            cms_warning ("unknown position source: %d", (field[1] >> 4) & 0x0f);
            target.TargetComposition.Detection = DETECTION_SOURCE_NONE;
        }
    }

   field += 2;

}
void asterix_048_tobuffer_target_composition ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   *out_str << spec << " [target_composition]";
   NEW_LINE(*out_str, 4, ' ');

    // Mode-3A Code Source
   *out_str << "[mode-3a_code_source=";
    switch ((field[0] >> 6) & 0x03) {
        case 0: *out_str << "none]"; break;
        case 1: *out_str << "sif]"; break;
        case 2: *out_str << "roll_call]"; break;
        case 3: *out_str << "ads]"; break;
    }
   NEW_LINE(*out_str, 4, ' ');

   *out_str << "[mode-c_code_source=";
    // Mode-C Code Source
    switch ((field[0] >> 4) & 0x03)
    {
        case 0: *out_str <<  "none]"; break;
        case 1: *out_str <<  "sif]"; break;
        case 2: *out_str <<  "roll_call]"; break;
        case 3: *out_str <<  "ads]"; break;
    }

   NEW_LINE(*out_str, 4, ' ');

   *out_str << "[position_source=";
    // Position Source
    switch (field[0] & 0x0f) {
        case 0: *out_str <<  "none]"; break;
        case 1: *out_str <<  "sif]"; break;
        case 2: *out_str <<  "sif_false]"; break;
        case 3: *out_str <<  "all_call]"; break;
        case 4: *out_str <<  "all_call_false]"; break;
        case 5: *out_str <<  "roll_call]"; break;
        case 6: *out_str <<  "roll_call_false]"; break;
        case 7: *out_str <<  "all_call_roll_call]"; break;
        case 8: *out_str <<  "sif_all_call]"; break;
        case 9: *out_str <<  "sif_roll_call]"; break;
        case 10: *out_str <<  "sif_all_call_roll_call]"; break;
        case 11: *out_str <<  "ads]"; break;
        default: *out_str << "unknown]"; break;
    }
   NEW_LINE(*out_str, 4, ' ');
   *out_str << "[detection_source=";
    // Detection Source
    switch ((field[1] >> 4) & 0x0f)
    {
        case 0: *out_str << "none]"; break;
        case 1: *out_str << "sif]"; break;
        case 2: *out_str << "all_call]"; break;
        case 3: *out_str << "roll_call]"; break;
        case 4: *out_str << "all_call_roll_call]"; break;
        case 5: *out_str << "ads]"; break;
        case 6: *out_str << "sif_all_call]"; break;
        case 7: *out_str << "sif_roll_call]"; break;
        case 8: *out_str << "sif_all_call_roll_call]"; break;
        case 9: *out_str << "sif_all_call_ads]"; break;
        case 10: *out_str << "sif_roll_call_ads]"; break;
        case 11: *out_str << "sif_ads]"; break;
        case 12: *out_str << "all_call_ads]"; break;
        case 13: *out_str << "roll_call_ads]"; break;
        case 14: *out_str << "all_call_roll_call_ads]"; break;
        case 15: *out_str << "sif_all_call_roll_call_ads]"; break;
        default: *out_str << "unknown]"; break;
    }
   NEW_LINE(*out_str, 0, '\0');
}

void asterix_048_write_target_composition (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned f = 0;

    // Mode-3A Code Source
    switch (target.TargetComposition.Mode3A)
    {
        case CODE_SOURCE_NONE: f = 0; break;
        case CODE_SOURCE_SIF: f = 1; break;
        case CODE_SOURCE_ROLL_CALL: f = 2; break;
        case CODE_SOURCE_ADS: f = 3; break;
        default:
        {
            cms_warning ("unknown Mode3A source: %d", target.TargetComposition.Mode3A);
            f = 0;
        }
    }

    field[0] |= f << 6;

    // Mode-C Code Source
    switch (target.TargetComposition.ModeC)
    {
        case CODE_SOURCE_NONE: f = 0; break;
        case CODE_SOURCE_SIF: f = 1; break;
        case CODE_SOURCE_ROLL_CALL: f = 2; break;
        case CODE_SOURCE_ADS: f = 3; break;
        default:
        {
            cms_warning ("unknown ModeC source: %d", target.TargetComposition.ModeC);
            f = 0;
        }
    }

    field[0] |= f << 4;

    // Position Source
    switch (target.TargetComposition.Position)
    {
        case POSITION_SOURCE_NONE: f = 0; break;
        case POSITION_SOURCE_SIF: f = 1; break;
        case POSITION_SOURCE_SIF_FALSE: f = 2; break;
        case POSITION_SOURCE_ALL_CALL: f = 3; break;
        case POSITION_SOURCE_ALL_CALL_FALSE: f = 4; break;
        case POSITION_SOURCE_ROLL_CALL: f = 5; break;
        case POSITION_SOURCE_ROLL_CALL_FALSE: f = 6; break;
        case POSITION_SOURCE_ALL_CALL_ROLL_CALL: f = 7; break;
        case POSITION_SOURCE_SIF_ALL_CALL: f = 8; break;
        case POSITION_SOURCE_SIF_ROLL_CALL: f = 9; break;
        case POSITION_SOURCE_SIF_ALL_CALL_ROLL_CALL: f = 10; break;
        case POSITION_SOURCE_ADS: f = 11; break;
        default:
        {
            cms_warning ("unknown Position source: %d", target.TargetComposition.Position);
            f = 0;
        }
    }

    field[0] |= f;

    // Detection Source
    switch (target.TargetComposition.Detection)
    {
        case DETECTION_SOURCE_NONE: f = 0; break;
        case DETECTION_SOURCE_SIF: f = 1; break;
        case DETECTION_SOURCE_ALL_CALL: f = 2; break;
        case DETECTION_SOURCE_ROLL_CALL: f = 3; break;
        case DETECTION_SOURCE_ALL_CALL_ROLL_CALL: f = 4; break;
        case DETECTION_SOURCE_ADS: f = 5; break;
        case DETECTION_SOURCE_SIF_ALL_CALL: f = 6; break;
        case DETECTION_SOURCE_SIF_ROLL_CALL: f = 7; break;
        case DETECTION_SOURCE_SIF_ALL_CALL_ROLL_CALL: f = 8; break;
        case DETECTION_SOURCE_SIF_ALL_CALL_ADS: f = 9; break;
        case DETECTION_SOURCE_SIF_ROLL_CALL_ADS: f = 10; break;
        case DETECTION_SOURCE_SIF_ADS: f = 11; break;
        case DETECTION_SOURCE_ALL_CALL_ADS: f = 12; break;
        case DETECTION_SOURCE_ROLL_CALL_ADS: f = 13; break;
        case DETECTION_SOURCE_ALL_CALL_ROLL_CALL_ADS: f = 14; break;
        case DETECTION_SOURCE_SIF_ALL_CALL_ROLL_CALL_ADS: f = 15; break;
        default:
        {
            cms_warning ("unknown Position source: %d", target.TargetComposition.Detection);
            f = 0;
        }
    }

    field[1] |= f << 4;

    field += 2;
}

// Target Detections
void asterix_048_read_target_detections (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char * fspec = field;
    field++;

    // SIF Mode-3A
    if (*fspec & 0x80)
    {
        target.TargetDetections.SifMode3APresent = true;
        target.TargetDetections.SifMode3ACount = field[0];
        target.TargetDetections.SifMode3AAge = field[1];
        field += 2;
    }

    // SIF Mode-C
    if (*fspec & 0x40)
    {
        target.TargetDetections.SifModeCPresent = true;
        target.TargetDetections.SifModeCCount = field[0];
        target.TargetDetections.SifModeCAge = field[1];
        field += 2;
    }

    // AllCall
    if (*fspec & 0x20)
    {
        target.TargetDetections.AllCallPresent = true;
        target.TargetDetections.AllCallCount = field[0];
        target.TargetDetections.AllCallAge = field[1];
        field += 2;
    }

    // RollCall Mode-3A
    if (*fspec & 0x10)
    {
        target.TargetDetections.RollCallMode3APresent = true;
        target.TargetDetections.RollCallMode3ACount = field[0];
        target.TargetDetections.RollCallMode3AAge = field[1];
        field += 2;
    }

    // RollCall Mode-C
    if (*fspec & 0x08)
    {
        target.TargetDetections.RollCallModeCPresent = true;
        target.TargetDetections.RollCallModeCCount = field[0];
        target.TargetDetections.RollCallModeCAge = field[1];
        field += 2;
    }

    // ADS
    if (*fspec & 0x04)
    {
        target.TargetDetections.AdsPresent = true;
        target.TargetDetections.AdsCount = field[0];
        target.TargetDetections.AdsAge = field[1];
        field += 2;
    }
}

void asterix_048_write_target_detections (RDFTargetTrack & target, unsigned char * &field)
{
    unsigned char * fspec = field;
    field++;

    // SIF Mode-3A
    if (target.TargetDetections.SifMode3APresent)
    {
        *fspec |= 0x80;
        field[0] = target.TargetDetections.SifMode3ACount;
        field[1] = target.TargetDetections.SifMode3AAge;
        field += 2;
    }

    // SIF Mode-C
    if (target.TargetDetections.SifModeCPresent)
    {
        *fspec |= 0x40;
        field[0] = target.TargetDetections.SifModeCCount;
        field[1] = target.TargetDetections.SifModeCAge;
        field += 2;
    }

    // AllCall
    if (target.TargetDetections.AllCallPresent)
    {
        *fspec |= 0x20;
        field[0] = target.TargetDetections.AllCallCount;
        field[1] = target.TargetDetections.AllCallAge;
        field += 2;
    }

    // RollCall Mode-3A
    if (target.TargetDetections.RollCallMode3APresent)
    {
        *fspec |= 0x10;
        field[0] = target.TargetDetections.RollCallMode3ACount;
        field[1] = target.TargetDetections.RollCallMode3AAge;
        field += 2;
    }

    // RollCall Mode-C
    if (target.TargetDetections.RollCallModeCPresent)
    {
        *fspec |= 0x08;
        field[0] =  target.TargetDetections.RollCallModeCCount;
        field[1] = target.TargetDetections.RollCallModeCAge;
        field += 2;
    }

    // ADS
    if (target.TargetDetections.AdsPresent)
    {
        *fspec |= 0x04;
        field[0] = target.TargetDetections.AdsCount;
        field[1] = target.TargetDetections.AdsAge;
        field += 2;
    }

}
void asterix_048_tobuffer_target_detections( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec)
{
   *out_str << spec << " [target_detections]";


   unsigned char fspec = *field;

   unsigned char *tmp_field = const_cast<unsigned char *>(field+1);

    // Mode-3A Code Source
   if (fspec & 0x80) {
      NEW_LINE(*out_str, 4, ' ');

      *out_str << "[mode-3a_code_source ";

        *out_str << "count=" << static_cast<int>(tmp_field[0]) <<
         " age=" << static_cast<int>(tmp_field[0]) << "]";

      tmp_field += 2;
    }

    // SIF Mode-C
    if (fspec & 0x40)
    {
       NEW_LINE(*out_str, 4, ' ');

      *out_str << "[sif_mode_c ";

        *out_str << "count=" << static_cast<int>(tmp_field[0]) <<
         " age=" << static_cast<int>(tmp_field[0]) << "]";

      tmp_field += 2;
    }

    // AllCall
    if (fspec & 0x20)
    {
       NEW_LINE(*out_str, 4, ' ');

      *out_str << "[all_call ";

        *out_str << "count=" << static_cast<int>(tmp_field[0]) <<
         " age=" << static_cast<int>(tmp_field[0]) << "]";

      tmp_field += 2;
    }

    // RollCall Mode-3A
    if (fspec & 0x10)
    {
       NEW_LINE(*out_str, 4, ' ');

      *out_str << "[roll_call_mode_3A ";

        *out_str << "count=" << static_cast<int>(tmp_field[0]) <<
         " age=" << static_cast<int>(tmp_field[0]) << "]";

      tmp_field += 2;
    }

    // RollCall Mode-C
    if (fspec & 0x08)
    {
       NEW_LINE(*out_str, 4, ' ');

      *out_str << "[roll_call_mode_C ";

        *out_str << "count=" << static_cast<int>(tmp_field[0]) <<
         " age=" << static_cast<int>(tmp_field[0]) << "]";

      tmp_field += 2;
    }

    // ADS
    if (fspec & 0x04)
    {
       NEW_LINE(*out_str, 4, ' ');

      *out_str << "[ADS ";

        *out_str << "count=" << static_cast<int>(tmp_field[0]) <<
         " age=" << static_cast<int>(tmp_field[0]) << "]";

      tmp_field += 2;
    }

   NEW_LINE(*out_str, 0, '\0');
}


// Transponder Capability (same I017/230)
void asterix_048_read_transponder_capability (RDFTargetTrack & target, unsigned char * &field)
{
    target.TransponderCapabilityPresent = true;
    target.TransponderCapabilityCA = ((field[0] >> 5) & 0x07);
    target.TransponderCapabilityNotCapableSI = ((field[0] & 0x10) ? true : false);
    field += 1;
}

void asterix_048_write_transponder_capability (RDFTargetTrack & target, unsigned char * &field)
{
    field[0] = ((target.TransponderCapabilityCA & 0x07) << 5) | (target.TransponderCapabilityNotCapableSI ? 0x10 : 0x00);
    field += 1;
}

void asterix_048_tobuffer_transponder_capability ( const uint8_t * const field, std::stringstream *out_str, bool one_line, const std::string spec) {
   *out_str << spec << " [transponder_capability CA=" << static_cast<int>((field[0] >> 5) & 0x07)
            << " SI_capable=" << std::boolalpha << (!(field[0] & 0x10)) << "]";

   NEW_LINE(*out_str, 0, '\0');
}
