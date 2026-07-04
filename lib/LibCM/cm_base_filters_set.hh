#ifndef CM_BASE_FILTERS_SET_H
#define CM_BASE_FILTERS_SET_H

// #include "cm_header_internal.hh"

// #include "cm_base_functs.hh"
// #include "cm_base_variable.hh"
#include "cm_base_filter.hh"

// Poznamka: CCmFiltersSet (asi) pouziva vnitrni cache, aby rychle
// zodpovedel opakované dotazy.
//
// Proto jsem se nesnazil predelat metody, aby byly const.  V zasade
// skoro vsechno by se muselo udelat mutable a skoro vsechny metody by
// musely byt const, tak to nema cenu. Navic by const metody nebyly
// thread-safe, atd.

// TODO: Chtelo by to zdokumentovat, co znamena parametr DegradateOnlyStars.

class CCmFiltersSet
{
 private:
    uint16_t StarMaxCount;

    // Filtry slozene jenom z hvezdicek
    std::map <std::string, class CCmFilter> MyMapOnlyStars;

    // Filtry, ktere obsahuji specialni znaky, ale nejsou slozene
    // jenom z hvezdicek.
    std::map <std::string, class CCmFilter> MyMapPatterns;

    // Filtry bez specialnich znaku
    std::map <std::string, class CCmFilter> MyMapExact;

    bool smart;

    struct smart_item
    {
//      std::string varname;
//      unsigned array_x, array_y;
      bool result;
//      std::list <unsigned> EventFields; // if result=Yes, then EventFields contain filtered items, or is empty, if all items required
      std::list <  std::list<uint16_t> > index_array; //    if result=Yes, then FILL for array, but if need full array, then don't fill
    };

    // Asi naka cache
    std::map<std::string, struct smart_item> smart_map;

    bool get_smart(std::string var_name, bool *result, std::list <  std::list<uint16_t> > *index_array); // return true, if found REC
    void set_smart(std::string var_name, bool  result, std::list <  std::list<uint16_t> > *index_array);
    void gen_smart(std::string var_name, bool *result, std::list <  std::list<uint16_t> > *index_array);

  public:

    CCmFiltersSet();
    ~CCmFiltersSet();

    void set_smart(bool _smart);

    void smart_clear(void);

//    virtual void smart_undefine(std::string *varname);
//    virtual void smart_undefine(std::string &varname);
//    virtual void smart_undefine(char *varname);

    std::map <std::string, class CCmFilter> getMap(bool enableOnlyStars, bool enablePatterns,bool enableExact);
    std::map <std::string, class CCmFilter> getMapAll(void);
    std::map <std::string, class CCmFilter> getMapOnlyStars(void);
    std::map <std::string, class CCmFilter> getMapPatterns(void);
    std::map <std::string, class CCmFilter> getMapExact(void);
    std::map <std::string, class CCmFilter> const *getMapOnlyStarsPTR(void) const { return &MyMapOnlyStars; }
    std::map <std::string, class CCmFilter> const *getMapPatternsPTR(void) const { return &MyMapPatterns; }
    std::map <std::string, class CCmFilter> const *getMapExactPTR(void) const { return &MyMapExact; }

    std::map <std::string, class CCmFilter> *getMapOnlyStarsPTR(void) { return &MyMapOnlyStars; }
    std::map <std::string, class CCmFilter> *getMapPatternsPTR(void) { return &MyMapPatterns; }
    std::map <std::string, class CCmFilter> *getMapExactPTR(void) { return &MyMapExact; }
    
    size_t      size(void) const { return MyMapOnlyStars.size() + MyMapPatterns.size() + MyMapExact.size(); }
    bool        empty(void) const { return MyMapOnlyStars.empty() && MyMapPatterns.empty() && MyMapExact.empty(); }
    void        clear(void);

    bool        getFlagPatterns() { return ((MyMapPatterns.size()>0) ? true : false); }
    bool        getFlagOnlyStar() { return ((MyMapOnlyStars.size()>0) ? true : false); }
    unsigned    getOnlyStarMaxCount() { return StarMaxCount;  }

    // TODO: Na co je RemoveItBefore?
    bool        add(class CCmFilter const *addf, bool DegradateOnlyStars=false, bool RemoveItBefore=false);
    bool        add(class CCmFilter const &addf, bool DegradateOnlyStars=false);

    // Prida postupne vsechny filtry z listu
    bool        add(std::list <class CCmFilter> *list, bool DegradateOnlyStars=false);
    bool        add(std::list <class CCmFilter> &list, bool DegradateOnlyStars=false);

    // Prida vsechny mapped_value z predane mapy, klice ignoruje.
    bool        add(std::map <std::string, class CCmFilter> *map, bool DegradateOnlyStars=false);
    bool        add(std::map <std::string, class CCmFilter> &map, bool DegradateOnlyStars=false);

    // Vytvori mapu z adds pomoci getMapAll(). Prida ji.
    bool        add(class CCmFiltersSet *adds, bool DegradateOnlyStars=false);
    bool        add(class CCmFiltersSet &adds, bool DegradateOnlyStars=false);

    // Vytvori ze filter pomoci fromString(). Prida ho.
    bool        add(std::string *str, bool DegradateOnlyStars=false);
    bool        add(std::string &str, bool DegradateOnlyStars=false);
    bool        add(const char *str, bool DegradateOnlyStars=false);

    bool        change(class CCmFilter *addf, bool DegradateOnlyStars=false);
    bool        change(class CCmFilter &addf, bool DegradateOnlyStars=false);
    bool        change(std::string *str, bool DegradateOnlyStars=false);
    bool        change(std::string &str, bool DegradateOnlyStars=false);
    bool        change(const char *str, bool DegradateOnlyStars=false);

    bool        erase(std::string *str);
    bool        erase(std::string &str);
    bool        erase(const std::string *str);
    bool        erase(const std::string &str);
    bool        erase(const char *str);
    bool        erase(std::list<std::string> *str);
    bool        erase(std::list<std::string> &str);
    bool        erase(std::map<std::string, class CCmVariable *> *map); // for cm_ext
    bool        erase(std::map<std::string, class CCmVariable *> &map); // for cm_ext

    bool        test(std::string TestName, std::string AddPrefix = std::string());
    bool        test(CCmFilter *filter, bool WithIndexes = true, std::string AddPrefix = std::string()); // with indexes
    bool        test(CCmFilter &filter, bool WithIndexes = true, std::string AddPrefix = std::string()); // with indexes
    bool        test(CCmVariable const *var, bool UseEventFields = true, std::string AddPrefix = std::string());
    bool        test(CCmVariable const &var, bool UseEventFields = true, std::string AddPrefix = std::string());

    bool        test(CCmVariable const *var, std::list <  std::list<uint16_t> > *index_array, bool UseEventFields = true, std::string AddPrefix = std::string());
    bool        test(CCmVariable const &var, std::list <  std::list<uint16_t> > &index_array, bool UseEventFields = true, std::string AddPrefix = std::string());

                         // compare cmFiltersSet / cmpMap std::map is OLD map, and myMap is NEW map
    void        diff(std::map <std::string, class CCmFilter> * myMap, std::map <std::string, class CCmFilter> *cmpMap,
                             class CCmFiltersSet *created, class CCmFiltersSet *deleted, class CCmFiltersSet *changed = NULL, bool CompareIndexes = true, bool ChangedToCreatedAndDeleted = false);

    void        diff(class CCmFiltersSet *compare, class CCmFiltersSet *created, class CCmFiltersSet *deleted, class CCmFiltersSet *changed = NULL,
                             bool CompareIndexes = true, bool ChangedToCreatedAndDeleted = false);
    void        diff(class CCmFiltersSet &compare, class CCmFiltersSet *created, class CCmFiltersSet *deleted, class CCmFiltersSet *changed = NULL,
                             bool CompareIndexes = true, bool ChangedToCreatedAndDeleted = false);

    std::string getNames(std::string delimiter=";", bool enableOnlyStars=true, bool enablePatterns=true, bool enableExact=true);
    std::string toString(std::string delimiter=";", bool enableOnlyStars=true, bool enablePatterns=true, bool enableExact=true) const;
    std::string fullPrint(std::string delimiter="\n", bool enableOnlyStars=true, bool enablePatterns=true, bool enableExact=true) const;


    std::string to_str() const { return fullPrint("\n", true, true, true); }
};

#endif // CM_BASE_FILTERS_SET_H
