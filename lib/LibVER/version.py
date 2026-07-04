#   LibVER pro Python
#
#   Funguje podobne jako verze pro c/c++, ale vse co by se v c/c++ delalo v Makefilu se provadi automaticky pri kazdem spusteni.
#   Pokud v korenovem adresari neexistuje .git slozka, "make" faze se preskoci (tedy kontrola a pripadne prepsani verze)
#   a provede se pouze kontrola vstupnich parametru a pripadny vypis verze/vytvoreni PID.version souboru, stejne jako v LibVER pro c/c++.
#   Pro potlaceni warningu o zmene verze lze zavolat funkci version s parametrem warning_enabled=False.
#   Pro spravne fungovani je nutne nejprve spustit program s inicializovanym .git repozitarem, aby se vytvorila slozka LibVER/versions,
#   kam se ulozi verze.
#

import platform
import subprocess
from datetime import datetime
import os


def actualize_version():
    command_ver = os.path.dirname(os.path.realpath(__file__)) + '/version.sh --version'
    command_git = os.path.dirname(os.path.realpath(__file__)) + '/version.sh --git --version'

    new_version = subprocess.Popen(command_ver, stdout=subprocess.PIPE, text=True, shell=True).communicate()[0].strip()
    git_version = subprocess.Popen(command_git, stdout=subprocess.PIPE, text=True, shell=True).communicate()[0].strip()

    f = open(os.path.dirname(os.path.realpath(__file__)) + "/versions", "w+")
    f.write("version: " + new_version + "\r\n")
    f.write("git_version: " + git_version + "\r\n")
    f.close()


def get_version(version_type):
    path = os.path.dirname(os.path.realpath(__file__)) + "/versions"
    if os.path.exists(path):
        ret = ""
        f = open(path, "r")
        line = f.readline()
        if version_type == 'git':
            line = f.readline()
            parts = line.split()[1:]
            for part in parts:
                ret += part + " "
            f.close()
            return ret
        else:
            parts = line.split()[1:]
            for part in parts:
                ret += part + " "
            f.close()
        return ret
    else:
        print("Soubor " + path + " neexistuje! Nejprve spustte skript s iniializovanym .git adresarem a souborem info.txt!")
        exit(1)




def version_generic(argv, err_num, warning_enabled):
    # pokud je pritomen .git repozitar, aktualizuji se verze v info.txt a gitu a aktualizovane hodnoty se ulozi do lib/LibVERPython/versions
    if os.path.isdir('.git'):
        if not warning_enabled:
            command = os.path.dirname(os.path.realpath(__file__)) + '/version.sh -x --status'
        else:
            command = os.path.dirname(os.path.realpath(__file__)) + '/version.sh --status'
        subprocess.Popen(command, stdout=subprocess.PIPE, text=True, shell=True)
        actualize_version()

    arg_type = 'file'
    if '--version' in argv:
        arg_type = 'version'
    if '--git-version' in argv:
        arg_type = 'git'
    if '--svn-version' in argv:
        arg_type = 'git'
    if '--eldis-version' in argv:
        arg_type = 'eldis'

    # vytvoreni promennych k vypsani
    # verze se nacita ze souboru lib/LibVERPython/versions, kde se aktualizuje pokud je pritomen git repozitar, jinak je nutne ho aktualizovat rucne
    version_str = get_version(arg_type)
    date = datetime.now().strftime("%Y/%m/%d %H:%M:%S")
    hostname = platform.node()
    kernel = platform.version()
    python_version = platform.python_version()
    platform_info = platform.platform()

    final_str = version_str
    if arg_type == 'eldis' or arg_type == 'file':
        final_str += "  " + date + " " + hostname + " " + kernel + " Python: " + python_version + " " + platform_info

    # pokud nebyl zadan zadny argument pro vypis, vytvori se slozka PID.version
    if arg_type == 'file':
        f = open('/tmp/' + str(os.getpid()) + ".version", "w")
        f.write(final_str)
    else:
        print(final_str)
        exit(err_num)


def version(argv, warning_enabled=True):
    version_generic(argv, 1, warning_enabled)


def version_no_exit(argv, warning_enabled=True):
    version_generic(argv, 0, warning_enabled)
