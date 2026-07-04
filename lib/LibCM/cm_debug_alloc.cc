#ifdef USE_DEBUG_MALLOC

#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <atomic>
#include <string.h>
#include <map>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define CHECK(cond) do { if (!(cond)) abort(); } while (0)

void copy_file(char const *from, char const *to)
{
    int fin = open(from, O_RDONLY);
    CHECK(fin > 0);
    int fout = creat(to, 0644);
    CHECK(fout > 0);
    char buffer[512];
    for (;;)
    {
        ssize_t len = read(fin, buffer, sizeof buffer);
        if (len <= 0)
            break;
        char *begin = buffer;
        do
        {
            ssize_t w = write(fout, begin, len);
            if (w < 0)
                abort();
            begin += w;
            len -= w;
        }
        while (len > 0);
    }
    close(fin);
    close(fout);
}

void *cm_debug_main_addr = NULL;

#define BT_GET(N) (__builtin_frame_address(N) ? __builtin_return_address(N) : 0)

// Tohle si muze nastavit uzivatel
#define BT_MAKE(bt, bt_size)                                            \
    void *bt[4];                                                        \
    size_t bt_size = 1;                                                 \
    bt[0] = __builtin_extract_return_addr(__builtin_return_address(0)); \
    if (cm_debug_main_addr && __builtin_frame_address(0) > cm_debug_main_addr) \
    {                                                                   \
        bt_size = 3;                                                    \
        bt[1] = __builtin_extract_return_addr(__builtin_return_address(1)); \
        bt[2] = __builtin_extract_return_addr(__builtin_return_address(2)); \
    }

class Storage
{
    char *storage;
    char *mem_begin;
    size_t storage_size;

public:
    Storage(size_t s)
        : storage_size(s)
    {
        storage = (char *)mmap(NULL, storage_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        CHECK(storage != MAP_FAILED);
        mem_begin = storage;
    }

    void *alloc(size_t alignment, size_t size)
    {
        if ((size_t)mem_begin % alignment != 0)
            mem_begin += alignment - ((size_t)mem_begin % alignment);
        char *ptr = mem_begin;
        mem_begin += size;
        CHECK(mem_begin <= storage + storage_size);
        return ptr;
    }

    bool is_owned(void *ptr)
    {
        return ptr >= storage && ptr < mem_begin;
    }
};

static size_t hash(void * key, size_t max_elements)
{
    static_assert(sizeof(key) == sizeof(size_t), "sizeof(void*) != sizeof(size_t)");
    size_t pos = 0;
    memcpy(&pos, &key, 8);
    return (pos * 699343) % max_elements;
}

// static size_t hash(int key, size_t max_elements)
// {
//     return (key * 699343) % max_elements;
// }

template <typename Key, typename T>
class Table
{
public:
    struct Node
    {
        void *key;
        bool present;
        T value;
    };

private:
    size_t max_elements;
    Storage storage;
    Node *array;
    size_t n_elements;
    
public:
    Table(size_t n)
        : max_elements(n),
          storage(max_elements * sizeof(Node)),
          array((Node*)storage.alloc(1, max_elements * sizeof(storage))),
          n_elements(0)
    {
    }

    T &insert_new(Key key)
    {
        CHECK(n_elements < max_elements);
        Node &n = find_node(key);
        CHECK(!n.present);
        n.key = key;
        n.present = true;
        ++n_elements;
        return n.value;
    }
    
    T &find_or_create(Key key)
    {
        CHECK(n_elements < max_elements);
        Node &n = find_node(key);
        if (!n.present)
        {
            n.key = key;
            n.present = true;
            ++n_elements;
        }
        return n.value;
    }
    T &find_or_abort(Key key)
    {
        Node &n = find_node(key);
        CHECK(n.present);
        CHECK(n.key == key);
        return n.value;
    }
    size_t mem_size() const { return max_elements; }

    size_t size() const { return n_elements; }
    Key *key_at(size_t n) { return array[n].present ? &array[n].key : 0; }
    T *value_at(size_t n) { return array[n].present ? &array[n].value : 0; }

private:
    Node &find_node(Key key)
    {
        size_t pos = hash(key, max_elements);
        for (;;)
        {
            if (array[pos].key == key || !array[pos].present)
                return array[pos];
            pos = (pos + 1) % max_elements;
        }
    }
};

struct PtrInfo
{
    void *caller;
    size_t size;
};

struct CallerInfo
{
    size_t times;
    size_t allocated;
    size_t accumulator;
    char name[200];
};

struct ThreadInfo
{
    void *stack_bottom;
};

//////////////////////////////////////////////////////////////////////

// Tohle vsechno musi zero-initialized
static pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

static int out = 0;

// static size_t allocation_count;
static size_t total_times = 0;
static size_t total_allocated = 0;
static size_t total_accumulator = 0;
static size_t invalid_free_count = 0;
static size_t invalid_realloc_count = 0;
static bool created_map_file = 0;

// #define GET_BACKTRACE void *bt[3] = {0, 0, 0 }; backtrace(bt, 3);
// #define __builtin_extract_return_addr(__builtin_return_address(0)) void *ret_addr]

// #define GET_BACKTRACE (void)0;
// #define __builtin_extract_return_addr(__builtin_return_address(0)) void *ret_addr_return_addr(__builtin_return_address(0))

static Storage &get_main_storage()
{
    static Storage storage(1000*1000*1000L);
    return storage;
}

static Table<void *, CallerInfo> &get_caller_info_table()
{
    static Table<void *, CallerInfo> t(1*1000*1000L);
    return t;
}

static Table<void *, PtrInfo> &get_ptr_info_table()
{
    static Table<void *, PtrInfo> t(100*1000*1000L);
    return t;
}

// REPORTING
//////////////////////////////////////////////////////////////////////

// static bool has_prefix(char const *pre, char const *str)
// {
//     size_t lenpre = strlen(pre);
//     size_t lenstr = strlen(str);
//     return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
// }

template <size_t N>
class FixedString
{
    char buffer[N];
    size_t size;

public:
    FixedString() : size(0)
    {
        memset(buffer, 0, N);
    }

    FixedString(char const *str)
        : size(0)
    {
        memset(buffer, 0, N);
        write(str);
    }

    char to_numchar(int n)
    {
        if (n < 10)
            return '0' +n;
        else
            return 'a' + (n - 10);
    }
    
    void write(size_t n, int base, size_t min_length = 0, char padchar = ' ')
    {
        char tmp[100];
        size_t pos = 100;
        do
        {
            --pos;
            tmp[pos] = to_numchar(n % base);
            n /= base;
        }
        while (n > 0);
        size_t len = 100 - pos;
        while (len < min_length)
        {
            --pos;
            ++len;
            tmp[pos] = padchar;
        }
        memcpy(buffer + size, tmp + pos, len);
        size += len;
    }
    
    void write(char const *str)
    {
        size_t len = strlen(str);
        memcpy(buffer + size, str, len);
        size += len;
    }

    void print(int fd)
    {
        ::write(fd, buffer, size);
    }

    char const *c_str()
    {
        return buffer;
    }
};

static size_t ptr_to_int(void *ptr)
{
    size_t x;
    memcpy(&x, &ptr, 8);
    return x;
}

void cm_debug_write_report()
{
    if (out == 0)
    {
        FixedString<30> fname;
        fname.write("/tmp/memout");
        // fname.write("/dev/shm/out");
        // fname.write(getpid(), 10);
        out = creat(fname.c_str(), 0644);
    }
    auto &caller_info_table = get_caller_info_table();
    for (size_t i = 0; i < caller_info_table.mem_size(); ++i)
    {
        void **key = caller_info_table.key_at(i);
        CallerInfo *info = caller_info_table.value_at(i);
        if (key && info)
        {
            FixedString<1000> line;
            line.write(total_times, 10, 10, ' ');
            line.write(" ");
            line.write(ptr_to_int(*key), 16, 16, '0');
            line.write(" ");
            line.write(info->times, 10, 10, ' ');
            line.write(" ");
            line.write(info->allocated, 10, 10, ' ');
            line.write(" ");
            line.write(info->accumulator, 10, 10, ' ');
            line.write(" ");
            line.write(info->name);
            line.write("\n");
            line.print(out);
        }
    }
    FixedString<100> line;
    line.write(total_times, 10, 10, ' ');
    line.write(" ");
    line.write("TOTAL            ");
    line.write(total_times, 10, 10, ' ');
    line.write(" ");
    line.write(total_allocated, 10, 10, ' ');
    line.write(" ");
    line.write(total_accumulator, 10, 10, ' ');
    line.write("\n");
    line.print(out);
    FixedString<100> separ;
    separ.write(total_times, 10, 10, ' ');
    separ.write(" ");
    separ.write("--------------- invalid_free=");
    separ.write(invalid_free_count, 10);
    separ.write(", invalid_realloc=");
    separ.write(invalid_realloc_count, 10);
    separ.write("\n");
    separ.print(out);
}

// static void fill_caller_name(char *where, size_t size, void **bt, size_t bt_size)
// {
//     Dl_info d;
//     char const *caller_name = "unknown";
//     size_t i;
//     for (i = 0; i < bt_size; ++i)
//     {
//         int dl_ok = dladdr(bt[i], &d);
//         if (dl_ok)
//         {
//             if (d.dli_sname)
//             {
//                 caller_name = d.dli_sname;
//                 break;
//             }
//         }
//     }
//     FixedString<1000> s;
//     s.write(ptr_to_int(bt[0]), 16, 16, '0');
//     s.write(":");
//     s.write(caller_name);
//     strncpy(where, s.c_str(), size);
// }

// static void report_caller_info()
// {
//     FixedString<1000> s;
    
// }

static void *malloc_internal(size_t alignment, size_t n, void **bt, size_t bt_size)
{
    CHECK(bt_size > 0);
    pthread_mutex_lock(&main_mutex);

    // Allocate memory
    char *ptr = (char *)get_main_storage().alloc(alignment, n);

    if (!created_map_file)
    {
        copy_file("/proc/self/maps", "/tmp/maps");
        created_map_file = true;
    }

    total_allocated += n;
    total_accumulator += n;
    total_times += 1;

    // Find and update caller
    auto &caller_info = get_caller_info_table().find_or_create(bt[0]);
    if (caller_info.allocated == 0)
    {
        // vidime ho poprve
        caller_info.name[0] = '\0';
        // fill_caller_name(caller_info.name, sizeof caller_info.name, bt, bt_size);
        //report_caller_info(bt[0], caller_info);
    }
    caller_info.allocated += n;
    caller_info.accumulator += n;
    caller_info.times += 1;

    // Fill ptr info
    auto &ptr_info = get_ptr_info_table().insert_new(ptr);
    ptr_info.caller = bt[0];
    ptr_info.size = n;

    if (total_times % 1000 == 0)
        cm_debug_write_report();
    pthread_mutex_unlock(&main_mutex);

    return ptr;
}

static void free_impl_locked(void *ptr)
{
    auto &ptr_info = get_ptr_info_table().find_or_abort(ptr);
    auto &caller_info = get_caller_info_table().find_or_abort(ptr_info.caller);
    caller_info.allocated -= ptr_info.size;
    total_allocated -= ptr_info.size;
}

// Implement malloc, free, ...
//////////////////////////////////////////////////////////////////////

void *aligned_alloc(size_t alignment, size_t n)
{
    BT_MAKE(bt, bt_size);
    return malloc_internal(alignment, n, bt, bt_size);
}

void free(void *ptr)
{
    pthread_mutex_lock(&main_mutex);
    if (get_main_storage().is_owned(ptr))
    {
        free_impl_locked(ptr);
    }
    else
    {
        ++invalid_free_count;
    }
    pthread_mutex_unlock(&main_mutex);
}

size_t malloc_usable_size(void *ptr)
{
    abort();
}

void *memalign(size_t alignment, size_t size)
{
    return aligned_alloc(alignment, size);
}

void *posix_memalign(size_t alignment, size_t size)
{
    return aligned_alloc(alignment, size);
}

void *pvalloc(size_t size)
{
    abort();
}

void *valloc(size_t size)
{
    abort();
}

void *malloc(size_t n)
{
    BT_MAKE(bt, bt_size);
    return malloc_internal(alignof(max_align_t), n, bt, bt_size);
}

void *calloc(size_t n)
{
    BT_MAKE(bt, bt_size);
    void *ptr = malloc_internal(alignof(max_align_t), n, bt, bt_size);
    memset(ptr, 0, n);
    return ptr;
}

void *realloc(void *ptr, size_t n)
{
    BT_MAKE(bt, bt_size);
    void *p = malloc_internal(alignof(max_align_t), n, bt, bt_size);
    if (ptr)
    {
        pthread_mutex_lock(&main_mutex);
        if (get_main_storage().is_owned(ptr))
        {
            auto &ptr_info = get_ptr_info_table().find_or_abort(ptr);
            size_t prev_size = ptr_info.size;
            memmove(p, ptr, prev_size);
            free_impl_locked(ptr);
        }
        else
        {
            ++invalid_realloc_count;

            // neni to nas pointer, co se da delat, zkopirujeme to i s
            // bordelem. Snad se netrefime mimo pamet, aby nas to sestrelilo
            memmove(p, ptr, n);
        }
        pthread_mutex_unlock(&main_mutex);
    }
    return p;
}

void * operator new(decltype(sizeof(0)) n) noexcept(false)
{
    BT_MAKE(bt, bt_size);
    return malloc_internal(alignof(max_align_t), n, bt, bt_size);
}

void operator delete(void *p) throw()
{
    free(p);
}

#endif
