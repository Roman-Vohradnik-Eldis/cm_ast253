#include "cm_utils.hh"
#include "cm_test.hh"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

struct FD
{
    int fd;
    ~FD()
    {
        if (fd > 0)
            close(fd);
    }
};

void read_whole_file_to_buffer(std::string const &name, std::vector<uint8_t> &buffer)
{
    FD fd{open(name.c_str(), O_RDONLY)};
    CHECK(fd.fd > 0, "Failed to open file '%s': %s", name.c_str(), strerror(errno));
    
    for (;;)
    {
        size_t prev_size = buffer.size();
        size_t chunk_size = 4096;
        buffer.resize(prev_size + chunk_size);
        ssize_t len = read(fd.fd, buffer.data() + prev_size, chunk_size);
        if (len == 0)
        {
            // End of file
            buffer.resize(prev_size);
            break;
        }
        else if (len > 0)
        {
            // Good
            buffer.resize(prev_size + len);
        }
        else
        {
            // Error
            int local_errno = errno;
            buffer.resize(prev_size);
            CHECK(false, "Failed to read '%s': %s", name.c_str(), strerror(local_errno));
        }
    }
}

std::vector<uint8_t> read_whole_file(std::string const &name)
{
    std::vector<uint8_t> buffer;
    read_whole_file_to_buffer(name, buffer);
    return buffer;
}

std::vector<uint8_t> compute_md5(View<uint8_t> bytes)
{
    GChecksumType type = G_CHECKSUM_MD5;
    GChecksum *ch = g_checksum_new(type);
    CHECK(ch, "g_checksum_new failed");
    g_checksum_update(ch, bytes.begin(), bytes.size());

    std::vector<uint8_t> digest(g_checksum_type_get_length(type));
    size_t len = digest.size();
    g_checksum_get_digest(ch, digest.data(), &len);
    assert(len == digest.size());
    g_checksum_free(ch);
    return digest;
}

std::string compute_md5_string(View<uint8_t> bytes)
{
    std::vector<uint8_t> digest = compute_md5(bytes);
    gchar *s = g_base64_encode(digest.data(), digest.size());
    if (!s)
        throw std::runtime_error("base64_encode failed");
    std::string str(s);
    g_free(s);
    return str;
}

std::string replace_nonprintable(View<uint8_t> str)
{
    std::string s(str.begin(), str.end());
    for (char &c : s)
    {
        if (!isgraph(c) && c != ' ')
            c = '?';
    }
    return s;
}

static char to_hex(unsigned c)
{
    static char s[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    return s[c];
}

std::string str_to_hex(View<uint8_t> bytes, std::string const &separator)
{
    std::string str;
    bool first = true;
    for (uint8_t c : bytes)
    {
        if (!first)
            str += separator;
        first = false;
        str.push_back(to_hex(c / 16u));
        str.push_back(to_hex(c % 16u));
    }
    return str;
}

std::string number_to_hex_string(unsigned long long value, size_t length)
{
    if (length % 2 != 0 || length == 0)
        throw std::runtime_error("number_to_hex_string: bad length");
    std::string str(length, '0');
    size_t pos = length;
    while (value != 0)
    {
        pos -= 2;
        if (pos == 0)
            throw std::runtime_error("number_to_hex_string: value too big");
        uint8_t n = value % 256;
        value /= 256;
        str[pos] = to_hex(n / 16);
        str[pos + 1] = to_hex(n % 16);
    }
    return str;
}

void push_hex_number(std::vector<uint8_t> &v, unsigned long long value, size_t length)
{
    std::string s = number_to_hex_string(value, length);
    // printf("Pushing %llu -> %s\n", value, s.c_str());
    v.insert(v.end(), s.begin(), s.end());
}

uint8_t hex_char_to_number(uint8_t c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    else
        throw std::runtime_error("Failed to read hex char");
}

// TODO: Handle overflow
unsigned long long read_hex_number(View<uint8_t> bytes)
{
    if (bytes.size() % 2 != 0)
        throw std::runtime_error("invalid length");
    unsigned long long result = 0;
    for (size_t i = 0; i < bytes.size(); i += 2)
    {
        uint8_t c0 = bytes[i];
        uint8_t c1 = bytes[i + 1];

        result *= 256;
        result += 16 * hex_char_to_number(c0) + hex_char_to_number(c1);
    }
    return result;
}

void test_cm_utils()
{
    CM_TEST_EQ(number_to_hex_string(0x12AB, 6), "0012AB");
    std::string text("0012AB");
    CM_TEST_EQ(read_hex_number(view_bytes(text.data(), text.data() + text.size())), 0x12ABU);
    printf("test cm_utils: OK\n");
}
