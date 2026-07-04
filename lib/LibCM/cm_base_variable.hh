#ifndef CM_BASE_VARIABLE_H
#define CM_BASE_VARIABLE_H

#include <sstream>
#include <string>
#include <map>
#include <list>
#include "cm_types.hh"
#include "CMSHeader.hh"
#include "CMSConfig.hh"

#ifdef CMS4_VERSION
typedef CMS::Config CmCompatConfig;
#else
typedef CMSConfig CmCompatConfig;
#endif

// SAVEFILE: Pokud chce zarizeni ukladat promennou do SaveFile, tak si
// to nastavi u te promenne v konfiguraku. Pokud si ji vytvari za
// behu, tak ji nastavi FlagSaveFile.
//
// Tento flag se s promennou posila vsem ostatnim zarizenim, takze
// napriklad v gui_simple, i kdyz jsme pripojeni pres router, ten flag
// vidime, coz je dobre.
//
// V ramci jednoho programu muzeme promenne nastavit flag
// CM_VARIABLE_FLAG_FOREIGN (tento flag se nikam neposila). Pokud je
// toto nastaveno, promenna se do SaveFile v tomto programu neuklada.
// Toho vyuziva cm_router.
//
#define CM_VARIABLE_FLAG_FOREIGN 6
#define CM_VARIABLE_FLAG_PRIORITY_CHANGED 7
#define CM_VARIABLE_FLAG_VIRTUAL 4
#define CM_VARIABLE_FLAG_SAVEFILE 3
#define CM_VARIABLE_FLAG_WRITABLE 2
#define CM_VARIABLE_FLAG_EXTERNAL 1
#define CM_VARIABLE_FLAG_OWNED 5
// #define CM_VARIABLE_FLAG_SIGNED 0
#define CM_VARIABLE_FLAGS_ALL   ((1<<CM_VARIABLE_FLAG_PRIORITY_CHANGED) | (1<<CM_VARIABLE_FLAG_VIRTUAL) | (1<<CM_VARIABLE_FLAG_SAVEFILE) | (1<<CM_VARIABLE_FLAG_WRITABLE) | (1<<CM_VARIABLE_FLAG_EXTERNAL) | (1<<CM_VARIABLE_FLAG_SIGNED))

// Rozměr promenne y_size x_size:
//
// rozmer 1 1: Neni pole
// rozmer Y 1: pro Y > 1, je jednorozmerné.
// rozmer Y X: pro Y > 1, X > 1, je dvourozmerne.
//
// Ukladani jednorozmerne |0         y_size-1|
// Ukladani dvourozmerne  |0         y_size-1||0          y_size-1| ... x_size-krat
//
// Tedy na pozici index v tech ulozenych datech je:
// y = index % y_size
// x = index / y_size
//
// Specialne pro jednorozmerne pole, je na pozici index, kde 0 <= index < y_size,
// y = index
// x = 0
//
// Prvek (x, y) ma pozici (x * y_size) + y

struct CmInvalidVariableName : std::runtime_error
{
    CmInvalidVariableName(std::string const &s) : std::runtime_error(s) {}
};

bool libCM_is_valid_variable_name(std::string const &str);


class CCmVariable
{
  private:
    double modif_time; // Asi v sekundach
    std::string name;
    char        type; // 'b'/'f'/'i'/'e'/'s'
    uint8_t     flags; // b0=Signed, b1=External, b2=Writable, b3=SaveFile, b4=Virtual, b5=Owned, b6=Foreign, b7=PriorityChanged 
    unsigned    array_x, array_y; // pokud je jen jednorozmerne, pak jedeme v Y
    double      f_min, f_max, f_step;
    int max_decimals; // calculated from "f_step"
    libcm_integer    i_min, i_max, i_step;
    unsigned    s_maxsize;
    std::map< int64_t, std::string> enum_values; // 0="M1":1="M2":3="M3/A":6="MC":23="empty"
    std::map< std::string, int64_t> enum_strings; // 0="M1":1="M2":3="M3/A":6="MC":23="empty"

    // Kdyz se enum nacte z definice, tak si ukladame poradi polozek enumu v
    // definici, a to nasledujcim zpusobem: Pokud jsou polozky zapsany vzestupne
    // podle cisel, tak nechame enum_sorting prazdne. Jinak si poradi ulozime do
    // enum_sortings.
    std::list<int64_t> enum_sorting;  // is enum have different sorting, than by value, here is sorted list of values
    uint8_t     priority; // priorita pro odesilani na druhou stranu (status bude mit vyssi, nez nejaka mereni)

private:
    StringList s;
    DoubleList f;
    UIntList i; // int/bool(0,1 striktne)/enum

public:
    // unsigned EventWhat; // 0=undefine , 1=define , 2=update
    std::list <unsigned> EventFields;

    // SendBlockUntil: Pro interni ucely.
    //
    // Kazdy partner ma seznam promennych, ktere bude posilat. Diky
    // MaxUpdateFreq neposílá modifikace hned. Do SendBlockUntil si uloží čas,
    // kdy nejdriv muze modifikaci poslat.
    double SendBlockUntil;

    // Z konfiguraku
    double MaxUpdateFreq;

private:
    // For DEBUGGING
    struct ConsistencyCheck
    {
	CCmVariable *m_v;
	ConsistencyCheck(CCmVariable *v) : m_v(v) {}
	~ConsistencyCheck() { m_v->check_consistency(); }
    };
    
public:

    // VALUE ... pokud neni array, tak je definovan pouze prvek [0], pokud to je
    // array, tak je tam array_x*array_y prvku + jeden na konci (default value)

    // inp: hodnota float/int/string reprezentovana jako string
    // idx: od ktereho indexu
    // count: nastavujeme idx+0,...,idx+(count-1)
    // ProtectedIndexes: vyjma techto vyjmenovanych indexu
    bool setValues(std::string inp, unsigned idx, unsigned count=1, std::list <unsigned> *ProtectedIndexes = NULL);
    std::string getValue(unsigned idx) const;
    void        recalculate_max_decimals(void);

    StringList const &getStringList() const { return s; }
    DoubleList const &getDoubleList() const { return f; }
    UIntList const &getUIntList() const { return i; }

    StringList  &theStringList() { return s; }
    DoubleList  &theDoubleList() { return f; }
    UIntList  &theUIntList() { return i; }

    CCmVariable();
    
    // CCmVariable(CCmVariable const &) = default;
    // CCmVariable(CCmVariable &&) = default;
    // CCmVariable &operator=(CCmVariable const &) = default;
    // CCmVariable &operator=(CCmVariable &&) = default;

    void check_consistency();

    uint8_t     getFlags() const { return flags; }

    bool        haveFlagWritable() const;
    bool        haveFlagExternal() const;
    bool        haveFlagSaveFile() const;
    bool        haveFlagPriorityChanged() const;
    bool        haveFlagVirtual() const;
    bool        haveFlagForeign() const;
    bool        haveFlagOwned() const;

    void        setFlagPriorityChanged(bool newVal);
    // void        setFlagSigned(bool newVal);
    void        setFlagVirtual(bool newVal);
    void        setFlagForeign(bool newVal);
    void        setFlagOwned(bool newVal);

    void        getLimitsF(double   &_f_min, double   &_f_max, double   &_f_step) const;
    void        getLimitsI(libcm_integer &_i_min, libcm_integer &_i_max, libcm_integer &_i_step) const;
    void        getLimitsS(unsigned &_s_maxsize) const;

    void        setLimitsS(unsigned _s_maxsize);
    void        setLimitsI(libcm_integer _i_min, libcm_integer _i_max, libcm_integer _i_step);
    void        setLimitsF(double   _f_min, double   _f_max, double   _f_step);

    int         getMaxDecimals() const;
    std::string generateEnumDefinition() const;
    std::map< int64_t, std::string> getEnumValues() const;
    std::map< std::string, int64_t> getEnumStrings() const;
    std::list< int64_t> getEnumSorting() const;

    void        addEventField(unsigned F);
    void        addEventFields(std::list<unsigned> *_EventFields);
    void        clearEventFields(void);

    // Porovnava se typ hodnoty + rozmery pole, vrati true, pokud jsou stejny
    bool        compareType(const CCmVariable &rhs) const ;

    // Porovnava se typ hodnoty + rozmery pole + limity (vcetne step).
    // Vrati true, pokud jsou stejny
    bool        compareTypeAndLimits(const CCmVariable &rhs) const;

    // Porovnava se typ hodnoty + rozmery pole + limity (vcetne step) + hodnoty.
    // Vrati true, pokud jsou stejny.
    bool        compare(const CCmVariable &rhs) const;

    // Pokud prommenne maji odlisny stejny typ nebo, rozmery,
    // tak vrati false. To je specialni pripad.
    //
    // Vratime true, pokud se lisime v nejake hodnote v poli od promenne rhs.
    // Nastavi EventFields na seznam tech odlisnych indexu.
    //
    // Pozor: Pokud je EventFields neprazdne, pak pak se porovnavaji pouze
    // polozky vyjmenovane v EventFields. EventFields promenne rhs se ignoruji.
    bool        diffFields(const CCmVariable &rhs);

    // Funguje jako diffFields, jenom se vzdy porovnaji vsechny polozky pole,
    // ignoruji se EventFields. Nastavi se EventFields na ty polozky, ktere byly
    // odlisne. Vrati se true, pokud se nektera polozka lisila.
    bool        diffAllFields(const CCmVariable &rhs);

    // Interni funkce: Argument je prepocitany dvorozmerny index na jednorozmerny.
    // Pozor!: Nekontroluji se, jestli je v rozsahu toho pole.
    void        copy_one_index(const CCmVariable &rhs, unsigned ptr);
    void        copy_indexes(const CCmVariable &rhs, std::list <unsigned> Fields);
    bool        compare_idx(const CCmVariable &rhs,  unsigned idx) const;

    // Pozn: Pokud ma rhs jiny typ nebo jine rozmery, neprovede se nic
    void        copy_limits (const CCmVariable &rhs);

    // Stejne jako compare
    bool operator == (const CCmVariable &rhs) const;
    bool operator != (const CCmVariable &rhs) const;


    char        getType() const;
    std::string getName() const;
    unsigned    getArrayY() const;
    unsigned    getArrayX() const;
    uint8_t     getPriority() const;
    double      getModifTime() const;
    bool        isArray() const;

    // Krome toho, ze nastavi tak, smaze vsechny hodnoty promenne (nastavi je na nulu/prazdny string).
    void        setType(char n);
    
    void        setName(std::string _name);
    void        setArrayYX(unsigned Y, unsigned X);
    void        setPriority(uint8_t newPriority);
    void        setModifTime(double Time);

    // Vezme serializovane hodnoty (ktere jsou rozsekane podle carek,
    // viz getVALUES) a ty updatne. Nemeni EventFields.
    //
    // Ignoruje hodnoty na indexech vyjmenovanych v ProtectedIndexes.
    bool        setVALUES(char **item, std::list <unsigned> *ProtectedIndexes = NULL);

    // Vezme serializovane hodnoty oddelene carkou (viz getVALUES) a ty nacte.
    // Nemeni EventFields.
    //
    // Ignoruje hodnoty na indexech vyjmenovanych v ProtectedIndexes.
    bool        setVALUES(std::string inp, std::list <unsigned> *ProtectedIndexes = NULL);

    // Serializuje hodnotu promenne (vsechny pozice v
    // poli). Neobsahuje jmeno a typ. Jednotilive hodnoty jsou
    // oddelene carkou.
    //
    // Pokud OnlyChanged == true, berou se v uvahu jen indexy v EventFields
    std::string getVALUES(bool OnlyChanged = false) const;

    bool        setVariableBool  (bool        value, unsigned idx = 0);
    bool        setVariableInt   (libcm_integer    value, unsigned idx = 0);
    bool        setVariableFloat (double      value, unsigned idx = 0);
    bool        setVariableString(std::string value, unsigned idx = 0);
    bool        setVariableEnum  (int64_t     value, unsigned idx = 0);
    bool        setVariableEnum  (std::string value, unsigned idx = 0);

    // set: Pokud ma promenna jiny typ nez pozadovany nebo je index pole mimo rozsah,
    // vypise se chyba a vrati se false.
    //
    // Pokud je hodnota mimo limity, tak se zaokrouhli na nejblizsi moznou.
    //
    // Pokud se hodnota zmenila, vrati se true, jinak false.
    //
    bool        setVariableBool  (bool        value, unsigned idx_y, unsigned idx_x);
    bool        setVariableInt   (libcm_integer    value, unsigned idx_y, unsigned idx_x);
    bool        setVariableFloat (double      value, unsigned idx_y, unsigned idx_x);
    bool        setVariableString(std::string value, unsigned idx_y, unsigned idx_x);
    bool        setVariableEnum  (int64_t     value, unsigned idx_y, unsigned idx_x);
    bool        setVariableEnum  (std::string value, unsigned idx_y, unsigned idx_x);

    // get: Pokud ma promenna jiny typ nez pozadovany nebo je index pole mimo rozsah,
    // vypise se chyba a vrati se false.
    //
    // Jinak se vrati true a nastavi se vystupni promenna.
    //
    bool        getVariableBool  (bool        &value, unsigned idx = 0) const;
    bool        getVariableInt   (libcm_integer    &value, unsigned idx = 0) const;
    bool        getVariableFloat (double      &value, unsigned idx = 0) const;
    bool        getVariableString(std::string &value, unsigned idx = 0) const;
    bool        getVariableEnum  (int64_t     &value, unsigned idx = 0) const;
    bool        getVariableEnum  (std::string &value, unsigned idx = 0) const;

    bool        getVariableBool  (bool        &value, unsigned idx_y, unsigned idx_x) const;
    bool        getVariableInt   (libcm_integer    &value, unsigned idx_y, unsigned idx_x) const;
    bool        getVariableFloat (double      &value, unsigned idx_y, unsigned idx_x) const;
    bool        getVariableString(std::string &value, unsigned idx_y, unsigned idx_x) const;
    bool        getVariableEnum  (int64_t     &value, unsigned idx_y, unsigned idx_x) const;
    bool        getVariableEnum  (std::string &value, unsigned idx_y, unsigned idx_x) const;

    bool        getVariableInt   (uint64_t    &value, unsigned idx_y, unsigned idx_x) const
        __attribute__ ((deprecated("Use int64_t version instead")));
    bool        getVariableInt   (uint64_t    &value, unsigned idx = 0) const
        __attribute__ ((deprecated("Use int64_t version instead")));

    std::string defineFromArray(char **item, std::string inp); // return empty, or error string
    std::string defineFromString(std::string inp); // return empty, or error string
    std::string toStringDefine() const;
    std::string toStringDefineTypeValue() const;
    std::string fullPrint(double ActTime = 0.0) const;
    std::string to_str(double ActTime = 0.0) const { return fullPrint(ActTime); }
};

inline CCmVariable make_void_variable(std::string const &name)
{
    CCmVariable v;
    v.setName(name);
    return v;
}

typedef std::map<std::string, CCmVariable> VarMap;

// libCM_LoadConfigVariables: Do LoadedVariables prida promenne z
// kofiguraku z aktualni sekce. Argument MyLibCM_name je pouzit pouze
// pri vypisovani erroru
//
// Pri chybe vyhodi vyjimku (std::string)
void libCM_LoadConfigVariables(CmCompatConfig *config, VarMap *LoadedVariables, std::string *MyLibCM_name = NULL);

// libCM_ListAllKnownVariablesCM: Asi jenom pro debugovani.  Vypise
// vsechny promenne matchujici pattern. Oddeluje je pomoci '\n',
std::string libCM_ListAllKnownVariablesCM(VarMap &_VariablesCM ,std::string pattern = "", bool printHeadFoot = true);

void fill_full_event_fields(CCmVariable &v);

//////////////////////////////////////////////////////////////////////

int calculate_max_decimals(double f_step);

#endif // CM_BASE_VARIABLE_H
