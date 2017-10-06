#pragma once
#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <map>
#include <ostream>
#include <functional>
#include <memory>
#include <queue>

class p_less
{
public:
    template<class t_key, class t_val, class t_comp>
    bool operator()(const std::pair<t_key, t_val>& first,
        const t_key& second)
    {
        return (first.first < second);
    }

    template<class t_key, class t_val, class t_comp>
    bool operator()(const t_key& first,
        const std::pair<t_key, t_val>& second)
    {
        return (first < second.first);
    }

    template<class t_key, class t_comp>
    bool operator()(const t_key& first, const t_key& second)
    {
        return (first < second);
    }

    template<class t_key, class t_val1, class t_val2, class t_comp>
    bool operator()(const std::pair<t_key, t_val1>& first,
        const std::pair<t_key, t_val2>& second)
    {
        return (first.first < second.first);
    }
};

/*
template<class datastruct1, class datastruct2, class lambda, class key_compare>
// Выполняет функцию lambda для элементов пересечения ds1 и ds2.
// Вычислительная сложность нахождения всех элементов пересечения - O(ds1.size() + ds2.size())
// Структуры данных должны быть отсортированы, причём в одном и том-же порядке
// comp - компаратор, используемый при сравнении элементов
void foreach_equal(const datastruct1& ds1, const datastruct2& ds2,
    const lambda& fcn, const key_compare& comp = p_less())
{
    auto it1 = ds1.begin();
    for (auto it2 = ds2.begin(); it2 != ds2.end(); ++it2, ++it1)
    {
        do {
            for (;; ++it1)
            {
                if (it1 == ds1.end())
                    return;
                if (!comp(*it1, *it2))
                    break;
            }
            for (;; ++it2)
            {
                if (it2 == ds2.end())
                    return;
                if (!comp(*it2, *it1))
                    break;
            }
        } while (comp(*it1, *it2));
        fcn(*it1, *it2);
    }
}*/

/*
// характеристики вершины графа
template<typename index_type, typename adjacent_e, typename type = adjacent_e>
class no_orient
{
protected:
std::map<index_type, adjacent_e> connections; // связи
unsigned lp; // кол. петель
unsigned d; // степень
public:
no_orient() :lp(0), d(0){}
no_orient(const no_orient& val) : connections(val.connections),
lp(val.lp), d(val.d){}
no_orient(no_orient&& val) :connections(std::move(val.connections)),
lp(std::move(val.lp)), d(std::move(val.d)){}
unsigned loop() const
{
return lp;
}

virtual unsigned deg() const
{
return d;
}
virtual void for_each(const std::function<void(
std::pair<index_type, adjacent_e>&)>& func)
{
for (auto& i : connections)
func(i);
}

virtual void add(const index_type& index, const type& val,
const std::function<void()>&)
{

}

virtual ~no_orient(){}
};

template<typename index_type, typename adjacent_e, typename type>
class orient : public no_orient<index_type, adjacent_e, type>
{
protected:
// connections - исход. связи
// d - исход. степень
unsigned i_d; // вход. степень
std::map<index_type, adjacent_e> input;
public:
unsigned out_d() const
{
return d;
}
unsigned in_d() const
{
return i_d;
}
virtual unsigned deg() const
{
return in_d() + out_d();
}
virtual void for_each(const std::function<void(
std::pair<const index_type, type>&)>& func)
{
no_orient<index_type, adjacent_e, type>::for_each(func);
for (auto& i : input)
func(i);
}
};

template<class first_type, class second_type>
class graph_iterator : public std::map<first_type, second_type>::iterator
{
private:

public:
graph_iterator(){}
};

template<typename index_type, typename adjacent_e, typename type>
class graph_interface
{
public:
typedef graph_iterator<index_type, adjacent_e> iterator;

graph_interface() :e_count(0){}
graph_interface(const graph_interface& val) :e_count(val.e_count){}
graph_interface(graph_interface&& val) :e_count(std::move(val.e_count)){}

virtual void connect(const iterator&, const iterator&, type) = 0;
virtual void connect(index_type, index_type, type) = 0;
virtual void disconnect(index_type, index_type, type) = 0;
virtual type connectivity(index_type, index_type) = 0;
virtual unsigned size() const = 0;
virtual iterator find() const = 0;
unsigned edge_count() const
{
return e_count;
}
virtual unsigned count();
// добавление вершины с заданным индексом
virtual iterator new_vertex(const index_type&) = 0;
~graph_interface(){}
protected:
unsigned e_count; // количество рёбер в графе
// сам граф будет в наследниках

template<typename gr_map>
iterator new_vertex(const index_type& index, gr_map& v)
{
if (v.find(index) == v.end())
return v.insert({ index, adjacent_e() }).first;
else
return v.end();
}
};

template<typename index_type, typename adjacent_e, typename type = adjacent_e>
class or_graph : public graph_interface<int, int, int>
{
private:
std::map<int, orient> v;
public:
virtual void connect(index_type, index_type, type);
virtual void disconnect(index_type, index_type, type);
virtual type connectivity(index_type, index_type);
virtual unsigned size() const
{
return v.size();
}

};
*/












typedef unsigned short graph_vertex_count_type;

class vertex
{
public:
    // Имя вершины
    unsigned name;
    // Входящая степень
    graph_vertex_count_type in_d,
        // Исходящая степень
        out_d,
        // Количество петель
        loop;
    // Входящие рёбра
    std::map<unsigned, graph_vertex_count_type> input,
        // Исходящие рёбра
        output;

    typedef std::map<unsigned, graph_vertex_count_type>::iterator iterator;

    vertex(unsigned _Name = 0) : in_d(0), out_d(0), loop(0), name(_Name){}
    vertex(const vertex& lnk) :input(lnk.input),
        output(lnk.output), in_d(lnk.in_d), out_d(lnk.out_d),
        loop(lnk.loop), name(0){}
    vertex(vertex&& lnk) :input(std::move(lnk.input)),
        in_d(std::move(lnk.in_d)), out_d(std::move(lnk.out_d)),
        output(std::move(lnk.output)), loop(std::move(lnk.loop)),
        name(std::move(lnk.name)){}

    vertex& operator=(vertex&& v)
    {
        name = v.name;
        in_d = v.in_d;
        out_d = v.out_d;
        loop = v.loop;
        input = std::move(v.input);
        output = std::move(v.output);
        return *this;
    }
    vertex& operator=(const vertex& v)
    {
        name = v.name;
        in_d = v.in_d;
        out_d = v.out_d;
        loop = v.loop;
        input = v.input;
        output = v.output;
        return *this;
    }
};

class graph
{
private:
    std::queue<unsigned> deleted;
    unsigned m; // Число рёбер
    mutable std::vector<vertex> v;
    mutable std::map<unsigned, unsigned> reindex;

    void erase(const std::pair<unsigned, unsigned>&);
    void merge(const std::pair<unsigned, unsigned>&,
        const std::pair<unsigned, unsigned>&);
public:
    typedef std::map<unsigned, unsigned>::iterator iterator;

    graph() :m(0){}
    graph(const graph& gr) :v(gr.v), m(gr.m), reindex(gr.reindex),
        deleted(gr.deleted){}
    graph(graph&& gr) :v(std::move(gr.v)), m(std::move(gr.m)),
        reindex(std::move(gr.reindex)), deleted(std::move(gr.deleted)){}
    graph(unsigned);

    void operator=(graph&& gr)
    {
        m = gr.m;
        v = std::move(gr.v);
        reindex = std::move(gr.reindex);
        deleted = std::move(gr.deleted);
    }

    graph& operator=(const graph& gr)
    {
        m = gr.m;
        v = gr.v;
        reindex = gr.reindex;
        deleted = gr.deleted;
        return *this;
    }

    // добавление вершины с заданным именем
    iterator new_vertex(unsigned name)
    {
        auto it = reindex.find(name);
        if (it == reindex.end())
        {
            if (this->deleted.empty())
            {
                it = reindex.insert({ name, v.size() }).first;
                v.push_back(vertex(name));
            }
            else
            {
                it = reindex.insert({ name, deleted.front() }).first;
                v[deleted.front()].name = name;
                deleted.pop();
            }
        }
        return it;
    }

    // Удаление вершины
    void erase(const graph::iterator& it)
    {
        erase(*it);
    }

    // Удаление вершины
    void erase_by_name(unsigned name)
    {
        auto it = this->find(name);
        if (it != this->end())
            erase(*it);
    }

    void erase_by_index(unsigned index)
    {
        erase({ index, v[index].name });
    }

    // Установка n связей first --> second
    void connect_by_index(unsigned, unsigned, graph_vertex_count_type n = 1);

    // Установка n связей first --> second
    void connect(const graph::iterator& it_f,
        const graph::iterator& it_s, graph_vertex_count_type n = 1)
    {
        connect_by_index(it_f->second, it_s->second, n);
    }

    // Установка n связей first --> second
    void connect_by_name(unsigned first, unsigned second, graph_vertex_count_type n = 1)
    {
        auto it_f = this->find(first);
        if (first == second)
            this->connect(it_f, it_f, n);
        else
        {
            this->connect(it_f, this->find(second), n);
        }
    }

    // Удаление n рёбер first --> second
    // Если рёбер меньше, чем n - они удалятся все
    // Возвращает количество удалённых рёбер
    graph_vertex_count_type disconnect_by_index(unsigned, unsigned, graph_vertex_count_type,
        bool ndel = true);

    // Удаление n рёбер first --> second
    // Если рёбер меньше, чем n - они удалятся все
    // Возвращает количество удалённых рёбер
    graph_vertex_count_type disconnect(const graph::iterator& it_f,
        const graph::iterator& it_s, graph_vertex_count_type n, bool ndel = true)
    {
        return this->disconnect_by_index(it_f->second, it_s->second, n, ndel);
    }

    // Удаление n рёбер first --> second
    // Если рёбер меньше, чем n - они удалятся все
    // Возвращает количество удалённых рёбер
    graph_vertex_count_type disconnect_by_name(unsigned,
        unsigned, graph_vertex_count_type, bool ndel = true);

    // Количество рёбер it_f --> it_s
    graph_vertex_count_type count_by_index(unsigned it_f,
        unsigned it_s) const
    {
        if (it_f == it_s)
            return v[it_f].loop;
        auto it = v[it_f].output.find(it_s);
        return (it == v[it_f].output.end()) ? 0 : it->second;
    }

    // Количество рёбер it_f --> it_s
    graph_vertex_count_type count(const graph::iterator& it_f,
        const graph::iterator& it_s) const
    {
        return this->count_by_index(it_f->second, it_s->second);
    }

    // Количество рёбер first --> second
    graph_vertex_count_type count_by_name(unsigned, unsigned) const;

    // Объединение двух вершин. it_f += it_s
    void merge(const graph::iterator& it_f, const graph::iterator& it_s)
    {
        merge(*it_f, *it_s);
    }

    // Объединение двух вершин. first += second
    void merge_by_name(unsigned first, unsigned second)
    {
        merge(this->find(first), this->find(second));
    }

    // Объединение двух вершин. first += second
    void merge_by_index(unsigned first, unsigned second)
    {
        merge({ first, v[first].name }, { second, v[second].name });
    }

    // Количество вершин в графе
    unsigned size() const
    {
        return reindex.size();
    }

    vertex& operator[](unsigned index)
    {
        return v[index];
    }

    const vertex& operator[](unsigned index) const
    {
        return v[index];
    }

    friend std::ostream& operator<<(std::ostream&, const graph&);

    void clear()
    {
        v.clear();
        deleted = std::move(std::queue<unsigned>());
        reindex.clear();
        m = 0;
    }

    void swap(graph& gr)
    {
        v.swap(gr.v);
        std::swap(m, gr.m);
        deleted.swap(gr.deleted);
        reindex.swap(gr.reindex);
    }

    graph::iterator begin()
    {
        return this->reindex.begin();
    }

    graph::iterator end()
    {
        return this->reindex.end();
    }

    const graph::iterator begin() const
    {
        return this->reindex.begin();
    }

    const graph::iterator end() const
    {
        return this->reindex.end();
    }

    // Поиск вешины по имени
    graph::iterator find(unsigned v_name) const
    {
        return reindex.find(v_name);
    }

    // Число рёбер
    unsigned edge_count() const
    {
        return m;
    }

	//Удаляет петли
	unsigned erase_loops();
};

// Удаляет петли
void erase_loops(graph&);

#endif // _GRAPH_H_
