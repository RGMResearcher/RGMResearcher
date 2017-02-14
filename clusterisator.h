#pragma once
#ifndef _CLUSTERISATOR_H_
#define _CLUSTERISATOR_H_
#include <set>
#include <map>
#include <vector>
#include <forward_list>
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
    std::unique_ptr<graph> temp_graph;
	std::map<unsigned, cluster> clusters; // номер кластера на кластер
	std::unordered_map<unsigned, std::forward_list<unsigned>> result; // номер кластера на список верщин в нём

	// первичное разбиение на кластеры
	void first_partition()
	{
        for (auto& i : *temp_graph)
			result.insert({ i.first, { i.first } });
	}
	// Удаление вершины из кластера
	void cluster_erase(std::map<unsigned, cluster>::iterator&,
		graph::iterator&);

	// Удаление вершины из кластера
	void cluster_erase(std::map<unsigned, cluster>::iterator& it,
		unsigned v_index)
	{
        auto current = temp_graph->find(v_index);
        if (current != temp_graph->end())
			cluster_erase(it, current);
	}

	// Добавляет вершину it в кластер с итератором cl
	// aij_sum_delt - кол. рёбер между вставляемой вершиной и
	// элементами кластера cl
	void cluster_add(const std::map<unsigned, cluster>::iterator& cl,
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
	void cluster_add(std::map<unsigned, cluster>::iterator& cl,
		unsigned v_index, unsigned aij_sum_delt)
	{
        cluster_add(cl, temp_graph->find(v_index), aij_sum_delt);
	}

	// Объединение двух вершин в новый кластер.
	// Возвращает номер созданного кластера.
	unsigned make_cluster(const graph::iterator& first, const graph::iterator&
		second)
	{
		unsigned ret = (clusters.empty()) ? 0 : (clusters.rbegin()->first + 1);
		auto it = clusters.insert({ ret, cluster() }).first;
        cluster_add(it, first->first, temp_graph->count(second->first,
			first->first));
        cluster_add(it, second->first, temp_graph->count(first->first,
			second->first));
		return ret;
	}

	// Объединение двух вершин в новый кластер.
	// Возвращает номер созданного кластера.
	unsigned make_cluster(unsigned v_first, unsigned v_second)
	{
        return this->make_cluster(temp_graph->find(v_first),
            temp_graph->find(v_second));
	}

    void new_graph();
	void _Init(std::map<unsigned, unsigned>&, std::map<unsigned,
		unsigned>&, const std::unordered_map<unsigned, unsigned>&);

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
        return ((4.0 * edge_count)*temp_graph->count(it1, it2) -
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
	clusterisator(){}
	std::unordered_map<unsigned, std::forward_list<unsigned>>
        operator()(std::unique_ptr<graph>&, unsigned);

	std::unordered_map<unsigned, std::forward_list<unsigned>>
		operator()(const graph& gr, unsigned min_cl_count = 2)
	{
        std::unique_ptr<graph> copy(new graph(gr));
        return this->operator()(copy, min_cl_count);
	}
};

#endif // _CLUSTERISATOR_H_
