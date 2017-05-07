#pragma once
#ifndef _GRAPHUTILITY_H_
#define _GRAPHUTILITY_H_

#include <random>
#include <ctime>
#include <algorithm>
#include <random>
#include <fstream>
#include "clusterisator.h"

// Модель Боллобаша-Риордана
graph preferred_attachment(unsigned, unsigned);

class top_sort: public std::function<
        std::vector<unsigned>(const graph&)>
{
    private:
    mutable std::set<unsigned> used;
    const graph* gr;
    void dfs(const unsigned&, std::vector<unsigned>&);
    public:
    std::vector<unsigned> operator()(const graph& grph)
    {
        gr = &grph;
        used.clear();
        std::vector<unsigned> ans;
        for (auto it = grph.begin(); it != grph.end(); ++it)
        {
            auto i = used.find(it->first);
            if (i == used.end())
                dfs(it->first, ans);
        }
        std::reverse(ans.begin(), ans.end());
        return ans;
    }
};

// не работает
std::map<unsigned, double> page_rank(const graph&, const double& c = 0.85,
                                     const double& delta = 0.01);

// Генерирует кластеризованый граф
// v_count - количество вершин, min_cl_size - минимальный размер кластера,
// max_cl_size- максимальный размер кластера, q_cl - вероятность наличия ребра в кластере
std::pair<graph, unsigned> clustered_graph(unsigned, unsigned,
                                           unsigned, double q_cl = 0.8);
unsigned clustered();
void print(const result_clusterisation&,
           const std::string&, double);
// void print(const result_clusterisation&, std::ostream&);
double cl_coef(const graph&);
graph cl_to_gr(const cluster&, const graph&);
void print(const result_clusterisation&, const std::string&,
           double log_base = 1.5);
void print(const result_clusterisation& result, std::ostream&,
           double log_base = 1.5);
void cl_elements(const result_clusterisation&, std::ostream&);
double cl_coef(const graph::iterator&);
double assort(const graph&);
std::map<unsigned, unsigned> diameter(const graph&);

#endif // _GRAPHUTILITY_H_
