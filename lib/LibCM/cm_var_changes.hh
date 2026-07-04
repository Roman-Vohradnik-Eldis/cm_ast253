#ifndef CM_VAR_CHANGES_HH_
#define CM_VAR_CHANGES_HH_

#include "cm_base_variable.hh"
#include "cm_utils.hh"

// Problem: Pokud mame promennou ve created a prijde nam jeji delete.
// To, ze je ve created, bud znamena, ze byla nadefinovana poprve, a
// nebo je to jeji redefinice. To my nevime.
//
// Takze ten delete musime zachovat, i za cenu toho, uzivateli pak
// prijde delete od promenne, ktera nebyla nikdy vytvorena.

// CmVarChanges jdou skladat, tj:
//
// CmVarChanges a;
// CmVarChanges b;
//
// CmVarChanges ab = a;
// ab.add_foreign_changes(b.created, b.updated, b.deleted);
// 
// VarMap vars;
//
// // Pak tohle
// apply_var_changes(vars, a.created, a.updated, a.deleted);
// apply_var_changes(vars, b.created, b.updated, b.deleted);
// // je ekvivalentni s:
// apply_var_changes(vars, ab.created, ab.updated, ab.deleted);

inline VarMap make_single_change(CCmVariable const &v)
{
    VarMap m;
    m.insert(std::make_pair(v.getName(), v));
    return m;
}

class CmVarChanges
{
    // Vsechny tri mnoziny jsou po dvou diskunktni
    //
    VarMap created; // nejsou pouzity EventFields
    VarMap updated; // pouzivaji se EventFields
    VarMap deleted; // hodnota neni pouzita.

public:
    CmVarChanges() {}

    CmVarChanges(VarMap c, VarMap u, VarMap d)
    {
        std::swap(c, created);
        std::swap(u, updated);
        std::swap(d, deleted);
    }

    bool empty() const
    {
        return created.empty() && updated.empty() && deleted.empty();
    }

    void clear()
    {
        created.clear();
        updated.clear();
        deleted.clear();
    }

    // Vrati false, pokud se nic nezmenilo, jinak true
    bool add_foreign_changes(VarMap const &c, VarMap const &u, VarMap const &d);

    // Vrati false, pokud se nic nezmenilo, jinak true
    bool add_my_changes(VarMap const &c, VarMap const &u, VarMap const &d);

    void move_some_changes(std::string const &pattern, VarMap &c, VarMap &u, VarMap &d);

    void move_all_changes(VarMap &c, VarMap &u, VarMap &d);

private:
    void add_my_updates(VarMap const &src);
    void add_foreign_updates(VarMap const &src, bool);
};

// std::string print_changes(CmVarChanges const &ch);

void apply_var_changes(VarMap &vars, VarMap const &c, VarMap const &u, VarMap const &d);

#endif // CM_VAR_CHANGES_HH_
