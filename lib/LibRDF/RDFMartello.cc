#include "RDFMartello.hh"
#include "CMSDebug.hh"

void set_plot_siteid (RDFTargetPlot & plot, unsigned short code)
{
}

void set_plot_position (RDFTargetPlot & plot, unsigned short code1, unsigned short code2)
{
	plot.Polar = new RDFCoorPolar ();
	plot.Polar->Rho = (int)code1 * 1852 / 64;
	plot.Polar->Theta = (double)code2 * (2 * M_PI) / 0x10000;
}

void set_plot_height (RDFTargetPlot & plot, unsigned short code)
{
	// height
	plot.MeasuredHeight = new RDFTargetHeight ();
	plot.MeasuredHeight->Height = (int) rint ((double) (code & 0x0fff) * 100 * 0.3048);

	// height_quality
	plot.MeasuredHeight->QualityPresent = TRUE;
	plot.MeasuredHeight->Quality = (code >> 12) & 0x07;
}

void set_plot_emergency (RDFTargetPlot & plot, unsigned short code)
{
/*
	switch (code)
	{
		case 4:
		{
			plot.Emergency = EMERGENCY_TYPE_HIJACK;
			break;
		}

		case 5:
		{
			plot.Emergency = EMERGENCY_TYPE_RADIOCOM;
			break;
		}

		case 6:
		{
			plot.Emergency = EMERGENCY_TYPE_EMERGENCY;
			break;
		}
		
		case 7:
		{
			plot.Emergency = EMERGENCY_TYPE_SIF;
			break;
		}

		default:
		{
			plot.Emergency = EMERGENCY_TYPE_NONE;
		}
	}
*/
}

void set_plot_secure (RDFTargetPlot & plot, unsigned short code)
{
}

void set_mode_binary (RDFTargetMode * & mode, bool & spi, unsigned short code)
{
	mode = new RDFTargetMode ();
	mode->Code = code & 0x0fff;
	mode->Valid = (code >> 12) & 1;
	mode->Garbled = (code >> 13) & 1;
	// ((code >> 14) & 1) X pulse
	if ((code >> 15) & 1) spi = true;
}

void set_mode (RDFTargetMode * & mode, bool & spi, unsigned short code)
{
	mode = new RDFTargetMode ();
	mode->Code = code & 0x0fff;
	mode->Valid = (code >> 12) & 1;
	mode->Garbled = (code >> 13) & 1;
	// ((code >> 14) & 1) X pulse
	if ((code >> 15) & 1) spi = true;
}

void set_flight_level (RDFTargetFlightLevel * & fl, bool & spi, unsigned short code)
{
	fl = new RDFTargetFlightLevel ();
	fl->Height = (int) rint ((double) (code & 0x0fff) * 100 * 0.3048);
	fl->Valid = (code >> 12) & 1;
	fl->Garbled = (code >> 13) & 1;
	// ((code >> 14) & 1) X pulse
	if ((code >> 15) & 1) spi = true;
}

void set_plot_mode (RDFTargetPlot & plot, unsigned char mode, unsigned short code)
{
	bool spi = false;
	
	switch (mode)
	{
		case 1:
		{
			//INFO ("mode 1");
			set_mode_binary (plot.Mode1, spi, code);
			plot.SPI = spi;
			break;
		}

		case 2:
		{
			//INFO ("mode 2");
			set_mode_binary (plot.Mode2, spi, code);
			plot.SPI = spi;
			break;
		}

		case 3:
		{
			//INFO ("mode 3");
			set_mode_binary (plot.Mode3A, spi, code);
			plot.SPI = spi;
			break;
		}

		case 5:
		{
			//cms_info ("mode B");
			set_mode (plot.ModeB, spi, code);
			plot.SPI = spi;
			break;
		}

		case 6:
		{
			//INFO ("mode C");
			set_flight_level (plot.FlightLevel, spi, code);
			plot.SPI = spi;
			break;
		}

		case 7:
		{
			//cms_info ("mode D");
			set_mode (plot.ModeD, spi, code);
			plot.SPI = spi;
			break;
		}
	}
}

bool psr_plot (RDFTargetPlot & plot, unsigned short * &msg, int &size)
{
	plot.TargetType = TARGET_TYPE_PRIMARY;
	
	// label
	if ((msg[0] & 0x3f) != 0x08)
	{
		cms_error ("");
		return FALSE;
	}

	// rtqc
	// (msg[0] >> 7) & 1

	// siteid
	set_plot_siteid (plot, (msg[0] >> 8) & 0x0f);

	// range
	// azimuth
	set_plot_position (plot, msg[1], msg[2]);

	// height
	set_plot_height (plot, msg[3]);

	// unused
	// msg[4]
	
	msg += 5;
	size -= 5;
	return TRUE;
}

bool ssr_plot (RDFTargetPlot & plot, unsigned short * &msg, int &size)
{
	plot.TargetType = TARGET_TYPE_SECONDARY;

	// label
	if ((msg[0] & 0x3f) != 0x28)
	{
		cms_error ("");
		return FALSE;
	}

	// rtqc
	// (msg[0] >> 7) & 1

	// siteid
	set_plot_siteid (plot, (msg[0] >> 8) & 0x0f);

	// range
	// azimuth
	set_plot_position (plot, msg[1], msg[2]);

	// emerg
	set_plot_emergency (plot, (msg[3] >> 9) & 0x07);

	// secure mode
	set_plot_secure (plot, (msg[3] >> 12) & 0x0f);

	// p mode
	set_plot_mode (plot, (msg[3] >> 6) & 0x07, msg[4]);

	// s mode
	set_plot_mode (plot, (msg[3] >> 3) & 0x07, msg[5]);

	// t mode
	set_plot_mode (plot, msg[3] & 0x07, msg[6]);

	msg += 7;
	size -= 7;
	return TRUE;
}

bool psr_ssr_plot (RDFTargetPlot & plot, unsigned short * &msg, int &size)
{
	plot.TargetType = TARGET_TYPE_COMBINED;

	// label
	if ((msg[0] & 0x3f) != 0x29)
	{
		cms_error ("");
		return FALSE;
	}

	// rtqc
	// (msg[0] >> 7) & 1

	// siteid
	set_plot_siteid (plot, (msg[0] >> 8) & 0x0f);

	// range
	// azimuth
	set_plot_position (plot, msg[1], msg[2]);

	// height
	set_plot_height (plot, msg[3]);

	// unused 
	// msg[4]
	
	// emerg
	set_plot_emergency (plot, (msg[5] >> 9) & 0x07);

	// secure mode
	set_plot_secure (plot, (msg[5] >> 12) & 0x0f);

	// p mode
	set_plot_mode (plot, (msg[5] >> 6) & 0x07, msg[6]);

	// s mode
	set_plot_mode (plot, (msg[5] >> 3) & 0x07, msg[7]);

	// t mode
	set_plot_mode (plot, msg[5] & 0x07, msg[8]);

	msg += 9;
	size -= 9;
	return TRUE;
}

bool jamming_strobe (RDFTargetPlot & plot, unsigned short * &msg, int &size)
{
	return FALSE;
/*	
	plot.erase ():
	plot.TargetType = TARGET_TYPE_JAM_STROBE;
	
	// label
	if ((msg[0] & 0x3f) != 0x0a)
	{
		cms_error ("");
		return FALSE;
	}

	// rtqc
	// (msg[0] >> 7) & 1

	// siteid
	// (msg[0] >> 8) & 0x0f

	// azimuth
	// (double)msg[1] * (2 * M_PI) / 0x10000;

	// azimuth width
	// (double)msg[2] * M_PI_2 / 0x100

	msg += 6;
	size -= 6;

	return plot;
*/
}

bool jamming_sector (RDFTargetPlot & plot, unsigned short * &msg, int &size)
{
	return FALSE;
/*
	plot.erase ():
	plot.TargetType = TARGET_TYPE_JAM_STROBE;
	
	// label
	if ((msg[0] & 0x3f) != 0x0b)
	{
		cms_error ("");
		return FALSE;
	}

	// rtqc
	// (msg[0] >> 7) & 1

	// siteid
	// (msg[0] >> 8) & 0x0f

	// azimuth
	// (double)msg[1] * (2 * M_PI) / 0x10000;

	// azimuth width
	// (double)msg[2] * M_PI_2 / 0x100

	msg += 6;
	size -= 6;

	return plot;
*/
}

bool clutter_map (RDFTargetPlot & plot, unsigned short * &msg, int &size)
{
	return FALSE;
/*
	plot.erase ();
	plot.TargetType = TARGET_TYPE_JAM_STROBE;
	
	// label
	if ((msg[0] & 0x3f) != 0x0e)
	{
		cms_error ("");
		return FALSE;
	}

	// rtqc
	// (msg[0] >> 7) & 1

	// siteid
	// (msg[0] >> 8) & 0x0f

	// azimuth sector
	// (double)((msg[1] >> 8) & 0xff) * (2 * M_PI) / 0x100;

 	// range start
 	// (int) rint ((double) (msg[2] & 0xff) * 0x100 *  * 1852 / 64

 	// range end
 	// (int) rint ((double) (msg[2] & 0xff) * 0x100 *  * 1852 / 64

	msg += 3;
	size -= 3;

	return plot;
*/
}

bool azimuth_status (RDFRadarService & service, unsigned short * &msg, int &size)
{
	// label
	if ((msg[0] & 0x3f) != 0x22)
	{
		cms_error ("");
		return FALSE;
	}

	// siteid
	// (msg[0] >> 8) & 0x0f

	// azimuth sector
	service.Sector = (double)msg[1] * (2 * M_PI) / 0x10000;
	service.Type = RADAR_SERVICE_TYPE_SECTOR;
	
	// sm
	// msg[2] & 1

	// ssr
	// (msg[2] >> 1) & 1

	// psr
	// (msg[2] >> 2) & 1

	msg += 3;
	size -= 3;

	return TRUE;
}

bool Martello_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list, deque<RDFRadarService> & service_list)
{
	plot_list.clear ();
	service_list.clear ();
	
	unsigned short * msg = (unsigned short *)data;
	size /= 2;

	while (size > 0)
	{
		switch (msg[0] & 0x3f)
		{
			case 0x08:
			{
				RDFTargetPlot plot;
				if (!psr_plot (plot, msg, size)) return FALSE;
				plot_list.push_back (plot);
				break;
			}

			case 0x28:
			{
				RDFTargetPlot plot;
				if (!ssr_plot (plot, msg, size)) return FALSE;
				plot_list.push_back (plot);
				break;
			}

			case 0x29:
			{
				RDFTargetPlot plot;
				if (!psr_ssr_plot (plot, msg, size)) return FALSE;
				plot_list.push_back (plot);
				break;
			}

			case 0x0a:
			{
				RDFTargetPlot plot;
				if (!jamming_strobe (plot, msg, size)) return FALSE;
				break;
			}

			case 0xb:
			{
				RDFTargetPlot plot;
				if (!jamming_sector (plot, msg, size)) return FALSE;
				break;
			}

			case 0x0e:
			{
				RDFTargetPlot plot;
				if (!clutter_map (plot, msg, size)) return FALSE;
				break;
			}

			case 0x22:
			{
				RDFRadarService service;
				if (!azimuth_status (service, msg, size)) return FALSE;
				service_list.push_back (service);
				break;
			}
			
			default:
			{
				cms_warning ("unknown message %02x", msg[0] & 0x3f);
				return FALSE;
			}
		}
	}

	if (size < 0)
	{
		cms_warning ("wrong message size %d", size);
	}

	return TRUE;
}


int TargetPlot_to_Martello (RDFTargetPlot & plot, void * data, int maxsize)
{
	memset (data, 0, maxsize);
	unsigned short * msg = (unsigned short *)data;
	int size = 0;

	switch (plot.TargetType)
	{
		case TARGET_TYPE_ADSB: // chova se jako neznamy
		case TARGET_TYPE_UNKNOWN:
		{
			//INFO ("unknown plot");
			break;
		}
		case TARGET_TYPE_PRIMARY:
		{
			//INFO ("PSR plot");
			// label
			msg[0] = 0x0008;
			// rtqc
			msg[0] |= plot.Tested  ? 0x80 : 0;

			// siteid
			if (plot.SIC > 0 && plot.SIC < 3) msg[0] |= (plot.SIC << 8) & 0x0fff;			// ? ma byt 0001 nebo 0010 =mozna & 0x03ff
			else msg[0] |= 0x0100;

			// range
			// azimuth
			if (plot.Polar)
			{
				unsigned short rho = (int) rint (plot.Polar->Rho * 64 / 1852);
				unsigned short theta = (int) rint (plot.Polar->Theta * 0x10000 / (2 * M_PI));
				msg[1] = rho;
				msg[2] = theta;
			}

			// height
			if (plot.MeasuredHeight)
			{
				unsigned short height = (int) rint (plot.MeasuredHeight->Height / 0.3048 / 100);
				msg[3] = height & 0x0fff;

				if (plot.MeasuredHeight->QualityPresent)
				{
					msg[3] |= (plot.MeasuredHeight->Quality << 12) & 0x7fff;
				}
			}

			// unused
			msg[4] = 0x0000;

			size = 10;
			break;
		}
		case TARGET_TYPE_SECONDARY:
		case TARGET_TYPE_MODES_ALL_CALL:
		case TARGET_TYPE_MODES_ROLL_CALL:
		{
			//INFO ("SSR plot");
			// label
			msg[0] = 0x0028;
			// rtqc
			msg[0] |= plot.Tested  ? 0x80 : 0;

			// siteid
			if (plot.SIC > 0 && plot.SIC < 3) msg[0] |= (plot.SIC << 8) & 0x0fff;			// ? ma byt 0001 nebo 0010 =mozna & 0x03ff
			else msg[0] |= 0x0100;

			// range
			// azimuth
			if (plot.Polar)
			{
				unsigned short rho = (int) rint (plot.Polar->Rho * 64 / 1852);
				unsigned short theta = (int) rint (plot.Polar->Theta * 0x10000 / (2 * M_PI));
				msg[1] = rho;
				msg[2] = theta;
			}
			
			// emergency
			msg[3] = 0;
/*
 			switch (plot.Emergency)
			{
				case EMERGENCY_TYPE_HIJACK:
				{
					msg[3] = 0x0800;
					break;
				}

				case EMERGENCY_TYPE_RADIOCOM:
				{
					msg[3] = 0x0A00;
					break;
				}

				case EMERGENCY_TYPE_EMERGENCY:
				{
					msg[3] = 0x0C00;
					break;
				}

				default:					// EMERGENCY_NONE
				{
					msg[3] = 0x0000;
				}
			}
*/

			// primary, secondary a tertiary code
			int code = 1;
			if (plot.Mode3A)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[3] |= 0x00C0;
						msg[4] = plot.Mode3A->Code & 0x0fff;
						msg[4] |= plot.Mode3A->Valid ? 0x1000 : 0;
						msg[4] |= plot.Mode3A->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[3] |= 0x0018;
						msg[5] = plot.Mode3A->Code & 0x0fff;
						msg[5] |= plot.Mode3A->Valid ? 0x1000 : 0;
						msg[5] |= plot.Mode3A->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[3] |= 0x0003;
						msg[6] = plot.Mode3A->Code & 0x0fff;
						msg[6] |= plot.Mode3A->Valid ? 0x1000 : 0;
						msg[6] |= plot.Mode3A->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.FlightLevel)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[3] |= 0x0180;
						msg[4] = (int) rint (plot.FlightLevel->Height / 0.3048 / 100) & 0x0fff;
						msg[4] |= plot.FlightLevel->Valid ? 0x1000 : 0;
						msg[4] |= plot.FlightLevel->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[3] |= 0x0030;
						msg[5] = (int) rint (plot.FlightLevel->Height / 0.3048 / 100) & 0x0fff;
						msg[5] |= plot.FlightLevel->Valid ? 0x1000 : 0;
						msg[5] |= plot.FlightLevel->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[3] |= 0x0006;
						msg[6] = (int) rint (plot.FlightLevel->Height / 0.3048 / 100) & 0x0fff;
						msg[6] |= plot.FlightLevel->Valid ? 0x1000 : 0;
						msg[6] |= plot.FlightLevel->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.Mode2)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[3] |= 0x0080;
						msg[4] = plot.Mode2->Code & 0x0fff;
						msg[4] |= plot.Mode2->Valid ? 0x1000 : 0;
						msg[4] |= plot.Mode2->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[3] |= 0x0010;
						msg[5] = plot.Mode2->Code & 0x0fff;
						msg[5] |= plot.Mode2->Valid ? 0x1000 : 0;
						msg[5] |= plot.Mode2->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[3] |= 0x0002;
						msg[6] = plot.Mode2->Code & 0x0fff;
						msg[6] |= plot.Mode2->Valid ? 0x1000 : 0;
						msg[6] |= plot.Mode2->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.Mode1)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[3] |= 0x0040;
						msg[4] = plot.Mode1->Code & 0x0fff;
						msg[4] |= plot.Mode1->Valid ? 0x1000 : 0;
						msg[4] |= plot.Mode1->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[3] |= 0x0008;
						msg[5] = plot.Mode1->Code & 0x0fff;
						msg[5] |= plot.Mode1->Valid ? 0x1000 : 0;
						msg[5] |= plot.Mode1->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[3] |= 0x0001;
						msg[6] = plot.Mode1->Code & 0x0fff;
						msg[6] |= plot.Mode1->Valid ? 0x1000 : 0;
						msg[6] |= plot.Mode1->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.ModeB)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[3] |= 0x0140;
						msg[4] = plot.ModeB->Code & 0x0fff;
						msg[4] |= plot.ModeB->Valid ? 0x1000 : 0;
						msg[4] |= plot.ModeB->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[3] |= 0x0040;
						msg[5] = plot.ModeB->Code & 0x0fff;
						msg[5] |= plot.ModeB->Valid ? 0x1000 : 0;
						msg[5] |= plot.ModeB->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[3] |= 0x0005;
						msg[6] = plot.ModeB->Code & 0x0fff;
						msg[6] |= plot.ModeB->Valid ? 0x1000 : 0;
						msg[6] |= plot.ModeB->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.ModeD)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[3] |= 0x01C0;
						msg[4] = plot.ModeD->Code & 0x0fff;
						msg[4] |= plot.ModeD->Valid ? 0x1000 : 0;
						msg[4] |= plot.ModeD->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[3] |= 0x0038;
						msg[5] = plot.ModeD->Code & 0x0fff;
						msg[5] |= plot.ModeD->Valid ? 0x1000 : 0;
						msg[5] |= plot.ModeD->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[3] |= 0x0007;
						msg[6] = plot.ModeD->Code & 0x0fff;
						msg[6] |= plot.ModeD->Valid ? 0x1000 : 0;
						msg[6] |= plot.ModeD->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			
			size = 14;
			break;
		}
		case TARGET_TYPE_COMBINED:
		case TARGET_TYPE_MODES_ALL_CALL_PSR:
		case TARGET_TYPE_MODES_ROLL_CALL_PSR:
		{
			//INFO ("PSR SSR plot");
			// label
			msg[0] = 0x0029;
			// rtqc
			msg[0] |= plot.Tested  ? 0x80 : 0;

			// siteid
			if (plot.SIC > 0 && plot.SIC < 3) msg[0] |= (plot.SIC << 8) & 0x0fff;			// ? ma byt 0001 nebo 0010 =mozna & 0x03ff
			else msg[0] |= 0x0100;

			// range
			// azimuth
			if (plot.Polar)
			{
				unsigned short rho = (int) rint (plot.Polar->Rho * 64 / 1852);
				unsigned short theta = (int) rint (plot.Polar->Theta * 0x10000 / (2 * M_PI));
				msg[1] = rho;
				msg[2] = theta;
			}

			// height
			if (plot.MeasuredHeight)
			{
				unsigned short height = (int) rint (plot.MeasuredHeight->Height / 0.3048 / 100);
				msg[3] = height & 0x0fff;

				if (plot.MeasuredHeight->QualityPresent)
				{
					msg[3] |= (plot.MeasuredHeight->Quality << 12) & 0x7fff;
				}				
			}

			// unused
			msg[4] = 0x0000;

			// emergency
			msg[5] = 0;
/*
 			switch (plot.Emergency)
			{
				case EMERGENCY_TYPE_HIJACK:
				{
					msg[5] = 0x0800;
					break;
				}

				case EMERGENCY_TYPE_RADIOCOM:
				{
					msg[5] = 0x0A00;
					break;
				}

				case EMERGENCY_TYPE_EMERGENCY:
				{
					msg[5] = 0x0C00;
					break;
				}

				default:					// EMERGENCY_NONE
				{
					msg[5] = 0x0000;
				}
			}
*/
			// primary, secondary and tertiary code
			int code = 1;
			if (plot.Mode3A)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[5] |= 0x00C0;
						msg[6] = plot.Mode3A->Code & 0x0fff;
						msg[6] |= plot.Mode3A->Valid ? 0x1000 : 0;
						msg[6] |= plot.Mode3A->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[5] |= 0x0018;
						msg[7] = plot.Mode3A->Code & 0x0fff;
						msg[7] |= plot.Mode3A->Valid ? 0x1000 : 0;
						msg[7] |= plot.Mode3A->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[5] |= 0x0003;
						msg[8] = plot.Mode3A->Code & 0x0fff;
						msg[8] |= plot.Mode3A->Valid ? 0x1000 : 0;
						msg[8] |= plot.Mode3A->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.FlightLevel)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[5] |= 0x0180;
						msg[6] = (int) rint (plot.FlightLevel->Height / 0.3048 / 100) & 0x0fff;
						msg[6] |= plot.FlightLevel->Valid ? 0x1000 : 0;
						msg[6] |= plot.FlightLevel->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[5] |= 0x0030;
						msg[7] = (int) rint (plot.FlightLevel->Height / 0.3048 / 100) & 0x0fff;
						msg[7] |= plot.FlightLevel->Valid ? 0x1000 : 0;
						msg[7] |= plot.FlightLevel->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[5] |= 0x0006;
						msg[8] = (int) rint (plot.FlightLevel->Height / 0.3048 / 100) & 0x0fff;
						msg[8] |= plot.FlightLevel->Valid ? 0x1000 : 0;
						msg[8] |= plot.FlightLevel->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.Mode2)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[5] |= 0x0080;
						msg[6] = plot.Mode2->Code & 0x0fff;
						msg[6] |= plot.Mode2->Valid ? 0x1000 : 0;
						msg[6] |= plot.Mode2->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[5] |= 0x0010;
						msg[7] = plot.Mode2->Code & 0x0fff;
						msg[7] |= plot.Mode2->Valid ? 0x1000 : 0;
						msg[7] |= plot.Mode2->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[5] |= 0x0002;
						msg[8] = plot.Mode2->Code & 0x0fff;
						msg[8] |= plot.Mode2->Valid ? 0x1000 : 0;
						msg[8] |= plot.Mode2->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.Mode1)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[5] |= 0x0040;
						msg[6] = plot.Mode1->Code & 0x0fff;
						msg[6] |= plot.Mode1->Valid ? 0x1000 : 0;
						msg[6] |= plot.Mode1->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[5] |= 0x0008;
						msg[7] = plot.Mode1->Code & 0x0fff;
						msg[7] |= plot.Mode1->Valid ? 0x1000 : 0;
						msg[7] |= plot.Mode1->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[5] |= 0x0001;
						msg[8] = plot.Mode1->Code & 0x0fff;
						msg[8] |= plot.Mode1->Valid ? 0x1000 : 0;
						msg[8] |= plot.Mode1->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.ModeB)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[5] |= 0x0140;
						msg[6] = plot.ModeB->Code & 0x0fff;
						msg[6] |= plot.ModeB->Valid ? 0x1000 : 0;
						msg[6] |= plot.ModeB->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[5] |= 0x0040;
						msg[7] = plot.ModeB->Code & 0x0fff;
						msg[7] |= plot.ModeB->Valid ? 0x1000 : 0;
						msg[7] |= plot.ModeB->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[5] |= 0x0005;
						msg[8] = plot.ModeB->Code & 0x0fff;
						msg[8] |= plot.ModeB->Valid ? 0x1000 : 0;
						msg[8] |= plot.ModeB->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}
			if (plot.ModeD)
			{
	 			switch (code++)
				{
					case 1:		// primary code
					{
						msg[5] |= 0x01C0;
						msg[6] = plot.ModeD->Code & 0x0fff;
						msg[6] |= plot.ModeD->Valid ? 0x1000 : 0;
						msg[6] |= plot.ModeD->Garbled ? 0x2000 : 0;
						
						break;
					}
					case 2:		// secondary code
					{
						msg[5] |= 0x0038;
						msg[7] = plot.ModeD->Code & 0x0fff;
						msg[7] |= plot.ModeD->Valid ? 0x1000 : 0;
						msg[7] |= plot.ModeD->Garbled ? 0x2000 : 0;

						break;
					}
					case 3:		// tertiary code
					{
						msg[5] |= 0x0007;
						msg[8] = plot.ModeD->Code & 0x0fff;
						msg[8] |= plot.ModeD->Valid ? 0x1000 : 0;
						msg[8] |= plot.ModeD->Garbled ? 0x2000 : 0;

						break;
					}
				}
			}

			size = 18;
			break;
		}
	}

	return size;
}

int RadarService_to_Martello (RDFRadarService & service, void * data, int maxsize)
{
	memset (data, 0, maxsize);
	unsigned short * msg = (unsigned short *)data;
	int size;

	// label
	msg[0] = 0x0022;

	// siteid
	if (service.SIC > 0 && service.SIC < 3) msg[0] |= (service.SIC << 8) & 0x0fff;			// ? ma byt 0001 nebo 0010 =mozna & 0x03ff
	else msg[0] |= 0x0100;

	// azimuth
	msg[1] = (int) rint (service.Sector * 0x10000 / (2 * M_PI));

	// SM, SSR, PSR
	msg[2] = 0x0001;	

	size = 6;
	return size;
}

