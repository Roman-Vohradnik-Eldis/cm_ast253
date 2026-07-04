#ifndef BLOB_HH_
#define BLOB_HH_

#include <vector>
#include <string>
#include "weak_string.hh"

//////////////////////////////////////////////////////////////////////

// Komprese: výsledek je vektor bajtů

//////////////////////////////////////////////////////////////////////

// Plati: binary_deserialize binary_serialize = id

std::string binary_serialize(std::vector<uint8_t> const &data);

std::vector<uint8_t> binary_deserialize(WeakString s);

//////////////////////////////////////////////////////////////////////

// Plati unescape_special_chars escape_special_chars = id

// Např: \par "hello"
//
// To převede na: \\par \"hello\"
//
// std::string escape_special_chars(WeakString ws);

// std::string escape_special_chars_and_advance(WeakString &ws, size_t limit);

// std::string unescape_special_chars(WeakString ws);

#endif // BLOB_HH_
