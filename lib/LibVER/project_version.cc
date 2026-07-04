
#include <unistd.h>
#include "project_version.hh"

namespace VER {

std::string format_version(GitInfo const &proj)
{
    std::string str;
    str += std::to_string(proj.version_a);
    str += ".";
    str += std::to_string(proj.version_b);
    str += ".";
    str += std::to_string(proj.version_c);
    return str;
}

bool is_git_clean(GitInfo const &info)
{
    return !info.has_modified_files;
}

bool is_version_clean(GitInfo const &info)
{
    return info.version_offset == 0 && is_git_clean(info);
}

std::string format_time(time_t sec)
{
    char buffer[128];
    buffer[0] = '\0';
    struct tm stm;
    if (gmtime_r(&sec, &stm))
    {
        strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &stm);
    }
    return buffer;
}

std::string create_version_line(std::string const &product_name, std::string const &user_string)
{
    auto modules = get_git_modules();
    auto self = get_git_info();
    std::string str;
    str += product_name;
    str += ":";
    str += format_version(self);
    if (!is_version_clean(self))
        str += ".0";
    for (auto &&mod : modules)
    {
        str += " ";
        str += mod.projname;
        str += ":";
        str += format_version(mod);
        if (!is_version_clean(mod))
            str += ".0";
    }
    if (!user_string.empty())
    {
        str += " ";
        str += user_string;
    }
    return str;
}

std::string create_eldis_version_line(std::string const &product_name, std::string const &user_string)
{
    auto project = get_project_info();
    auto git_info = get_git_info();

    std::string version = create_version_line(product_name, user_string);
    version += " ";
    version += project.OS_PRETTY_NAME;
    return version;
}

std::string create_git_version_line(std::string const &product_name)
{
    auto modules = get_git_modules();
    auto git_info = get_git_info();
    std::string str;
    str += product_name;
    str += ":";
    str += git_info.hash;
    if (!is_git_clean(git_info))
        str += "(modified)";
    for (auto &&mod : modules)
    {
        str += " ";
        str += mod.projname;
        str += ":";
        str += mod.hash;
        if (!is_git_clean(mod))
            str += "(modified)";
    }
    return str;
}

struct VersionFile
{
    std::string fname;

public:
    VersionFile(std::string const &content)
    {
        char path[256];
        snprintf(path, sizeof (path), "/tmp/%d.version", getpid());
        fname = path;

        FILE *file = fopen(path, "w");
        if (!file)
        {
            fprintf(stderr, "Error: cannot write file: %s\n", path);
        }
        else
        {
            fprintf(file, "%s\n", content.c_str());
            fclose(file);
        }
    }
    VersionFile(const VersionFile &) = delete;
    VersionFile &operator=(VersionFile const &) = delete;
    ~VersionFile()
    {
        unlink(fname.c_str());
    }
};

void create_version_file(std::string const &product_name, std::string const &user_string)
{
    // The file is created when the function is called for the first time.
    // The file is removed when the program exits.
    static VersionFile file(create_eldis_version_line(product_name, user_string));
}

void version(int argc,
             char **argv,
             std::string const &product_name,
             std::string const &user_string)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            printf("%s\n", create_version_line(product_name, user_string).c_str());
            exit(0);
        }
        else if (strcmp(argv[i], "--eldis-version") == 0)
        {
            printf("%s\n", create_eldis_version_line(product_name, user_string).c_str());
            exit(0);
        }
        else if (strcmp(argv[1], "--git-version") == 0)
        {
            printf("%s\n", create_git_version_line(product_name).c_str());
            exit(0);
        }
    }

    create_version_file(product_name, user_string);
}

} // namespace VER
