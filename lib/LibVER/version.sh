#!/bin/bash

APPROOT=$(git rev-parse --show-toplevel 2>/dev/null)

usage()
{
  echo "usage: ${0} [--fullpath] {--status|[--git] --version}";
}

warn()
{
  echo -e "\e[0;31mWarning\e[0m(\e[0;34m$(basename ${0})\e[0m@\e[033m${FUNCNAME[2]}\e[0m): ${1}" >&2
}

info()
{
  echo -e "\e[0;32mInfo\e[0m(\e[0;34m$(basename ${0})\e[0m@\e[033m${FUNCNAME[2]}\e[0m): ${1}" >&2
}

if [ $# -lt 1 ]; then
  echo $(usage) >&2
  exit 1
fi

mkpath()
{
  cd ${APPROOT}

  fpath=$(readlink -f "${1}")

  if [ -n "${fullpath}" ]; then
    echo ${fpath}
  else
    echo $(basename ${fpath})
  fi

  cd ~-
}

parse()
{
  delim=${3:-:}
  echo $(grep -oh "${item:=${1}}[ ]*${delim}.*" "${file:=${2}}" 2>/dev/null | sed "s/[^${delim}]\+${delim}[ ]*\(.*\)/\1/")
}

parse_name()
{
  info=${1}

  info_name=$(parse name ${info})
  echo ${info_name}
}

parse_version()
{
  info=${1}

  if [ ! -f ${PWD}/${info} ]; then
    warn "Trying to parse version for non-existing file \"${info}\", path=\"${PWD}\""
    return
  fi

  version_file=$(parse version_file ${info})
  version_string=$(parse version_string ${info})

  while [ -z "${version_file}" ] && [ -z "${version_string}" ] && ! [ -d .git ]; do
    cd ..
    version_file=$(parse version_file ${info})
    version_string=$(parse version_string ${info})
  done

  if [ -n "${version_file}" ]; then
    version="$(parse "${version_string}" "${version_file}" '"' | tr -d '"')"

    if [ -z "${version}" ]; then
      warn "Cannot find version for version_string=\"${version_string}\" in file=\"${version_file}\""
      echo "invalid"
    fi

    echo ${version}
  else
    echo ${version_string}
  fi
}

get_version()
{
  cd ${1}
  type=${2}

  if [ -n "$(git status --porcelain 2> /dev/null)" ]; then
    flag="(modified)"
  fi

  if [ -z "${gitversion}" ]; then
    if [ ! -f info.txt ]; then
      warn "${type} \"$(mkpath ${1})\" doesn't contain file info.txt"
    else
      echo $(parse name info.txt):$(parse_version info.txt)' '
    fi
  else
    if [ ! -f info.txt ]; then
      echo $(basename ${1}):$(git rev-parse HEAD 2> /dev/null)"${flag} "
    else
      echo $(parse name info.txt):$(git rev-parse HEAD 2> /dev/null)"${flag} "
    fi
  fi
}

parse_versions()
{
  versions="$(get_version ${PWD} Project)"

  for module in $(git submodule status | awk '{ print $2 }'); do
    versions+="$(get_version ${module} Module)"
  done

  echo "${versions}"
}

make_temporary_version()
{
  version=${1}
  #remove everything except `.'
  dots=${version//[^\.]/}

  if [ ${#dots} -eq 2 ]; then
    echo ${version}.0
  else
    echo ${version%[0-9]}0
  fi
}

check_status()
{
  dest=${1}
  type=${2}

  typeset -x IFS=$'\n'

  changes=0
  changed_files=()

  for change in $(git status --short --porcelain); do
    # status name -> name
    chfname=${change:3}
    changes=$((changes + 1))
    changed_files+="${chfname} "
  done

  typeset -x IFS=$' \n'

  if [ ${changes} -gt 0 ]; then
    if [ "${report_changes}" = "yes" ]; then
      info "Changed: ${changed_files[*]}"
    fi

    old_version=$(parse_version info.txt)

    if [ "${old_version}"  = "invalid" ]; then
      warn "Version of $(basename ${dest}) will be ignored"
      return
    fi

    new_version=$(make_temporary_version ${old_version})
    if [ -z "${supress_warning}" ]; then
      warn "${type} \"$(mkpath ${dest})\" has differences against last commit, version will be set to ${new_version}"
    fi

    if [ "${old_version}" != "${new_version}" ]; then
      set_version ${new_version} ${type}
    fi
  fi
}

check_statuses()
{
  check_status $(readlink -f .) "Project"

  for module in $(git submodule status | awk '{ print $2 }'); do
    cd ${module}
    check_status ${module} "Module"
    cd ~-
  done
}

set_version()
{
  version=${1}
  type=${2}

  if [ ! -f info.txt ]; then
    warn "${type} \"$(mkpath .)\" doesn't contain file info.txt, file will be created"
  else
    version_file=$(parse version_file info.txt)
    version_string=$(parse version_string info.txt)

    while [ -z "${version_file}" ] && [ -z "${version_string}" ] && ! [ -d .git ]; do
      cd ..
      version_file=$(parse version_file ${info})
      version_string=$(parse version_string ${info})
    done

    if [ -n "${version_file}" ]; then
      sed -i "s/\(${version_string}\).*/\1 \"${version}\"/" ${version_file}
    else
      sed -i "s/\(version_string:\).*/\1${version}/" info.txt
    fi
  fi
}

while [ $# -gt 0 ]; do
  case "${1}" in
    -h | --help     ) usage; exit 0; ;;
    -s | --status   ) run_function=check_statuses; shift 1;;
    -v | --version  ) run_function=parse_versions; shift 1;;
    -f | --fullpath ) fullpath=yes; shift 1;;
    -g | --git      ) gitversion=yes; shift 1;;
    -r | --report-changes) report_changes=yes; shift 1;;
    -x | --supress-warning) supress_warning=yes; shift 1;;
    *               ) echo "error: unkown option \"${1}\""; echo $(usage) >&2; exit 2;;
  esac
done

${run_function}

