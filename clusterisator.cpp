﻿#include "clusterisator.h"

// Удаление вершины current из кластера it
void clusterisator::cluster_erase(std::map<unsigned, cluster>::iterator& it,
	graph::iterator& current)
{
	it->second.g.erase(current->first);
	v_to_cl.erase(current->first);
	if (it->second.g.empty())
	{
		clusters.erase(it);
		return;
	}
	// пересчёт параметров
	unsigned aij_temp = 0;
	if (it->second.g.size() < current->second.in_d +
		current->second.out_d)
	{
		for (auto& i : it->second.g)
		{
			auto it1 = current->second.input.find(i);
			if (it1 != current->second.input.end())
				aij_temp += it1->second;
			it1 = current->second.output.find(i);
			if (it1 != current->second.output.end())
				aij_temp += it1->second;
		}
	}
	else
	{
		for (auto& i : current->second.input)
		{
			auto it1 = it->second.g.find(i.first);
			if (it1 != it->second.g.end())
				aij_temp += i.second;
		}
		for (auto& i : current->second.output)
		{
			auto it1 = it->second.g.find(i.first);
			if (it1 != it->second.g.end())
				aij_temp += i.second;
		}
	}
	it->second.aij_sum -= aij_temp + current->second.loop;
	it->second.di_sum -= current->second.in_d + current->second.out_d;
}

// иннициализация cl(смежные кластеры) и no_cl(смежные вершины, не лежащие в кластере)
void clusterisator::_Init(std::map<unsigned, unsigned>& cl, std::map<unsigned,
	unsigned>& no_cl, const std::unordered_map<unsigned, unsigned>& graph_v)
{
	for (auto& i : graph_v)
	{
		auto it1 = v_to_cl.find(i.first);
		if (it1 != v_to_cl.end())
		{
			auto it2 = cl.find(it1->second);
			if (it2 != cl.end())
				it2->second += i.second;
			else
				cl.insert({ it1->second, i.second });
		}
		else
		{
			auto it2 = no_cl.find(i.first);
			if (it2 != no_cl.end())
				it2->second += i.second;
			else
				no_cl.insert({ i.first, i.second });
		}
	}
}

// Перемещает вершину it туда, куда нужно
// Возвращает true, если перемещение произошло
bool clusterisator::move_anywhere(graph::iterator& it)
{
	/*
	пройтись по смежным вершинам
	получить множество смежных кластеров и смежных вершин
	определить предпочтительный для присоединения кластер
	определить предпочтительную для присоединения вершину
	выполнить либо cluster_add либо make_cluster(модулярность не должна ухудшиться),
	перед этим выполнив cluster_erase если нужно
	*/
	std::map<unsigned, unsigned> cl; // смежный кластер на aij_temp(кол. рёбер между it и кластером)
	std::map<unsigned, unsigned> no_cl; // безкластерная вершина на кол. рёбер между ней и it
	_Init(cl, no_cl, it->second.input);
	_Init(cl, no_cl, it->second.output);

	auto del = v_to_cl.find(it->first);
	int clcl = (v_to_cl.end() == del) ? -1 : del->second;
	if (clcl != -1)
		cl.erase(clcl);

	std::pair<double, unsigned> max;
	std::map<unsigned, unsigned>::iterator it1;
	char flag = !no_cl.empty();
	if (flag == 1)
	{
		it1 = no_cl.begin();
        max = { d_modular(it, temp_graph->find(it1->first)), it1->first };
		// d_mod на номер вершины

		for (++it1; it1 != no_cl.end(); ++it1)
		{
			std::pair<double, unsigned> ifmax = {
                d_modular(it, temp_graph->find(it1->first)), it1->first
			};
			if (ifmax > max)
				max = ifmax;
		}
	}
	// max - наилучшее внекластерное
	if (!cl.empty())
	{
		it1 = cl.begin();
		if (flag == 0)
		{
			max = { d_modular(it,
				clusters[it1->first], it1->second), it1->first };
			++it1;
			flag = 2;
		}
		for (; it1 != cl.end(); ++it1)
		{
			std::pair<double, unsigned> tmp = { d_modular(it,
				clusters[it1->first], it1->second), it1->first };
			if (tmp > max)
			{
				flag = 2;
				max = tmp;
			}
		}
	}
	if (flag == 0)
		return false;
	// clcl если != 1 номер текущего кластера; -1 - если вне кластера
	if (clcl != -1)
	{
		auto yhg = clusters.find(clcl);
		if (d_modular(yhg->second) >= max.first)
			return false;
		this->cluster_erase(yhg, it);
	}
	if (flag == 1)
        this->make_cluster(it, temp_graph->find(max.second));
	else if (flag == 2)
		this->cluster_add(clusters.find(max.second), it, cl[max.second]);

	return true;
}

// Создаёт метаграф
void clusterisator::new_graph()
{
    std::unique_ptr<graph> meta_gr(new graph());
	std::unordered_map<unsigned, std::forward_list<unsigned>> tmp_result;
	for (auto& i : clusters)
	{
        auto it = meta_gr->new_vertex(i.first);
        meta_gr->connect(it, it, i.second.aij_sum);
		tmp_result.insert({ i.first, std::forward_list<unsigned>() });
	}
    for (auto& i : *temp_graph) // Если знать степени кластеров заранее - можно проще
	{
		unsigned out = v_to_cl[i.first];
		auto it = tmp_result.find(out);
        auto out_it = meta_gr->find(out);
		for (auto& j : i.second.output)
		{
			unsigned fr = v_to_cl[j.first];
			if (out == fr)
			{
				it->second.splice_after(it->second.before_begin(),
					result[j.first]);
			}
			else
			{
                meta_gr->connect(meta_gr->find(fr), out_it, j.second);
			}
		}
		for (auto& j : i.second.input)
		{
			unsigned fr = v_to_cl[j.first];
			if (out == fr)
			{
				it->second.splice_after(it->second.before_begin(),
					result[j.first]);
			}
		}
	}
	for (auto& i : result) // добиваем остатки result(костыль)
	{
		if (!i.second.empty())
		{
			auto it = tmp_result.find(v_to_cl[i.first]);
			it->second.splice_after(it->second.end(), i.second);
		}
	}
	v_to_cl.clear();
	clusters.clear();
	result = std::move(tmp_result);
	temp_graph = std::move(meta_gr);
}

// min_cl_count - минимально допустимое количество кластеров
std::unordered_map<unsigned, std::forward_list<unsigned>>
clusterisator::operator()(std::unique_ptr<graph>& gr, unsigned min_cl_count = 2)
{
    temp_graph = std::move(gr);
    edge_count = temp_graph->edge_count();
	clusters.clear();
	first_partition();
	bool contin;
	for (;;)
	{
		do {
			contin = false;
            for (auto it = temp_graph->begin(); it !=
                temp_graph->end(); ++it)
			{
				contin = (move_anywhere(it) || contin);
			}
		} while (contin);
		if (clusters.size() < min_cl_count)
			break;
        new_graph();
	}
	v_to_cl.clear();
    temp_graph->clear();
	clusters.clear();
	return result;
}
