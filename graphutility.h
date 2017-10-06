#pragma once
#ifndef _GRAPHUTILITY_H_
#define _GRAPHUTILITY_H_

#include <fstream>
#include <random>
#include <ctime>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include "clusterisator.h"

#include <cuda.h>

template<class type>
// костыль для работы с vector<atomic<...>>
// разрешает копирование
class crutch_atomic : public std::atomic<type>
{
public:
	crutch_atomic() : std::atomic<type>(){}
	crutch_atomic(const crutch_atomic& val) :std::atomic<type>()
	{
		this->store(val.load());
	}
	crutch_atomic(const type& val) :std::atomic<type>(val){}
	operator type() const
	{
		return this->load();
	}
};

// Модель Боллобаша-Риордана
graph preferred_attachment(unsigned, unsigned);

class top_sort
{
private:
    mutable std::vector<unsigned> ans;
    mutable std::vector<bool> used;
    const graph* gr;
    void dfs(unsigned);
public:
    const std::vector<unsigned>& sort(const graph& grph)
    {
        gr = &grph;
        used.clear();
        ans.clear();
        for (auto& i : grph)
        {
            if (!used[i.second])
                dfs(i.second);
        }
        std::reverse(ans.begin(), ans.end());
        return ans;
    }
};

struct cl_incl
{
	std::list<unsigned> g;
	unsigned aij_sum, di_sum; // сумма рёбер и сумма степеней вершин

	cl_incl():aij_sum(0), di_sum(0){}
};

struct result_graph_generator
{
	std::vector<cl_incl> clusters; // кластер на номер вершины
	std::vector<unsigned> vertexes; // вершина на кластер
	graph gr; // сам граф
	unsigned loop_count;

	result_graph_generator(): loop_count(0){}
};

std::map<unsigned, double> page_rank(const graph&, const double& c = 0.85,
    const double& delta = 0.01);

// Генерирует кластеризованый граф
// v_count - количество вершин, min_cl_size - минимальный размер кластера,
// max_cl_size- максимальный размер кластера, q_cl - вероятность наличия ребра в кластере
result_graph_generator clustered_graph(unsigned, unsigned,
    unsigned, double q_cl = 0.8);

double modular(const result_graph_generator&);

template<class _Iterator, class _Function>
void parallel_for_each(const _Iterator&, const _Iterator&,
	const _Function&, unsigned thread_count =
	std::thread::hardware_concurrency() * 2);

template<class type, class _Function>
void parallel_for(const type& begin, const type& end, const type& step,
	const _Function& func, unsigned thread_count =
	std::thread::hardware_concurrency() * 2);

//
double cl_coef(const graph&);
graph cl_to_gr(const cluster&, const graph&);

void print(const graph&, std::ostream&);

bool compare(const std::pair<unsigned, unsigned>&,
    const std::pair<unsigned, unsigned>&);

void print(const result_clusterisation&, const std::string &,
    double log_base = 1.5);

void print(const result_clusterisation&, std::ostream&,
    double log_base = 1.5);

// Ассортативность
double assort(const graph&);

// Ассортативность(вычисляется на gpu)
double assort_gpu(const std::vector<std::vector<unsigned>>&,
    const std::vector<std::vector<unsigned>>&, unsigned);

// Ассортативность для графа в виде списка рёбер
// gr - путь к текстовому файлу списка рёбер
double assort(const std::string &);

// Диаметр графа(возвращает отображение длины пути на количество путей такой длины)
std::vector<unsigned> diameter(const graph&, unsigned ret_size = 0);

#endif // _GRAPHUTILITY_H_
