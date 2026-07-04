#include "RDFAsterix.hh"
#include "CMSDebug.hh"

int field2signed (const unsigned char *const field, unsigned bits)
{
  if (bits > 32)
  {
    cms_error ("internal error");
    return 0;
  }

  if (bits > 24)
  {
    return ((unsigned)field[3]) | (((unsigned)field[2]) << 8) | (((unsigned)field[1]) << 16) | ((((int)((char)field[0])) << (32 - bits + 24)) >> (32 - bits));
  }

  if (bits > 16)
  {
    return ((unsigned)field[2]) | (((unsigned)field[1]) << 8) | ((((int)((char)field[0])) << (24 - bits + 24)) >> (32 - bits));
  }

  if (bits > 8)
  {
    return ((unsigned)field[1]) | ((((int)((char)field[0])) << (16 - bits + 24)) >> (32 - bits));
  }


  return ((((int)((char)field[0])) << (8 - bits + 24)) >> (32 - bits));
}

unsigned field2unsigned (const unsigned char *const field, unsigned bits)
{
  if (bits > 32)
  {
    cms_error ("internal error");
    return 0;
  }

  if (bits > 24)
  {
    return ((unsigned)field[3]) | (((unsigned)field[2]) << 8) | (((unsigned)field[1]) << 16) | ((((unsigned)field[0]) << (32 - bits + 24)) >> (32 - bits));
  }

  if (bits > 16)
  {
    return ((unsigned)field[2]) | (((unsigned)field[1]) << 8) | ((((unsigned)field[0]) << (24 - bits + 24)) >> (32 - bits));
  }

  if (bits > 8)
  {
    return ((unsigned)field[1]) | ((((unsigned)field[0]) << (16 - bits + 24)) >> (32 - bits));
  }


  return ((((unsigned)field[0]) << (8 - bits + 24)) >> (32 - bits));
}

unsigned asterix_decode_mode123 (unsigned code)
{
  return ((((code) >> 9) & 7) * 1000 + (((code) >> 6) & 7) * 100 + (((code) >> 3) & 7) * 10 + ((code) & 7));
}

unsigned asterix_encode_mode123 (unsigned value)
{
  return ((value % 10) & 7) | ((((value / 10) % 10) & 7) << 3) | ((((value / 100) % 10) & 7) << 6) | ((((value / 1000) % 10) & 7) << 9);
}

unsigned asterix_decode_modec (unsigned code)
{
  int             a, b, c, d;

  a = b = c = d = 0;

  if (code & 0x001)
    d += 4;
  if (code & 0x002)
    b += 4;
  if (code & 0x004)
    d += 2;
  if (code & 0x008)
    b += 2;
  if (code & 0x010)
    d += 1;
  if (code & 0x020)
    b += 1;
  if (code & 0x040)
    a += 4;
  if (code & 0x080)
    c += 4;
  if (code & 0x100)
    a += 2;
  if (code & 0x200)
    c += 2;
  if (code & 0x400)
    a += 1;
  if (code & 0x800)
    c += 1;

  return (a * 01000 + b * 0100 + c * 010 + d);
}

unsigned asterix_encode_modec (unsigned value)
{
  int             a, b, c, d, code;

  d = value % 010;
  value /= 010;
  c = value % 010;
  value /= 010;
  b = value % 010;
  value /= 010;
  a = value % 010;
  code = 0;

  if (d & 4)
    code |= 0x001;
  if (b & 4)
    code |= 0x002;
  if (d & 2)
    code |= 0x004;
  if (b & 2)
    code |= 0x008;
  if (d & 1)
    code |= 0x010;
  if (b & 1)
    code |= 0x020;
  if (a & 4)
    code |= 0x040;
  if (c & 4)
    code |= 0x080;
  if (a & 2)
    code |= 0x100;
  if (c & 2)
    code |= 0x200;
  if (a & 1)
    code |= 0x400;
  if (c & 1)
    code |= 0x800;

  return code;
}

char asterix_decode_char (unsigned char code)
{
  if (code == 0)
    return 0x20;
  if (code & 0x20)
    return code;
  return (code | 0x40);
}

unsigned char asterix_encode_char (char ch)
{
  if (ch & 0x20)
    return ch;
  return (ch & 0x3f);
}

void asterix_decode_callsign (const unsigned char *code, string & callsign)
{
  callsign.clear ();

  callsign.push_back (asterix_decode_char ((code[0] >> 2) & 0x3F));
  callsign.push_back (asterix_decode_char (((code[0] << 4) & 0x30) | ((code[1] >> 4) & 0x0F)));
  callsign.push_back (asterix_decode_char (((code[1] << 2) & 0x3C) | ((code[2] >> 6) & 0x03)));
  callsign.push_back (asterix_decode_char (code[2] & 0x3F));
  callsign.push_back (asterix_decode_char (((code[3] >> 2) & 0x3F)));
  callsign.push_back (asterix_decode_char (((code[3] << 4) & 0x30) | ((code[4] >> 4) & 0x0F)));
  callsign.push_back (asterix_decode_char (((code[4] << 2) & 0x3C) | ((code[5] >> 6) & 0x03)));
  callsign.push_back (asterix_decode_char (code[5] & 0x3F));

  while (callsign.size () && g_ascii_isspace (callsign[callsign.size () - 1]))
  {
    callsign.resize (callsign.size () - 1);
  }
}

void asterix_encode_callsign (string callsign, unsigned char *code)
{
  callsign.resize (8, 0);

  for (unsigned i = 0; i < callsign.size (); i++)
  {
    if (!g_ascii_isalnum (callsign[i]))
    {
      callsign[i] = 32;
    }
  }

  code[0] = ((asterix_encode_char (callsign[0]) & 0x3F) << 2) | ((asterix_encode_char (callsign[1]) & 0x30) >> 4);
  code[1] = ((asterix_encode_char (callsign[1]) & 0x0F) << 4) | ((asterix_encode_char (callsign[2]) & 0x3C) >> 2);
  code[2] = ((asterix_encode_char (callsign[2]) & 0x03) << 6) | (asterix_encode_char (callsign[3]) & 0x3F);
  code[3] = ((asterix_encode_char (callsign[4]) & 0x3F) << 2) | ((asterix_encode_char (callsign[5]) & 0x30) >> 4);
  code[4] = ((asterix_encode_char (callsign[5]) & 0x0F) << 4) | ((asterix_encode_char (callsign[6]) & 0x3C) >> 2);
  code[5] = ((asterix_encode_char (callsign[6]) & 0x03) << 6) | (asterix_encode_char (callsign[7]) & 0x3F);
}

bool CheckCallSign (const char *callsign, unsigned length)
{
  bool            empty = true;

  for (unsigned i = 0; i < length; i++)
  {
    if (callsign[i] == 0)
    {
      return !empty;
    } else if (callsign[i] > 32)
    {
      empty = false;
    } else if (callsign[i] < 32)
    {
      cms_warning ("wrong characters in callsign");
      return false;
    }
  }

  return !empty;
}

bool AsterixGetSACSIC (const void *data, unsigned size, unsigned char cat, unsigned char &sac, unsigned char &sic)
{
  unsigned char  *msg;
  unsigned char  *field;
  unsigned        len;

  if (!data || size > 4096)
  {
    cms_error ("wrong packet %x %u", data, size);
    return false;
  }

  msg = (unsigned char *)data;

  if (msg[0] != cat)
  {
    cms_error ("unknown category %u", msg[0]);

    return false;
  }

  len = (((unsigned)msg[1]) << 8) | msg[2];
  if (len < size)
  {
    cms_warning ("wrong length %u %u", len, size);
    size = len;
  } else if (len > size)
  {
    cms_error ("wrong length %u %u", len, size);
    return false;
  }

  field = msg + 3;
  while (*field & 1)
  {
    if (++field >= msg + len)
    {
      cms_error ("end of message");
      return false;
    }
  }

  field++;
  sac = field[0];
  sic = field[1];

  return true;
}
