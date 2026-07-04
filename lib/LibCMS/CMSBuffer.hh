#ifndef _CMS_BUFFER_HH
#define _CMS_BUFFER_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_buffer.h"

class CMSRoundBuffer
{
  protected:
	cms_round_buffer_t * buffer;

  public:
	CMSRoundBuffer (std::string name, unsigned size, unsigned maxsize, int flags)
	{
		buffer = cms_round_buffer_new (name.c_str (), maxsize, flags);
	}
	
	~CMSRoundBuffer ()
	{
		cms_round_buffer_delete (buffer);
	}

	bool IsEmpty ()
	{
		return cms_round_buffer_is_empty (buffer);
	}

	void SetFlags (int flags)
	{
		return cms_round_buffer_set_flags (buffer, flags);
	}
	
	void Clear ()
	{
		return cms_round_buffer_clear (buffer);
	}

	unsigned GetFillSpace ()
	{
		return cms_round_buffer_get_fill_space (buffer);
	}
	
	unsigned GetMessages ()
	{
		return cms_round_buffer_get_messages (buffer);
	}
	
	int Write (const void * data, unsigned size)
	{
		return cms_round_buffer_write (buffer, data, size);
	}

	int Read (void * data, unsigned size)
	{
		return cms_round_buffer_read (buffer, data, size);
	}
};

class CMSRoundBlockBuffer : public CMSRoundBuffer
{
  public:
	CMSRoundBlockBuffer (std::string name, unsigned size, unsigned maxsize, int flags)
	: CMSRoundBuffer (name, size, maxsize, flags)
	{
		buffer->type = CMS_ROUND_BUFFER_TYPE_BLOCK;
	}
	
	~CMSRoundBlockBuffer ()
	{
	}
};

#endif /* _CMS_BUFFER_HH */
