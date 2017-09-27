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
	std::map<unsigned, cluster> const& clusters;
	// Состав вешин в каждом кластере
	std::map<unsigned, std::list<unsigned>> const& vertexes;
	// число петель в исходном графе
	unsigned loop_count;

	result_clusterisation(graph const& gr, std::map<unsigned,
		cluster> const& cl, std::map<unsigned,
		std::list<unsigned >> const& temp_result, unsigned loops) :clusters(cl),
		connections(gr), vertexes(temp_result), loop_count(loops){}
};

class clusterisator
{
private:
	unsigned edge_count, loop_count;
	std::vector<int> v_to_cl;
	graph temp_graph;
	// номер кластера на кластер
	std::map<unsigned, cluster> clusters;
	// номер кластера на список верщин в нём
	std::map<unsigned, std::list<unsigned>> temp_result;

	// Удаление вершины из кластера
	void erase_from_cl(std::map<unsigned, cluster>::iterator&,
		unsigned, unsigned);

	// Добавляет вершину it в кластер с итератором cl
	// aij_sum_delt - кол. рёбер между вставляемой вершиной и
	// элементами кластера cl
	void add_to_cl(const std::map<unsigned, cluster>::iterator& cl,
		unsigned gr_index, unsigned aij_sum_delt)
	{
		v_to_cl[gr_index] = cl->first;
		cl->second.g.insert(gr_index);
		cl->second.aij_sum += aij_sum_delt + temp_graph[gr_index].loop;
		cl->second.di_sum += temp_graph[gr_index].in_d + temp_graph[gr_index].out_d;
	}

	// Объединение двух вершин в новый кластер.
	// Возвращает номер созданного кластера.
	unsigned make_cluster(unsigned gr_first, unsigned gr_second)
	{
		unsigned ret = (clusters.empty()) ? 0 : (clusters.rbegin()->first + 1);
		auto it = clusters.insert({ ret, cluster() }).first;
		add_to_cl(it, gr_first, temp_graph.count_by_index(gr_second, gr_first));
		add_to_cl(it, gr_second, temp_graph.count_by_index(gr_first, gr_second));
		return ret;
	}

	void meta_graph();
	void _Init(std::map<unsigned, unsigned>&, std::map<unsigned,
		unsigned>&, const std::map<unsigned, graph_vertex_count_type>&);
	void _Init();

	// Возврат. знач. не является значением модулярность
	// Только для операций сравнения < > =
	int d_modular(const cluster& cl, unsigned current_aij, unsigned gr_index,
		const double& e_factor = 4.0)
	{
		int cur = temp_graph[gr_index].in_d + temp_graph[gr_index].out_d;
		return 2 * edge_count*current_aij - cur*(cl.di_sum - cur - current_aij);
	}

	// Возврат. знач. не является значением модулярность
	// Только для операций сравнения < > =
	int d_modular(unsigned gr_first, unsigned gr_second,
		const double& e_factor = 4.0)
	{
		return 2 * edge_count*(temp_graph.count_by_index(gr_first, gr_second) +
			temp_graph.count_by_index(gr_second, gr_first)) -
			(temp_graph[gr_first].in_d + temp_graph[gr_first].out_d)*(
			temp_graph[gr_second].in_d + temp_graph[gr_second].out_d);
	}

	// Возврат. знач. не является значением модулярность
	// Только для операций сравнения < > =
	int d_modular(unsigned gr_index, const std::map<unsigned, cluster>::iterator& it_cl,
		unsigned aij_temp, const double& e_factor = 4.0)
	{
		return 2 * edge_count*aij_temp - (temp_graph[gr_index].in_d +
			temp_graph[gr_index].out_d)*it_cl->second.di_sum;
	}

	bool move_anywhere(unsigned, const double &e_factor = 4.0);

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

	bool next_iteration(unsigned = 1, const double &e_factor = 4.0);

	result_clusterisation result() const
	{
		return result_clusterisation(temp_graph, clusters, temp_result, loop_count);
	}

	virtual ~clusterisator(){}
};

double modular(const result_clusterisation&);

#endif // _CLUSTERISATOR_H_
