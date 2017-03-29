#pragma once
#ifndef _CLUSTERISATOR_H_
#define _CLUSTERISATOR_H_

#include <set>
#include <map>
#include <vector>
#include <list>
#include <memory>
#include <math.h>
#include "graph.h"

class clusterisator
{
    struct cluster
    {
    public:
        unsigned long long di_sum;
        unsigned aij_sum;
        std::set<unsigned> g;

        cluster() :di_sum(0), aij_sum(0){}
    };

private:
    unsigned edge_count;
    std::map<unsigned, unsigned> v_to_cl;
    graph temp_graph;
    std::map<unsigned, cluster> clusters; // номер кластера на кластер
    std::map<unsigned, std::list<unsigned>> result; // номер кластера на список верщин в нём

    // Удаление вершины из кластера
    void erase_from_cl(std::map<unsigned, cluster>::iterator&,
        graph::iterator&);

    // Удаление вершины из кластера
    void erase_from_cl(std::map<unsigned, cluster>::iterator& it,
        unsigned v_index)
    {
        auto current = temp_graph.find(v_index);
        if (current != temp_graph.end())
            erase_from_cl(it, current);
    }

    // Добавляет вершину it в кластер с итератором cl
    // aij_sum_delt - кол. рёбер между вставляемой вершиной и
    // элементами кластера cl
    void add_to_cl(const std::map<unsigned, cluster>::iterator& cl,
        const graph::iterator& it, unsigned aij_sum_delt)
    {
        v_to_cl.insert({ it->first, cl->first });
        cl->second.g.insert(it->first);
        cl->second.aij_sum += aij_sum_delt + it->second.loop;
        cl->second.di_sum += it->second.in_d + it->second.out_d;
    }

    // Добавляет вершину v_index в кластер с итератором cl
    // aij_sum_delt - кол. рёбер между вставляемой вершиной и
    // элементами кластера cl
    void add_to_cl(std::map<unsigned, cluster>::iterator& cl,
        unsigned v_index, unsigned aij_sum_delt)
    {
        add_to_cl(cl, temp_graph.find(v_index), aij_sum_delt);
    }

    // Объединение двух вершин в новый кластер.
    // Возвращает номер созданного кластера.
    unsigned make_cluster(const graph::iterator& first,
        const graph::iterator& second)
    {
        unsigned ret = (clusters.empty()) ? 0 : (clusters.rbegin()->first + 1);
        auto it = clusters.insert({ ret, cluster() }).first;
        add_to_cl(it, first->first, temp_graph.count(second->first,
            first->first));
        add_to_cl(it, second->first, temp_graph.count(first->first,
            second->first));
        return ret;
    }

    // Объединение двух вершин в новый кластер.
    // Возвращает номер созданного кластера.
    unsigned make_cluster(unsigned v_first, unsigned v_second)
    {
        return this->make_cluster(temp_graph.find(v_first),
            temp_graph.find(v_second));
    }

    void meta_graph();
    void _Init(std::map<unsigned, unsigned>&, std::map<unsigned,
        unsigned>&, const std::map<unsigned, unsigned>&);
    void _Init();

    // Возврат. знач. не является значением модулярность
    // Только для операций сравнения < > =
    double d_modular(const cluster& cl)
    {
        return ((4.0 * edge_count)*cl.aij_sum - (cl.di_sum*cl.di_sum));
    }

    // Возврат. знач. не является значением модулярность
    // Только для операций сравнения < > =
    double d_modular(const graph::iterator& it1, const graph::iterator& it2)
    {
        return ((4.0 * edge_count)*temp_graph.count(it1, it2) -
            pow(it1->second.in_d + it1->second.out_d +
            it2->second.in_d + it2->second.out_d, 2));
    }

    // Возврат. знач. не является значением модулярность
    // Только для операций сравнения < > =
    double d_modular(const graph::iterator& it, const cluster& cl,
        unsigned aij_temp)
    {
        return ((4.0 * edge_count)*(cl.aij_sum + aij_temp) -
            pow(cl.di_sum + it->second.in_d + it->second.out_d, 2));
    }

    bool move_anywhere(graph::iterator&);

public:
    typedef std::function<void()> distributor;
    static const distributor default_distribution;

    clusterisator(){}
    void init(graph&& gr)
    {
        temp_graph = std::move(gr);
        _Init();
    }

    void init(const graph& gr)
    {
        temp_graph = gr;
        _Init();
    }
    // первичное разбиение на кластеры
    void first_partition() {}

    bool next_iteration();

    std::pair<std::map<unsigned, std::list<unsigned>> const&, graph const&>
        temp_result() const
    {
        return std::make_pair(result, temp_graph);
    }

    virtual ~clusterisator(){}
};

#endif // _CLUSTERISATOR_H_
