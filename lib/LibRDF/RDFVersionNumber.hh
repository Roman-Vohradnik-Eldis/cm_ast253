#ifndef _RDF_VERSION_NUMBER_H
#define _RDF_VERSION_NUMBER_H

#include "RDFHeader.hh"
#include "RDFTargetStruct.hh"

struct RDFCategoryVersion
{
	uint8_t Category;
	uint8_t MainVersion;
	uint8_t SubVersion;

	RDFCategoryVersion ();
};

struct RDFVersionNumber
{
	RDFTime Time;
	unsigned char SAC;
	unsigned char SIC;
	std::vector<struct RDFCategoryVersion> CategoryVersion;

	RDFVersionNumber ();
	~RDFVersionNumber ();

  protected:
	void init ();
};

#endif /* _RDF_VERSION_NUMBER_H */
