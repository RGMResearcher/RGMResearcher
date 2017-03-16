#pragma once
#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <unordered_map>
#include <ostream>
#include <functional>

class vertex
{
public:
    // Входящая степень
    unsigned in_d,
    // Исходящая степень
    out_d,
    // Количество петель
    loop;
    // Входящие рёбра
    std::unordered_map<unsigned, unsigned> input,
    // Исходящие рёбра
    output;

    typedef std::unordered_map<unsigned, unsigned>::iterator iterator;

    vertex() : in_d(0), out_d(0), loop(0){}
    vertex(const vertex& lnk) :input(lnk.input),
        output(lnk.output), in_d(lnk.in_d), out_d(lnk.out_d),
        loop(lnk.loop){}
    vertex(vertex&& lnk) :input(std::move(lnk.input)),
        in_d(std::move(lnk.in_d)), out_d(std::move(lnk.out_d)),
        output(std::move(lnk.output)), loop(std::move(lnk.loop)){}
};

// параметры для взвешенного графа
class v_vertex
{

};

class graph
{
private:
    unsigned m; // Число рёбер
    mutable std::unordered_map<unsigned, vertex> v;
public:
    typedef std::unordered_map<unsigned, vertex>::iterator iterator;

    graph() :m(0){}
    graph(const graph& gr) :v(gr.v), m(gr.m){}
    graph(graph&& gr) :v(std::move(gr.v)), m(std::move(gr.m)){}

    void operator=(graph&& gr)
    {
        v = std::move(gr.v);
        m = std::move(gr.m);
    }

    graph& operator=(const graph& gr)
    {
        v = gr.v;
        m = gr.m;
        return *this;
    }

    // добавление вершины с заданным индексом
    iterator new_vertex(unsigned index)
    {
        auto it = v.find(index);
        if (it == v.end())
            return v.insert({index, vertex()}).first;
        else
            return it;
    }

    // Установка n связей first --> second
    void connect(unsigned, unsigned, unsigned n = 1);

    // Удаление n рёбер first --> second
    // Если рёбер меньше, чем n - они удалятся все
    // Возвращает количество удалённых рёбер
    unsigned disconnect(unsigned, unsigned, unsigned,
                        bool ndel = true);

    // Установка n рёбер it_f <-- it_s
    void connect(const graph::iterator&,
                 const graph::iterator&, unsigned n = 1);
    unsigned disconnect(const graph::iterator&,
                        const graph::iterator&, unsigned, bool ndel = true);

    // Количество рёбер it_f --> it_s
    unsigned count(const graph::iterator& it_f,
                   const graph::iterator& it_s) const
    {
        if (it_f->first == it_s->first)
            return it_f->second.loop;
        auto it = it_f->second.output.find(it_s->first);
        return (it == it_f->second.output.end()) ? 0 : it->second;
    }

    void merge(const graph::iterator&, const graph::iterator&, bool);
    void merge(unsigned, unsigned, bool _Ndel = true);

    // Количество рёбер first --> second
    unsigned count(unsigned first, unsigned second) const
    {
        auto it_f = v.find(first);
        if (first == second)
            return it_f->second.loop;
        auto it = it_f->second.output.find(second);
        return (it == it_f->second.output.end()) ? 0 : it->second;
    }

    // Количество входящих рёбер
    unsigned in_deg(unsigned index) const
    {
        return v[index].in_d;
    }

    // Количество иходящих рёбер
    unsigned out_deg(unsigned index) const
    {
        return v[index].out_d;
    }

    // Количество вершин в графе
    unsigned size() const
    {
        return v.size();
    }

    void clear()
    {
        v.clear();
        m = 0;
    }

    void swap(graph& gr)
    {
        v.swap(gr.v);
        std::swap(m, gr.m);
    }

    graph::iterator begin()
    {
        return this->v.begin();
    }

    graph::iterator end()
    {
        return this->v.end();
    }

    const graph::iterator begin() const
    {
        return this->v.begin();
    }

    const graph::iterator end() const
    {
        return this->v.end();
    }

    // Поиск вешины по индексу
    graph::iterator find(unsigned v_index) const
    {
        return v.find(v_index);
    }

    // Число рёбер
    unsigned edge_count() const
    {
        return m;
    }
};

// Удаляет петли
void erase_loops(graph&);

#endif // _GRAPH_H_
