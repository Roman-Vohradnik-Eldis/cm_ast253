#include "RDFProjection.hh"
#include "CMSDebug.hh"

#if (PROJ_VERSION == 4)
#include <projects.h>
#else
#include <proj.h>
#endif

#define WGS84_SEMIMAJOR_AXIS	6378137.000
#define WGS84_SEMIMINOR_AXIS	6356752.314
#define WGS84_ECCENTRICITY		0.0818191908426
#define WGS84_ECCENTRICITY2		0.00669437999013

#define DEFAULT_PJ_INIT "+proj=sterea +ellps=WGS84"

static string PjInit;

RDFProjection::RDFProjection ()
{
	SourcePoint.Latitude = SourcePoint.Longitude = 0;
	DestinationPoint.Latitude = DestinationPoint.Longitude = 0;
	SourcePJ = DestinationPJ = NULL;
	CountPJ ();
}

RDFProjection::RDFProjection (const RDFProjection & projection)
{
	SourcePoint = projection.SourcePoint;
	DestinationPoint = projection.DestinationPoint;
	SourcePJ = DestinationPJ = NULL;
	CountPJ ();
}

RDFProjection::~RDFProjection ()
{
#if (PROJ_VERSION == 4)
	if (SourcePJ)
	{
		pj_free ((PJ *)SourcePJ);
	}
	if (DestinationPJ)
	{
		pj_free ((PJ *)DestinationPJ);
	}
#else
	if (SourcePJ)
	{
		proj_destroy ((PJ *)SourcePJ);
	}
	if (DestinationPJ)
	{
		proj_destroy ((PJ *)DestinationPJ);
	}
#endif
}

void RDFProjection::SetPjInit (string init)
{
	PjInit = init;
	CountPJ ();
}

void RDFProjection::SetCenter (const RDFCoorGeographic & center)
{
	DestinationPoint = SourcePoint = center;
	CountPJ ();
}

void RDFProjection::Geo2Car (const RDFCoorGeographic & geographic, RDFCoorCartesian & cartesian)
{
	if (!SourcePJ)
	{
		return;
	}
	
#if (PROJ_VERSION == 4)
	projLP lp = { geographic.Longitude, geographic.Latitude };
	projXY xy = pj_fwd (lp, (PJ *)SourcePJ);

	if (xy.u == HUGE_VAL || xy.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", geographic.Latitude * RAD_TO_DEG, 
			geographic.Longitude * RAD_TO_DEG, pj_strerrno (pj_errno));
	}

	cartesian.X = xy.u;
	cartesian.Y = xy.v;
#else
	PJ_COORD lp = proj_coord (geographic.Longitude, geographic.Latitude, 0, 0);
	PJ_COORD xy = proj_trans ((PJ *)SourcePJ, PJ_FWD, lp);

	if (xy.xy.x == HUGE_VAL || xy.xy.y == HUGE_VAL)
	{
		cms_error ("%f %f error: %s",
			proj_todeg (geographic.Latitude),
			proj_todeg (geographic.Longitude),
			proj_errno_string (proj_errno ((PJ *) SourcePJ)));
	}

	cartesian.X = xy.xy.x;
	cartesian.Y = xy.xy.y;
#endif
}

void RDFProjection::Geo2Car (const RDFCoorGeographic * geographic, RDFCoorCartesian * cartesian)
{
	if (!SourcePJ)
	{
		return;
	}
	
#if (PROJ_VERSION == 4)
	projLP lp = { geographic->Longitude, geographic->Latitude };
	projXY xy = pj_fwd (lp, (PJ *)SourcePJ);

	if (xy.u == HUGE_VAL || xy.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", geographic->Latitude * RAD_TO_DEG, 
			geographic->Longitude * RAD_TO_DEG, pj_strerrno (pj_errno));
	}

	cartesian->X = xy.u;
	cartesian->Y = xy.v;
#else
	PJ_COORD lp = proj_coord (geographic->Longitude, geographic->Latitude, 0, 0);
	PJ_COORD xy = proj_trans ((PJ *)SourcePJ, PJ_FWD, lp);

	if (xy.xy.x == HUGE_VAL || xy.xy.y == HUGE_VAL)
	{
		cms_error ("%f %f error: %s",
			proj_todeg (geographic->Latitude),
			proj_todeg (geographic->Longitude),
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	cartesian->X = xy.xy.x;
	cartesian->Y = xy.xy.y;
#endif
}

void RDFProjection::Geo2Pol (const RDFCoorGeographic & geographic, RDFCoorPolar & polar)
{
	if (!SourcePJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projLP lp = { geographic.Longitude, geographic.Latitude };
	projXY xy = pj_fwd (lp, (PJ *)SourcePJ);

	if (xy.u == HUGE_VAL || xy.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", geographic.Latitude * RAD_TO_DEG, 
			geographic.Longitude * RAD_TO_DEG, pj_strerrno (pj_errno));
	}

	rdf_xy2rt (xy.u, xy.v, polar.Rho, polar.Theta);
#else
	PJ_COORD lp = proj_coord (geographic.Longitude, geographic.Latitude, 0, 0);
	PJ_COORD xy = proj_trans ((PJ *)SourcePJ, PJ_FWD, lp);

	if (xy.xy.x == HUGE_VAL || xy.xy.y == HUGE_VAL)
	{
		cms_error ("%f %f error: %s",
			proj_todeg (geographic.Latitude),
			proj_todeg (geographic.Longitude),
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	rdf_xy2rt (xy.xy.x, xy.xy.y, polar.Rho, polar.Theta);
#endif
}

void RDFProjection::Geo2Pol (const RDFCoorGeographic * geographic, RDFCoorPolar * polar)
{
	if (!SourcePJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projLP lp = { geographic->Longitude, geographic->Latitude };
	projXY xy = pj_fwd (lp, (PJ *)SourcePJ);

	if (xy.u == HUGE_VAL || xy.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", geographic->Latitude * RAD_TO_DEG, 
			geographic->Longitude * RAD_TO_DEG, pj_strerrno (pj_errno));
	}

	rdf_xy2rt (xy.u, xy.v, polar->Rho, polar->Theta);
#else
	PJ_COORD lp = proj_coord (geographic->Longitude, geographic->Latitude, 0, 0);
	PJ_COORD xy = proj_trans ((PJ *)SourcePJ, PJ_FWD, lp);

	if (xy.xy.x == HUGE_VAL || xy.xy.y == HUGE_VAL)
	{
		cms_error ("%f %f error: %s",
			proj_todeg (geographic->Latitude),
			proj_todeg (geographic->Longitude),
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	rdf_xy2rt (xy.xy.x, xy.xy.y, polar->Rho, polar->Theta);
#endif
}

void RDFProjection::Car2Geo (const RDFCoorCartesian & cartesian, RDFCoorGeographic & geographic)
{
	if (!SourcePJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { cartesian.X, cartesian.Y };
	projLP lp = pj_inv (xy, (PJ *)SourcePJ);

	if (lp.u == HUGE_VAL || lp.v == HUGE_VAL)
	{
		cms_error ("%d %d error: %s", cartesian.X, 
			cartesian.Y, pj_strerrno (pj_errno));
	}

	geographic.Longitude = lp.u;
	geographic.Latitude = lp.v;
#else
	PJ_COORD xy = proj_coord (cartesian.X, cartesian.Y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *)SourcePJ, PJ_INV, xy);

	if (lp.lp.lam == HUGE_VAL || lp.lp.phi == HUGE_VAL)
	{
		cms_error ("%d %d error: %s",
			cartesian.X, cartesian.Y,
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	geographic.Longitude = lp.lp.lam;
	geographic.Latitude = lp.lp.phi;
#endif
}

void RDFProjection::Car2Geo (const RDFCoorCartesian * cartesian, RDFCoorGeographic * geographic)
{
	if (!SourcePJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { cartesian->X, cartesian->Y };
	projLP lp = pj_inv (xy, (PJ *)SourcePJ);

	if (lp.u == HUGE_VAL || lp.v == HUGE_VAL)
	{
		cms_error ("%d %d error: %s", cartesian->X, 
			cartesian->Y, pj_strerrno (pj_errno));
	}

	geographic->Longitude = lp.u;
	geographic->Latitude = lp.v;
#else
	PJ_COORD xy = proj_coord (cartesian->X, cartesian->Y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *)SourcePJ, PJ_INV, xy);

	if (lp.lp.lam == HUGE_VAL || lp.lp.phi == HUGE_VAL)
	{
		cms_error ("%d %d error: %s",
			cartesian->X, cartesian->Y,
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	geographic->Longitude = lp.lp.lam;
	geographic->Latitude = lp.lp.phi;
#endif
}

void RDFProjection::Car2Pol (const RDFCoorCartesian & cartesian, RDFCoorPolar & polar)
{
	if (!SourcePJ)
	{
		return;
	}

	rdf_xy2rt (cartesian.X, cartesian.Y, polar.Rho, polar.Theta);
}

void RDFProjection::Car2Pol (const RDFCoorCartesian * cartesian, RDFCoorPolar * polar)
{
	if (!SourcePJ)
	{
		return;
	}

	rdf_xy2rt (cartesian->X, cartesian->Y, polar->Rho, polar->Theta);
}

void RDFProjection::Pol2Geo (const RDFCoorPolar & polar, RDFCoorGeographic & geographic)
{
	if (!SourcePJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { 0, 0 };
	rdf_rt2xy (polar.Rho, polar.Theta, xy.u, xy.v);	
	projLP lp = pj_inv (xy, (PJ *)SourcePJ);

	if (lp.u == HUGE_VAL || lp.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", polar.Rho, 
			polar.Theta, pj_strerrno (pj_errno));
	}

	geographic.Longitude = lp.u;
	geographic.Latitude = lp.v;
#else
	PJ_COORD xy = proj_coord (0, 0, 0, 0);
	rdf_rt2xy (polar.Rho, polar.Theta, xy.xy.x, xy.xy.y);	
	PJ_COORD lp = proj_trans ((PJ *)SourcePJ, PJ_INV, xy);

	if (lp.lp.lam == HUGE_VAL || lp.lp.phi == HUGE_VAL)
	{
		cms_error ("%f %f error: %s",
			polar.Rho, polar.Theta,
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	geographic.Longitude = lp.lp.lam;
	geographic.Latitude = lp.lp.phi;
#endif
}

void RDFProjection::Pol2Geo (const RDFCoorPolar * polar, RDFCoorGeographic * geographic)
{
	if (!SourcePJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { 0, 0 };
	rdf_rt2xy (polar->Rho, polar->Theta, xy.u, xy.v);	
	projLP lp = pj_inv (xy, (PJ *)SourcePJ);

	if (lp.u == HUGE_VAL || lp.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", polar->Rho, 
			polar->Theta, pj_strerrno (pj_errno));
	}

	geographic->Longitude = lp.u;
	geographic->Latitude = lp.v;
#else
	PJ_COORD xy = proj_coord (0, 0, 0, 0);
	rdf_rt2xy (polar->Rho, polar->Theta, xy.xy.x, xy.xy.y);	
	PJ_COORD lp = proj_trans ((PJ *)SourcePJ, PJ_INV, xy);

	if (lp.lp.lam == HUGE_VAL || lp.lp.phi == HUGE_VAL)
	{
		cms_error ("%f %f error: %s",
			polar->Rho, polar->Theta,
			proj_errno_string (proj_errno ((PJ *)SourcePJ)));
	}

	geographic->Longitude = lp.lp.lam;
	geographic->Latitude = lp.lp.phi;
#endif
}

void RDFProjection::Pol2Car (const RDFCoorPolar & polar, RDFCoorCartesian & cartesian)
{
	rdf_rt2xy (polar.Rho, polar.Theta, cartesian.X, cartesian.Y);
}

void RDFProjection::Pol2Car (const RDFCoorPolar * polar, RDFCoorCartesian * cartesian)
{
	rdf_rt2xy (polar->Rho, polar->Theta, cartesian->X, cartesian->Y);
}

void RDFProjection::SetSourceDestination (const RDFCoorGeographic & src, const RDFCoorGeographic & dst)
{
	SourcePoint = src;
	DestinationPoint = dst;
	CountPJ ();
}

void RDFProjection::RecalTarget (int & x, int & y)
{
	if (!Recal)
	{
		return;
	}
	if (!SourcePJ || !DestinationPJ)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { (double) x, (double) y };
	projLP lp = pj_inv (xy, (PJ *)SourcePJ);
	xy = pj_fwd (lp, (PJ *)DestinationPJ);
	x = (int) rint (xy.u);
	y = (int) rint (xy.v);
#else
	PJ_COORD xy = proj_coord (x, y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *) SourcePJ, PJ_INV, xy);
	xy = proj_trans ((PJ *) DestinationPJ, PJ_FWD, lp);
	x = lrint (xy.xy.x);
	y = lrint (xy.xy.y);
#endif
}

void RDFProjection::RecalTarget (double & x, double & y)
{
	if (!Recal)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { x, y };
	projLP lp = pj_inv (xy, (PJ *)SourcePJ);
	xy = pj_fwd (lp, (PJ *)DestinationPJ);
	x = xy.u;
	y = xy.v;
#else
	PJ_COORD xy = proj_coord (x, y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *) SourcePJ, PJ_INV, xy);
	xy = proj_trans ((PJ *) DestinationPJ, PJ_FWD, lp);
	x = xy.xy.x;
	y = xy.xy.y;
#endif
}

void RDFProjection::CountPJ ()
{
	char text[1024];
	char lat[256];
	char lon[256];

	g_ascii_dtostr (lat, sizeof (lat), SourcePoint.Latitude * RAD_TO_DEG);
	g_ascii_dtostr (lon, sizeof (lon), SourcePoint.Longitude * RAD_TO_DEG);
	
	sprintf (text, "%s +lat_0=%s +lon_0=%s", 
		PjInit.size () ? PjInit.c_str () : DEFAULT_PJ_INIT, 
		lat, lon);

#if (PROJ_VERSION == 4)
	if (SourcePJ)
	{
		pj_free ((PJ *)SourcePJ);
	}
	
	SourcePJ = pj_init_plus (text);
	if (!SourcePJ)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}

	g_ascii_dtostr (lat, sizeof (lat), DestinationPoint.Latitude * RAD_TO_DEG);
	g_ascii_dtostr (lon, sizeof (lon), DestinationPoint.Longitude * RAD_TO_DEG);

	sprintf (text, "%s +lat_0=%s +lon_0=%s", 
		PjInit.size () ? PjInit.c_str () : DEFAULT_PJ_INIT, 
		lat, lon);

	if (DestinationPJ)
	{
		pj_free ((PJ *)DestinationPJ);
	}
	
	DestinationPJ = pj_init_plus (text);
	if (!DestinationPJ)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	if (SourcePJ)
	{
		proj_destroy ((PJ *)SourcePJ);
	}
	
	SourcePJ = proj_create (PJ_DEFAULT_CTX, text);
	if (!SourcePJ)
	{
		cms_error ("proj_create error: %s", text);
	}

	g_ascii_dtostr (lat, sizeof (lat), DestinationPoint.Latitude * RAD_TO_DEG);
	g_ascii_dtostr (lon, sizeof (lon), DestinationPoint.Longitude * RAD_TO_DEG);

	sprintf (text, "%s +lat_0=%s +lon_0=%s", 
		PjInit.size () ? PjInit.c_str () : DEFAULT_PJ_INIT, 
		lat, lon);

	if (DestinationPJ)
	{
		proj_destroy ((PJ *)DestinationPJ);
	}
	
	DestinationPJ = proj_create (PJ_DEFAULT_CTX, text);
	if (!DestinationPJ)
	{
		cms_error ("proj_create error: %s", text);
	}
#endif

	Recal = (SourcePoint.Latitude != DestinationPoint.Latitude 
		|| SourcePoint.Longitude != DestinationPoint.Longitude);
}

RDFTargetProjection::RDFTargetProjection ()
{
	pjinit = string ("+proj=sterea +ellps=WGS84");

#if (PROJ_VERSION == 4)
	pjinit = string ("+proj=sterea");
	source = pj_init_plus (pjinit.c_str ());
	if (!source)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	source = proj_create (PJ_DEFAULT_CTX, pjinit.c_str ());
	if (!source)
	{
		cms_error ("proj_create error: %s", pjinit.c_str ());
	}
#endif
	
	srcR = _ComputeEarthRadius (source);
	srcN = _ComputeNormalisedEarthRadius (source);
	srcHeight = 0.;
	_ComputeMatrices (source, srcN, srcT1, srcS1);

#if (PROJ_VERSION == 4)
	destination = pj_init_plus (pjinit.c_str ());
	if (!destination)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	destination = proj_create (PJ_DEFAULT_CTX, pjinit.c_str ());
	if (!destination)
	{
		cms_error ("proj_create error: %s", pjinit.c_str ());
	}
#endif

	dstR =  srcR;
	dstN =  srcN;
	dstHeight = 0.;
	_ComputeMatrices (destination, dstN, dstT1, dstS1);
	Recal = false;
}

RDFTargetProjection::~RDFTargetProjection ()
{
#if (PROJ_VERSION == 4)
	if (source)
	{
		pj_free ((PJ *)source);
	}
	if (destination)
	{
		pj_free ((PJ *)destination);
	}
#else
	if (source)
	{
		proj_destroy ((PJ *)source);
	}
	if (destination)
	{
		proj_destroy ((PJ *)destination);
	}
#endif
}

void RDFTargetProjection::SetCenter (const RDFCoorGeographic & center, double height)
{
	char text[1024];
	char lat[256];
	char lon[256];

	g_ascii_dtostr (lat, sizeof (lat), center.Latitude * RAD_TO_DEG);
	g_ascii_dtostr (lon, sizeof (lon), center.Longitude * RAD_TO_DEG);
	
	sprintf (text, "%s +lat_0=%s +lon_0=%s", pjinit.c_str (), lat, lon);

#if (PROJ_VERSION == 4)
	if (source)
	{
		pj_free ((PJ *)source);
	}

	source = pj_init_plus (text);
	if (!source)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	if (source)
	{
		proj_destroy ((PJ *)source);
	}

	source = proj_create (PJ_DEFAULT_CTX, text);
	if (!source)
	{
		cms_error ("proj_create error: %s", text);
	}
#endif
	
	srcR = _ComputeEarthRadius (source);
	srcN = _ComputeNormalisedEarthRadius (source);
	srcHeight = height;
	_ComputeMatrices (source, srcN, srcT1, srcS1, height);

#if (PROJ_VERSION == 4)
	if (destination)
	{
		pj_free ((PJ *)destination);
	}
	
	destination = pj_init_plus (text);
	if (!destination)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	if (destination)
	{
		proj_destroy ((PJ *)destination);
	}
	
	destination = proj_create (PJ_DEFAULT_CTX, text);
	if (!destination)
	{
		cms_error ("proj_create error: %s", text);
	}
#endif
	
	dstR =  srcR;
	dstN =  srcN;
	dstHeight = height;
	_ComputeMatrices (destination, dstN, dstT1, dstS1, height);

	Recal = false;
}

void RDFTargetProjection::Geo2Car (const RDFCoorGeographic & geographic, RDFCoorCartesian & cartesian)
{
#if (PROJ_VERSION == 4)
	projLP lp = { geographic.Longitude, geographic.Latitude };
	projXY xy = pj_fwd (lp, (PJ *)destination);

	if (xy.u == HUGE_VAL || xy.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", geographic.Latitude * RAD_TO_DEG, 
			geographic.Longitude * RAD_TO_DEG, pj_strerrno (pj_errno));
	}

	cartesian.X = xy.u;
	cartesian.Y = xy.v;
#else
	PJ_COORD lp = proj_coord (geographic.Longitude, geographic.Latitude, 0, 0);
	PJ_COORD xy = proj_trans ((PJ *)destination, PJ_FWD, lp);

	if (xy.xy.x == HUGE_VAL || xy.xy.y == HUGE_VAL)
	{
		cms_error ("librdf", "%f %f error: %s",
			proj_todeg (geographic.Latitude),
			proj_todeg (geographic.Longitude),
			proj_errno_string (proj_errno ((PJ *) destination)));
	}

	cartesian.X = xy.xy.x;
	cartesian.Y = xy.xy.y;
#endif
}

void RDFTargetProjection::Geo2Pol (const RDFCoorGeographic & geographic, RDFCoorPolar & polar)
{
#if (PROJ_VERSION == 4)
	projLP lp = { geographic.Longitude, geographic.Latitude };
	projXY xy = pj_fwd (lp, (PJ *)destination);

	if (xy.u == HUGE_VAL || xy.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", geographic.Latitude * RAD_TO_DEG, 
			geographic.Longitude * RAD_TO_DEG, pj_strerrno (pj_errno));
	}

	rdf_xy2rt (xy.u, xy.v, polar.Rho, polar.Theta);
#else
	PJ_COORD lp = proj_coord (geographic.Longitude, geographic.Latitude, 0, 0);
	PJ_COORD xy = proj_trans ((PJ *)destination, PJ_FWD, lp);

	if (xy.xy.x == HUGE_VAL || xy.xy.y == HUGE_VAL)
	{
		cms_error ("librdf", "%f %f error: %s",
			proj_todeg (geographic.Latitude),
			proj_todeg (geographic.Longitude),
			proj_errno_string (proj_errno ((PJ *) destination)));
	}

	rdf_xy2rt (xy.xy.x, xy.xy.y, polar.Rho, polar.Theta);
#endif
}

void RDFTargetProjection::Car2Geo (const RDFCoorCartesian & cartesian, RDFCoorGeographic & geographic)
{
#if (PROJ_VERSION == 4)
	projXY xy = { cartesian.X, cartesian.Y };
	projLP lp = pj_inv (xy, (PJ *)destination);

	if (lp.u == HUGE_VAL || lp.v == HUGE_VAL)
	{
		cms_error ("%d %d error: %s", cartesian.X, 
			cartesian.Y, pj_strerrno (pj_errno));
	}

	geographic.Longitude = lp.u;
	geographic.Latitude = lp.v;
#else
	PJ_COORD xy = proj_coord (cartesian.X, cartesian.Y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *)destination, PJ_INV, xy);

	if (lp.lp.lam == HUGE_VAL || lp.lp.phi == HUGE_VAL)
	{
		ERROR ("librdf", "%d %d error: %s",
			cartesian.X, cartesian.Y,
			proj_errno_string (proj_errno ((PJ *)destination)));
	}

	geographic.Longitude = lp.lp.lam;
	geographic.Latitude = lp.lp.phi;
#endif
}

void RDFTargetProjection::Car2Pol (const RDFCoorCartesian & cartesian, RDFCoorPolar & polar)
{
	rdf_xy2rt (cartesian.X, cartesian.Y, polar.Rho, polar.Theta);
}

void RDFTargetProjection::Pol2Geo (const RDFCoorPolar & polar, RDFCoorGeographic & geographic)
{
#if (PROJ_VERSION == 4)
	projXY xy = { 0, 0 };
	rdf_rt2xy (polar.Rho, polar.Theta, xy.u, xy.v);
	projLP lp = pj_inv (xy, (PJ *)destination);

	if (lp.u == HUGE_VAL || lp.v == HUGE_VAL)
	{
		cms_error ("%f %f error: %s", polar.Rho, 
			polar.Theta, pj_strerrno (pj_errno));
	}

	geographic.Longitude = lp.u;
	geographic.Latitude = lp.v;
#else
	PJ_COORD xy = proj_coord (0, 0, 0, 0);
	rdf_rt2xy (polar.Rho, polar.Theta, xy.xy.x, xy.xy.y);
	PJ_COORD lp = proj_trans ((PJ *)destination, PJ_INV, xy);

	if (lp.lp.lam == HUGE_VAL || lp.lp.phi == HUGE_VAL)
	{
		cms_error ("librdf", "%f %f error: %s",
			polar.Rho, polar.Theta,
			proj_errno_string (proj_errno ((PJ *)destination)));
	}

	geographic.Longitude = lp.lp.lam;
	geographic.Latitude = lp.lp.phi;
#endif
}

void RDFTargetProjection::Pol2Car (const RDFCoorPolar & polar, RDFCoorCartesian & cartesian)
{
	rdf_rt2xy (polar.Rho, polar.Theta, cartesian.X, cartesian.Y);
}

/* prevod z lokalnich souradnic (rho, theta, height), tj. sikma dalka, azimut a vyska, 
 * do lokalnich kartezskych souradnic (x, y, z)
 * oba souradne systemy se stredem v poloze anteny radaru
 */

void RDFTargetProjection::Rth2Car (double rho, double theta, double height, RDFCoorCartesian & cartesian)
{
	// transformace lokalnich polarnich do lokalnich sferickych souradnic
	double psi;		// elevace cile oproti rovine radaru
	_rth2spherical (rho, theta, height, srcHeight, psi);
	// transformace lokálních sferickych do lokalnich kartezskych souradnic
	double x, y, z;
	rdf_spherical2xyz (rho, theta, psi, x, y, z, 0);	// deviation zatím 0
	cartesian.X = x;
	cartesian.Y = y;
}

/* prevod z lokalnich souradnic (rho, theta, height), tj. sikma dalka, azimut a vyska, 
 * do geocentrickych zemepisnych souradnic (lattitude, longitude) ve WGS84, tj. zemepisna sirka a delka
 */
void RDFTargetProjection::Rth2Geo (double rho, double theta, double height, RDFCoorGeographic &geographic)
{
	// transformace lokalnich polarnich do lokalnich sferickych souradnic
	double psi;		// elevace cile oproti rovine radaru
	_rth2spherical (rho, theta, height, srcHeight, psi);
	// transformace lokálních sferickych do lokalnich kartezskych souradnic
	double x, y, z;
	rdf_spherical2xyz (rho, theta, psi, x, y, z, 0);	// deviation zatím 0
	// transformace z lokalnich kartezskych do geocentrickych zemepisnych souradnic
	double xg, yg, zg;
	xg = srcS1[0][0] * x + srcS1[1][0] * y + srcS1[2][0] * z + srcT1[0];
	yg = srcS1[0][1] * x + srcS1[1][1] * y + srcS1[2][1] * z + srcT1[1];
	zg = srcS1[0][2] * x + srcS1[1][2] * y + srcS1[2][2] * z + srcT1[2];
	double Hn = height / srcN;
	double tanL = (zg / hypot (xg, yg)) * ((1 + Hn) / ((1 - WGS84_ECCENTRICITY2) + Hn));
	geographic.Latitude = atan (tanL);
	geographic.Longitude = atan2 (yg, xg);
}

/* prevod z geocentrickych zemepisnych souradnic (lattitude, longitude) ve WGS84, tj. zemepisna sirka a delka, 
 * do lokalnich souradnic (rho, theta, height), tj. sikma dalka, azimut a vyska
 */
void RDFTargetProjection::Geo2Rth (RDFCoorGeographic &geographic, double height, double &rho, double &theta)
{
	double N;             /*  normalised Earth radius (normal distance) at aircraft position  */
	double Sin_Lat;       /*  sin(Latitude)  */
	double Sin2_Lat;      /*  Square of sin(Latitude)  */
	double Sin_Lon;       /*  sin(Longitude)  */
	double Cos_Lat;       /*  cos(Latitude)  */
	double Cos_Lon;       /*  cos(Longitude)  */
	double xg, yg, zg;
	double x, y, z;

	// transformace z geocentrickych zemepisnych do lokalnich kartezskych souradnic
	Sin_Lat = sin (geographic.Latitude);
	Sin2_Lat = Sin_Lat * Sin_Lat;
	Cos_Lat = cos (geographic.Latitude);
	Sin_Lon = sin (geographic.Longitude);
	Cos_Lon = cos (geographic.Longitude);
	N = WGS84_SEMIMAJOR_AXIS / sqrt (1.0e0 - WGS84_ECCENTRICITY2 * Sin2_Lat);
    xg = (N + height) * Cos_Lat * Cos_Lon;
	yg = (N + height) * Cos_Lat * Sin_Lon;
	zg = ((N * (1 - WGS84_ECCENTRICITY2)) + height) * Sin_Lat;
	x = srcS1[0][0] * (xg - srcT1[0]) + srcS1[0][1] * (yg - srcT1[1]) + srcS1[0][2] * (zg - srcT1[2]);
	y = srcS1[1][0] * (xg - srcT1[0]) + srcS1[1][1] * (yg - srcT1[1]) + srcS1[1][2] * (zg - srcT1[2]);
	z = srcS1[2][0] * (xg - srcT1[0]) + srcS1[2][1] * (yg - srcT1[1]) + srcS1[2][2] * (zg - srcT1[2]);

	// transformace lokalnich kartezskych souradnic do lokalnich polarnich
	rdf_xyz2rt (x, y, z, rho, theta);
}

void RDFTargetProjection::SetSourceDestination (const RDFCoorGeographic &src, const RDFCoorGeographic &dst, double srcheight, double dstheight)
{
	char text[1024];
	char lat[256];
	char lon[256];

	g_ascii_dtostr (lat, sizeof (lat), src.Latitude * RAD_TO_DEG);
	g_ascii_dtostr (lon, sizeof (lon), src.Longitude * RAD_TO_DEG);

	sprintf (text, "%s +lat_0=%s +lon_0=%s", pjinit.c_str (), lat, lon);

#if (PROJ_VERSION == 4)
	if (source)
	{
		pj_free ((PJ *)source);
	}
	
	source = pj_init_plus (text);
	if (!source)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	if (source)
	{
		proj_destroy ((PJ *)source);
	}
	
	source = proj_create (PJ_DEFAULT_CTX, text);
	if (!source)
	{
		cms_error ("proj_create error: %s", text);
	}
#endif

	srcR = _ComputeEarthRadius (source);
	srcN = _ComputeNormalisedEarthRadius (source);
	srcHeight = srcheight;
	_ComputeMatrices (source, srcN, srcT1, srcS1, srcHeight);

	g_ascii_dtostr (lat, sizeof (lat), dst.Latitude * RAD_TO_DEG);
	g_ascii_dtostr (lon, sizeof (lon), dst.Longitude * RAD_TO_DEG);

	sprintf (text, "%s +lat_0=%s +lon_0=%s", pjinit.c_str (), lat, lon);

#if (PROJ_VERSION == 4)
	if (destination)
	{
		pj_free ((PJ *)destination);
	}
	
	destination = pj_init_plus (text);
	if (!destination)
	{
		cms_error ("pj_init_plus error: %s", pj_strerrno (pj_errno));
	}
#else
	if (destination)
	{
		proj_destroy ((PJ *)destination);
	}
	
	destination = proj_create (PJ_DEFAULT_CTX, text);
	if (!destination)
	{
		cms_error ("pj_init_plus error: %s", text);
	}
#endif

	dstR = _ComputeEarthRadius (destination);
	dstN = _ComputeNormalisedEarthRadius (destination);
	dstHeight = dstheight;
	_ComputeMatrices (destination, dstN, dstT1, dstS1, dstHeight);

	Recal = (src.Latitude != dst.Latitude || src.Longitude != dst.Longitude);
}

void RDFTargetProjection::RecalTarget (int &x, int &y)
{
	if (!Recal)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { (double) x, (double) y };
	projLP lp = pj_inv (xy, (PJ *)source);
	xy = pj_fwd (lp, (PJ *)destination);
	x = (int) rint (xy.u);
	y = (int) rint (xy.v);
#else
	PJ_COORD xy = proj_coord (x, y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *)source, PJ_INV, xy);
	xy = proj_trans ((PJ *)destination, PJ_FWD, lp);
	x = (int) rint (xy.xy.x);
	y = (int) rint (xy.xy.y);
#endif
}

void RDFTargetProjection::RecalTarget (double &x, double &y)
{
	if (!Recal)
	{
		return;
	}

#if (PROJ_VERSION == 4)
	projXY xy = { x, y };
	projLP lp = pj_inv (xy, (PJ *)source);
	xy = pj_fwd (lp, (PJ *)destination);
	x = xy.u;
	y = xy.v;
#else
	PJ_COORD xy = proj_coord (x, y, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *)source, PJ_INV, xy);
	xy = proj_trans ((PJ *)destination, PJ_FWD, lp);
	x = xy.xy.x;
	y = xy.xy.y;
#endif
}

/* vypocet normalizovaneho polomeru Zeme v miste stredu projekce (source nebo destination)
 * s parametry WGS84 elipsoidu
 */
double RDFTargetProjection::_ComputeEarthRadius (void * P)
{
	double R;             /*  Earth radius at location  */
	double Sin_Lat;       /*  sin(Latitude)  */
	double Sin2_Lat;      /*  Square of sin(Latitude)  */
	
#if (PROJ_VERSION == 4)
	Sin_Lat = sin (((PJ *)P)->phi0);
#else
	PJ_COORD xy = proj_coord (0, 0, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *) P, PJ_INV, xy);
	Sin_Lat = sin (lp.lp.phi);
#endif
	Sin2_Lat = Sin_Lat * Sin_Lat;
	R = WGS84_SEMIMAJOR_AXIS  * (1 - WGS84_ECCENTRICITY2) / sqrt (pow (1.0e0 - WGS84_ECCENTRICITY2 * Sin2_Lat, 3));
	return R;
}

/* vypocet normalizovaneho polomeru Zeme (kolme vzdalenosti) v miste stredu projekce (source nebo destination)
 * jedna se o vzdalenost od povrchu k ose Z ve smeru (prodlouzeni) kolmice k elipsoidu v miste stredu projekce
 * s parametry WGS84 elipsoidu
 */
double RDFTargetProjection::_ComputeNormalisedEarthRadius (void * P)
{
	double N;             /*  normalised Earth radius (normal distance) at location  */
	double Sin_Lat;       /*  sin(Latitude)  */
	double Sin2_Lat;      /*  Square of sin(Latitude)  */
	
#if (PROJ_VERSION == 4)
	Sin_Lat = sin (((PJ *)P)->phi0);
#else
	PJ_COORD xy = proj_coord (0, 0, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *) P, PJ_INV, xy);
	Sin_Lat = sin (lp.lp.phi);
#endif
	Sin2_Lat = Sin_Lat * Sin_Lat;
	N = WGS84_SEMIMAJOR_AXIS / sqrt (1.0e0 - WGS84_ECCENTRICITY2 * Sin2_Lat);
	return N;
}

/* vypocet matic posunu a rotace pro prevod z lokalniho kartezskeho souradneho 
 * systemu v miste stredu projekce (source nebo destination) do geocentrickeho kartezskeho souradneho systemu a zpet
 * s parametry WGS84 elipsoidu
 */
void RDFTargetProjection::_ComputeMatrices (void * P, double N, double *t, double (*s)[3], double height)
{
	double Sin_Lat;       /*  sin(Latitude)  */
	//double Sin2_Lat;      /*  Square of sin(Latitude)  */
	double Sin_Lon;       /*  sin(Longitude)  */
	double Cos_Lat;       /*  cos(Latitude)  */
	double Cos_Lon;       /*  cos(Longitude)  */

#if (PROJ_VERSION == 4)
	Sin_Lat = sin (((PJ *)P)->phi0);
	Cos_Lat = cos (((PJ *)P)->phi0);
	//Sin2_Lat = Sin_Lat * Sin_Lat;
	Sin_Lon = sin (((PJ *)P)->lam0);
	Cos_Lon = cos (((PJ *)P)->lam0);
#else
	PJ_COORD xy = proj_coord (0, 0, 0, 0);
	PJ_COORD lp = proj_trans ((PJ *) P, PJ_INV, xy);
	Sin_Lat = sin (lp.lp.phi);
	Cos_Lat = cos (lp.lp.phi);
	Sin_Lon = sin (lp.lp.lam);
	Cos_Lon = cos (lp.lp.lam);
#endif
    t[0] = (N + height) * Cos_Lat * Cos_Lon;
	t[1] = (N + height) * Cos_Lat * Sin_Lon;
	t[2] = ((N * (1 - WGS84_ECCENTRICITY2)) + height) * Sin_Lat;

	s[0][0] = -Sin_Lon;
	s[0][1] = Cos_Lon;
	s[0][2] = 0;
	s[1][0] = - Sin_Lat * Cos_Lon;
	s[1][1] = - Sin_Lat * Sin_Lon;
	s[1][2] = Cos_Lat;
	s[2][0] = Cos_Lat * Cos_Lon;
	s[2][1] = Cos_Lat * Sin_Lon;
	s[2][2] = Sin_Lat;
}

/* prevod z lokalnich souradnic (rho, theta, height), tj. sikma dalka, azimut a vyska, do 
 * lokalnich polarnich (sferickych) souradnic (rho, theta, psi), tj. sikma dalka, azimut a elevace
 * oba souradne systemy se stredem v poloze anteny radaru
 */
void RDFTargetProjection::_rth2spherical (double rho, double theta, double height, double radar_height, double &psi)
{
	double a = (2 * srcR * (height - radar_height) + height * height - radar_height * radar_height - rho * rho) / (2 * rho * (srcR + radar_height));
	// a musi byt v <-1, 1> jinak asin () skonci chybou
	if (fabs (a) <= 1.)
	{
		psi = asin (a);
	}
	else
	{
		psi = 0.;
	}
}

