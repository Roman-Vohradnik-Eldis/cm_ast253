#include "RDFAsterix.hh"
#include "RDFAsterix250Private.hh"
#include "CMSDebug.hh"

const static std::map<uint8_t, std::string> PERIOD_MODE = map_list_of
    (1, "Mode1")
    (2, "Mode2")
    (3, "Mode 3/A")
    (6, "Mode C")
    (8, "Mode 3/AS")
    (9, "Mode CS")
    (10, "Mode 3/A Only")
    (11, "Mode C Only")
    (12, "Mode 3K1")
    (13, "Mode 3K2")
    (14, "Mode 3K3")
    (15, "Mode 3K4")
    (17, "Mode Allcall")
    (18, "Mode Allcall-3/A Only")
    (19, "Mode Allcall-C Only")
    (20, "Mode 3K1 3A")
    (21, "Mode 3K2 3A")
    (22, "Mode 3K3 3A")
    (24, "Mode 3K1 C")
    (25, "Mode 3K2 C")
    (26, "Mode 3K3 C")
    (31, "Mode RollCall")
    (32, "Mode Not Transmit")
    (23 ,"Mode Unknown");

static const char*
pulse_name[] = {
    "F1",
    "C1",
    "A1",
    "C2",
    "A2",
    "C4",
    "A4",
    "X",
    "B1",
    "D1",
    "B2",
    "D2",
    "B4",
    "D4",
    "F2",
    "SPI",
   "S",
};

const char *
pulse_to_str_(const uint8_t pulse_num) {
   if(pulse_num <= PULSES && pulse_num >= PULSE_F1)
      return pulse_name[pulse_num];

   return NULL;
}


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

// Time Of Day
void asterix_250_read_201 (RDFSifDetections & sif, unsigned char *&field)
{
    sif.Time.Present = true;
    sif.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
    field += 3;
}

void asterix_250_write_201 (RDFSifDetections & sif, unsigned char *&field)
{
    if (!sif.Time.Present)
    {
        cms_error ("not present");
        return;
    }

    int time = RDF_time_to_asterix_time(sif.Time.Time);

    field[0] = (time >> 16) & 0xff;
    field[1] = (time >> 8) & 0xff;
    field[2] = time & 0xff;
    field += 3;
}

void asterix_250_tobuffer_201 (std::stringstream & out, unsigned char *&field, bool one_line)
{
   uint64_t time = field2unsigned (field, 24);

   uint64_t mod = time % 128;
   time = time / 128;

   out << "201 [time=" << time << " sec (" << mod << "/128) "
   << std::setw(2)  << std::setfill('0')  << time / 3600 //h
   << ":" << std::setw(2)  << std::setfill('0')
   << ((time  % 3600) / 60) << ":" << std::setw(2) //m
   << std::setfill('0') << ((time % 3600) % 60) //s
   << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
   << "]";

   NEW_LINE(out, 0, '\0')
}


// Reply Report Descriptor
void asterix_250_read_202 (RDFSifDetections & sif, unsigned char *&field)
{
    sif.AzimuthNotValid = field[0] & 0x04 ? true : false;
    sif.AzimuthRSLSOut = field[0] & 0x02 ? true : false;
    field += 1;
}

void asterix_250_write_202 (RDFSifDetections & sif, unsigned char *&field)
{
    field[0] |= sif.AzimuthNotValid ? 0x04 : 0;
    field[0] |= sif.AzimuthRSLSOut ? 0x02 : 0;
    field += 1;
}

void asterix_250_tobuffer_202 (std::stringstream & out, unsigned char *&field, bool one_line)
{
    out << "202 [report_desc ";
    out << " AZAL=" << (field[0] & 0x04 ? true : false) <<
        " RSLS=" << (field[0] & 0x02 ? true : false) << "]";
    NEW_LINE (out, 0, '\0');
}

// Interrogation Mode
void asterix_250_read_203 (RDFSifDetections & sif, unsigned char *&field)
{
    sif.PeriodMode = field[0];
    field += 1;
}

void asterix_250_write_203 (RDFSifDetections & sif, unsigned char *&field)
{
    field[0] = sif.PeriodMode;
    field += 1;
}

void asterix_250_tobuffer_203 (std::stringstream & out, unsigned char *&field, bool one_line)
{
    out << "203 [period_mode=" << static_cast < int >(field[0]) << "]";
    NEW_LINE (out, 0, '\0');
}

// Sweep Number
void asterix_250_read_204 (RDFSifDetections & sif, unsigned char *&field)
{
    sif.SweepNumber = field2unsigned (field, 12);
    field += 2;
}

void asterix_250_write_204 (RDFSifDetections & sif, unsigned char *&field)
{
    field[0] = (sif.SweepNumber >> 8) & 0x0f;
    field[1] = sif.SweepNumber & 0xff;
    field += 2;
}

void asterix_250_tobuffer_204 (std::stringstream & out, unsigned char *&field, bool one_line)
{
    out << "204 [sweep_number=" << field2unsigned (field, 12) << "]";
    NEW_LINE (out, 0, '\0');
}

// Code
void asterix_250_read_205 (RDFSifDetections & sif, unsigned char *&field)
{
    sif.RawCodeX = field[0] & 0x20 ? true : false;
    sif.RawCodeSPI = field[0] & 0x10 ? true : false;
    sif.RawCodeValue = field2unsigned (field, 12);
    field += 2;
}

void asterix_250_write_205 (RDFSifDetections & sif, unsigned char *&field)
{
    field[0] |= sif.RawCodeX ? 0x20 : 0;
    field[0] |= sif.RawCodeSPI ? 0x10 : 0;
    field[0] |= (sif.RawCodeValue >> 8) & 0x0f;
    field[1] = sif.RawCodeValue & 0xff;
    field += 2;
}

void asterix_250_tobuffer_205 (std::stringstream & out, unsigned char *&field, bool one_line)
{
    out << "205 [raw_code ";
    out << "x=" << (field[0] & 0x20 ? true : false) << " ";

   out << "SPI=" << (field[0] & 0x10 ? true : false) << " ";

    out << "value=" << field2unsigned (field, 12) << "]";
    NEW_LINE (out, 0, '\0');
}



// Ref Amplitude
void asterix_250_read_206 (RDFSifDetections & sif, unsigned char *&field)
{
    //sif.SumSignum = field[0] & 0x80 ? true : false;
    sif.SumValue = field2unsigned (field, 14);

    sif.DiffSignum = (field[2] & 0x80) ? true : false;
    sif.DiffRSLS = (field[2] & 0x40) ? true : false;
    sif.DiffValue = field2unsigned (field + 2, 14);

    sif.PhaseValue = field2unsigned (field + 4, 14);

    field += 6;
}

void asterix_250_write_206 (RDFSifDetections & sif, unsigned char *&field)
{
    //field[0] |= sif.SumSignum ? 0x80 : 0;
    field[0] |= (sif.SumValue >> 8) & 0x3f;
    field[1] |= sif.SumValue & 0xff;

    field[2] |= sif.DiffSignum ? 0x80 : 0;
    field[2] |= sif.DiffRSLS ? 0x40 : 0;

    field[2] |= (sif.DiffValue >> 8) & 0x3f;
    field[3] |= sif.DiffValue & 0xff;

    field[4] |= (sif.PhaseValue  >> 8) & 0x3f;
    field[5] |= sif.PhaseValue  & 0xff;

    field += 6;
}

void asterix_250_tobuffer_206 (std::stringstream & out, unsigned char *field, bool one_line)
{
    out << "206 [ref_amplitudes]";

    NEW_LINE (out, 4, ' ');

    out << "[sum signum=" <<
        (field[0] & 0x80 ? true : false) << " value=" <<
        (-100.0 + (float) field2unsigned (field, 14) / 128.0) << " dBm]";

    NEW_LINE (out, 4, ' ');

    out  << "[diff_signum=" << (field[2] & 0x80 ? true : false)
        << " diff_rsls=" << (field[2] & 0x40 ? true : false)
        << " diff_value=" << (-100.0 + (float) field2unsigned (field + 2, 14) / 128.0)
        << " dBm]";

   NEW_LINE(out, 4, ' ');

   float phase = (float) field2unsigned (field + 4, 14) * (2 * M_PI) / 0x10000;

   out << "[phase=" << std::setprecision(2) << phase << "]";

    NEW_LINE (out, 0, '\0');
}

// Measured Position in Slant Range Coordinates
void asterix_250_read_207 (RDFSifDetections & sif, unsigned char *&field)
{
    sif.Rho = (float) field2unsigned (field, 16) * 1852 / 256;
    sif.Theta = (float) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
    sif.ThetaEncoder = (float) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000;

    sif.AzimuthValid = field[6] & 0x80;
    sif.AzimuthInBeam = field[6] & 0x40;
    sif.AzimuthDiffOk = field[6] & 0x20;
    sif.AzimuthRSLS = field[6] & 0x1;

    sif.AzimuthQuality = (float)field2unsigned(field + 7, 8);

    field += 8;
}

void asterix_250_write_207 (RDFSifDetections & sif, unsigned char *&field)
{
    unsigned short rho = (int) rint (sif.Rho * 256 / 1852);
    unsigned short theta = (int) rint (sif.Theta * 0x10000 / (2 * M_PI));
    unsigned short encoder = (int) rint (sif.ThetaEncoder * 0x10000 / (2 * M_PI));

    field[0] = (rho >> 8) & 0xff;
    field[1] = rho & 0xff;
    field[2] = (theta >> 8) & 0xff;
    field[3] = theta & 0xff;
    field[4] = (encoder >> 8) & 0xff;
    field[5] = encoder & 0xff;
    field[6] = sif.AzimuthRSLS;
    field[6] |= (sif.AzimuthDiffOk << 5);
    field[6] |= (sif.AzimuthInBeam << 6);
    field[6] |= (sif.AzimuthValid << 7);
    field[7] = sif.AzimuthQuality;

    field += 8;
}

void asterix_250_tobuffer_207 (std::stringstream & out, unsigned char *field, bool one_line)
{
    //  NEW_LINE(out, 4, ' ');
   double rho = (double)field2unsigned (field, 16) * 1852 / 256;
   double theta = (double) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
   double theta_enc = (double) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000;

    out << "207 [measured position in slant range coords]";

    NEW_LINE (out, 4, ' ');
    out << "[rho=" << static_cast<int>(rho) <<
        " theta=" << std::fixed << theta * RAD_TO_DEG <<
      " theta_encoder=" << std::fixed << theta_enc * RAD_TO_DEG
      << "]";

    NEW_LINE(out, 4, ' ');
    out << std::boolalpha << "[rsls=" << ((field[6] & 0x1) ? true : false)
        << " diff_ok=" << ((field[6] & 0x20) ? true : false)
        << " in_beam=" << ((field[6] & 0x40) ? true : false)
        << " valid=" << ((field[6] & 0x80) ? true : false);

    out << " quality=" << field2unsigned(field + 7, 8)  << "]";

    NEW_LINE (out, 0, '\0');
}

// SUM Pulse
void asterix_250_read_sum_pulse (RDFSumPulse & sum, unsigned char *&field)
{
    sum.OtherUsed = field[0] & 0x80 ? true : false;
    sum.OtherMatched = field[0] & 0x40 ? true : false;
    sum.AmplitudeNotMatched = field[0] & 0x20 ? true : false;
    sum.Garbled = field[0] & 0x10 ? true : false;
    sum.Amplitude = field2unsigned (field, 12);
    field += 2;
}


void
asterix_250_to_buffer_sum_pulse(std::stringstream & out,const unsigned char *field
      , bool one_line, const char *pulse_name) {

   NEW_LINE(out, 4, ' ')

    out << "[" << pulse_name << " OU=" <<
      (field[0] & 0x80 ? true : false) <<
        " OM=" << (field[0] & 0x40 ? true : false) <<
        " ANM=" << (field[0] & 0x20 ? true : false);
    out << " GB=" << (field[0] & 0x10 ? true : false) <<
        " AMP=" << field2unsigned (field, 12);
}
void
asterix_250_to_buffer_diff_pulse(std::stringstream & out,
      unsigned char *field, bool one_line) {

//   NEW_LINE(out, 4, ' ')

    out << " " << "S=" <<
      (field[0] & 0x80 ? true : false) <<
        " RSLS=" << (field[0] & 0x40 ? true : false) <<
        " DAMP=" << field2unsigned (field, 12) << "]";
}

void
asterix_250_tobuffer_209 (std::stringstream & out, unsigned char *field, bool one_line){}

#define TO_STR(token) /*""#token*/ pulse_to_str_(token)

void
asterix_250_to_vect_209 (std::vector<std::string> & out, unsigned char *field, bool one_line)
{
   std::stringstream pulse;

   unsigned char *tmp_field = field;
   uint8_t pos = 0;
   while(pos++ < PULSES) {
      asterix_250_to_buffer_diff_pulse(pulse, tmp_field, one_line);

      out.push_back(pulse.str());
      pulse.str("");

      tmp_field+=2;
   }
}
void
asterix_250_tobuffer_208 (std::stringstream & out, unsigned char *field, bool one_line)
{

    out << "298 [sum_pulses]";
   unsigned char *tmp_field = field;
   uint8_t pos = 0;
   while(pos++ < PULSES) {
      asterix_250_to_buffer_sum_pulse (out, tmp_field, one_line,
           pulse_to_str_(pos));
      tmp_field+=2;
   }

   NEW_LINE (out, 0, '\0')

}
void asterix_250_read_208 (RDFSifDetections & sif, unsigned char *&field)
{
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_F1], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_C1], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_A1], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_C2], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_A2], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_C4], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_A4], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_X], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_B1], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_D1], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_B2], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_D2], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_B4], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_D4], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_F2], field);
    asterix_250_read_sum_pulse (sif.SumPulse[PULSE_SPI], field);
}

void asterix_250_write_sum_pulse (RDFSumPulse & sum, unsigned char *&field)
{
    field[0] |= sum.OtherUsed ? 0x80 : 0;
    field[0] |= sum.OtherMatched ? 0x40 : 0;
    field[0] |= sum.AmplitudeNotMatched ? 0x20 : 0;
    field[0] |= sum.Garbled ? 0x10 : 0;
    field[0] |= (sum.Amplitude >> 8) & 0x0f;
    field[1] |= sum.Amplitude & 0xff;
    field += 2;
}

void asterix_250_write_208 (RDFSifDetections & sif, unsigned char *&field)
{
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_F1], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_C1], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_A1], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_C2], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_A2], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_C4], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_A4], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_X], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_B1], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_D1], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_B2], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_D2], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_B4], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_D4], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_F2], field);
    asterix_250_write_sum_pulse (sif.SumPulse[PULSE_SPI], field);
}

// DIFF Pulse
void asterix_250_read_diff_pulse (RDFDiffPulse & diff, unsigned char *&field)
{
    diff.Signum = field[0] & 0x80 ? true : false;
    diff.RSLS = field[0] & 0x40 ? true : false;
    diff.Amplitude = field2unsigned (field, 12);
    field += 2;
}

//verze pro vector
void
asterix_250_to_vect_208 (std::vector<std::string> & out, unsigned char *field, bool one_line)
{
   std::stringstream pulse;

   uint8_t *tmp_field = field;
   uint8_t pos = 0;
   while(pos < PULSES) {
      asterix_250_to_buffer_sum_pulse(pulse, tmp_field, one_line,
              pulse_to_str_(pos++));

      out.push_back(pulse.str());
      pulse.str("");

      tmp_field +=2 ;
   }
}
void asterix_250_read_209 (RDFSifDetections & sif, unsigned char *&field)
{
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_F1], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_C1], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_A1], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_C2], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_A2], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_C4], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_A4], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_X], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_B1], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_D1], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_B2], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_D2], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_B4], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_D4], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_F2], field);
    asterix_250_read_diff_pulse (sif.DiffPulse[PULSE_SPI], field);
}

void asterix_250_write_diff_pulse (RDFDiffPulse & diff, unsigned char *&field)
{
    field[0] |= diff.Signum ? 0x80 : 0;
    field[0] |= diff.RSLS ? 0x40 : 0;
    field[0] |= (diff.Amplitude >> 8) & 0x0f;
    field[1] |= diff.Amplitude & 0xff;
    field += 2;
}

void asterix_250_write_209 (RDFSifDetections & sif, unsigned char *&field)
{
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_F1], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_C1], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_A1], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_C2], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_A2], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_C4], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_A4], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_X], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_B1], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_D1], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_B2], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_D2], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_B4], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_D4], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_F2], field);
    asterix_250_write_diff_pulse (sif.DiffPulse[PULSE_SPI], field);
}

void asterix_250_to_buffer_208_209(std::stringstream &out, const
        std::vector<std::string> sum_diff[2], bool one_line) {

    if(sum_diff[0].size() != sum_diff[1].size())
    {
        std::cerr << __func__ << ": count sum, diff pulses is not equal,"
                  << " skipping them at output." << std::endl;
        return;
    }

    out << "298 [sums]";
    uint16_t vect_no = 0;
    while(vect_no < sum_diff[0].size())
    {
        out << sum_diff[0][vect_no] << sum_diff[1][vect_no];
        vect_no++;
    }
}

// Time Of Day
void asterix_250_read_301 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.Time.Present = true;
    modes.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
    field += 3;
}

void
asterix_250_tobuffer_301 (std::stringstream & out, unsigned char *field, bool one_line)
{
    uint64_t time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;

    out << "301 [time=" << time / 1000 << " sec (" << std::setw (2) << std::setfill ('0') << time / 3600000 //h
        << ":" << std::setw (2) << std::setfill ('0') << (((time / 1000) % 3600) / 60) << ":" << std::setw (2)  //m
        << std::setfill ('0') << (((time / 1000) % 3600) % 60)  //s
        << ")]";

    NEW_LINE (out, 0, '\0');
}


void asterix_250_write_301 (RDFModeSDetections & modes, unsigned char *&field)
{
    if (!modes.Time.Present)
    {
        cms_error ("not present");
        return;
    }

    int time = RDF_time_to_asterix_time(modes.Time.Time);

    field[0] = (time >> 16) & 0xff;
    field[1] = (time >> 8) & 0xff;
    field[2] = time & 0xff;
    field += 3;
}

// Reply Report Descriptor
void asterix_250_read_302 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.RollCallNotReceived = field[0] & 0x80 ? true : false;
    modes.ReplyCorrected = field[0] & 0x40 ? true : false;
    modes.TargetBDS = (field[0] & 0x30) >> 4;
    modes.InvalidBDS = field[0] & 0x08 ? true : false;
    modes.LockoutLockedOut = field[0] & 0x01 ? true : false;
    field += 1;
}

void
asterix_250_tobuffer_302 (std::stringstream & out, unsigned char *field, bool one_line)
{
    out << "302 [report_desc ";
    out << " NRC=" << (field[0] & 0x80 ? true : false);
    out << " Corrected=" << (field[0] & 0x40 ? true : false);

    if(TargetBDS_to_String.end() != TargetBDS_to_String.find((field[0] & 0x30) >> 4))
        out << " TargetBDS=" << ((field[0] & 0x30) >> 4) << " (" << ((TargetBDS_to_String.find((field[0] & 0x30) >> 4)->second).c_str()) << ")";
    else
        out << " TargetBDS=" << ((field[0] & 0x30) >> 4) << "(not_defined)";

    out << " IvalidBDS=" << (field[0] & 0x08 ? true : false) <<
        " Lock=" << (field[0] & 0x01 ? true : false) << "]";

    NEW_LINE (out, 0, '\0');
}

void asterix_250_write_302 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] |= modes.RollCallNotReceived ? 0x80 : 0;
    field[0] |= modes.ReplyCorrected ? 0x40 : 0;
    field[0] |= (modes.TargetBDS & 0x03) << 4;
    field[0] |= modes.InvalidBDS ? 0x08 : 0;
    field[0] |= modes.LockoutLockedOut ? 0x01 : 0;
    field += 1;
}

// DF Raw
void asterix_250_read_303 (RDFModeSDetections & modes, unsigned char *&field)
{
    unsigned length = field[0];
    field++;

    for (unsigned i = 0; i < length; i++)
    {
        modes.DfRaw.Data.push_back (field[0]);
        field++;
    }
}

void
asterix_250_tobuffer_303 (std::stringstream & out, unsigned char *field, bool one_line)
{
    uint8_t length = field[0];

    out << "303 [DF-raw length=" << static_cast<int>(length) << "]";
    NEW_LINE (out, 4, ' ');

    out << "[data_hex=" << std::noshowbase;
    for (uint8_t i = 1; i <= length; i++)
    {
        out << std::hex;
        out << static_cast <int>(field[i]) << " ";
    }
    out << std::dec;
    out << "]";

    NEW_LINE (out, 0, '\0');
}

void asterix_250_write_303 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = modes.DfRaw.Data.size ();
    field++;

    for (unsigned i = 0; i < modes.DfRaw.Data.size (); i++)
    {
        field[0] = modes.DfRaw.Data[i];
        field++;
    }
}

// DF Confidence
void asterix_250_read_304 (RDFModeSDetections & modes, unsigned char *&field)
{
    unsigned length = field[0];
    field++;

    for (unsigned i = 0; i < length; i++)
    {
        modes.DfConfidence.push_back (field[0]);
        field++;
    }
}

void
asterix_250_tobuffer_304 (std::stringstream & out, unsigned char *field, bool one_line)
{
    uint8_t length = field[0];

    out << "304 [DF-confidence length=" << static_cast<int>(length) << "]";
    NEW_LINE (out, 4, ' ');
    out << "[data_hex=" << std::noshowbase;
    for (uint8_t i = 0; i < length; i++)
    {
        out << std::hex;
        out << static_cast <int>(field[i]) << " ";
    }
    out << "]";
    out << std::dec;
    NEW_LINE (out, 0, '\0');
}

void asterix_250_write_304 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = modes.DfConfidence.size ();
    field++;

    for (unsigned i = 0; i < modes.DfConfidence.size (); i++)
    {
        field[0] = modes.DfConfidence[i];
        field++;
    }
}

// DF Corrected
void asterix_250_read_305 (RDFModeSDetections & modes, unsigned char *&field)
{
    unsigned length = field[0];
    field++;

    for (unsigned i = 0; i < length; i++)
    {
        modes.DfCorrected.Data.push_back (field[0]);
        field++;
    }
}

void
asterix_250_tobuffer_305 (std::stringstream & out, unsigned char *field, bool one_line)
{
    uint8_t length = field[0];

   std::vector<uint8_t> data;

    out << "305 [DF-corrected length=" << static_cast<int>(length) << "]";

    NEW_LINE (out, 4, ' ');
    out << "[data_hex=" << std::noshowbase;

    for (uint8_t i = 1; i <= length; i++)
    {
        out << std::hex;
        out << std::setw(2) << std::setfill('0') << static_cast < int >(field[i]) << " ";

      data.push_back(field[i]);
    }
    out << "]";
    out << std::dec << std::setfill(' ');

   df_items df;

   df.Data_to_Items(data);

   df.to_stringstream(out, one_line);
}


void asterix_250_write_305 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = modes.DfCorrected.Data.size ();
    field++;

    for (unsigned i = 0; i < modes.DfCorrected.Data.size (); i++)
    {
        field[0] = modes.DfCorrected.Data[i];
        field++;
    }
}

// Measured Position in Slant Polar Coordinates
void asterix_250_read_308 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.Rho = (float) field2unsigned (field, 16) * 1852 / 256;
    modes.Theta = (float) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
    modes.ThetaEncoder = (float) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000;
    modes.AzimuthValid = field[6] & 0x80;
    modes.AzimuthInBeam = field[6] & 0x40;
    modes.AzimuthDiffOk = field[6] & 0x20;
    modes.AzimuthRSLS = field[6] & 0x1;

    modes.AzimuthQuality = (float)field2unsigned(field + 7, 8);

    field += 8;
}

void
asterix_250_tobuffer_308 (std::stringstream & out, unsigned char *field, bool one_line)
{
    out << "308 [MP_in_SLC";
    out << " rho=" << static_cast<int>(
         field2unsigned (field, 16) * 1852.0 / 256.0)
        << " m theta=" << ((float) field2unsigned (field + 2, 16)
            * (2 * M_PI) / 0x10000) * RAD_TO_DEG
        << " deg  theta_enc=" << ((float) field2unsigned (field + 4,
            16) * (2 * M_PI) / 0x10000) * RAD_TO_DEG << " deg]";

    NEW_LINE(out, 4, ' ');
    out << std::boolalpha << "[rsls=" << ((field[6] & 0x1) ? true : false)
        << " diff_ok=" << ((field[6] & 0x20) ? true : false)
        << " in_beam=" << ((field[6] & 0x40) ? true : false)
        << " valid=" << ((field[6] & 0x80) ? true : false);

    out << " quality=" << field2unsigned(field + 7, 8)  << "]";

    NEW_LINE (out, 0, '\0');
}

void asterix_250_write_308 (RDFModeSDetections & modes, unsigned char *&field)
{
    unsigned rho = lrint (modes.Rho * 256 / 1852);
    unsigned theta = lrint (modes.Theta * 0x10000 / (2 * M_PI));
    unsigned encoder = lrint (modes.ThetaEncoder * 0x10000 / (2 * M_PI));

    field[0] = (rho >> 8) & 0xff;
    field[1] = rho & 0xff;
    field[2] = (theta >> 8) & 0xff;
    field[3] = theta & 0xff;
    field[4] = (encoder >> 8) & 0xff;
    field[5] = encoder & 0xff;
    field[6] = modes.AzimuthRSLS;
    field[6] |= (modes.AzimuthDiffOk << 5);
    field[6] |= (modes.AzimuthInBeam << 6);
    field[6] |= (modes.AzimuthValid << 7);
    field[7] = modes.AzimuthQuality;
    field += 8;
}

// Measured Amplitude
void asterix_250_read_309 (RDFModeSDetections & modes, unsigned char *&field)
{
    //modes.SumSignum = field[0] & 0x80 ? true : false;
    modes.SumValue = field2unsigned (field, 14);

    modes.DiffSignum = field[2] & 0x80 ? true : false;
    modes.DiffRSLS = field[2] & 0x40 ? true : false;
    modes.DiffValue = field2unsigned (field + 2, 14);

    modes.PhaseValue = field2unsigned (field + 4, 14);

    field += 6;
}
void asterix_250_tobuffer_309(std::stringstream &out,unsigned char *field, bool one_line)
{
    out << "309 [measured amplitude]";
    NEW_LINE (out, 4, ' ');

    out << "[sum_value=" << std::dec
      << (-100.0 + (float) field2unsigned (field + 0, 14) / 128.0) << " dBm]";

   NEW_LINE(out, 4, ' ');
   out  << "[diff_signum=" << (field[2] & 0x80 ? true : false)
        << " diff_rsls=" << (field[2] & 0x40 ? true : false)
        << " diff_value=" << (-100.0 + (float) field2unsigned (field + 2, 14) / 128.0)
        << " dBm]";

   NEW_LINE(out, 4, ' ');
   out << "[phase=" << std::setprecision(2) << field2unsigned(field + 4, 14) << "]";

    NEW_LINE (out, 0, '\0');
}


void asterix_250_write_309 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] |= (modes.SumValue >> 8) & 0x3f;
    field[1] |= modes.SumValue & 0xff;

    field[2] |= modes.DiffSignum ? 0x80 : 0;
    field[2] |= modes.DiffRSLS ? 0x40 : 0;
    field[2] |= (modes.DiffValue >> 8) & 0x3f;

    field[3] |= modes.DiffValue & 0xff;
    field[4] |= (modes.PhaseValue >> 8) & 0x3f;
    field[5] |= modes.PhaseValue & 0xff;

    field += 6;
}

// Interrogation Mode
void asterix_250_read_310 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.PeriodType = field[0];
    field++;
}

void asterix_250_tobuffer_310(std::stringstream &out,unsigned char *field, bool one_line)
{

  out << "310 [period_mode period_type=" << static_cast<int>(field[0])
      << "~\"";

  if(PERIOD_MODE.find(field[0]) != PERIOD_MODE.end())
  {
      out << "Period "<< PERIOD_MODE.find(field[0])->second;
  }
  else
  {
      out << "Period not_defined";
  }

   out << "\"]";

   NEW_LINE(out, 0, '\0')
}
void asterix_250_write_310 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = modes.PeriodType;
    field++;
}

// Sweep Number
void asterix_250_read_311 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.SweepNumber = field2unsigned (field, 12);
    field += 2;
}
void asterix_250_tobuffer_311(std::stringstream &out,unsigned char *field, bool one_line)
{
   out << "311 [sweep_number value=" << static_cast<int>(field2unsigned(field, 12)) << "]";

   NEW_LINE(out, 0, '\0')
}
void asterix_250_write_311 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = (modes.SweepNumber >> 8) & 0x0f;
    field[1] = modes.SweepNumber & 0xff;
    field += 2;
}

// Command Number
void asterix_250_read_312 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.CommandNumber = field2unsigned (field, 12);
    field += 2;
}
void asterix_250_tobuffer_312(std::stringstream &out,unsigned char *field, bool one_line)
{
   out << "312 [command_number value=" << static_cast<int>(field2unsigned(field, 12)) << "]";

   NEW_LINE(out, 0, '\0')
}
void asterix_250_write_312 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = (modes.CommandNumber >> 8) & 0x0f;
    field[1] = modes.CommandNumber & 0xff;
    field += 2;
}

// Track Number
void asterix_250_read_313 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.TrackNumber.Present = true;
    modes.TrackNumber.Temporary = field[0] & 0x80 ? true : false;
    modes.TrackNumber.Number = field2unsigned (field, 12);
    field += 2;
}
void asterix_250_tobuffer_313(std::stringstream &out,unsigned char *field, bool one_line)
{
   out << "313 [track_number present=" << 1 << " temporary="
      << (field[0] & 0x80 ? true : false) << " number=" <<
      field2unsigned (field, 12) << "]";

   NEW_LINE(out, 0, '\0')
}
void asterix_250_write_313 (RDFModeSDetections & modes, unsigned char *&field)
{
//  if (!modes.TrackNumber.Present)
//  {
//      cms_error ("not present");
//      return;
//  }

    field[0] |= modes.TrackNumber.Temporary ? 0x80 : 0;
    field[0] |= (modes.TrackNumber.Number >> 8) & 0x0f;
    field[1] = modes.TrackNumber.Number & 0xff;
    field += 2;
}

// BDS
void asterix_250_read_314 (RDFModeSDetections & modes, unsigned char *&field)
{
    modes.ExpectedBDS.present = true;
   modes.ExpectedBDS.BDS_1 = (field[0] >> 4) & 0x0f;
    modes.ExpectedBDS.BDS_2 = field[0] & 0x0f;
    field += 1;
}

void asterix_250_tobuffer_314(std::stringstream & out, unsigned char *field,    bool one_line) {
   out << "314 [BDS_hex=" << std::hex << static_cast<int>((field[0] >> 4) & 0x0f) <<
      "," << static_cast<int>(field[0] & 0x0f) << "]";

   NEW_LINE(out, 0, '\0')
}


void asterix_250_write_314 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] |= (modes.ExpectedBDS.BDS_1 << 4) & 0xf0;
    field[0] |= modes.ExpectedBDS.BDS_2 & 0x0f;
    field += 1;
}

// Uplink Data
void asterix_250_read_315 (RDFModeSDetections & modes, unsigned char *&field)
{
    unsigned length = field[0];
    field++;

    for (unsigned i = 0; i < length; i++)
    {
        modes.Uplink.Data.push_back (field[0]);
        field++;
    }
}

void
asterix_250_tobuffer_315(std::stringstream & out, unsigned char *field, bool one_line) {

   uint8_t length = field[0];

    out << std::dec << "315 [uplink_data length=" <<
      static_cast<int>(length) << "]";

    NEW_LINE (out, 4, ' ');
    out << "[data_hex=" << std::noshowbase;

   std::vector<uint8_t> data;

    for (uint8_t i = 1; i <= length; i++)
    {
        out << std::hex << setw(2) << std::setfill('0');
        out << static_cast < int >(field[i]) << " ";

      data.push_back(field[i]);
    }
    out << "]";

   uf_items uf;
   uf.Data_to_Items(data);


   uf.to_stringstream(out, one_line);
    out << "]";

    out << std::dec << std::setfill(' ');

    NEW_LINE (out, 0, '\0')
}

void
asterix_250_write_315 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] = modes.Uplink.Data.size ();
    field++;

    for (unsigned i = 0; i < modes.Uplink.Data.size (); i++)
    {
        field[0] = modes.Uplink.Data[i];
        field++;
    }
}

// Datalink Control
void asterix_250_read_316 (RDFModeSDetections & modes, unsigned char *&field)
{

   modes.DatalinkControl.present = true;

    modes.DatalinkControl.CommATms = field[0] & 0x0f;
    modes.DatalinkControl.DownElmCtrl = (field[1] >> 4) & 0x03;
    modes.DatalinkControl.UpElmCtrl = (field[1] >> 2) & 0x03;
    modes.DatalinkControl.CommBCtrl = field[1] & 0x03;

    field += 2;
}
void
asterix_250_tobuffer_316(std::stringstream & out, unsigned char *field, bool one_line) {
   out << "316 [data_control CATMS=" << static_cast<int>(field[0] & 0x0f)
      << " ELM_ctrl=" << static_cast<int>((field[1] >> 4) & 0x03)
      << " UP ELM_ctrl=" << static_cast<int>((field[1] >> 2) & 0x03)
      << " CB_ctrl=" << static_cast<int>(field[1] & 0x03) << "]";

   NEW_LINE(out, 0, '\0')
}
void asterix_250_write_316 (RDFModeSDetections & modes, unsigned char *&field)
{
    field[0] |= modes.DatalinkControl.CommATms & 0x0f;
    field[1] |= (modes.DatalinkControl.DownElmCtrl << 4) & 0x30;
    field[1] |= (modes.DatalinkControl.UpElmCtrl << 2) & 0xc0;
    field[1] |= modes.DatalinkControl.CommBCtrl & 0x03;
    field += 2;
}

//Sweep Number
void asterix_250_read_317 (RDFModeSDetections & modes, unsigned char *&field)
{
   modes.SweepLengthPresent = true;
   modes.SweepLength = field2unsigned(field, 16);
    field += 2;
}
void
asterix_250_tobuffer_317(std::stringstream & out, unsigned char *field, bool one_line) {

   out << "317 [sweep_length=" << field2unsigned(field, 16) << "]";

   NEW_LINE(out, 0, '\0')
}

void asterix_250_write_317 (RDFModeSDetections & modes, unsigned char *&field)
{
   field[0] = (modes.SweepLength >> 8) & 0xff;
   field[1] = modes.SweepLength & 0xff;

    field += 2;
}

//Interrogation Delay
void asterix_250_read_318 (RDFModeSDetections & modes, unsigned char *&field)
{
   modes.InterrogationDelayPresent = true;
   modes.InterrogationDelay = field2unsigned(field, 16);
    field += 2;
}

void
asterix_250_tobuffer_318(std::stringstream & out, unsigned char *field, bool one_line) {

   out << "318 [dl=" << field2unsigned(field, 16) << "]";
   NEW_LINE(out, 0, '\0')
}

void asterix_250_write_318 (RDFModeSDetections & modes, unsigned char *&field)
{

   field[0] = (modes.InterrogationDelay >> 8) & 0xff;
   field[1] = modes.InterrogationDelay & 0xff;

    field += 2;
}

void asterix_250_read_samples(unsigned char *&field, std::vector<uint16_t> &samples) {
    unsigned length = field2unsigned(field, 16);

    field += 2;

   samples.clear();
    for (unsigned i = 0; i < length; i++) {
        samples.push_back(field2unsigned(field, 16));
        field += 2;
    }
}

void asterix_250_write_samples(unsigned char *&field, std::vector<uint16_t> &samples) {

   field[0] = ((samples.size() & 0xffff) >> 8) & 0xff;
   field[1] = samples.size() & 0xff;

    field += 2;

    for (unsigned i = 0; i < samples.size(); i++) {
      field[0] = (samples[i] >> 8) & 0xff;
      field[1] = samples[i] & 0xff;
        field += 2;
    }
}

void asterix_250_tobuffer_samples(std::stringstream & out, unsigned char *field, bool one_line) {

}

// Sample Sum
void asterix_250_read_319(RDFModeSDetections & modes, unsigned char *&field)
{
   asterix_250_read_samples(field, modes.SumSamples);
}

void
asterix_250_tobuffer_319(std::stringstream & out, unsigned char *field, bool one_line) {

   out << "319 [samples=" << field2unsigned(field, 16) << "]";
   if(!one_line)
      out << std::endl;

   asterix_250_tobuffer_samples(out, field, one_line);
}


void asterix_250_write_319(RDFModeSDetections & modes, unsigned char *&field)
{
   asterix_250_write_samples(field, modes.SumSamples);
}

// Sample Diff
void asterix_250_read_320(RDFModeSDetections & modes, unsigned char *&field)
{
   asterix_250_read_samples(field, modes.DiffSamples);
}

void
asterix_250_tobuffer_320(std::stringstream & out, unsigned char *field, bool one_line) {
   out << "320 [samples=" << field2unsigned(field, 16) << "]";
   if(!one_line)
      out << std::endl;

   asterix_250_tobuffer_samples(out, field, one_line);
}

void asterix_250_write_320(RDFModeSDetections & modes, unsigned char *&field)
{
   asterix_250_write_samples(field, modes.DiffSamples);
}

// Sample Phase
void asterix_250_read_321(RDFModeSDetections & modes, unsigned char *&field)
{
   asterix_250_read_samples(field, modes.PhaseSamples);
}

void
asterix_250_tobuffer_321(std::stringstream & out, unsigned char *field, bool one_line) {
   out << "321 [samples=" << field2unsigned(field, 16) << "]";
   if(!one_line)
      out << std::endl;

   asterix_250_tobuffer_samples(out, field, one_line);
}

void asterix_250_write_321(RDFModeSDetections & modes, unsigned char *&field)
{
   asterix_250_write_samples(field, modes.PhaseSamples);
}

// Sample Sum
void asterix_250_read_210(RDFSifDetections &sif, unsigned char *&field)
{
   asterix_250_read_samples(field, sif.SumSamples);
}

void
asterix_250_tobuffer_210(std::stringstream & out, unsigned char *field, bool one_line) {
   asterix_250_tobuffer_samples(out, field, one_line);
}


void asterix_250_write_210(RDFSifDetections &sif, unsigned char *&field)
{
   asterix_250_write_samples(field, sif.SumSamples);
}

// Sample Diff
void asterix_250_read_211(RDFSifDetections &sif, unsigned char *&field)
{
   asterix_250_read_samples(field, sif.DiffSamples);
}

void
asterix_250_tobuffer_211(std::stringstream & out, unsigned char *field, bool one_line) {
   asterix_250_tobuffer_samples(out, field, one_line);
}


void asterix_250_write_211(RDFSifDetections &sif, unsigned char *&field)
{
   asterix_250_write_samples(field, sif.DiffSamples);
}

// Sample Phase
void asterix_250_read_212(RDFSifDetections &sif, unsigned char *&field)
{
   asterix_250_read_samples(field, sif.PhaseSamples);
}

void
asterix_250_tobuffer_212(std::stringstream & out, unsigned char *field, bool one_line) {
   asterix_250_tobuffer_samples(out, field, one_line);
}


void asterix_250_write_212(RDFSifDetections &sif, unsigned char *&field)
{
   asterix_250_write_samples(field, sif.PhaseSamples);
}

void asterix_250_read_136(RDFTargetPlot & plot, unsigned char *&field)
{
    plot.UvdFriendStatus.Code = field[0];
    plot.UvdFriendStatus.Present = true;
    field++;
}

void asterix_250_write_136(RDFTargetPlot & plot, unsigned char *&field)
{
    field[0] = plot.UvdFriendStatus.Code;
    field++;
}

void asterix_250_tobuffer_136(std::stringstream &out,unsigned char *field, bool one_line)
{
    out << "136 [friend=";
    switch(field[0]) {
        case 0:
            out << "No Reply";
        case 1:
            out << "True Friend";
        case 2: case 3:
            out << "Reserved";
        default:
            out << "Invalid(" << static_cast<int>(field[0]) << ")";
    }
    out << "]";
    NEW_LINE(out, 0, '\0');
}

void asterix_250_read_137(RDFTargetPlot & plot, unsigned char *&field)
{
    plot.UvdCode.Code = field2unsigned(field, 32);
    plot.UvdCode.Present = true;

    field += 4;
}

void asterix_250_write_137(RDFTargetPlot & plot, unsigned char *&field)
{
    for(int8_t i = 3, j = 0; i >= 0; i--, j++)
    {
        field[j] = (plot.UvdCode.Code >> (i * 8)) & 0xff;
    }
    field += 4;
}

void asterix_250_tobuffer_137(std::stringstream &out,unsigned char *field, bool one_line)
{
    RDFUvdCode code;
    code.Code = field2unsigned(field, 32);

    out << "137 [id=" << code.get_identification() << " "
        << "validated=" << std::boolalpha << (code.Code & (1<< 31)) << " "
        << "spi=" << (code.Code & (1 << 30)) << "]";

    NEW_LINE(out, 0, '\0');
}

void asterix_250_read_138(RDFTargetPlot & plot, unsigned char *&field)
{
    plot.UvdAltitude.Code = field2unsigned(field, 32);
    plot.UvdAltitude.Present = true;

    field += 4;
}

void asterix_250_write_138(RDFTargetPlot & plot, unsigned char *&field)
{
   for(int8_t i = 3, j = 0; i >= 0; i--, j++)
   {
        field[j] = (plot.UvdAltitude.Code >> (i * 8)) & 0xff;
   }
   field += 4;
}

void asterix_250_tobuffer_138(std::stringstream &out,unsigned char *field, bool one_line)
{
    RDFUvdAltitude alt;
    alt.Code = field2unsigned(field, 32);

    out << "138 [altitude=" << alt.get_altitude() << " "
        << "relative=" << std::boolalpha << alt.get_altitude_relative() << " "
        << "emg=" << alt.get_emergency() << " "
        << "fuel=" << alt.get_fuel() << "%"
        << "validated=" << (alt.Code & (1 << 31))
        << "]";

    NEW_LINE(out, 0, '\0');
}

void asterix_250_read_139(RDFTargetPlot & plot, unsigned char *&field)
{
    plot.UvdSpeed.Code = field2unsigned(field, 32);
    plot.UvdSpeed.Present = true;

    field += 4;
}
void asterix_250_write_139(RDFTargetPlot & plot, unsigned char *&field)
{
   for(int8_t i = 3, j = 0; i >= 0; i--, j++)
   {
        field[j] = (plot.UvdSpeed.Code >> (i * 8)) & 0xff;
   }
   field += 4;

}

void asterix_250_tobuffer_139(std::stringstream &out,unsigned char *field, bool one_line)
{
    RDFUvdSpeed speed;
    speed.Code = field2unsigned(field, 32);

    out << "139 [speed=" << speed.get_speed() << " "
        << "sign=" << (speed.Code & (1 << 9)) << " "
        << "heading=" << speed.get_heading() << " "
        << "validated=" << (speed.Code & (1 << 31))
        << "]";
    NEW_LINE(out, 0, '\0');
}


void asterix_250_read_501(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.Time.Present = true;
    uvd.Time.Time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;
    field += 3;
}

void asterix_250_write_501(RDFUvdDetections & uvd, unsigned char *&field)
{
    if(!uvd.Time.Present)
    {
        cms_error ("not present");
        return;
    }

    int time = RDF_time_to_asterix_time(uvd.Time.Time);

    field[0] = (time >> 16) & 0xff;
    field[1] = (time >> 8) & 0xff;
    field[2] = time & 0xff;
    field += 3;

}

void
asterix_250_tobuffer_501 (std::stringstream & out, unsigned char *field, bool one_line)
{
    uint64_t time = ((long long) field2unsigned (field, 24) * 1000 + 64) / 128;

    out << "501 [time=" << time / 1000 << " sec (" << std::setw (2) << std::setfill ('0') << time / 3600000 //h
        << ":" << std::setw (2) << std::setfill ('0') << (((time / 1000) % 3600) / 60) << ":" << std::setw (2)  //m
        << std::setfill ('0') << (((time / 1000) % 3600) % 60)  //s
        << ")]";

    NEW_LINE (out, 0, '\0');
}

void asterix_250_read_502(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.ReplyReportDescriptor = field[0];
    field++;
}

void asterix_250_write_502(RDFUvdDetections & uvd, unsigned char *&field)
{
    field[0]= uvd.ReplyReportDescriptor;
    field++;
}
void
asterix_250_tobuffer_502 (std::stringstream & out, unsigned char *field, bool one_line)
{
    out << "502 [reply_report=" << static_cast<int>(field[0]) << "]";
    NEW_LINE(out, 0, '\0');
}

void asterix_250_read_503(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.InterrogationMode = field[0];
    field++;
}

void asterix_250_write_503(RDFUvdDetections & uvd, unsigned char *&field)
{
    field[0] = uvd.InterrogationMode;
    field++;
}

void
asterix_250_tobuffer_503 (std::stringstream & out, unsigned char *field, bool one_line)
{
  out << "503 [interrogation_mode=" << static_cast<int>(field[0])
      << "~\"";

  if(PERIOD_MODE.find(field[0]) != PERIOD_MODE.end())
  {
      out << "Period "<< PERIOD_MODE.find(field[0])->second;
  } else
  {
      out << "Period is not defined";
  }

   out << "\"]";

   NEW_LINE(out, 0, '\0');
}


void asterix_250_read_504(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.SweepNumber = field2unsigned(field, 12);
    field += 2;
}

void asterix_250_write_504(RDFUvdDetections & uvd, unsigned char *&field)
{
    field[0] = (uvd.SweepNumber >> 8) & 0x0f;
    field[1] = uvd.SweepNumber & 0xff;
    field += 2;
}
void
asterix_250_tobuffer_504 (std::stringstream & out, unsigned char *field, bool one_line)
{
    out << "504 [sweep_number=" << field2unsigned(field, 12) << "]";
    NEW_LINE(out, 0, '\0');
}


void asterix_250_read_505(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.RawCode = 0;
    for(int8_t shift = 0, pos = 4; pos >= 0; shift++, pos--) {
        uvd.RawCode |=  field[pos] << (8 * shift);
    }
    field += 5;
}

void
asterix_250_tobuffer_505 (std::stringstream & out, unsigned char *field, bool one_line)
{

    uint64_t code = 0;
    for(int8_t shift = 0, pos = 4; pos >= 0; shift++, pos--) {
        code |=  field[pos] << (8 * shift);
    }

    out << "505 [code=" << code << "]";
    NEW_LINE(out, 0, '\0');
}


void asterix_250_write_505(RDFUvdDetections & uvd, unsigned char *&field)
{
    for(uint8_t pos = 0, shift = 4; pos <= 4; pos++, shift--) {
        field[pos] = (uvd.RawCode >> (8 * shift)) & 0xff;
    }
    field += 5;
}

void asterix_250_read_506(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.SumValue = field2unsigned (field, 14);

    uvd.DiffSignum = field[2] & 0x80 ? true : false;
    uvd.DiffRSLS = field[2] & 0x40 ? true : false;
    uvd.DiffValue = field2unsigned (field + 2, 14);

    uvd.PhaseValue = field2unsigned(field + 4, 14);

    field += 6;
}

void
asterix_250_tobuffer_506 (std::stringstream & out, unsigned char *field, bool one_line)
{

    out << "506 [sum=" << field2unsigned(field, 14)
        << " diff_sign=" << (field[2] & 0x80 ? true : false)
        << " diff_rsls=" << (field[2] & 0x40 ? true : false)
        << " diff=" << field2unsigned(field + 2, 14)
        << " phase=" << field2unsigned(field + 4, 14)
        << "]";
    NEW_LINE(out, 0, '\0');
}


void asterix_250_write_506(RDFUvdDetections & uvd, unsigned char *&field)
{
    field[0] |= (uvd.SumValue >> 8) & 0x3f;
    field[1] |= uvd.SumValue & 0xff;

    field[2] |= uvd.DiffSignum ? 0x80 : 0;
    field[2] |= uvd.DiffRSLS ? 0x40 : 0;

    field[2] |= (uvd.DiffValue >> 8) & 0x3f;
    field[3] |= uvd.DiffValue & 0xff;

    field[4] |= (uvd.PhaseValue >> 8) & 0x3f;
    field[5] |= uvd.PhaseValue & 0xff;


    field += 6;
}

void asterix_250_read_507(RDFUvdDetections & uvd, unsigned char *&field)
{
    uvd.Rho = (float) field2unsigned (field, 16) * 1852 / 256;
    uvd.Theta = (float) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000;
    uvd.ThetaEncoder = (float) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000;
    uvd.ThetaValid = (((field[6] & 0x80) != 0) ? true : false);
    uvd.ThetaInBeam = field[6] & 0x40;
    uvd.ThetaRSLS = field[6] & 0x1;

    uvd.ThetaQuality = (float)field2unsigned(field + 7, 8);

    field += 8;
}

void
asterix_250_tobuffer_507 (std::stringstream & out, unsigned char *field, bool one_line)
{
    out << "507 [MP_in_SLC"<< " rho=" << static_cast<int>(field2unsigned (field, 16) * 1852.0 / 256.0)
        << " m theta=" << ((float) field2unsigned (field + 2, 16) * (2 * M_PI) / 0x10000) * RAD_TO_DEG
        << " deg  theta_enc=" << ((float) field2unsigned (field + 4, 16) * (2 * M_PI) / 0x10000) * RAD_TO_DEG
        << " deg]";

    NEW_LINE(out, 4, ' ');

    out << std::boolalpha << "[rsls=" << ((field[6] & 0x1) ? true : false)
        << " in_beam=" << ((field[6] & 0x40) ? true : false)
        << " valid=" << ((field[6] & 0x80) ? true : false);

    out << " quality=" << field2unsigned(field + 7, 8)  << "]";

    NEW_LINE (out, 0, '\0');
}

void asterix_250_write_507(RDFUvdDetections & uvd, unsigned char *&field)
{
    unsigned rho = lrint (uvd.Rho * 256 / 1852);
    unsigned theta = lrint (uvd.Theta * 0x10000 / (2 * M_PI));
    unsigned encoder = lrint (uvd.ThetaEncoder * 0x10000 / (2 * M_PI));

    field[0] = (rho >> 8) & 0xff;
    field[1] = rho & 0xff;
    field[2] = (theta >> 8) & 0xff;
    field[3] = theta & 0xff;
    field[4] = (encoder >> 8) & 0xff;
    field[5] = encoder & 0xff;
    field[6] = uvd.ThetaRSLS;
    field[6] |= (uvd.ThetaDiffOk << 5);
    field[6] |= (uvd.ThetaInBeam << 6);
    field[6] |= (uvd.ThetaValid << 7);
    field[7] = uvd.ThetaQuality;

    field += 8;
}

void asterix_250_read_508(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_read_sum_pulse (uvd.SumPulse[PULSE_RK1], field);
    asterix_250_read_sum_pulse (uvd.SumPulse[PULSE_RK2], field);
    asterix_250_read_sum_pulse (uvd.SumPulse[PULSE_RK3], field);
    asterix_250_read_sum_pulse (uvd.SumPulse[PULSE_RK11], field);
    asterix_250_read_sum_pulse (uvd.SumPulse[PULSE_RK12], field);
    asterix_250_read_sum_pulse (uvd.SumPulse[PULSE_RK13], field);
}

void asterix_250_write_508(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_write_sum_pulse (uvd.SumPulse[PULSE_RK1], field);
    asterix_250_write_sum_pulse (uvd.SumPulse[PULSE_RK2], field);
    asterix_250_write_sum_pulse (uvd.SumPulse[PULSE_RK3], field);
    asterix_250_write_sum_pulse (uvd.SumPulse[PULSE_RK11], field);
    asterix_250_write_sum_pulse (uvd.SumPulse[PULSE_RK12], field);
    asterix_250_write_sum_pulse (uvd.SumPulse[PULSE_RK13], field);
}

void asterix_250_read_509(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_read_diff_pulse (uvd.DiffPulse[PULSE_RK1], field);
    asterix_250_read_diff_pulse (uvd.DiffPulse[PULSE_RK2], field);
    asterix_250_read_diff_pulse (uvd.DiffPulse[PULSE_RK3], field);
    asterix_250_read_diff_pulse (uvd.DiffPulse[PULSE_RK11], field);
    asterix_250_read_diff_pulse (uvd.DiffPulse[PULSE_RK12], field);
    asterix_250_read_diff_pulse (uvd.DiffPulse[PULSE_RK13], field);
}

void asterix_250_write_509(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_write_diff_pulse (uvd.DiffPulse[PULSE_RK1], field);
    asterix_250_write_diff_pulse (uvd.DiffPulse[PULSE_RK2], field);
    asterix_250_write_diff_pulse (uvd.DiffPulse[PULSE_RK3], field);
    asterix_250_write_diff_pulse (uvd.DiffPulse[PULSE_RK11], field);
    asterix_250_write_diff_pulse (uvd.DiffPulse[PULSE_RK12], field);
    asterix_250_write_diff_pulse (uvd.DiffPulse[PULSE_RK13], field);

}

void asterix_250_read_510(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_read_samples(field, uvd.SumSamples);
}

void asterix_250_write_510(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_write_samples(field, uvd.SumSamples);
}

void asterix_250_read_511(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_read_samples(field, uvd.DiffSamples);
}

void asterix_250_write_511(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_write_samples(field, uvd.DiffSamples);
}

void asterix_250_read_512(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_read_samples(field, uvd.PhaseSamples);
}

void asterix_250_write_512(RDFUvdDetections & uvd, unsigned char *&field)
{
    asterix_250_write_samples(field, uvd.PhaseSamples);
}


