
#ifndef CM_BASE_FILTER_H
#define CM_BASE_FILTER_H

// #include "cm_header_internal.hh"

// //#include "cm_base_functs.hh"
#include "cm_base_variable.hh"



class CCmFilter
{
  private:
    // Jmeno je pattern, ktery muze obsahovat *, ?, {}.
    //
    // Z nejakeho duvodu pri nastavovani nahrazujeme znak '{' za
    // '['. Pri konverzi filtru do stringu to zase konvertujeme
    // zpatky. To nechapu.
    //
    std::string name; //    FILL

    // bit0 isPattern
    // bit1 isPatternWithReplace"["->"{","]"->"}";
    // bit8-15 OnlyStarsCount
    //
    // OnlyStarsCount je nenulovy jen v pripade, ze name je slozen
    // jenom ze znaku '*'. V tomtom pripade je roven name.size()
    // (Asi).
    uint16_t flags; 

  public:
    // Seznam indexu:
    // Jednorozmerny index je jednoprvkovy senzam.
    // Dvourozmerny index je dvouprvkobvy seznam.
    std::list <  std::list<uint16_t> > index_array; //    FILL for array, but if need full array, then don't fill
//    std::list<void *>UserPointers; //    FILL

    CCmFilter();

    CCmFilter(std::string name)
    {
        fromString(name);
    }
    CCmFilter(char const *name)
    {
        fromString(name);
    }

    // Vstup musi byt pattern (bez indexů).
    void        setName(std::string _name);

    // Rozseka vstupni string na pattern + index_array, zavola setName() na pattern a nastavi indexy.
    void        fromString(std::string inp);
    
    // Vrati pattern (bez indexů): To, co by nastaveno pomoci setName().
    std::string getName(void) const;

    // Vrati pattern + indexy: To, co bylo nastaveno pomoci fromString().
    std::string toString(void) const;

    // to same jako toString().
    std::string fullPrint(void) const;

    std::string getNameRAW(void);

    // Vrati true, pokud filter obsahuje zpecialni znaky '*', '?' apod.
    bool        getFlagPattern(void) const { return ((flags & (1<<0)) ? true : false); };
    uint16_t    getFlags(void) { return flags; };
    bool        getFlagPatternReplace(void) { return ((flags & (1<<1)) ? true : false); };

    // Vrati true prave kdyz (neprazdny) name je slozen jenom z hvezdicek.
    bool        getFlagOnlyStar(void) { return (((flags >> 8) & 0xff) ? true : false); };

    // Pokud je name slozeno jenom z hvezdicek, vrati jejich pocet (==
    // delku jmena).  Jinak vrati 0.
    unsigned    getOnlyStarCount(void) const { return ((flags >> 8) & 0xff);  };

    // Testuje pouze pattern, ignoruje indexy.
    // Pokud je nastaven AddPrefix, testuje AddPrefix+TestName.
    //
    bool        test(std::string TestName, std::string AddPrefix = std::string()) const;

    // Pokud je UseEventFields == false, tak testuje jen jmeno promenne.
    // Pokud je UseEventFields == true, tak navic testuje, jestli 
    bool        test(CCmVariable const &var, bool UseEventFields = true, std::string AddPrefix = std::string());
    bool        test(CCmVariable const *var, bool UseEventFields = true, std::string AddPrefix = std::string());

    std::string index_array_toString(void) const;
    unsigned    addIndex  (             std::list<uint16_t>   add_index_item , bool *changed = NULL);
//    unsigned    addIndexes(std::list <  std::list<uint16_t> > add_index_array, bool *changed = NULL);
    unsigned    addIndexesSmart(std::list <  std::list<uint16_t> > add_index_array, bool *changed = NULL);
    unsigned    addIndexesSmart(const CCmFilter &rhs, bool *changed = NULL);
    bool operator == (const CCmFilter &rhs);

    std::string to_str(void) { return fullPrint(); }
};


#endif // CM_BASE_FILTER_H
