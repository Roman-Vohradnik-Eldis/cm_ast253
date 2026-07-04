#include "blob.hh"
#include <stdexcept>
#include <glib.h>
#include <sstream>
#include <cassert>

//////////////////////////////////////////////////////////////////////

std::string binary_serialize(std::vector<uint8_t> const &data)
{
    // TODO: Tady se to musi z Glibu prekopirovat...
    //
    // NOTE: V připadě, že to
    // selže, v manualu nic nepíšou, tak to aspon testujeme na NULL.
    gchar *s = g_base64_encode(data.data(), data.size());
    if (!s)
        throw std::runtime_error("binary_serialize failed");
    std::string str(s);
    g_free(s);
    return str;
}

std::vector<uint8_t> binary_deserialize(WeakString s)
{
    // TODO: Tady se to musi z Glibu prekopirovat...
    //
    // NOTE: V připadě, že to selže, v manualu nic nepíšou, tak to aspoň
    // testujeme na NULL.
    gsize out_len;
    guchar *bin = g_base64_decode(s.str().c_str(), &out_len);
    if (!bin)
        throw std::runtime_error("binary_deserialize failed");
    std::vector<uint8_t> result(bin, bin + out_len);
    g_free(bin);
    return result;
}

//////////////////////////////////////////////////////////////////////
