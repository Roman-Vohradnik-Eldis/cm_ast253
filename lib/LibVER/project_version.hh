#ifndef GIT_PROJECT_HH_
#define GIT_PROJECT_HH_

#include <cstring>
#include <cstdio>
#include <sstream>
#include <vector>
#include <string>

namespace VER {

struct TargetInfo
{
    std::string name;
    std::string ROOT;
    std::string INCLUDE;
    std::string CC;
    std::string CXX;
    std::string LIBS;
    std::string PKGS;
    std::string CXXFLAGS;
    std::string LDFLAGS;
};

struct ProjectInfo
{
    std::vector<std::string> TARGET_BINS;
    std::vector<std::string> TARGET_LIBS;
    std::string CXXFLAGS;
    std::string LDFLAGS;
    std::string COMPILER_VERSION;
    std::string OS_PRETTY_NAME;
};

struct GitInfo
{
    std::string projname;
    std::string path;
    std::string url;
    int version_a;
    int version_b;
    int version_c;
    int version_offset;
    std::string hash;
    bool has_modified_files;
    time_t commit_time;
};

//////////////////////////////////////////////////////////////////////

/// Generated
std::vector<GitInfo> get_git_modules();

/// Generated
GitInfo get_git_info();

/// Generated
ProjectInfo get_project_info();

/// Generated
std::vector<TargetInfo> get_targets();

//////////////////////////////////////////////////////////////////////

// --version
std::string create_version_line(std::string const &product_name, std::string const &user_string);

// --eldis-version
std::string create_eldis_version_line(std::string const &product_name, std::string const &user_string);

// --git-version
std::string create_git_version_line(std::string const &product_name);

void version(int argc,
             char **argv,
             std::string const &product_name,
             std::string const &user_string = "");

} // namespace VER

#endif // GIT_PROJECT_HH_
