#include "RDFHadr.hh"
#include "RDFAsterix.hh"
#include "CMSDebug.hh"

double vector_2_radians (double x, double y)
{
	if (x > 0 && y == 0) return 0;
	if (x > 0 && y > 0) return atan(y/x);
	if (x == 0 && y > 0) return M_PI_2;
	if (x < 0 && y > 0) return M_PI - atan(y / -x);
	if (x < 0 && y == 0) return M_PI;
	if (x < 0 && y < 0) return M_PI + atan(y/x);
	if (x == 0 && y < 0) return M_PI + M_PI_2;
	if (x > 0 && y < 0) return M_PI + M_PI - atan(y / -x);
	return 0;
}

double sinus_cosinus_2_radians (double sinus, double cosinus)
{
	if (sinus >= 0)
	{
		if (cosinus >= 0) return asin (sinus);
		return M_PI - asin (sinus);
	}

	if (cosinus < 0) return M_PI + asin (-sinus);
	return M_PI + M_PI - asin (-sinus);
}

bool Hadr_to_TargetTrack (const void * data, int size, 
	deque<RDFTargetTrack> & track_list)
{
	unsigned short * msg;
	unsigned short word;

	track_list.clear ();
	
	msg = (unsigned short *)data;
	size = size / 2;

	// start of burst message
	if (GUINT16_FROM_BE (msg[0]) == 0xFFf4)
	{
		// ok
		msg++;
		size--;
	}
	else
	{
		cms_warning ("wrong start of burst message %04x %02x", 
			GUINT16_FROM_BE (msg[0]));
		return FALSE;
	}
	
	// first category header
	if (GUINT16_FROM_BE (msg[0]) == 0xe000 
		&& GUINT16_FROM_BE (msg[1]) == 0x0642)
	{
		// ok
		msg += 2;
		size -= 2;
	}
	else if (GUINT16_FROM_BE (msg[0]) == 0xe000
		&& GUINT16_FROM_BE (msg[1]) == 0x046d)
	{
		// data for plots
		return FALSE;
	}
	else
	{
		cms_warning ("wrong first category header %04x %04x", 
			GUINT16_FROM_BE (msg[0]), GUINT16_FROM_BE (msg[1]));
		return FALSE;
	}

	// primary category header 
	if ((GUINT16_FROM_BE (msg[0]) == 0x8ec8 
		&& GUINT16_FROM_BE (msg[1]) == 0x8640)
		|| (GUINT16_FROM_BE (msg[0]) == 0x8cc8 
		&& GUINT16_FROM_BE (msg[1]) == 0x8640))
	{
		// ok
		msg += 2;
		size -= 2;
	}
	else
	{
		cms_warning ("wrong primary category header %04x %04x", 
			GUINT16_FROM_BE (msg[0]), GUINT16_FROM_BE (msg[1]));
		return FALSE;
	}

	// data for 200 track
	for (int i = 0; i < 200; i++)
	{
		//int addr = UINT16_FROM_BE (msg[0]);
		//DEBUG ("addr %d", addr);

		word = GUINT16_FROM_BE (msg[1]);
		//int mod = word & 0x03;
		int symbol = (word >> 2) & 0x3f;
		//bool mss = (word >> 8) & 1;
		//bool rem = (word >> 9) & 1;
		//bool aut = (word >> 10) & 1;
		//bool blk = (word >> 11) & 1;
		//bool sim = (word >> 12) & 1;
		bool act = (word >> 15) & 1;
		//DEBUG ("mod %d, symbol %o, mss %d, rem %d, aut %d, blk %d, sim %d, act %d",
		//	mod, symbol, mss, rem, aut, blk, sim, act);

		int y = (int) rint (GINT16_FROM_BE (msg[2]) * (0.3048 * 6000 / 64));
		//DEBUG ("Y %d", y);

		int x = (int) rint (GINT16_FROM_BE (msg[3]) * (0.3048 * 6000 / 64));
		//DEBUG ("X %d", x);

		word = GUINT16_FROM_BE (msg[4]);
		int yvel = ((word & 0x8000) ? -1 : +1) * ((word >> 1) & 0x7f);
		//bool ld = (word >> 8) & 1;
		//DEBUG ("Yvel %d, ld %d", yvel, ld);

		word = GUINT16_FROM_BE (msg[5]);
		int xvel = ((word & 0x8000) ? -1 : +1) * ((word >> 1) & 0x7f);
		//int pd = (word >> 8) & 0x7;
		//DEBUG ("Xvel %d, pd %d", xvel, pd);
		double heading = M_PI_2 - vector_2_radians (xvel, yvel);
		if (heading < 0) heading += 2 * M_PI;
		double speed = hypot (xvel, yvel) * 6.5;
		//XXX v jakych jednotkach je rychlost, to nevim,
		// experimentalne na dvou tracich jsem pouzil tuto hodnotu
		// a docela to i sedelo
		
		word = GUINT16_FROM_BE (msg[6]);
		int ch3 = (word >> 11) & 0x1f;
		int ch4 = (word >> 6) & 0x1f;
		int ch5 = (word >> 1) & 0x1f;
		bool track_id_present = FALSE;
		int track_id = 0;
		if (ch3 < 10 && ch4 < 10 && ch5 < 10)
		{
			track_id_present = TRUE;
			track_id = ch3 * 100 + ch4 * 10 + ch5;
		}
		//DEBUG ("char %o %o %o %d", ch3, ch4, ch5, track_id);

		word = GUINT16_FROM_BE (msg[7]);
		int ch1 = (word >> 11) & 0x1f;
		int ch2 = (word >> 6) & 0x1f;
		bool height_present = FALSE;
		int height = 0;
		if (ch1 < 10 && ch2 < 10)
		{
			height_present = TRUE;
			height = (int) rint ((ch1 * 10 + ch2) * 304.8);
		}
		//DEBUG ("char %o %o %d\n\n", ch1, ch2, height);

		// chybi trackid, tohle by se nemelo stat
		if (act && !track_id_present)
		{
			cms_error ("trackid missing");
		}

		// je trackid a track je aktivni
		else if (act)
		{
			// vytvorime novou polozku
			RDFTargetTrack track;

			switch (symbol)
			{
				case 044:
				{
					track.PSR = true;
					track.TargetType = TARGET_TYPE_PRIMARY;
					track.WarningError.push_back (WARNING_ERROR_TARGET_IN_CLUTTER_AREA);
					break;
				}
				case 046:
				{
					track.PSR = true;
					track.TargetType = TARGET_TYPE_PRIMARY;
					break;
				}
				default:
				{
					cms_warning ("unknown symbol %o", symbol);
				}
			}

			track.TrackNumber.Present = true;
			track.TrackNumber.Number = track_id;
			track.Cartesian = new RDFCoorCartesian ();
			track.Cartesian->X = x;
			track.Cartesian->Y = y;
			track.GroundSpeed = new RDFTargetGroundSpeed ();
			track.GroundSpeed->Speed = speed;
			track.GroundSpeed->Heading = heading;
			if (height_present)
			{
				track.MeasuredHeight = new RDFTargetHeight ();
				track.MeasuredHeight->Height = height;
			}

			// pridame ji do seznamu
			track_list.push_back (track);
		}

		// je trackid a track neni aktivni
		else
		{
			// vytvorime novou polozku
			RDFTargetTrack track;

			track.TrackNumber.Present = true;
			track.TrackNumber.Number = track_id;
			track.Cancel = TRUE;

			// pridame ji do seznamu
			track_list.push_back (track);
		}

		msg += 8;
		size -= 8;
		if (size < 8) break;
	}

	if (size != 0)
	{
		cms_warning ("wrong message length %d", size);
		//return FALSE;
	}

	return TRUE;
}

bool Hadr_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list, deque<RDFJamStrobe> & jam_list)
{
	unsigned short * msg;
	unsigned short word;

	plot_list.clear ();
	jam_list.clear ();
	
	msg = (unsigned short *)data;
	size = size / 2;

	// start of burst message
	if (GUINT16_FROM_BE (msg[0]) == 0xFFf4)
	{
		// ok
		msg++;
		size--;
	}
	else
	{
		cms_warning ("wrong start of burst message %04x %02x", 
			GUINT16_FROM_BE (msg[0]));
		return FALSE;
	}
	
	// first category header
	if (GUINT16_FROM_BE (msg[0]) == 0xe000 
		&& GUINT16_FROM_BE (msg[1]) == 0x0642)
	{
		// data for tracks
		return FALSE;
	}
	else if (GUINT16_FROM_BE (msg[0]) == 0xe000
		&& GUINT16_FROM_BE (msg[1]) == 0x046d)
	{
		msg += 2;
		size -= 2;
	}
	else
	{
		cms_warning ("wrong first category header %04x %04x", 
			GUINT16_FROM_BE (msg[0]), GUINT16_FROM_BE (msg[1]));
		return FALSE;
	}

	// primary category header 
	if ((GUINT16_FROM_BE (msg[0]) == 0x8c64 
		&& GUINT16_FROM_BE (msg[1]) == 0x8320))
	{
		// ok
		msg += 2;
		size -= 2;
	}
	else
	{
		cms_warning ("wrong primary category header %04x %04x", 
			GUINT16_FROM_BE (msg[0]), GUINT16_FROM_BE (msg[1]));
		return FALSE;
	}

	// data for 100 track
	for (int i = 0; i < 100; i++)
	{
		//int addr = UINT16_FROM_BE (msg[0]);
		//DEBUG ("addr %d", addr);

		word = GUINT16_FROM_BE (msg[1]);
		int symbol = (word >> 2) & 0x3f;
		bool act = (word >> 15) & 1;
		//DEBUG ("symbol %o, act %d", symbol, act);

		int y = (int) rint (GINT16_FROM_BE (msg[2]) * (0.3048 * 6000 / 64));
		//DEBUG ("Y %d", y);

		int x = (int) rint (GINT16_FROM_BE (msg[3]) * (0.3048 * 6000 / 64));
		//DEBUG ("X %d", x);

		word = GUINT16_FROM_BE (msg[4]);
		int yvel = ((word & 0x8000) ? -1 : +1) * ((word >> 1) & 0x7f);
		//DEBUG ("Yvel %d", yvel);

		word = GUINT16_FROM_BE (msg[5]);
		int xvel = ((word & 0x8000) ? -1 : +1) * ((word >> 1) & 0x7f);
		//DEBUG ("Xvel %d, ", xvel);
		
		word = GUINT16_FROM_BE (msg[6]);
		int ch3 = (word >> 11) & 0x1f;
		int ch4 = (word >> 6) & 0x1f;
		int ch5 = (word >> 1) & 0x1f;
		//bool plot_id_present = FALSE;
		int mode3a = 0;
		if (ch3 < 10 && ch4 < 10 && ch5 < 10)
		{
			//plot_id_present = TRUE;
			mode3a = ch3 * 100 + ch4 * 10 + ch5;
		}
		//DEBUG ("char %o %o %o %d", ch3, ch4, ch5, plot_id);

		word = GUINT16_FROM_BE (msg[7]);
		int ch1 = (word >> 11) & 0x1f;
		int ch2 = (word >> 6) & 0x1f;
		bool height_present = FALSE;
		int height = 0;
		if (ch1 < 10 && ch2 < 10)
		{
			height_present = TRUE;
			height = (int) rint ((ch1 * 10 + ch2) * 304.8);
		}
		//DEBUG ("char %o %o %d\n\n", ch1, ch2, height);

		if (xvel || yvel)
		{
			cms_warning ("plot speed is defined");
		}

		if (act)
		{
			// vytvorime novou polozku
			RDFTargetPlot plot;

			switch (symbol)
			{
				case 054: plot.TargetType = TARGET_TYPE_PRIMARY; break;
				case 042: plot.TargetType = TARGET_TYPE_SECONDARY; break;
				case 040: plot.TargetType = TARGET_TYPE_COMBINED; break;
				default: cms_warning ("unknown symbol %o", symbol);
			}

			plot.Polar = new RDFCoorPolar ();
			rdf_xy2rt (x, y, plot.Polar->Rho, plot.Polar->Theta);

			if (height_present)
			{
				plot.MeasuredHeight = new RDFTargetHeight ();
				plot.MeasuredHeight->Height = height;
			}

			if (plot.TargetType == TARGET_TYPE_SECONDARY
				|| plot.TargetType == TARGET_TYPE_COMBINED)
			{
				plot.Mode3A = new RDFTargetMode ();
				plot.Mode3A->Valid = true;
				plot.Mode3A->Code = asterix_encode_mode123 (mode3a);
			}

			// pridame ji do seznamu
			plot_list.push_back (plot);
		}

		msg += 8;
		size -= 8;
		if (size < 8) break;
	}

	// primary category header 
	if ((GUINT16_FROM_BE (msg[0]) == 0x9c28 
		&& GUINT16_FROM_BE (msg[1]) == 0x8140))
	{
		// ok
		msg += 2;
		size -= 2;
	}
	else
	{
		cms_warning ("wrong primary category header %04x %04x", 
			GUINT16_FROM_BE (msg[0]), GUINT16_FROM_BE (msg[1]));
		return FALSE;
	}

	// data for 40 jam-strobes
	for (int i = 0; i < 40; i++)
	{
		//int addr = UINT16_FROM_BE (msg[0]);
		//DEBUG ("addr %d", addr);
		
		word = GUINT16_FROM_BE (msg[1]);
		int strobe_lines = (int) rint ((word & 0x0fff) * (0.3048 * 6000 / 4));
		bool act = (word >> 15) & 1;

		int y = (int) rint (GINT16_FROM_BE (msg[2]) * (0.3048 * 6000 / 64));
		//DEBUG ("Y %d", y);

		int x = (int) rint (GINT16_FROM_BE (msg[3]) * (0.3048 * 6000 / 64));
		//DEBUG ("X %d", x);

		word = GUINT16_FROM_BE (msg[4]);
		double sinus = ((word & 0x07ff) / 2048.0) * (word & 0x80 ? -1 : 1);
	
		word = GUINT16_FROM_BE (msg[5]);
		double cosinus = ((word & 0x07ff) / 2048.0) * (word & 0x80 ? -1 : 1);

		double azimuth = sinus_cosinus_2_radians (sinus, cosinus);
		azimuth = M_PI + M_PI - azimuth;
		
		word = GUINT16_FROM_BE (msg[6]);
		int ch3 = (word >> 11) & 0x1f;
		int ch4 = (word >> 6) & 0x1f;
		int ch5 = (word >> 1) & 0x1f;
		int angle = 0;
		if (ch4 < 10 && ch5 < 10)
		{
			angle = ch4 * 10 + ch5;
			if (ch3 != 15 && ch3 != 16) angle = - angle;
		}
		//DEBUG ("char %o %o %o %d", ch3, ch4, ch5, angle);

		word = GUINT16_FROM_BE (msg[7]);
		int ch1 = (word >> 11) & 0x1f;
		int ch2 = (word >> 6) & 0x1f;
		int amplitude = 0;
		if (ch1 < 10 && ch2 < 10)
		{
			amplitude = ch1 * 10 + ch2;
		}
		//DEBUG ("char %o %o %d\n\n", ch1, ch2, amplitude);
		

		if (act)
		{
			// vytvorime novou polozku
			RDFJamStrobe jam;

			jam.StrobeLines = strobe_lines;
			jam.X = x;
			jam.Y = y;
			jam.Azimuth = azimuth;
			jam.Angle = angle;
			jam.Amplitude = amplitude;

			// pridame ji do seznamu
			jam_list.push_back (jam);
		}

		msg += 8;
		size -= 8;
		if (size < 8) break;
	}

	// sweep update message
	msg += 7;
	size -= 7;

	// last category header 
	if ((GUINT16_FROM_BE (msg[0]) == 0x1c00 
		&& GUINT16_FROM_BE (msg[1]) == 0x0000))
	{
		// ok
		msg += 2;
		size -= 2;
	}
	else
	{
		cms_warning ("wrong primary category header %04x %04x", 
			GUINT16_FROM_BE (msg[0]), GUINT16_FROM_BE (msg[1]));
		//return FALSE;
	}

	if (size != 0)
	{
		cms_warning ("wrong message length %d", size);
		//return FALSE;
	}

	return TRUE;
}

