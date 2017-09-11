#pragma once
#ifndef _CLUSTERISATOR_H_
#define _CLUSTERISATOR_H_

#include <set>
#include <vector>
#include <list>
#include <math.h>
#include "graph.h"

struct cluster
{
public:
    unsigned long long di_sum;
    unsigned aij_sum;
    std::set<unsigned> g;

    cluster() :di_sum(0), aij_sum(0){}
};

struct result_clusterisation
{
    // Сруктура связей метаграфа
    graph const& connections;
    // Структура объединения(для дендограммы)
    std::vector<cluster> const& clusters;
    // Состав вешин в каждом кластере
    std::vector<std::list<unsigned>> const& vertexes;

    result_clusterisation(graph const& gr, std::vector<cluster> const& cl,
        std::vector<std::list<unsigned>> const& temp_result) :
        connections(gr), clusters(cl), vertexes(temp_result){}
};

class clusterisator
{
private:
    unsigned edge_count; // первоначальное число рёбер(для модулярности)
    std::vector<int> v_to_cl; // индекс вершины на индекс кластера
    graph temp_graph; // Структора связей кластеров
    std::vector<cluster> clusters; // номер кластера на кластер
    std::vector<std::list<unsigned>> temp_result; // номер кластера на список верщин в нём
    std::queue<unsigned> deleted; // номера удалённых кластеров

    // переиндексирование кластеров(удаление пустых мест)
    void reindex_deleted();

    // Удаление вершины из кластера
    void erase_from_cl(unsigned, unsigned);

    // Добавляет вершину it в кластер с итератором cl
    // aij_sum_delt - кол. рёбер между вставляемой вершиной и
    // элементами кластера cl
    void add_to_cl(unsigned cl_index, unsigned gr_index, unsigned aij_sum_delt)
    {
        v_to_cl[gr_index] = cl_index;
        clusters[cl_index].g.insert(gr_index);
        clusters[cl_index].aij_sum += aij_sum_delt + temp_graph[gr_index].loop;
        clusters[cl_index].di_sum += temp_graph[gr_index].in_d +
            temp_graph[gr_index].out_d;
    }

    // Объединение двух вершин в новый кластер.
    // Возвращает номер созданного кластера.
    unsigned make_cluster(unsigned gr_first, unsigned gr_second)
    {
        unsigned index;
        if (!deleted.empty())
        {
            index = deleted.front();
            deleted.pop();
        }
        else
        {
            index = clusters.size();
            clusters.push_back(cluster());
        }
        add_to_cl(index, gr_first, temp_graph.
            count_by_index(gr_first, gr_second));
        add_to_cl(index, gr_second, temp_graph.
            count_by_index(gr_second, gr_first));
        return index;
    }

    void meta_graph();
    void _Init(std::map<unsigned, graph_vertex_count_type>&,
        std::map<unsigned, graph_vertex_count_type>&,
        const std::map<unsigned, graph_vertex_count_type>&);
    void _Init();

    // Возврат. знач. не является значением модулярность
    // Только для операций сравнения < > =
    double d_modular(unsigned cl_index, const double& e_factor = 4.0)
    {
        return ((e_factor * edge_count)*clusters[cl_index].aij_sum -
            (clusters[cl_index].di_sum * clusters[cl_index].di_sum));
    }

    // Возврат. знач. не является значением модулярность
    // Только для операций сравнения < > =
    double d_modular(unsigned gr_first, unsigned gr_second,
        const double& e_factor = 4.0)
    {
        return ((e_factor * edge_count)*(temp_graph.count_by_index(
            gr_first, gr_second) + temp_graph.count_by_index(gr_second,
            gr_first)) - pow(temp_graph[gr_first].in_d + temp_graph[gr_first].out_d +
            temp_graph[gr_second].in_d + temp_graph[gr_second].out_d, 2));
    }

    // Возврат. знач. не является значением модулярность
    // Только для операций сравнения < > =
    double d_modular(unsigned gr_index, unsigned cl_index,
        unsigned aij_temp, const double& e_factor = 4.0)
    {
        return ((e_factor * edge_count)*(clusters[cl_index].aij_sum + aij_temp) -
            pow(clusters[cl_index].di_sum + temp_graph[gr_index].in_d +
            temp_graph[gr_index].out_d, 2));
    }

    bool move_anywhere(unsigned, const double &e_factor = 4.0);

    // not used
    /*unsigned aijtemp_calc(const std::map<unsigned, graph_vertex_count_type>& mp,
        const std::set<unsigned>& st)
    {
        unsigned ret = 0;
        if (mp.size() < st.size())
        {
            if (mp.size()*log(st.size()) / log(2.0) <
                mp.size() + st.size())
            {
                for (auto& i : mp)
                {
                    auto it = st.find(i.first);
                    if (it != st.end())
                        ret += i.second;
                }
            }
            else
            {
                foreach_equal(mp, st, [&ret](const std::pair<unsigned,
                    graph_vertex_count_type>& first, const unsigned& second)
                {

                }, p_less());
            }
        }
        else
        {
            if (st.size()*log(mp.size()) / log(2.0) <
                mp.size() + st.size())
            {

            }
            else
            {

            }
        }
    }*/

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

    bool next_iteration(const double &e_factor = 4.0);

    result_clusterisation result() const
    {
        return result_clusterisation(temp_graph, clusters, temp_result);
    }

    virtual ~clusterisator(){}
};

#endif // _CLUSTERISATOR_H_
