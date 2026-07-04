#ifndef CM_VALUE_HISTORY_HH_
#define CM_VALUE_HISTORY_HH_

#include <iostream>
#include <deque>
#include <stdlib.h>
#include <algorithm>

template <typename T>
struct HistoryItem
{
    int64_t time;
    T value;
};

template <typename T>
bool operator<(HistoryItem<T> const &a, HistoryItem<T> const &b)
{
    return a.time < b.time;
}

template <typename T>
std::ostream &operator<<(std::ostream &out, HistoryItem<T> const &x)
{
    return out << x.value;
}

template <typename T>
class ValueHistory
{
    std::deque<HistoryItem<T>> values;
    int64_t keep_time = 3*1000*1000;
    T sum = 0;

public:
    void set_keep_time(int64_t n) { keep_time = n; }

    typedef typename decltype(values)::const_iterator iterator;
    iterator begin() const { return values.begin(); }
    iterator end() const { return values.end(); }
    size_t size() const { return values.size(); }
    bool empty() const { return values.empty(); }
    void push(HistoryItem<T> p)
    {
        auto it = std::upper_bound(values.begin(), values.end(), p);
        values.insert(it, p);
        sum += p.value;
        {
            auto it = values.begin();
            for (; it != values.end(); ++it)
            {
                if (it->time + keep_time >= p.time)
                    break;
                sum -= it->value;
            }
            values.erase(values.begin(), it);
        }
    }

    T get_average(T const &default_val) const
    {
        if (values.empty())
            return default_val;
        else
            return sum / values.size();
    }

    T get_median(T const &default_val) const
    {
        if (values.empty())
            return default_val;
        size_t n = values.size();
        if (n % 2 == 0)
        {
            size_t i = n / 2;
            return (values[i-1].value + values[i].value) / 2;
        }
        else
        {
            return values[n/2].value;
        }
    }
};

template <typename T, typename U>
T value_history_average(ValueHistory<T> const &values, U const &default_val)
{
    return values.get_average(default_val);
}

#endif // CM_VALUE_HISTORY_HH_
