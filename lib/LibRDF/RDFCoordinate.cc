#include "RDFCoordinate.hh"
#include "CMSDebug.hh"

RDFCoorGeographic::RDFCoorGeographic ()
{
	Latitude = 0;
	Longitude = 0;
}

RDFCoorGeographic::RDFCoorGeographic (double latitude, double longitude)
{
	Latitude = latitude;
	Longitude = longitude;
}

RDFCoorGeographic::RDFCoorGeographic (const RDFCoorGeographic & geographic)
{
	Latitude = geographic.Latitude;
	Longitude = geographic.Longitude;
}

RDFCoorGeographic::RDFCoorGeographic (const RDFCoorGeographic * geographic)
{
	if (geographic)
	{
		Latitude = geographic->Latitude;
		Longitude = geographic->Longitude;
	}
	else
	{
		Latitude = 0;
		Longitude = 0;
	}
}

void RDFCoorGeographic::init ()
{
	Latitude = 0;
	Longitude = 0;
}

bool RDFCoorGeographic::equal (const RDFCoorGeographic & geographic)
{
	if (Latitude != geographic.Latitude)
	{
		return false;
	}
	if (Longitude != geographic.Longitude)
	{
		return false;
	}
	return true;
}

RDFCoorCartesian::RDFCoorCartesian ()
{
	X = 0;
	Y = 0;
}

RDFCoorCartesian::RDFCoorCartesian (double x, double y)
{
	X = x;
	Y = y;
}

RDFCoorCartesian::RDFCoorCartesian (const RDFCoorCartesian & cartesian)
{
	X = cartesian.X;
	Y = cartesian.Y;
}

RDFCoorCartesian::RDFCoorCartesian (const RDFCoorCartesian * cartesian)
{
	if (cartesian)
	{
		X = cartesian->X;
		Y = cartesian->Y;
	}
	else
	{
		X = 0;
		Y = 0;
	}
}

void RDFCoorCartesian::init ()
{
	X = 0;
	Y = 0;
}

bool RDFCoorCartesian::equal (const RDFCoorCartesian & cartesian)
{
	if (X != cartesian.X)
	{
		return false;
	}
	if (Y != cartesian.Y)
	{
		return false;
	}
	return true;
}

RDFCoorPolar::RDFCoorPolar ()
{
	Rho = 0;
	Theta = 0;
}

RDFCoorPolar::RDFCoorPolar (double rho, double theta)
{
	Rho = rho;
	Theta = theta;
}

RDFCoorPolar::RDFCoorPolar (const RDFCoorPolar & polar)
{
	Rho = polar.Rho;
	Theta = polar.Theta;
}

RDFCoorPolar::RDFCoorPolar (const RDFCoorPolar * polar)
{
	if (polar)
	{
		Rho = polar->Rho;
		Theta = polar->Theta;
	}
	else
	{
		Rho = 0;
		Theta = 0;
	}
}

void RDFCoorPolar::init ()
{
	Rho = 0;
	Theta = 0;
}

bool RDFCoorPolar::equal (const RDFCoorPolar & polar)
{
	if (Rho != polar.Rho)
	{
		return false;
	}
	if (Theta != polar.Theta)
	{
		return false;
	}
	return true;
}

void rdf_xy2ra (double x, double y, double & rho, double & angle)
{
	rho = rint (hypot (x, y));

	// mozne nahradit angle = M_PI + atan2 (-y, -x);	// <0;2*M_PI>
	if (x > 0 && y == 0)
	{
		angle = 0;
	}
	else if (x > 0 && y > 0)
	{
		angle = atan(y/x);
	}
	else if (x == 0 && y > 0)
	{
		angle = M_PI_2;
	}
	else if (x < 0 && y > 0)
	{
		angle = M_PI - atan(y / -x);
	}
	else if (x < 0 && y == 0)
	{
		angle = M_PI;
	}
	else if (x < 0 && y < 0)
	{
		angle = M_PI + atan(y/x);
	}
	else if (x == 0 && y < 0)
	{
		angle = M_PI + M_PI_2;
	}
	else if (x > 0 && y < 0)
	{
		angle = M_PI + M_PI - atan(y / -x);
	}
	else
	{
		angle = 0;
	}
}

void rdf_xy2rt (double x, double y, double & rho, double & theta)
{
	rdf_xy2ra (x, y, rho, theta);
	theta = M_PI_2 - theta;
	if (theta < 0)
	{
		theta += 2 * M_PI;
	}
}

void rdf_rt2xy (double rho, double theta, double & x, double & y)
{
	double angle = M_PI_2 - theta;
	x = rho * cos (angle);
	y = rho * sin (angle);
}

void rdf_ra2xy (double rho, double angle, double & x, double & y)
{
	x = rho * cos (angle);
	y = rho * sin (angle);
}

void rdf_xyz2ra (double x, double y, double z, double & rho, double & angle)
{
	rho = rint (sqrt (x * x + y * y + z * z));
	
	// mozne nahradit angle = M_PI + atan2 (-y, -x);	// <0;2*M_PI>
	if (x > 0 && y == 0)
	{
		angle = 0;
	}
	else if (x > 0 && y > 0)
	{
		angle = atan(y/x);
	}
	else if (x == 0 && y > 0)
	{
		angle = M_PI_2;
	}
	else if (x < 0 && y > 0)
	{
		angle = M_PI - atan(y / -x);
	}
	else if (x < 0 && y == 0)
	{
		angle = M_PI;
	}
	else if (x < 0 && y < 0)
	{
		angle = M_PI + atan(y/x);
	}
	else if (x == 0 && y < 0)
	{
		angle = M_PI + M_PI_2;
	}
	else if (x > 0 && y < 0)
	{
		angle = M_PI + M_PI - atan(y / -x);
	}
	else
	{
		angle = 0;
	}
}

void rdf_xyz2rt (double x, double y, double z, double & rho, double & theta)
{
	rdf_xyz2ra (x, y, z, rho, theta);
	theta = M_PI_2 - theta;
	if (theta < 0)
	{
		theta += 2 * M_PI;
	}
}
	
/* prevod z lokalnich polarnich (sferickych) souradnic (rho, theta, psi), 
 * tj. sikma dalka, azimut a elevace do 
 * lokalnich kartezskych souradnic (x, y, z),
 * oba souradne systemy se stredem v poloze anteny radaru
 */
void rdf_spherical2xyz (double rho, double theta, double psi, 
	double & x, double & y, double & z, double deviation)
{
	x = rho * cos (psi) * sin (theta + deviation);
	y = rho * cos (psi) * cos (theta + deviation);
	z = rho * sin (psi);
}

bool rdf_dms2rad (int way, int deg, int min, double sec, double & rad)
{
	if (way)
	{
		if (deg >= 180 || deg < 0)
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}
		if (min >= 60 || min < 0)
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}
		if (sec >= 60 || sec < 0)
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}

		rad = ((((sec / 60) + min) / 60) + deg) * DEG_TO_RAD;
		if (way < 0)
		{
			rad = -rad;
		}
	}

	else
	{
		if (deg >= 180 || deg <= -180)
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}
		if (min >= 60 || min <= -60)
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}
		if (sec >= 60 || sec <= -60)
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}
		if (!((deg >= 0 && min >= 0 && sec >= 0)
			|| (deg <= 0 && min <= 0 && sec <= 0)))
		{
			cms_error ("%d %d %d %f", way, deg, min, sec);
			goto failed;
		}
		
		rad = ((((sec / 60) + min) / 60) + deg) * DEG_TO_RAD;
	}

	return true;

failed:
	rad = 0;
	return false;
}

bool rdf_rad2dms (double rad, int & way, int & deg, int & min, int & sec)
{
	if (rad >= M_PI || rad <= -M_PI )
	{
		cms_error ("%lf", rad);
		deg = 0;
		min = 0;
		sec = 0;
		return false;
	}

	if (rad >= 0)
	{
		way = 1;
		rad = rad * RAD_TO_DEG;
	}
	else
	{
		way = -1;
		rad = -rad * RAD_TO_DEG;
	}

	int tmp = lrint (rad * 60 * 60);
	deg = (tmp / 60) / 60;
	min = (tmp / 60) % 60;
	sec = tmp % 60;

	return true;
}

bool rdf_rad2dms (double rad, int & way, int & deg, int & min, double & sec)
{
	if (rad >= M_PI || rad <= -M_PI )
	{
		cms_error ("%lf", rad);
		deg = 0;
		min = 0;
		sec = 0;
		return false;
	}

	if (rad >= 0)
	{
		way = 1;
		rad = rad * RAD_TO_DEG;
	}
	else
	{
		way = -1;
		rad = -rad * RAD_TO_DEG;
	}

#if 0
	int tmp = lrint (rad * 60 * 6000);
	deg = (tmp / 6000) / 60;
	min = (tmp / 6000) % 60;
	deg = (tmp % 6000) / 100.0;
#else
	deg = (int) trunc (rad);
	rad = (rad - deg) * 60;
	min = (int) trunc (rad);
	sec = (rad - min) * 60;
#endif

	return true;
}

void rdf_geo2strgeo (double lat, double lon, char * strlat, char * strlon)
{
	int way, deg, min;
	double sec;

	rdf_rad2dms (lat, way, deg, min, sec);
	sprintf (strlat, "%03d %02d %02d.%02d %c", deg, min, 
		(int) sec, (int) (sec * 100) % 100, way > 0 ? 'N' : 'S');

	rdf_rad2dms (lon, way, deg, min, sec);
	sprintf (strlon, "%03d %02d %02d.%02d %c", deg, min, 
		(int) sec, (int) (sec * 100) % 100, way > 0 ? 'E' : 'W');
}

char rdf_Longitude_to_Georef1 (double longitude)
{
	int lon = (int) floor (longitude * RAD_TO_DEG / 15);

	switch (lon)
	{
		case -12: return 'A';
		case -11: return 'B';
		case -10: return 'C';
		case  -9: return 'D';
		case  -8: return 'E';
		case  -7: return 'F';
		case  -6: return 'G';
		case  -5: return 'H';
		case  -4: return 'J';
		case  -3: return 'K';
		case  -2: return 'L';
		case  -1: return 'M';
		case   0: return 'N';
		case   1: return 'P';
		case   2: return 'Q';
		case   3: return 'R';
		case   4: return 'S';
		case   5: return 'T';
		case   6: return 'U';
		case   7: return 'V';
		case   8: return 'W';
		case   9: return 'X';
		case  10: return 'Y';
		case  11: return 'Z';
	}

	cms_error ("%f %d", longitude, lon);
	return 32;
}

int rdf_Georef1_to_Longitude (char ch)
{
	switch (ch)
	{
		case 'A': return -12 * 15;
		case 'B': return -11 * 15;
		case 'C': return -10 * 15;
		case 'D': return -9 * 15;
		case 'E': return -8 * 15;
		case 'F': return -7 * 15;
		case 'G': return -6 * 15;
		case 'H': return -5 * 15;
		case 'J': return -4 * 15;
		case 'K': return -3 * 15;
		case 'L': return -2 * 15;
		case 'M': return -1 * 15;
		case 'N': return 0;
		case 'P': return 1 * 15;
		case 'Q': return 2 * 15;
		case 'R': return 3 * 15;
		case 'S': return 4 * 15;
		case 'T': return 5 * 15;
		case 'U': return 6 * 15;
		case 'V': return 7 * 15;
		case 'W': return 8 * 15;
		case 'X': return 9 * 15;
		case 'Y': return 10 * 15;
		case 'Z': return 11 * 15;
	}

	cms_error ("");
	return 0;
}

char rdf_Latitude_to_Georef1 (double latitude)
{
	int lat = (int) floor (latitude * RAD_TO_DEG / 15);

	switch (lat)
	{
		case -6: return 'A';
		case -5: return 'B';
		case -4: return 'C';
		case -3: return 'D';
		case -2: return 'E';
		case -1: return 'F';
		case  0: return 'G';
		case  1: return 'H';
		case  2: return 'J';
		case  3: return 'K';
		case  4: return 'L';
		case  5: return 'M';
	}

	cms_error ("%f %d", latitude, lat);
	return 32;
}

int rdf_Georef1_to_Latitude (char ch)
{
	switch (ch)
	{
		case 'A': return -6 * 15;
		case 'B': return -5 * 15;
		case 'C': return -4 * 15;
		case 'D': return -3 * 15;
		case 'E': return -2 * 15;
		case 'F': return -1 * 15;
		case 'G': return 0;
		case 'H': return 1 * 15;
		case 'J': return 2 * 15;
		case 'K': return 3 * 15;
		case 'L': return 4 * 15;
		case 'M': return 5 * 15;
	}

	cms_error ("");
	return 0;
}

char rdf_Longitude_to_Georef2 (double longitude)
{
	int lon = (int) floor (longitude * RAD_TO_DEG) 
		- (int) floor (longitude * RAD_TO_DEG / 15) * 15;
	lon = lon - (lon / 15) * 15;

	switch (lon)
	{
		case 0: return 'A';
		case 1: return 'B';
		case 2: return 'C';
		case 3: return 'D';
		case 4: return 'E';
		case 5: return 'F';
		case 6: return 'G';
		case 7: return 'H';
		case 8: return 'J';
		case 9: return 'K';
		case 10: return 'L';
		case 11: return 'M';
		case 12: return 'N';
		case 13: return 'P';
		case 14: return 'Q';
	}

	cms_error ("%f %d", longitude, lon);
	return 32;
}

int rdf_Georef2_to_Longitude (char ch)
{
	switch (ch)
	{
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
		case 'E': return 4;
		case 'F': return 5;
		case 'G': return 6;
		case 'H': return 7;
		case 'J': return 8;
		case 'K': return 9;
		case 'L': return 10;
		case 'M': return 11;
		case 'N': return 12;
		case 'P': return 13;
		case 'Q': return 14;
	}

	cms_error ("");
	return 0;
}

char rdf_Latitude_to_Georef2 (double latitude)
{
	int lat = (int) floor (latitude * RAD_TO_DEG) 
		- (int) floor (latitude * RAD_TO_DEG / 15) * 15;

	switch (lat)
	{
		case 0: return 'A';
		case 1: return 'B';
		case 2: return 'C';
		case 3: return 'D';
		case 4: return 'E';
		case 5: return 'F';
		case 6: return 'G';
		case 7: return 'H';
		case 8: return 'J';
		case 9: return 'K';
		case 10: return 'L';
		case 11: return 'M';
		case 12: return 'N';
		case 13: return 'P';
		case 14: return 'Q';
	}

	cms_error ("%f %d", latitude, lat);
	return 32;
}			

int rdf_Georef2_to_Latitude (char ch)
{
	switch (ch)
	{
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
		case 'E': return 4;
		case 'F': return 5;
		case 'G': return 6;
		case 'H': return 7;
		case 'J': return 8;
		case 'K': return 9;
		case 'L': return 10;
		case 'M': return 11;
		case 'N': return 12;
		case 'P': return 13;
		case 'Q': return 14;
	}

	cms_error ("");
	return 0;
}			

int rdf_Longitude_to_Georef3 (double longitude)
{
	int way, deg, min;
	double sec;
	
	if (longitude > 0)
	{
		rdf_rad2dms (longitude, way, deg, min, sec);
		return min;
	}

	rdf_rad2dms (longitude + M_PI, way, deg, min, sec);
	return min;
}

int rdf_Latitude_to_Georef3 (double latitude)
{
	int way, deg, min;
	double sec;
	
	if (latitude > 0)
	{
		rdf_rad2dms (latitude, way, deg, min, sec);
		return min;
	}

	rdf_rad2dms (latitude + M_PI_2, way, deg, min, sec);
	return min;
}

void rdf_geo2strgeoref (const RDFCoorGeographic & geo, char * strgeoref)
{
	sprintf (strgeoref, "%c%c%c%c %02d%02d",
		rdf_Longitude_to_Georef1 (geo.Longitude), 
		rdf_Latitude_to_Georef1 (geo.Latitude),
		rdf_Longitude_to_Georef2 (geo.Longitude), 
		rdf_Latitude_to_Georef2 (geo.Latitude),
		rdf_Longitude_to_Georef3 (geo.Longitude), 
		rdf_Latitude_to_Georef3 (geo.Latitude));
}

void rdf_strgeoref2geo (const char * strgeoref, RDFCoorGeographic & geo)
{
	geo.Longitude = rdf_Georef1_to_Longitude (strgeoref[0]);
	geo.Latitude = rdf_Georef1_to_Latitude (strgeoref[1]);
	geo.Longitude += rdf_Georef2_to_Longitude (strgeoref[2]);
	geo.Latitude += rdf_Georef2_to_Latitude (strgeoref[3]);

	if (strgeoref[4] != 32)
	{
		cms_error ("");
	}

	geo.Longitude += ((strgeoref[5] - '0') * 10 + strgeoref[6] - '0') / 60.0;
	geo.Latitude += ((strgeoref[7] - '0') * 10 + strgeoref[8] - '0') / 60.0; 
	geo.Longitude *= DEG_TO_RAD;
	geo.Latitude *= DEG_TO_RAD;
}

bool rdf_geo2strgeoatc (const RDFCoorGeographic & geo, char * strgeoatc)
{
	int way1, deg1, min1, sec1;
	int way2, deg2, min2, sec2;

	if (!rdf_rad2dms (geo.Latitude, way1, deg1, min1, sec1))
	{
		cms_error ("rdf_geo2strgeoatc error %lf", geo.Latitude);
		strcpy (strgeoatc, "N000000E0000000");
		return false;
	}
	if (!rdf_rad2dms (geo.Longitude, way2, deg2, min2, sec2))
	{
		cms_error ("rdf_geo2strgeoatc error %lf", geo.Longitude);
		strcpy (strgeoatc, "N000000E0000000");
		return false;
	}

	sprintf (strgeoatc, "%c%02d%02d%02d%c%03d%02d%02d", 
		way1 > 0 ? 'N' : 'S', deg1, min1, sec1,
		way2 > 0 ? 'E' : 'W', deg2, min2, sec2);

	return true;
}

bool rdf_strgeoatc2geo (const char * strgeoatc, RDFCoorGeographic & geo)
{
	int way, min, deg, sec;

	for (unsigned i = 1; i < 7; i++)
	{
		if (!g_ascii_isdigit (strgeoatc[i]))
		{
			cms_error ("%d %c", i, strgeoatc[i]);
			geo.Latitude = geo.Longitude = 0;
			return false;
		}
	}

	deg = 10 * g_ascii_digit_value (strgeoatc[1]) 
		+ g_ascii_digit_value (strgeoatc[2]);
	min = 10 * g_ascii_digit_value (strgeoatc[3]) 
		+ g_ascii_digit_value (strgeoatc[4]);
	sec = 10 * g_ascii_digit_value (strgeoatc[5]) 
		+ g_ascii_digit_value (strgeoatc[6]);

	if (strgeoatc[0] == 'N')
	{
		way = 1;
	}
	else if (strgeoatc[0] == 'S')
	{
		way = -1;
	}
	else
	{
		cms_error ("%c", strgeoatc[0]);
		geo.Latitude = geo.Longitude = 0;
		return false;
	}

	if (!rdf_dms2rad (way, deg, min, sec, geo.Latitude))
	{
		cms_error ("rdf_dms2rad error");
		geo.Latitude = geo.Longitude = 0;
		return false;
	}

	for (unsigned i = 8; i < 15; i++)
	{
		if (!g_ascii_isdigit (strgeoatc[i]))
		{
			cms_error ("%d %c", i, strgeoatc[i]);
			geo.Latitude = geo.Longitude = 0;
			return false;
		}
	}

	deg = 100 * g_ascii_digit_value (strgeoatc[8]) 
		+ 10 * g_ascii_digit_value (strgeoatc[9])
		+ g_ascii_digit_value (strgeoatc[10]);
	min = 10 * g_ascii_digit_value (strgeoatc[11]) 
		+ g_ascii_digit_value (strgeoatc[12]);
	sec = 10 * g_ascii_digit_value (strgeoatc[13]) 
		+ g_ascii_digit_value (strgeoatc[14]);

	if (strgeoatc[7] == 'E')
	{
		way = 1;
	}
	else if (strgeoatc[7] == 'W')
	{
		way = -1;
	}
	else
	{
		cms_error ("%c", strgeoatc[7]);
		geo.Latitude = geo.Longitude = 0;
		return false;
	}

	if (!rdf_dms2rad (way, deg, min, sec, geo.Longitude))
	{
		cms_error ("rdf_dms2rad error");
		geo.Latitude = geo.Longitude = 0;
		return false;
	}

	return true;
}

