#pragma once
#ifndef _GRAPHUTILITY_H_
#define _GRAPHUTILITY_H_
#include <random>
#include <ctime>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include "clusterisator.h"

#include <fstream>

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

std::map<unsigned, double> page_rank(const graph&, const double& c = 0.85,
    const double& delta = 0.01);

// Генерирует кластеризованый граф
// v_count - количество вершин, min_cl_size - минимальный размер кластера,
// max_cl_size- максимальный размер кластера, q_cl - вероятность наличия ребра в кластере
graph clustered_graph(unsigned, unsigned,
    unsigned, double q_cl = 0.8);



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

double assort(const graph&);

double assort_gpu(const std::vector<std::vector<unsigned>>&,
    const std::vector<std::vector<unsigned>>&, unsigned);

double assort(const std::string &);

std::map<unsigned, unsigned> diameter(const graph&);

double rim(double, double e = 0.0001);

double deg_distribution(const graph&);

typedef std::vector<std::pair<std::vector<unsigned>,
std::vector<unsigned>>> new_graph;

new_graph generate(unsigned, unsigned, unsigned);

#endif // _GRAPHUTILITY_H_
