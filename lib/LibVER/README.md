
# LiBVER

For the Rules-based build system, see [makefile-rules](makefile-rules.org).

## Eldis Version

Funkci `version()` je nutne zavolat na zacatku mainu. Je nutne ji
predat argumenty programu: `version(&argc, &argv)`.  Pokud se soubor
demonizuje, je potreba ho zavolat znovu i po demonizaci.

Funkce `version()` zkontroluje argumenty se, kterymi byl program
spusten a pokud je jeden z nich `--version`, `--eldis-version`,
`--git-version` nebo `--svn-version`, vypise prislusne informace a
ukonci program.

`--version`:
    Vypise jmeno a verzi programu a vsech ELDIS knihoven ve tvaru `name:version [libname:libversion]`, kde name je jméno programu a version je verze programu,
    libname je jméno knihovny vyvíjené ve firme ELDIS, kterou program vyuzivá a se kterou je zkompilovany a libversion je verze knihovny.
    Aby se knihovna vypsala musi byt v gitu ulozena jako submodul.

`--eldis-version`:
    Vypise jmena a verze stejne jako --version, ale prida za ne konfiguracni parametry prekladu ve tvaru `timestamp hostname kernel compiler arch`, 
    kde timestamp je cas prekladu, hostname je jmeno zarizeni, kernel je verze kernelu, compiler je verze prekladace a arch je cilova architektura.

`--git-version`/`--svn-version`:
    Vypise jmeno a git verzi projektu a knihoven ve tvaru `name:git_version [libname:lib_git_version]`.

Pokud neni zadny z argumentu vyse pritomen, vytvori se soubor
`/tmp/PID.version`, kde `PID` je cislo procesu, a do nej se zapisou
informace, ktere se vypisuji pomoci `--eldis-version`.  

V pravidelnych intervalech (kazdou hodinu) se kontroluje, zda kazdy
proces jeste bezi, a pokud ne, odpovidajici PID.version soubor se
smaze.  Bezici a naistalovane programy na instalacce se kontroluji
pomoci spusteni programu s prepinacem `--eldis-version` nebo pomoci
PID.version souboru.  Pokud se shoduje PID i jmeno beziciho programu
neni PID.version smazan.

## Legacy LibVER usage

Jak funguje LibVER

V Makefilu se zavola prikaz `lib/LibVER/version.sh --status`, ktery zkontroluje zda se adresar shoduje s posledni commitnutou verzi. 
Pokud ne vytvori prechodnou verzi tak, ze na jeji konec pripoji `.0` (pokud uz neni verze tvaru a.b.c.d, pak pouze prepise posledni cislo na 0), vysledna verze je tedy ve tvaru `a.b.c.0`. 
Tato verze se propise i do info.txt.
Pote se zavola make na knihovnu LibVER, ktera zkompiluje soubory version.c a version.h 
a prostrednictvim parametru -D k nim preda informace o jmenu a verzi z info.txt (pomoci skriptu version.sh) a informace o prekladu ziskane z shellu. 

V info.txt se jmeno definuje pomoci retezce `name:` nasledovaneho jmenem programu.
Verze se definuje bud primo jako `version_string: VERSION`, kde VERSION je verze nebo pomoci odkazu na jiny soubor. 
V druhem pripade je potreba k `version_file:` doplinit jmeno souboru (s relativni cestou), ve kterem je definovana verze a dale k retezeci `version_string:`
pridat retezec, za kterym je definovana verze (verze musi byt v uvozovkach).
 
Napr.:

````
info.txt:
name:example
version_file:main.cc
version_string= #define VERSION
changelog_file:

main.cc:
#define VERSION "1.0.0"
````

````
git commit:
pre-commit hook - otevre soubor, kam se vyplni nova verze. Ta se propise do info.txt (pripadne tam, kam se odkazuje info.txt).
prepare-commit-msg hook - zkontroluje zda commit message obsahuje verzi programu, jmeno autora a informace o git submodulech, pripadne je prida
post-commit hook - pokud je verze programu ve tvaru a.b.c.0, prida se verze jako tag ke commitu
````

## LibVER pro Python

Soubor version.py obsahuje verzi LibVER pro python, ktera funguje temer identicky jako pro c/c++.
Python neni nutne pred spustenim prekladat, proto mu nelze primo pri prekladu dat verzi souboru a git verzi jako promennou.
To co se provede pri predkladu pomoci c/c++ se provadi pri kazdem spusteni python skriptu, pokud v korenovem adresari skriptu existuje slozka .git:
    Provede se porovnani verzi a pripadne prepsani v info.txt a ulozeni verze programu a git verze do souboru LibVER/versions. 

Pote se provede cteni verzi ze souboru LibVER/versions a vypis verze na stdout/vytvoreni PID.version souboru. 
Pro vytvoreni a naplneni souboru versions je potreba alespon jednou spustit skript s inicializovanym .git adresarem.

Format vypisu pro prepinac --eldis-version je nasledujici:

`name:version [libname:libversion] timestamp hostname kernel Python: version platform`, kde zacatek az po `kernel` je stejny jako vypis pro c/c++,
`Python: version` je verze pouziteho pythonu a `platform` jsou informace o pouzitem pocitaci (pomoci platform.platform()).

