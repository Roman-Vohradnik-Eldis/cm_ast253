#include "RDFAsterix.hh"
#include "RDFAsterix048Private.hh"
#include "RDFAsterix250Private.hh"
#include "CMSDebug.hh"

#include <sstream>

#define FSPEC_FX 0x01

#define FSPEC01_I250_010 0x80
#define FSPEC02_I250_011 0x40
#define FSPEC03_I250_012 0x20
#define FSPEC04_I250_100 0x10
#define FSPEC05_I250_200 0x08
#define FSPEC06_I250_300 0x04
#define FSPEC07_I250_400 0x02
#define FSPEC08_I250_500 0x80

// Composite Target Data
#define FSPEC01_I250_100_101 0x80
#define FSPEC02_I250_100_102 0x40
#define FSPEC03_I250_100_103 0x20
#define FSPEC04_I250_100_104 0x10
#define FSPEC05_I250_100_105 0x08
#define FSPEC06_I250_100_106 0x04
#define FSPEC07_I250_100_107 0x02

#define FSPEC08_I250_100_108 0x80
#define FSPEC09_I250_100_109 0x40
#define FSPEC10_I250_100_110 0x20
#define FSPEC11_I250_100_111 0x10
#define FSPEC12_I250_100_112 0x08
#define FSPEC13_I250_100_spa 0x04
#define FSPEC14_I250_100_spa 0x02

#define FSPEC15_I250_100_115 0x80
#define FSPEC16_I250_100_116 0x40
#define FSPEC17_I250_100_117 0x20
#define FSPEC18_I250_100_118 0x10
#define FSPEC19_I250_100_119 0x08
#define FSPEC20_I250_100_spa 0x04
#define FSPEC21_I250_100_121 0x02

#define FSPEC22_I250_100_122 0x80
#define FSPEC23_I250_100_123 0x40
#define FSPEC24_I250_100_124 0x20
#define FSPEC25_I250_100_125 0x10
#define FSPEC26_I250_100_126 0x08
#define FSPEC27_I250_100_127 0x04
#define FSPEC28_I250_100_128 0x02

#define FSPEC29_I250_100_129 0x80
#define FSPEC30_I250_100_130 0x40
#define FSPEC31_I250_100_131 0x20
#define FSPEC32_I250_100_132 0x10
#define FSPEC33_I250_100_133 0x08
#define FSPEC34_I250_100_134 0x04
#define FSPEC35_I250_100_spa 0x02

#define FSPEC36_I250_100_136 0x80
#define FSPEC37_I250_100_137 0x40
#define FSPEC38_I250_100_138 0x20
#define FSPEC39_I250_100_139 0x10
#define FSPEC40_I250_100_spa 0x08
#define FSPEC41_I250_100_spa 0x04
#define FSPEC42_I250_100_spa 0x02
#define FSPEC43_I250_100_spa 0x01

// SIF Detections
#define FSPEC01_I250_200_201 0x80
#define FSPEC02_I250_200_202 0x40
#define FSPEC03_I250_200_203 0x20
#define FSPEC04_I250_200_204 0x10
#define FSPEC05_I250_200_205 0x08
#define FSPEC06_I250_200_206 0x04
#define FSPEC07_I250_200_207 0x02

#define FSPEC08_I250_200_208 0x80
#define FSPEC09_I250_200_209 0x40
#define FSPEC10_I250_200_210 0x20
#define FSPEC11_I250_200_211 0x10
#define FSPEC12_I250_200_212 0x08
#define FSPEC13_I250_200_spa 0x04
#define FSPEC14_I250_200_spa 0x02

// Mode-S Detections
#define FSPEC01_I250_300_301 0x80
#define FSPEC02_I250_300_302 0x40
#define FSPEC03_I250_300_303 0x20
#define FSPEC04_I250_300_304 0x10
#define FSPEC05_I250_300_305 0x08
#define FSPEC06_I250_300_spa 0x04
#define FSPEC07_I250_300_spa 0x02

#define FSPEC08_I250_300_308 0x80
#define FSPEC09_I250_300_309 0x40
#define FSPEC10_I250_300_310 0x20
#define FSPEC11_I250_300_311 0x10
#define FSPEC12_I250_300_312 0x08
#define FSPEC13_I250_300_313 0x04
#define FSPEC14_I250_300_314 0x02

#define FSPEC15_I250_300_315 0x80
#define FSPEC16_I250_300_316 0x40
#define FSPEC17_I250_300_317 0x20
#define FSPEC18_I250_300_318 0x10
#define FSPEC19_I250_300_319 0x08
#define FSPEC20_I250_300_320 0x04
#define FSPEC21_I250_300_321 0x02
#define FSPEC22_I250_300_spa 0x01

//UVD
#define FSPEC01_I250_500_001 0x80
#define FSPEC02_I250_500_002 0x40
#define FSPEC03_I250_500_003 0x20
#define FSPEC04_I250_500_004 0x10
#define FSPEC05_I250_500_005 0x08
#define FSPEC06_I250_500_006 0x04
#define FSPEC07_I250_500_007 0x02

#define FSPEC08_I250_500_008 0x80
#define FSPEC09_I250_500_009 0x40
#define FSPEC10_I250_500_010 0x20
#define FSPEC11_I250_500_011 0x10
#define FSPEC12_I250_500_012 0x08



//pro strukturovany vypis do bufferu
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


/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                       Asterix250_to_Target                                */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/

//
// Composite Target Data
bool Asterix250_read_100 (RDFTargetTrack & target, unsigned char *msg, int len,
	std::stringstream * out_str = NULL, bool one_line = false)
{
	unsigned char *fspec;
	unsigned char *field;
	// najdeme zacatek datovych poli
	fspec = field = msg;
	while (*field & FSPEC_FX)
	{
		if (++field >= msg + len)
		{
			cms_error ("end of message");
			goto error;
		}
	}

	field++;

	// Target Report Description
	if (*fspec & FSPEC01_I250_100_101)
	{
		if (out_str)
			asterix_048_tobuffer_020 (field, out_str, one_line, "101");

		if (!asterix_048_read_020 (target, field))
		{
			goto error;
		}
	}

	// Measured Position in Slant Polar Coordinates
	if (*fspec & FSPEC02_I250_100_102)
	{
		if (out_str)
			asterix_048_tobuffer_040 (field, out_str, one_line, "102");

		asterix_048_read_040 (target, field);
	}

	// Mode-3/A Code in Octal Representation
	if (*fspec & FSPEC03_I250_100_103)
	{
		if (out_str)
			asterix_048_tobuffer_070 (field, out_str, one_line, "103");

		asterix_048_read_070 (target, field);
	}

	// Flight Level in Binary Representation
	if (*fspec & FSPEC04_I250_100_104)
	{
		if (out_str)
			asterix_048_tobuffer_090 (field, out_str, one_line, "104");

		asterix_048_read_090 (target, field);
	}

	// Radar Plot Characteristics
	if (*fspec & FSPEC05_I250_100_105)
	{
		if (out_str)
			asterix_048_tobuffer_130 (field, out_str, one_line, "105");

		if (!asterix_048_read_130 (target, field))
		{
			goto error;
		}
	}

	// Warning/Error Conditions
	if (*fspec & FSPEC06_I250_100_106)
	{
		if (out_str)
			asterix_048_tobuffer_030 (field, out_str, one_line, "106");

		asterix_048_read_030 (target, field);
	}

	// Radial Doppler Speed
	if (*fspec & FSPEC07_I250_100_107)
	{
		if (out_str)
			asterix_048_tobuffer_120 (field, out_str, one_line, "107");

		if (!asterix_048_read_120 (target, field))
		{
			goto error;
		}
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	// Aircraft Address
	if (*fspec & FSPEC08_I250_100_108)
	{
		if (out_str)
			asterix_048_tobuffer_220 (field, out_str, one_line, "108");

		asterix_048_read_220 (target, field);
	}

	// Communications / ACAS Capability and Flight Status
	if (*fspec & FSPEC09_I250_100_109)
	{
		if (out_str)
			asterix_048_tobuffer_230 (field, out_str, one_line, "109");

		asterix_048_read_230 (target, field);
	}

	// Aircraft Identification
	if (*fspec & FSPEC10_I250_100_110)
	{
		if (out_str)
			asterix_048_tobuffer_240 (field, out_str, one_line, "110");
		asterix_048_read_240 (target, field);
	}

	// Mode S MB Data with Age
	if (*fspec & FSPEC11_I250_100_111)
	{
		if (out_str)
			asterix_048_tobuffer_250_with_age (field, out_str, one_line, "111");

		asterix_048_read_250_with_age (target, field);
	}

	// ACAS Resolution Advisory Report
	if (*fspec & FSPEC12_I250_100_112)
	{
		if (out_str)
			asterix_048_tobuffer_260 (field, out_str, one_line, "112");

		asterix_048_read_260 (target, field);
	}

	if (*fspec & FSPEC13_I250_100_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	if (*fspec & FSPEC14_I250_100_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	// Track Number
	if (*fspec & FSPEC15_I250_100_115)
	{
		if (out_str)
			asterix_048_tobuffer_161 (field, out_str, one_line, "115");

		asterix_048_read_161 (target, field);
	}

	// Track Status
	if (*fspec & FSPEC16_I250_100_116)
	{
		if (out_str)
			asterix_048_tobuffer_170 (field, out_str, one_line, "116");

		asterix_048_read_170 (target, field);
	}

	// Calculated Position in Cartesian Coordinates
	if (*fspec & FSPEC17_I250_100_117)
	{
		if (out_str)
			asterix_048_tobuffer_042 (field, out_str, one_line, "117");

		asterix_048_read_042 (target, field);
	}

	// Calculated Track Velocity in Polar Representation
	if (*fspec & FSPEC18_I250_100_118)
	{
		if (out_str)
			asterix_048_tobuffer_200 (field, out_str, one_line, "118");

		asterix_048_read_200 (target, field);
	}

	// Track Quality
	if (*fspec & FSPEC19_I250_100_119)
	{
		if (out_str)
			asterix_048_tobuffer_210 (field, out_str, one_line, "119");

		asterix_048_read_210 (target, field);
	}

	if (*fspec & FSPEC20_I250_100_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	// Height Measured by 3D Radar
	if (*fspec & FSPEC21_I250_100_121)
	{
		if (out_str)
			asterix_048_tobuffer_110 (field, out_str, one_line, "121");

		asterix_048_read_110 (target, field);
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	// Mode-1 Code in Octal Representation
	if (*fspec & FSPEC22_I250_100_122)
	{
		if (out_str)
			asterix_048_tobuffer_055 (field, out_str, one_line, "122");

		asterix_048_read_055 (target, field);
	}

	// Mode-2 Code in Octal Representation
	if (*fspec & FSPEC23_I250_100_123)
	{
		if (out_str)
			asterix_048_tobuffer_050 (field, out_str, one_line, "123");

		asterix_048_read_050 (target, field);
	}

	//  Mode-3/A Code Confidence Indicator
	if (*fspec & FSPEC24_I250_100_124)
	{
		if (out_str)
			asterix_048_tobuffer_080 (field, out_str, one_line, "124");

		if(!target.Mode3A) {
			cms_warning ("Mode3A is not present");
			field += 2;
		} else {
			asterix_048_read_080 (target, field);
		}
	}

	// Mode-C Code and Confidence Indicator
	if (*fspec & FSPEC25_I250_100_125)
	{
		if (out_str)
			asterix_048_tobuffer_100 (field, out_str, one_line, "125");

		asterix_048_read_100 (target, field);
	}

	// Mode-1 Code Confidence Indicator
	if (*fspec & FSPEC26_I250_100_126)
	{
		if (out_str)
			asterix_048_tobuffer_065 (field, out_str, one_line, "126");

		asterix_048_read_065 (target, field);
	}

	// Mode-2 Code Confidence Indicator
	if (*fspec & FSPEC27_I250_100_127)
	{
		if (out_str)
			asterix_048_tobuffer_060 (field, out_str, one_line, "127");

		asterix_048_read_060 (target, field);
	}

	// User Text
	if (*fspec & FSPEC28_I250_100_128)
	{
		if (out_str)
			asterix_048_tobuffer_user_text (field, out_str, one_line);

		asterix_048_read_user_text (target, field);
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	// Calculated Rate of Climb Descent
	if (*fspec & FSPEC29_I250_100_129)
	{
		if (out_str)
			asterix_048_tobuffer_rate_of_climb (field, out_str, one_line, "129");

		asterix_048_read_rate_of_climb (target, field);
	}

	// Calculated Acceleration
	if (*fspec & FSPEC30_I250_100_130)
	{
		if (out_str)
			asterix_048_tobuffer_acceleration (field, out_str, one_line, "130");

		asterix_048_read_acceleration (target, field);
	}

	// Mode of Movement
	if (*fspec & FSPEC31_I250_100_131)
	{
		if (out_str)
			asterix_048_tobuffer_mode_of_movement (field, out_str, one_line, "131");

		asterix_048_read_mode_of_movement (target, field);
	}

	// Target Composition
	if (*fspec & FSPEC32_I250_100_132)
	{
		if (out_str)
			asterix_048_tobuffer_target_composition (field, out_str, one_line, "132");

		asterix_048_read_target_composition (target, field);
	}

	// Target Detections
	if (*fspec & FSPEC33_I250_100_133)
	{

		if (out_str)
			asterix_048_tobuffer_target_detections (field, out_str, one_line, "133");

		asterix_048_read_target_detections (target, field);
	}
	// Transponder Capability (same I017/230
	if (*fspec & FSPEC34_I250_100_134)
	{
		if (out_str)
			asterix_048_tobuffer_transponder_capability (field, out_str, one_line,
				"134");

		asterix_048_read_transponder_capability (target, field);
	}

	if (*fspec & FSPEC35_I250_100_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

    fspec++;

    // UVD IFF
	if (*fspec & FSPEC36_I250_100_136)
	{
        if (out_str)
        {
			asterix_250_tobuffer_136(*out_str, field, one_line);
        }
		asterix_250_read_136 (target, field);

	}
    // UVD ID
	if (*fspec & FSPEC37_I250_100_137)
	{
        if (out_str)
        {
			asterix_250_tobuffer_137(*out_str, field, one_line);
        }
		asterix_250_read_137 (target, field);

	}
    // UVD ALT
	if (*fspec & FSPEC38_I250_100_138)
	{
        if (out_str)
        {
			asterix_250_tobuffer_138(*out_str, field, one_line);
        }
		asterix_250_read_138 (target, field);
	}
    // UVD SPEED
	if (*fspec & FSPEC39_I250_100_139)
	{
        if (out_str)
        {
			asterix_250_tobuffer_139(*out_str, field, one_line);
        }
		asterix_250_read_139 (target, field);
	}

    for(uint8_t i = 0; i < 3; i++)
    {
        if(*fspec & (1 << i))
        {
 		    cms_error ("unable to handle spare bit(%d)", (1 << i));
    		goto error;
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
	if (field != msg + len)
	{
		cms_error ("wrong field 100 length: %ld != %ld + %d",
			(long) field, (long) msg, len);

		goto error;
	}
	return true;

error:
	cms_data (len, msg);
	return false;
}

// SIF Detections
bool Asterix250_read_sif (RDFSifDetections & sif, unsigned char *&field,
	std::stringstream * out_str = NULL, bool one_line = false)
{
	unsigned char *fspec;

	// najdeme zacatek datovych poli
	fspec = field;
	while (*field & FSPEC_FX)
	{
		field++;
	}

	field++;
	if (out_str)
		*out_str << "\n";

	// Time Of Day
	if (*fspec & FSPEC01_I250_200_201)
	{
		if (out_str)
		{
			asterix_250_tobuffer_201 (*out_str, field, one_line);
		}
		asterix_250_read_201 (sif, field);
	}

	// Reply Report Descriptor
	if (*fspec & FSPEC02_I250_200_202)
	{
		if (out_str)
		{
			asterix_250_tobuffer_202 (*out_str, field, one_line);
		}
		asterix_250_read_202 (sif, field);
	}

	// Interrogation Mode
	if (*fspec & FSPEC03_I250_200_203)
	{
		if (out_str)
		{
			asterix_250_tobuffer_203 (*out_str, field, one_line);
		}
		asterix_250_read_203 (sif, field);
	}

	// Sweep Number
	if (*fspec & FSPEC04_I250_200_204)
	{
		if (out_str)
		{
			asterix_250_tobuffer_204 (*out_str, field, one_line);
		}
		asterix_250_read_204 (sif, field);
	}

	// Code
	if (*fspec & FSPEC05_I250_200_205)
	{
		if (out_str)
		{
			asterix_250_tobuffer_205 (*out_str, field, one_line);
		}
		asterix_250_read_205 (sif, field);
	}

	// Ref Amplitude
	if (*fspec & FSPEC06_I250_200_206)
	{
		if (out_str)
		{
			asterix_250_tobuffer_206 (*out_str, field, one_line);
		}
		asterix_250_read_206 (sif, field);
	}

	// Measured Position in Slant Range Coordinates
	if (*fspec & FSPEC07_I250_200_207)
	{
		if (out_str)
		{
			asterix_250_tobuffer_207 (*out_str, field, one_line);
		}
		asterix_250_read_207 (sif, field);
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;
	{
		std::vector < std::string > out[2];

		// SUM Pulse
		if (*fspec & FSPEC08_I250_200_208)
		{
			if (out_str)
			{
				asterix_250_to_vect_208 (out[0], field, one_line);
			}

			asterix_250_read_208 (sif, field);
		}

		// DIFF Pulse
		if (*fspec & FSPEC09_I250_200_209)
		{
			if (out_str)
			{
				asterix_250_to_vect_209 (out[1], field, one_line);
			}

			asterix_250_read_209 (sif, field);
		}

		if ((*fspec & FSPEC09_I250_200_209) && (*fspec & FSPEC08_I250_200_208))
		{
			if (out_str)
			{
				asterix_250_to_buffer_208_209 (*out_str, out, one_line);
			}
		}
	}
	// Sample SUM
	if (*fspec & FSPEC10_I250_200_210)
	{
		if (out_str)
		{
			asterix_250_tobuffer_210 (*out_str, field, one_line);
		}

		asterix_250_read_210 (sif, field);
	}

	// Sample DIFF
	if (*fspec & FSPEC11_I250_200_211)
	{
		if (out_str)
		{
			asterix_250_tobuffer_211 (*out_str, field, one_line);
		}

		asterix_250_read_211 (sif, field);

	}

	// Sample Phase
	if (*fspec & FSPEC12_I250_200_212)
	{
		if (out_str)
		{
			asterix_250_tobuffer_212 (*out_str, field, one_line);
		}

		asterix_250_read_212 (sif, field);

	}

	if (*fspec & FSPEC13_I250_200_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	if (*fspec & FSPEC14_I250_200_spa)
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
	return true;

error:
	return false;
}

bool Asterix250_read_200 (RDFTargetTrack & target, unsigned char *msg, int len,
	std::stringstream * out_str, bool one_line)
{
	unsigned rep = msg[0];
	unsigned char *field = msg + 1;

	for (unsigned i = 0; i < rep; i++)
	{
		RDFSifDetections sif;

		if (!Asterix250_read_sif (sif, field, out_str, one_line))
		{
			cms_error ("Asterix250_read_sif error");
			goto error;
		}

		target.SifDetections.push_back (sif);
	}

	if (field != msg + len)
	{
		cms_error ("wrong field 200 length: %ld != %ld + %d",
			(long) field, (long) msg, len);
		goto error;
	}
	return true;

error:
	cms_data (len, msg);
	return false;
}

// Mode-S Detections
bool Asterix250_read_modes (RDFModeSDetections & modes, unsigned char *&field,
	std::stringstream * out_str = NULL, bool one_line = false)
{
	unsigned char *fspec;

	// najdeme zacatek datovych poli
	fspec = field;
	while (*field & FSPEC_FX)
	{
		field++;
	}

	field++;

	// Time Of Day
	if (*fspec & FSPEC01_I250_300_301)
	{
		if (out_str)
			asterix_250_tobuffer_301 (*out_str, field, one_line);

		asterix_250_read_301 (modes, field);
	}

	// Report Descriptor
	if (*fspec & FSPEC02_I250_300_302)
	{
		if (out_str)
			asterix_250_tobuffer_302 (*out_str, field, one_line);

		asterix_250_read_302 (modes, field);
	}

	// DF Raw
	if (*fspec & FSPEC03_I250_300_303)
	{
		if (out_str)
			asterix_250_tobuffer_303 (*out_str, field, one_line);

		asterix_250_read_303 (modes, field);
	}

	// DF Confidence
	if (*fspec & FSPEC04_I250_300_304)
	{
		if (out_str)
			asterix_250_tobuffer_304 (*out_str, field, one_line);

		asterix_250_read_304 (modes, field);
	}

	// DF Corrected
	if (*fspec & FSPEC05_I250_300_305)
	{
		if (out_str)
			asterix_250_tobuffer_305 (*out_str, field, one_line);

		asterix_250_read_305 (modes, field);
	}

	if (*fspec & FSPEC06_I250_300_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	if (*fspec & FSPEC07_I250_300_spa)
	{
		cms_error ("unable to handle spare bit");
		goto error;
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	// Measured Position in Slant Polar Coordinates
	if (*fspec & FSPEC08_I250_300_308)
	{
		if (out_str)
			asterix_250_tobuffer_308 (*out_str, field, one_line);

		asterix_250_read_308 (modes, field);
	}

	// Measured Amplitude
	if (*fspec & FSPEC09_I250_300_309)
	{
		if (out_str)
			asterix_250_tobuffer_309 (*out_str, field, one_line);

		asterix_250_read_309 (modes, field);
	}

	// Interrogation Mode
	if (*fspec & FSPEC10_I250_300_310)
	{
		if (out_str)
			asterix_250_tobuffer_310 (*out_str, field, one_line);

		asterix_250_read_310 (modes, field);
	}

	// Sweep Number
	if (*fspec & FSPEC11_I250_300_311)
	{
		if (out_str)
			asterix_250_tobuffer_311 (*out_str, field, one_line);


		asterix_250_read_311 (modes, field);
	}

	// Command Number
	if (*fspec & FSPEC12_I250_300_312)
	{
		if (out_str)
			asterix_250_tobuffer_312 (*out_str, field, one_line);

		asterix_250_read_312 (modes, field);
	}

	// Track Number
	if (*fspec & FSPEC13_I250_300_313)
	{
		if (out_str)
			asterix_250_tobuffer_313 (*out_str, field, one_line);

		asterix_250_read_313 (modes, field);
	}

	// BDS
	if (*fspec & FSPEC14_I250_300_314)
	{
		if (out_str)
			asterix_250_tobuffer_314 (*out_str, field, one_line);

		asterix_250_read_314 (modes, field);
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	// Uplink Data
	if (*fspec & FSPEC15_I250_300_315)
	{
		if (out_str)
			asterix_250_tobuffer_315 (*out_str, field, one_line);


		asterix_250_read_315 (modes, field);
	}

	// Datalink Control
	if (*fspec & FSPEC16_I250_300_316)
	{
		if (out_str)
			asterix_250_tobuffer_316 (*out_str, field, one_line);


		asterix_250_read_316 (modes, field);
	}

	//Sweep Length
	if (*fspec & FSPEC17_I250_300_317)
	{
		if (out_str)
			asterix_250_tobuffer_317 (*out_str, field, one_line);

		asterix_250_read_317 (modes, field);
	}

	//Interrogation Delay
	if (*fspec & FSPEC18_I250_300_318)
	{
		if (out_str)
			asterix_250_tobuffer_318 (*out_str, field, one_line);

		asterix_250_read_318 (modes, field);
	}

	// Sample SUM
	if (*fspec & FSPEC19_I250_300_319)
	{
		if (out_str)
			asterix_250_tobuffer_319 (*out_str, field, one_line);

		asterix_250_read_319 (modes, field);
	}

	// Sample DIFF
	if (*fspec & FSPEC20_I250_300_320)
	{
		if (out_str)
			asterix_250_tobuffer_320 (*out_str, field, one_line);

		asterix_250_read_320 (modes, field);
	}

	// Sample Phase
	if (*fspec & FSPEC21_I250_300_321)
	{
		if (out_str)
			asterix_250_tobuffer_321 (*out_str, field, one_line);

		asterix_250_read_321 (modes, field);
	}

	if (*fspec & FSPEC22_I250_300_spa)
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
	if (out_str)
		*out_str << "\n";
	return true;

error:
	return false;
}
bool Asterix250_read_uvd (RDFUvdDetections & uvd, unsigned char *&field,
	std::stringstream * out_str = NULL, bool one_line = false)
{
	unsigned char *fspec;

	// najdeme zacatek datovych poli
	fspec = field;
	while (*field & FSPEC_FX)
	{
		field++;
	}

	field++;

	if (*fspec & FSPEC01_I250_500_001)
	{
        if(out_str)
        {
    		asterix_250_tobuffer_501 (*out_str, field, one_line);

        }
		asterix_250_read_501 (uvd, field);
	}

	if (*fspec & FSPEC02_I250_500_002)
	{
	        if(out_str)
        {
    		asterix_250_tobuffer_502 (*out_str, field, one_line);

        }
        asterix_250_read_502 (uvd, field);
	}

	if (*fspec & FSPEC03_I250_500_003)
	{
        if(out_str)
        {
    		asterix_250_tobuffer_503 (*out_str, field, one_line);

        }
		asterix_250_read_503 (uvd, field);
	}

	if (*fspec & FSPEC04_I250_500_004)
	{
        if(out_str)
        {
    		asterix_250_tobuffer_504 (*out_str, field, one_line);

        }
		asterix_250_read_504 (uvd, field);

	}

	if (*fspec & FSPEC05_I250_500_005)
	{
        if(out_str)
        {
    		asterix_250_tobuffer_505 (*out_str, field, one_line);

        }
		//XXX nasledujici radek obsahuje chybu
		//XXX po porade s Alesem ji zde zatim nechavam
		asterix_250_read_504 (uvd, field);
	}

	if (*fspec & FSPEC06_I250_500_006)
	{
        if(out_str)
        {
    		asterix_250_tobuffer_506 (*out_str, field, one_line);

        }


		asterix_250_read_506 (uvd, field);
	}

	if (*fspec & FSPEC07_I250_500_007)
	{
        if(out_str)
        {
    		asterix_250_tobuffer_507 (*out_str, field, one_line);

        }

		asterix_250_read_507 (uvd, field);
	}

	if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	if (*fspec & FSPEC08_I250_500_008)
	{
		asterix_250_read_508 (uvd, field);
	}

	if (*fspec & FSPEC09_I250_500_009)
	{
		asterix_250_read_509 (uvd, field);
	}

	if (*fspec & FSPEC10_I250_500_010)
	{
		asterix_250_read_510 (uvd, field);
	}

	if (*fspec & FSPEC11_I250_500_011)
	{
		asterix_250_read_511 (uvd, field);
	}

	if (*fspec & FSPEC12_I250_500_012)
	{
		asterix_250_read_512 (uvd, field);
	}

    if (!(*fspec & FSPEC_FX))
	{
		goto end;
	}

	fspec++;

	cms_error ("unable to handle more fspec");
	goto error;

end:
	if (out_str)
		*out_str << "\n";
	return true;

error:
	return false;
}

bool Asterix250_read_300 (RDFTargetTrack & target, unsigned char *msg, int len,
	std::stringstream * out_str = NULL, bool one_line = false)
{
	unsigned rep = msg[0];
	unsigned char *field = msg + 1;

	for (unsigned i = 0; i < rep; i++)
	{
		if (out_str)
		{
			*out_str << "$$$ [modes_detection=" << i << " from=" << rep << "]";
			NEW_LINE (*out_str, '\0', '\0');
		}

		RDFModeSDetections modes;

		if (!Asterix250_read_modes (modes, field, out_str, one_line))
		{
			cms_error ("Asterix250_read_modes error");
			goto error;
		}

		target.ModeSDetections.push_back (modes);
	}

	if (field != msg + len)
	{
		cms_error ("wrong field 300 length: %ld != %ld + %d",
			(long) field, (long) msg, len);
		goto error;
	}
	return true;

error:
	if (out_str)
	{
		out_str->setstate (std::ios::badbit);
	}
	cms_data (len, msg);
	return false;
}

bool Asterix250_read_400 (RDFTargetTrack & target, unsigned char *msg, int len)
{
	// XXX
	return false;
}

bool Asterix250_read_500 (RDFTargetTrack & target, unsigned char *msg, int len,
	std::stringstream * out_str = NULL, bool one_line = false)
{
	unsigned rep = msg[0];
	unsigned char *field = msg + 1;

	for (unsigned i = 0; i < rep; i++)
	{
		if (out_str)
		{
			*out_str << "$$$ [uvd_detection=" << i << " from=" << rep << "]";
			NEW_LINE (*out_str, '\0', '\0');
		}

		RDFUvdDetections uvd;

		if (!Asterix250_read_uvd (uvd, field, out_str, one_line))
		{
			cms_error ("Asterix250_read_uvd error");
			goto error;
		}

		target.UvdDetections.push_back (uvd);
	}

	if (field != msg + len)
	{
		cms_error ("wrong field 500 length: %ld != %ld + %d",
			(long) field, (long) msg, len);
		goto error;
	}
	return true;

error:
	if (out_str)
	{
		out_str->setstate (std::ios::badbit);
	}
	cms_data (len, msg);
	return false;
}


bool Asterix250_to_Target (const void *data, int size,
	deque < RDFTargetTrack > &target_list, std::stringstream * out_str, bool one_line)
{
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	unsigned char *subfield;
	int subsize;
	int cat;
	int len;

	target_list.clear ();

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

	//out->str("");

	if (cat != 250)
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
		RDFTargetTrack target;
		target.CAT = 250;
		if (out_str)
		{
			NEW_LINE (*out_str, 0, '\0');
		}
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
		if (*fspec & FSPEC01_I250_010)
		{
			if (out_str)
				asterix_048_tobuffer_010 (field, out_str, one_line);
			asterix_048_read_010 (target, field);
		}

		// Time of Day
		if (*fspec & FSPEC02_I250_011)
		{
			if (out_str)
				asterix_048_tobuffer_140 (field, out_str, one_line, "011");

			asterix_048_read_140 (target, field);
		}

		// Message Type
		if (*fspec & FSPEC03_I250_012)
		{
			//XXX
			//cms_info ("message type: %d", field[0] & 1);
			//cms_info ("extractor report type: %d", (field[0] >> 1) & 3);
			field += 1;
		}

		// Composite Target Data
		if (*fspec & FSPEC04_I250_100)
		{
			subsize = field2unsigned (field, 16);
			subfield = field += 2;
			if (!Asterix250_read_100 (target, subfield, subsize, out_str, one_line))
			{
				cms_error ("Asterix250_read_100 failed");
				goto error;
			}
			field += subsize;
		}

		// SIF detections
		if (*fspec & FSPEC05_I250_200)
		{
			subsize = field2unsigned (field, 16);
			subfield = field += 2;
			if (!Asterix250_read_200 (target, subfield, subsize, out_str, one_line))
			{
				cms_error ("Asterix250_read_200 failed");
				goto error;
			}
			field += subsize;
		}

		// Mode-S detections
		if (*fspec & FSPEC06_I250_300)
		{
			subsize = field2unsigned (field, 16);
			subfield = field += 2;
			if (!Asterix250_read_300 (target, subfield, subsize, out_str, one_line))
			{
				cms_error ("Asterix250_read_300 failed");
				goto error;
			}
			field += subsize;
		}

		// ADS detections
		if (*fspec & FSPEC07_I250_400)
		{
			subsize = field2unsigned (field, 16);
			subfield = field += 2;
			if (!Asterix250_read_400 (target, subfield, subsize))
			{
				cms_error ("Asterix250_read_400 failed");
				goto error;
			}
			field += subsize;
		}

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;

        if(*fspec & FSPEC08_I250_500)
        {
 			subsize = field2unsigned (field, 16);
//            std::cerr << "subsize=" << subsize << std::endl;
			subfield = field += 2;
			if (!Asterix250_read_500 (target, subfield, subsize, out_str, one_line))
			{
				cms_error ("Asterix250_read_500 failed");
//               std::cerr << "Partial:" << out_str->str() << std::endl;
				goto error;
			}

			field += subsize;

        }

		if (!(*fspec & FSPEC_FX))
		{
			goto end;
		}

		fspec++;
//        *fspec &= ~FSPEC08_I250_500;
//
//        while(!(*fspec))
//        {
//            std::bitset<8> act_spec(*fspec);
//
//            const uint8_t fspec_count = act_spec.count();
//
//            for(uint8_t i = 1; i < fspec_count; i++)
//            {
//                const uint16_t field_size = field2unsigned(field, 16);
//                field += field_size + 2;
//            }
//
//            if(*fspec & FSPEC_FX)
//            {
//                fspec++;
//                continue;
//            }
//            else
//            {
//                goto end;
//            }
//        }

		cms_error ("unable to handle more fspec");
		goto error;

end:
		// pridame polozku do seznamu
		target_list.push_back (target);

		// presuneme se na dalsi zaznam
		record = field;
	}

	if (record > msg + len)
	{
		cms_error ("wrong packet length (%d > %d)", record - msg, size);
		goto error;
	}

	if (out_str)
		*out_str << "\n\n";

	return true;

error:
	cms_data (size, data);
	target_list.clear ();
	return false;
}

bool Asterix250_to_TargetTrack (const void *data, int size,
	deque < RDFTargetTrack > &track_list)
{
	deque < RDFTargetTrack > target_list;

	track_list.clear ();

	if (!Asterix250_to_Target (data, size, target_list))
	{
		return false;
	}

	while (!target_list.empty ())
	{
		if (target_list.front ().TrackNumber.Present)
		{
			track_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}

bool Asterix250_to_TargetPlot (const void *data, int size,
	deque < RDFTargetPlot > &plot_list)
{
	deque < RDFTargetTrack > target_list;

	plot_list.clear ();

	if (!Asterix250_to_Target (data, size, target_list))
	{
		return false;
	}

	while (!target_list.empty ())
	{
		if (!target_list.front ().TrackNumber.Present)
		{
			plot_list.push_back (target_list.front ());
		}
		target_list.pop_front ();
	}

	return true;
}

/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                       Target_to_Asterix250                                */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/

bool Asterix250_write_100 (RDFTargetTrack & target, unsigned char *&msg)
{
	unsigned char buffer[MSGMAX];
	unsigned char *fspec;
	unsigned char *field;
	unsigned size;

	memset (buffer, 0, sizeof (buffer));
	fspec = msg;
	field = buffer;

	// Target Report Description
	{
		*fspec |= FSPEC01_I250_100_101;
		asterix_048_write_020 (target, field);
	}

	// Measured Position in Slant Polar Coordinates
	if (target.Polar)
	{
		*fspec |= FSPEC02_I250_100_102;
		if (!asterix_048_write_040 (target, field))
		{
			return false;
		}
	}

	// Mode-3/A Code in Octal Representation
	if (target.Mode3A)
	{
		*fspec |= FSPEC03_I250_100_103;
		asterix_048_write_070 (target, field);
	}

	// Flight Level in Binary Representation
	if (target.FlightLevel)
	{
		*fspec |= FSPEC04_I250_100_104;
		asterix_048_write_090 (target, field);
	}

	// Radar Plot Characteristics
	if (target.TargetCharacteristics)
	{
		*fspec |= FSPEC05_I250_100_105;
		asterix_048_write_130 (target, field);
	}

	// Warning/Error Conditions
	if (target.WarningError.size ())
	{
		*fspec |= FSPEC06_I250_100_106;
		asterix_048_write_030 (target, field);
	}

	// Radial Doppler Speed
	if (target.DopplerSpeed.CalculatedPresent || target.DopplerSpeed.RawPresent)
	{
		*fspec |= FSPEC07_I250_100_107;
		asterix_048_write_120 (target, field);
	}

	*fspec |= FSPEC_FX;
	fspec++;

	// Aircraft Address
	if (target.AircraftAddress.Present)
	{
		*fspec |= FSPEC08_I250_100_108;
		asterix_048_write_220 (target, field);
	}

	// Communications / ACAS Capability and Flight Status
	if (target.CommCapabilityAndFlightStatus)
	{
		*fspec |= FSPEC09_I250_100_109;
		asterix_048_write_230 (target, field);
	}

	// Aircraft Identification
	if (target.AircraftIdentification.size ())
	{
		*fspec |= FSPEC10_I250_100_110;
		asterix_048_write_240 (target, field);
	}

	// Mode S MB Data with Age
	if (target.ModeSAge.size ())
	{
		*fspec |= FSPEC11_I250_100_111;
		asterix_048_write_250_with_age (target, field);
	}

	// ACAS Resolution Advisory Report
	if (target.ResolutionAdvisory.Present)
	{
		*fspec |= FSPEC12_I250_100_112;
		asterix_048_write_260 (target, field);
	}

	*fspec |= FSPEC_FX;
	fspec++;

	// Track Number
	if (target.TrackNumber.Present)
	{
		*fspec |= FSPEC15_I250_100_115;
		asterix_048_write_161 (target, field);
	}

	// Track Status
	if (target.TrackNumber.Present)
	{
		*fspec |= FSPEC16_I250_100_116;
		asterix_048_write_170 (target, field);
	}

	// Calculated Position in Cartesian Coordinates
	if (target.Cartesian)
	{
		*fspec |= FSPEC17_I250_100_117;
		if (!asterix_048_write_042 (target, field))
		{
			return false;
		}
	}

	// Calculated Track Velocity in Polar Representation
	if (target.GroundSpeed)
	{
		*fspec |= FSPEC18_I250_100_118;
		asterix_048_write_200 (target, field);
	}

	// Track Quality
	if (target.Accuracy)
	{
		*fspec |= FSPEC19_I250_100_119;
		asterix_048_write_210 (target, field);
	}

	// Height Measured by 3D Radar
	if (target.MeasuredHeight)
	{
		*fspec |= FSPEC21_I250_100_121;
		asterix_048_write_110 (target, field);
	}

	*fspec |= FSPEC_FX;
	fspec++;

	// Mode-1 Code in Octal Representation
	if (target.Mode1)
	{
		*fspec |= FSPEC22_I250_100_122;
		asterix_048_write_055 (target, field);
	}

	// Mode-2 Code in Octal Representation
	if (target.Mode2)
	{
		*fspec |= FSPEC23_I250_100_123;
		asterix_048_write_050 (target, field);
	}

	//  Mode-3/A Code Confidence Indicator
	if (target.Mode3A && target.Mode3A->ConfidencePresent)
	{
		*fspec |= FSPEC24_I250_100_124;
		asterix_048_write_080 (target, field);
	}

	// Mode-C Code and Confidence Indicator
	if (target.ModeC)
	{
		*fspec |= FSPEC25_I250_100_125;
		asterix_048_write_100 (target, field);
	}

	// Mode-1 Code Confidence Indicator
	if (target.Mode1 && target.Mode1->ConfidencePresent)
	{
		*fspec |= FSPEC26_I250_100_126;
		asterix_048_write_065 (target, field);
	}

	// Mode-2 Code Confidence Indicator
	if (target.Mode2 && target.Mode2->ConfidencePresent)
	{
		*fspec |= FSPEC27_I250_100_127;
		asterix_048_write_060 (target, field);
	}

	// User Text
	if (target.UserText.size ())
	{
		*fspec |= FSPEC28_I250_100_128;
		asterix_048_write_user_text (target, field);
	}

	*fspec |= FSPEC_FX;
	fspec++;

	// Calculated Rate of Climb Descent
	if (target.ClimbSpeed)
	{
		*fspec |= FSPEC29_I250_100_129;
		asterix_048_write_rate_of_climb (target, field);
	}

	// Calculated Acceleration
	if (target.Acceleration)
	{
		*fspec |= FSPEC30_I250_100_130;
		asterix_048_write_acceleration (target, field);
	}

	// Mode of Movement
	if (target.TransversalAcceleration != TRANS_ACCEL_UNDETERMINED
		|| target.LongitudinalAcceleration != LONG_ACCEL_UNDETERMINED
		|| target.TrackClimb != TRACK_CLIMB_UNKNOWN)
	{
		*fspec |= FSPEC31_I250_100_131;
		asterix_048_write_mode_of_movement (target, field);
	}

	// Target Composition
	if (target.TargetComposition.Mode3A != CODE_SOURCE_NONE
		|| target.TargetComposition.ModeC != CODE_SOURCE_NONE
		|| target.TargetComposition.Position != POSITION_SOURCE_NONE
		|| target.TargetComposition.Detection != DETECTION_SOURCE_NONE)
	{
		*fspec |= FSPEC32_I250_100_132;
		asterix_048_write_target_composition (target, field);
	}

	// Target Detections
	if (target.TargetDetections.SifMode3APresent
		|| target.TargetDetections.SifModeCPresent
		|| target.TargetDetections.AllCallPresent
		|| target.TargetDetections.RollCallMode3APresent
		|| target.TargetDetections.RollCallModeCPresent
		|| target.TargetDetections.AdsPresent)
	{
		*fspec |= FSPEC33_I250_100_133;
		asterix_048_write_target_detections (target, field);
	}

	// Transponder Capability (same I017/230)
	if (target.TransponderCapabilityPresent)
	{
		*fspec |= FSPEC34_I250_100_134;
		asterix_048_write_transponder_capability (target, field);
	}

    *fspec |= FSPEC_FX;
    fspec++;

    // UVD IFF
    if(target.UvdFriendStatus.Present)
	{
		*fspec |= FSPEC36_I250_100_136;
		asterix_250_write_136 (target, field);
	}

    // UVD ID
    if(target.UvdCode.Present)
	{
		*fspec |= FSPEC37_I250_100_137;
		asterix_250_write_137 (target, field);
	}

    // UVD ALT
    if(target.UvdAltitude.Present)
	{
		*fspec |= FSPEC38_I250_100_138;
		asterix_250_write_138 (target, field);
	}
    // UVD SPEED
    if(target.UvdSpeed.Present)
	{
		*fspec |= FSPEC39_I250_100_139;
		asterix_250_write_139 (target, field);
	}

	*fspec |= FSPEC_FX;
	fspec++;

	// vymazeme nevyuzite polozky fspec
	while (!(*fspec & ~FSPEC_FX))
	{
		fspec--;
	}

	*fspec &= ~FSPEC_FX;
	fspec++;

	size = (long) field - (long) buffer;
	memcpy (fspec, buffer, size);
	msg = fspec + size;
	return true;
}

// SIF Detections
void Asterix250_write_sif (RDFSifDetections & sif, unsigned char *&msg)
{
	unsigned char buffer[MSGMAX];
	unsigned char *fspec;
	unsigned char *field;
	unsigned size;

	memset (buffer, 0, sizeof (buffer));
	fspec = msg;
	field = buffer;

	// Time Of Day
	if (sif.Time.Present)
	{
		*fspec |= FSPEC01_I250_200_201;
		asterix_250_write_201 (sif, field);
	}

	// Reply Report Descriptor
	if (true)
	{
		*fspec |= FSPEC02_I250_200_202;
		asterix_250_write_202 (sif, field);
	}

	// Interrogation Mode
	if (true)
	{
		*fspec |= FSPEC03_I250_200_203;
		asterix_250_write_203 (sif, field);
	}

	// Sweep Number
	if (true)
	{
		*fspec |= FSPEC04_I250_200_204;
		asterix_250_write_204 (sif, field);
	}

	// Code
	if (true)
	{
		*fspec |= FSPEC05_I250_200_205;
		asterix_250_write_205 (sif, field);
	}

	// Ref Amplitude
	if (true)
	{
		*fspec |= FSPEC06_I250_200_206;
		asterix_250_write_206 (sif, field);
	}

	// Measured Position in Slant Range Coordinates
	if (true)
	{
		*fspec |= FSPEC07_I250_200_207;
		asterix_250_write_207 (sif, field);
	}

	// konec polozky fspec
	*fspec |= FSPEC_FX;
	fspec++;

	// SUM Pulse
	if (true)
	{
		*fspec |= FSPEC08_I250_200_208;
		asterix_250_write_208 (sif, field);
	}

	// DIFF Pulse
	if (true)
	{
		*fspec |= FSPEC09_I250_200_209;
		asterix_250_write_209 (sif, field);
	}

	// Sample SUM
	if (!sif.SumSamples.empty ())
	{
		*fspec |= FSPEC10_I250_200_210;
		asterix_250_write_210 (sif, field);

	}

	// Sample DIFF
	if (!sif.DiffSamples.empty ())
	{
		*fspec |= FSPEC11_I250_200_211;
		asterix_250_write_211 (sif, field);

	}

	// Sample Phase
	if (!sif.PhaseSamples.empty ())
	{
		*fspec |= FSPEC12_I250_200_212;
		asterix_250_write_212 (sif, field);

	}

	// vymazeme nevyuzite polozky fspec
	while (!(*fspec & ~FSPEC_FX))
	{
		fspec--;
	}

	*fspec &= ~FSPEC_FX;
	fspec++;

	size = (long) field - (long) buffer;
	memcpy (fspec, buffer, size);
	msg = fspec + size;
}

void Asterix250_write_200 (RDFTargetTrack & target, unsigned char *&field,
	const int64_t & max_size)
{
	field[0] = target.SifDetections.size ();
	field++;

	bool __may_throw = false;	// may i clear all samples
	int64_t __samples_size = 0;
	for (unsigned i = 0; i < target.SifDetections.size (); i++)
	{
		const RDFSifDetections & __det = target.SifDetections[i];
		__samples_size += 2 * (__det.SumSamples.size () + __det.DiffSamples.size ());
		__samples_size += 2 * __det.PhaseSamples.size ();

		if (__samples_size > max_size)
		{
			__may_throw = true;
			break;
		}
	}

	for (unsigned i = 0; i < target.SifDetections.size (); i++)
	{
		if (__may_throw)
		{
			target.SifDetections[i].SumSamples.clear ();
			target.SifDetections[i].PhaseSamples.clear ();
			target.SifDetections[i].DiffSamples.clear ();
		}

		Asterix250_write_sif (target.SifDetections[i], field);
	}
}

// Mode-S Detections
void Asterix250_write_modes (RDFModeSDetections & modes, unsigned char *&msg)
{
	unsigned char buffer[MSGMAX];
	unsigned char *fspec;
	unsigned char *field;
	unsigned size;

	memset (buffer, 0, sizeof (buffer));
	fspec = msg;
	field = buffer;

	// Time Of Day
	if (modes.Time.Present)
	{
		*fspec |= FSPEC01_I250_300_301;
		asterix_250_write_301 (modes, field);
	}

	// Report Descriptor
	if (true)
	{
		*fspec |= FSPEC02_I250_300_302;
		asterix_250_write_302 (modes, field);
	}

	// DF Raw
	if (true)
	{
		*fspec |= FSPEC03_I250_300_303;
		asterix_250_write_303 (modes, field);
	}

	// DF Confidence
	if (true)
	{
		*fspec |= FSPEC04_I250_300_304;
		asterix_250_write_304 (modes, field);
	}

	// DF Corrected
	if (true)
	{
		*fspec |= FSPEC05_I250_300_305;
		asterix_250_write_305 (modes, field);
	}

	// konec polozky fspec
	*fspec |= FSPEC_FX;
	fspec++;

	// Measured Position in Slant Polar Coordinates
	if (true)
	{
		*fspec |= FSPEC08_I250_300_308;
		asterix_250_write_308 (modes, field);
	}

	// Measured Amplitude
	if (true)
	{
		*fspec |= FSPEC09_I250_300_309;
		asterix_250_write_309 (modes, field);
	}

	// Interrogation Mode
	if (true)
	{
		*fspec |= FSPEC10_I250_300_310;
		asterix_250_write_310 (modes, field);
	}

	// Sweep Number
	if (true)
	{
		*fspec |= FSPEC11_I250_300_311;
		asterix_250_write_311 (modes, field);
	}

	// Command Number
	if (true)
	{
		*fspec |= FSPEC12_I250_300_312;
		asterix_250_write_312 (modes, field);
	}

	// Track Number
	if (true)
	{
		*fspec |= FSPEC13_I250_300_313;
		asterix_250_write_313 (modes, field);
	}

	// BDS
	if (modes.ExpectedBDS.present)
	{
		*fspec |= FSPEC14_I250_300_314;
		asterix_250_write_314 (modes, field);
	}

	// konec polozky fspec
	*fspec |= FSPEC_FX;
	fspec++;

	// Uplink Data
	if (true)
	{
		*fspec |= FSPEC15_I250_300_315;
		asterix_250_write_315 (modes, field);
	}

	// Datalink Control
	if (modes.DatalinkControl.present)
	{
		*fspec |= FSPEC16_I250_300_316;

		asterix_250_write_316 (modes, field);
	}

	//SweepLength
	if (modes.SweepLengthPresent)
	{
		*fspec |= FSPEC17_I250_300_317;

		asterix_250_write_317 (modes, field);
	}

	if (modes.InterrogationDelayPresent)
	{
		*fspec |= FSPEC18_I250_300_318;

		asterix_250_write_318 (modes, field);
	}

	// Sample SUM
	if (!modes.SumSamples.empty ())
	{
		*fspec |= FSPEC19_I250_300_319;
		asterix_250_write_319 (modes, field);

	}

	// Sample DIFF
	if (!modes.DiffSamples.empty ())
	{
		*fspec |= FSPEC20_I250_300_320;
		asterix_250_write_320 (modes, field);
	}

	// Sample Phase
	if (!modes.PhaseSamples.empty ())
	{
		*fspec |= FSPEC21_I250_300_321;
		asterix_250_write_321 (modes, field);
	}

	// konec polozky fspec
	*fspec |= FSPEC_FX;
	fspec++;

	// vymazeme nevyuzite polozky fspec
	while (!(*fspec & ~FSPEC_FX))
	{
		fspec--;
	}

	*fspec &= ~FSPEC_FX;
	fspec++;

	size = (long) field - (long) buffer;
	memcpy (fspec, buffer, size);
	msg = fspec + size;
}

void Asterix250_write_300 (RDFTargetTrack & target, unsigned char *&field,
	const int64_t & max_size)
{

	field[0] = target.ModeSDetections.size ();
	field++;

	bool __may_throw = false;	// may i clear all samples
	int64_t __samples_size = 0;
	for (unsigned i = 0; i < target.ModeSDetections.size (); i++)
	{
		const RDFModeSDetections & __det = target.ModeSDetections[i];
		__samples_size += 2 * (__det.SumSamples.size () + __det.DiffSamples.size ());
		__samples_size += 2 * (__det.PhaseSamples.size ());

		if (__samples_size > max_size)
		{
			__may_throw = true;
			break;
		}
	}

	for (unsigned i = 0; i < target.ModeSDetections.size (); i++)
	{
		if (__may_throw)
		{
			target.ModeSDetections[i].SumSamples.clear ();
			target.ModeSDetections[i].PhaseSamples.clear ();
			target.ModeSDetections[i].DiffSamples.clear ();
		}

		Asterix250_write_modes (target.ModeSDetections[i], field);
	}
}

void Asterix250_write_uvd (RDFUvdDetections & uvd, unsigned char *&msg)
{
	unsigned char buffer[MSGMAX];
	unsigned char *fspec;
	unsigned char *field;
	unsigned size;

	memset (buffer, 0, sizeof (buffer));
	fspec = msg;
	field = buffer;

    if(true)
	{
		*fspec |= FSPEC01_I250_500_001;
		asterix_250_write_501 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC02_I250_500_002;
		asterix_250_write_502 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC03_I250_500_003;
		asterix_250_write_503 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC04_I250_500_004;
		asterix_250_write_504 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC05_I250_500_005;
		//XXX nasledujici radek obsahuje chybu
		//XXX po porade s Alesem ji zde zatim nechavam
		asterix_250_write_504 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC06_I250_500_006;
		asterix_250_write_506 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC07_I250_500_007;
		asterix_250_write_507 (uvd, field);
	}

    *fspec |= FSPEC_FX;
	fspec++;

	if(true)
	{
		*fspec |= FSPEC08_I250_500_008;
		asterix_250_write_508 (uvd, field);
	}

	if(true)
	{
		*fspec |= FSPEC09_I250_500_009;
		asterix_250_write_509 (uvd, field);
	}

	if(uvd.SumSamples.size())
	{
		*fspec |= FSPEC10_I250_500_010;
		asterix_250_write_510 (uvd, field);
	}

	if(uvd.DiffSamples.size())
	{
		*fspec |= FSPEC11_I250_500_011;
		asterix_250_write_511 (uvd, field);
	}

	if(uvd.PhaseSamples.size())
	{
		*fspec |= FSPEC12_I250_500_012;
		asterix_250_write_512 (uvd, field);
	}

	*fspec |= FSPEC_FX;
	fspec++;

	// vymazeme nevyuzite polozky fspec
	while (!(*fspec & ~FSPEC_FX))
	{
		fspec--;
	}

	*fspec &= ~FSPEC_FX;
	fspec++;

	size = (long) field - (long) buffer;
	memcpy (fspec, buffer, size);
	msg = fspec + size;
}

void Asterix250_write_500 (RDFTargetTrack & target, unsigned char *&field)
{
    field[0] = target.UvdDetections.size();
    field++;

	for (unsigned i = 0; i < target.UvdDetections.size (); i++)
	{
		Asterix250_write_uvd (target.UvdDetections[i], field);
	}
}

int Target_to_Asterix250 (deque < RDFTargetTrack > &target_list,
	void *data, int maxsize)
{
	unsigned char buffer[MSGMAX];
	unsigned char *msg;
	unsigned char *record;
	unsigned char *fspec;
	unsigned char *field;
	unsigned char *subfield;
	int fspec_size;
	int field_size;
	int size;
	int subsize;
	int64_t free_space = 0;

	memset (data, 0, maxsize);
	msg = (unsigned char *) data;
	msg[0] = 250;				// kategorie
	msg[1] = 0;					// delka
	msg[2] = 0;

	// seznam je prazdny, neni co zpracovavat
	if (target_list.empty ())
	{
		return 0;
	}

	size = 3;
	while (target_list.size () > 0)
	{
		RDFTargetTrack & target = target_list.front ();
		memset (buffer, 0, sizeof (buffer));
		record = fspec = msg + size;
		field = buffer;
		*fspec = 0;

		// Data Source Identifier
		{
			*fspec |= FSPEC01_I250_010;
			asterix_048_write_010 (target, field);
		}

		// Time of Day
		if (target.Time.Present)
		{
			*fspec |= FSPEC02_I250_011;
			asterix_048_write_140 (target, field);
		}

		// Message Type
		{
			*fspec |= FSPEC03_I250_012;
			field[0] = 0;		//XXX
			field += 1;
		}

		// Composite Target Data
		if (true)
		{
			*fspec |= FSPEC04_I250_100;
			subfield = field + 2;
			if (!Asterix250_write_100 (target, subfield))
			{
				goto skip;
			}
			subsize = (long) subfield - (long) field - 2;
			field[0] = (subsize >> 8) & 0xff;
			field[1] = subsize & 0xff;
			field = subfield;
		}

		// SIF detections
		if (target.SifDetections.size ())
		{
			*fspec |= FSPEC05_I250_200;
			subfield = field + 2;
			free_space = maxsize - (subfield - buffer);
			Asterix250_write_200 (target, subfield, free_space);
			subsize = (long) subfield - (long) field - 2;
			field[0] = (subsize >> 8) & 0xff;
			field[1] = subsize & 0xff;
			field = subfield;
		}

		// Mode-S detections
		if (target.ModeSDetections.size ())
		{
			*fspec |= FSPEC06_I250_300;
			subfield = field + 2;
			free_space = maxsize - (subfield - buffer);
			Asterix250_write_300 (target, subfield, free_space);
			subsize = (long) subfield - (long) field - 2;
			field[0] = (subsize >> 8) & 0xff;
			field[1] = subsize & 0xff;
			field = subfield;
		}

		// ADS detections
		if (false)
		{
			*fspec |= FSPEC07_I250_400;
		}

        // UVD Detections
		if (target.UvdDetections.size ())
		{
            *fspec |= FSPEC_FX;
    		fspec++;
			*fspec |= FSPEC08_I250_500;
			subfield = field + 2;
			Asterix250_write_500 (target, subfield);
			subsize = (long) subfield - (long) field - 2;
			field[0] = (subsize >> 8) & 0xff;
			field[1] = subsize & 0xff;
			field = subfield;
		}

        fspec++;

		// zkontrolujeme, jestli se polozka vejde do zaznamu
		fspec_size = fspec - record;
		field_size = field - buffer;
		if (size + fspec_size + field_size > maxsize)
		{
			if (size == 3)
			{
				cms_error ("not enough space in output buffer");
				goto error;
			}
			break;
		}

		// zkopirujeme polozku do zaznamu
		memcpy (fspec, buffer, field_size);
		size += fspec_size + field_size;

skip:
		// odstranime target z fronty
		target_list.pop_front ();
	}

	if (size == 3)
	{
		return 0;
	}

	msg[1] = (size >> 8) & 0xff;
	msg[2] = size & 0xff;
	return size;

error:
	target_list.clear ();
	return 0;
}

int TargetTrack_to_Asterix250 (deque < RDFTargetTrack > &track_list,
	void *data, int maxsize)
{
	deque < RDFTargetTrack > tmp;

	while (!track_list.empty ())
	{
		if (track_list.front ().TrackNumber.Present)
		{
			tmp.push_back (track_list.front ());
		}
		track_list.pop_front ();
	}

	int send_size = Target_to_Asterix250 (tmp, data, maxsize);

	if (tmp.size ())
	{
		track_list.resize (tmp.size ());
		copy (tmp.begin (), tmp.end (), track_list.begin ());
	}

	return send_size;
}

int TargetPlot_to_Asterix250 (deque < RDFTargetPlot > &plot_list,
	void *data, int maxsize)
{
	deque < RDFTargetTrack > tmp;

	while (!plot_list.empty ())
	{
		tmp.push_back (plot_list.front ());
		plot_list.pop_front ();
	}

	int send_size = Target_to_Asterix250 (tmp, data, maxsize);

	if (tmp.size ())
	{
		plot_list.resize (tmp.size ());
		copy (tmp.begin (), tmp.end (), plot_list.begin ());
	}

	return send_size;
}
