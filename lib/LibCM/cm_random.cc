
#include "cm_debug.hh"
#include "cm_random.hh"

#include <assert.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <random>

void cm_get_random_fallback(uint8_t *buffer, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        buffer[i] = rand();
}

template <typename Read>
bool read_size(uint8_t *buffer, size_t size, Read &&read)
{
    size_t len = 0;
    while (len < size)
    {
        ssize_t result = read(buffer + len, size - len);
        if (result < 0)
        {
            return false;
        }
        len += result;
    }
    return true;
}

static void handle_error(uint8_t *buffer, size_t size, char const *msg)
{
    int err = errno;
    fprintf(stderr, "%s: %s", msg, strerror(err));
    assert(false);
    cm_get_random_fallback(buffer, size);
}

void cm_get_random_urandom(uint8_t *buffer, size_t size)
{
    static int fd = open("/dev/urandom", O_RDONLY);
    if (fd > 0)
    {
        bool ok = read_size(buffer, size, [](uint8_t *b, size_t n){return read(fd, b, n);});
        if (!ok)
        {
            handle_error(buffer, size, "Failed to read /dev/urandom");
        }
    }
    else
    {
        handle_error(buffer, size, "Failed to open /dev/urandom");
    }
}

// Tohle na slackware 14.2 neni, tak to nakonec neni pouzite. Pouzil
// jsem misto toho /dev/urandom
//

#if __GLIBC__ > 2 || __GLIBC_MINOR__ > 24

#include <sys/random.h>

void cm_get_random_linux(uint8_t *buffer, size_t size)
{
    bool ok = read_size(buffer, size, [](uint8_t *b, size_t n){return getrandom(b, n, 0);});
    if (!ok)
    {
        handle_error(buffer, size, "Failed to call getrandom");
    }
}

#endif

void cm_get_random(uint8_t *buffer, size_t size)
{
    cm_get_random_urandom(buffer, size);
}
