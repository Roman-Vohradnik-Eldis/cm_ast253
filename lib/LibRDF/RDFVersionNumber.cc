#include "RDFVersionNumber.hh"

RDFCategoryVersion::RDFCategoryVersion ()
{
	Category = 0;
	MainVersion = 0;
	SubVersion = 0;
}

RDFVersionNumber::RDFVersionNumber ()
{
	init ();
}

RDFVersionNumber::~RDFVersionNumber ()
{
}

void RDFVersionNumber::init ()
{
	Time.init ();
	SAC = 0;
	SIC = 0;
	CategoryVersion.clear ();
}
