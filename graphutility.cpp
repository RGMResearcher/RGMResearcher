#include "graphutility.h"

// Модель Боллобаша-Риордана
graph preferred_attachment(unsigned n, unsigned m)
{
    /* n вершин, n*m рёбер
    граф строится до достижения n*m рёбер и n*m вершин
    затем вершины объединяются друг с другом (каждые m штук в одну), пока их не станет ровно n
    */
    n *= m;
    std::vector<unsigned> vec; // хранит номера вершин
    // при вставке ребра между first и second вершинами
    // в vec помещаются и first и second
    graph gr; unsigned i;
    std::mt19937 gen(std::time(0));

    for (i = 0; i < n; ++i)
    {
        gr.new_vertex(i);
        vec.push_back(i);
        // Кидать не 1 исходящую вершину, а несколько
        std::uniform_int_distribution<unsigned> dist(0, vec.size() - 1);
        unsigned temp = vec[dist(gen)];
        gr.connect(i, temp);
        vec.push_back(temp);
    }
    if (m < 2)
        return gr;
    // объединение вершин
    for (i = 0; i < n; i += m)
    {
        for (unsigned k = i + m - 1; k > i;)
        {
            for (unsigned h = i; h < k; ++h, --k)
            {
                gr.merge(h, k);
            }
        }
    }
    return gr;
}

void top_sort::dfs(const unsigned &v, std::vector<unsigned>& ans)
{
    used.insert(v);
    auto it1 = gr->find(v);
    for (auto& it : it1->second.output)
    {
        if (it.first != v && used.find(it.first) == used.end())
            dfs(it.first, ans);
    }
    ans.push_back(v);
}

std::map<unsigned, double> page_rank(const graph& gr, const double& c,
                                     const double& delta)
{
    std::map<unsigned, double> ret;
    for (auto& i : gr)
        ret.insert({ i.first, 1.0 });
    top_sort srt;
    std::vector<unsigned> v_s(std::move(srt(gr)));
    double dlt;
    do
    {
        dlt = 0;
        for (auto& i : v_s)
        {
            double temp = 0, total = 0;
            auto it = gr.find(i);
            for (auto& v : it->second.input)
            {
                // петли не влияют на ранг
                if (v.first == i)
                    continue;
                auto it1 = gr.find(v.first);
                temp += ret[v.first] / (it1->second.out_d -
                                        it1->second.loop);
            }
            total += temp*c;
            temp = 0;
            // место для суммы с никуда не указывающими вершинами
            total += (1 - c) / gr.size();

            temp = ret[i];
            if (abs(temp - total) > dlt)
                dlt = std::abs(temp - total);
            ret[i] = total;
        }
    } while (/*++count <= 2 * gr.size() && */dlt > delta);
    return ret;
}

// Генерирует кластеризованый граф
// v_count - количество вершин, min_cl_size - минимальный размер кластера,
// max_cl_size- максимальный размер кластера, q_cl - вероятность наличия ребра в кластере
std::pair<graph, unsigned> clustered_graph(unsigned v_count, unsigned min_cl_size,
                                           unsigned max_cl_size, double q_cl)
{
    if (min_cl_size > max_cl_size || max_cl_size > v_count ||
            min_cl_size <= 1)
    {
        throw(std::string("Wrong cl_size params"));
    }
    if (q_cl <= 0.5 || q_cl > 1)
    {
        throw(std::string("Wrong cl_size params"));
    }

    std::uniform_int_distribution<int> dist(min_cl_size, max_cl_size);
    std::uniform_int_distribution<int> in_out(0, 1);
    std::uniform_real_distribution<double> q_def(0, 1);

    std::mt19937 gen(time(0));
    std::vector<unsigned> size;
    unsigned count;
    for (count = 0;;)
    {
        unsigned temp = dist(gen);
        count += temp;
        size.push_back(temp);

        if (count >= v_count - max_cl_size && count <= v_count -
                min_cl_size)
        {
            break;
        }
        if (count > v_count - min_cl_size)
            break;
    }
    size.push_back(v_count - count);
    while (size.back() < min_cl_size)
    {
        bool err = true;
        for (unsigned i = 0; i < size.size() - 1 && size.back() <
             min_cl_size; ++i)
        {
            if (size[i] > min_cl_size)
            {
                ++size.back();
                --size[i];
                err = false;
            }
        }
        if (size.back() < min_cl_size && err)
            size.back() = v_count - size.size() * min_cl_size;
    }
    // генерим кластеры размерностью size[i] каждый.
    std::pair<graph, unsigned> ret;
    for (unsigned i = 0; i < v_count; ++i)
        ret.first.new_vertex(i);
    std::vector<std::pair<unsigned, unsigned>> clusters;
    for (unsigned b_begin = 0, cl = 0; cl < size.size(); ++cl)
    {
        unsigned b_end = b_begin + size[cl];
        clusters.push_back({ b_begin, b_end });
        for (unsigned i = b_begin; i < b_end; ++i)
            for (unsigned j = i + 1; j < b_end; ++j)
            {
                if (q_def(gen) <= q_cl)
                {
                    if (in_out(gen) == 0)
                        ret.first.connect(j, i, 1);
                    else
                        ret.first.connect(i, j, 1);
                }
            }
        auto end = ret.first.find(b_end);
        for (auto it = ret.first.find(b_begin); it != end; ++it)
        {
            if (it->second.in_d + it->second.out_d > 0)
                continue;
            std::uniform_int_distribution<int> rand_v(b_begin, b_end - 1);
            unsigned r_v;
            do{
                r_v = rand_v(gen);
            } while (r_v == it->first);

            auto it1 = ret.first.find(r_v);
            if (in_out(gen) == 0)
                ret.first.connect(it1, it, 1);
            else
                ret.first.connect(it, it1, 1);
        }
        b_begin = b_end;
    }
    ret.second = size.size();
    size.clear();
    // Зашумление
    for (unsigned i = 0; i < clusters.size(); ++i)
    {
        for (unsigned k = i + 1; k < clusters.size(); ++k)
        {
            auto end_f = ret.first.find(clusters[i].second);
            for (auto it_f = ret.first.find(clusters[i].first);
                 it_f != end_f; ++it_f)
            {
                auto end_s = ret.first.find(clusters[k].second);
                for (auto it_s = ret.first.find(clusters[k].first);
                     it_s != end_s; ++it_s)
                {
                    if (q_def(gen) <= q_cl)
                        continue;
                    if (in_out(gen) == 0)
                        ret.first.connect(it_s, it_f, 1);
                    else
                        ret.first.connect(it_f, it_s, 1);
                }
            }
        }
    }
    return ret;
}

void print(const result_clusterisation& result, std::ostream& out)
{
    out << result.clusters.size() << '\n';
    auto it = result.vertexes.begin();
    if (it != result.vertexes.end())
    {
        out << it->first << '(' << it->second.size() << ')';
        for (++it; it != result.vertexes.end(); ++it)
            out << ' ' << it->first << '(' << it->second.size() << ')';
    }
    out << '\n';
    for (auto& i : result.vertexes)
    {
        for (auto& k : i.second)
            out << k << '(' << i.first << ")\n";
    }
}

void print(const result_clusterisation& result, const std::string& file,
           double log_base = 1.5)
{
    std::ofstream fout(file);
    fout << "graph result {\n";
    fout << "\tnode [shape = circle];\n";
    for (auto& i : result.vertexes)
    {
        double size = (i.second.size() > 1) ?
                    (log(i.second.size()) / log(log_base)) : 1.0;
        fout << '\t' << i.first << " [ label = \"" << i.second.size() <<
                "\", height = " << size << ", width = " << size << " ];\n";
    }
    for (auto& i : result.connections)
    {
        for (auto& k : i.second.output)
        {
            for (unsigned n = 0; n < k.second; ++n)
                fout << '\t' << i.first << " -- " << k.first << ";\n";
        }
    }
    fout << '}';
    fout.close();
}

unsigned clustered()
{
    graph gr;
    //clusterisator ob;
    std::string str;
    std::pair<unsigned, unsigned> p;
    std::ifstream fin("buddahs.gv");
    while (getline(fin, str))
    {
        unsigned pos = str.find(" -- ");
        std::string tmp_str = str.substr(0, pos);
        p.first = atoi(tmp_str.c_str());
        tmp_str = str.substr(pos + 4, str.size() - pos - 4);
        p.second = atoi(tmp_str.c_str());
        gr.new_vertex(p.first);
        gr.new_vertex(p.second);
        gr.connect(p.first, p.second);
    }
    fin.close();
    /*ob.init(gr);
    for (unsigned i = 0; ob.next_iteration(); ++i)
    {
        std::string name = "result" + std::to_string(i) + ".gv";
        to_gv(ob.result(), name);
    }*/
    std::ofstream f("out.txt");
    f << triangls(gr).first;

    f.close();
    return 0;
}

std::pair<unsigned, double> triangls(const graph& gr)
{
    unsigned trgl = 0;
    for (auto& i : gr)
    {
        for (auto k = i.second.input.upper_bound(i.first);
             k != i.second.input.end(); ++k)
        {
            auto it = gr.find(k->first);
            if (i.second.input.size() < it->second.input.size())
            {
                for (auto l = i.second.input.upper_bound(k->first);
                     l != i.second.input.end(); ++l)
                {
                    auto it_2 = it->second.input.find(l->first);
                    if (it_2 != it->second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.input.upper_bound(k->first);
                     l != it->second.input.end(); ++l)
                {
                    auto it_2 = i.second.input.find(l->first);
                    if (it_2 != i.second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }

            if (i.second.output.size() < it->second.input.size())
            {
                for (auto l = i.second.output.upper_bound(k->first);
                     l != i.second.output.end(); ++l)
                {
                    auto it_2 = it->second.input.find(l->first);
                    if (it_2 != it->second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.input.upper_bound(k->first);
                     l != it->second.input.end(); ++l)
                {
                    auto it_2 = i.second.output.find(l->first);
                    if (it_2 != i.second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }

            if (i.second.input.size() < it->second.output.size())
            {
                for (auto l = i.second.input.upper_bound(k->first);
                     l != i.second.input.end(); ++l)
                {
                    auto it_2 = it->second.output.find(l->first);
                    if (it_2 != it->second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.output.upper_bound(k->first);
                     l != it->second.output.end(); ++l)
                {
                    auto it_2 = i.second.input.find(l->first);
                    if (it_2 != i.second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }

            if (i.second.output.size() < it->second.output.size())
            {
                for (auto l = i.second.output.upper_bound(k->first);
                     l != i.second.output.end(); ++l)
                {
                    auto it_2 = it->second.output.find(l->first);
                    if (it_2 != it->second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.output.upper_bound(k->first);
                     l != it->second.output.end(); ++l)
                {
                    auto it_2 = i.second.output.find(l->first);
                    if (it_2 != i.second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
        }

        for (auto k = i.second.output.upper_bound(i.first);
             k != i.second.output.end(); ++k)
        {
            auto it = gr.find(k->first);
            if (i.second.input.size() < it->second.input.size())
            {
                for (auto l = i.second.input.upper_bound(k->first);
                     l != i.second.input.end(); ++l)
                {
                    auto it_2 = it->second.input.find(l->first);
                    if (it_2 != it->second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.input.upper_bound(k->first);
                     l != it->second.input.end(); ++l)
                {
                    auto it_2 = i.second.input.find(l->first);
                    if (it_2 != i.second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }

            if (i.second.output.size() < it->second.input.size())
            {
                for (auto l = i.second.output.upper_bound(k->first);
                     l != i.second.output.end(); ++l)
                {
                    auto it_2 = it->second.input.find(l->first);
                    if (it_2 != it->second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.input.upper_bound(k->first);
                     l != it->second.input.end(); ++l)
                {
                    auto it_2 = i.second.output.find(l->first);
                    if (it_2 != i.second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }

            if (i.second.input.size() < it->second.output.size())
            {
                for (auto l = i.second.input.upper_bound(k->first);
                     l != i.second.input.end(); ++l)
                {
                    auto it_2 = it->second.output.find(l->first);
                    if (it_2 != it->second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.output.upper_bound(k->first);
                     l != it->second.output.end(); ++l)
                {
                    auto it_2 = i.second.input.find(l->first);
                    if (it_2 != i.second.input.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }

            if (i.second.output.size() < it->second.output.size())
            {
                for (auto l = i.second.output.upper_bound(k->first);
                     l != i.second.output.end(); ++l)
                {
                    auto it_2 = it->second.output.find(l->first);
                    if (it_2 != it->second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
            else
            {
                for (auto l = it->second.output.upper_bound(k->first);
                     l != it->second.output.end(); ++l)
                {
                    auto it_2 = i.second.output.find(l->first);
                    if (it_2 != i.second.output.end())
                        trgl += k->second * l->second * it_2->second;
                }
            }
        }
    }
    std::pair<unsigned, double> ret(trgl, 0.0);
    ret.second = (((gr.edge_count() - 1)*gr.edge_count()*
                   (gr.edge_count() + 1)) / 48.0) * std::pow(log(gr.size()), 3.0);
    return ret;
}

graph cl_to_gr(const cluster& cl, const graph& parrent_graph)
{
    graph ret;
    for (auto& i : cl.g)
    {
        ret.new_vertex(i);
    }
    for (auto it = ret.begin(); it != ret.end(); ++it)
    {
        auto it0 = parrent_graph.find(it->first); // вершина с тем же индексом, но в parrent_graph
        ret.connect(it, it, it0->second.loop);
        for (auto& k : it0->second.output)
        {
            auto it1 = ret.find(k.first);
            if (it1 != ret.end())
                ret.connect(it, it1, k.second);
        }
    }
    return ret;
}

void print(const graph& gr, std::ostream& os)
{
    os << "graph gr {\n";
    for (auto& v : gr)
    {
        for (auto& i : v.second.output)
        {
            for (unsigned k = 0; k < i.second; ++k)
                os << v.first << " -- " << i.first << std::endl;
        }
    }
    os << '}';
}

bool compare(const std::pair<unsigned, unsigned>& i,
             const std::pair<unsigned, unsigned>& j)
{
    return (i > j);
}

void print(const result_clusterisation& result, const std::string& file,
           double log_base)
{
    std::ofstream fout(file);
    fout << "graph result {\n";
    fout << "\tnode [shape = circle];\n";
    std::vector<std::pair<unsigned, unsigned>> v; // размер номер
    /*for (auto& i : result.vertexes)
    {
    double size = (i.second.size() > 1) ?
    (log(i.second.size()) / log(log_base)) : 1.0;
    fout << '\t' << i.first << " [ label = \"" << i.second.size() <<
    "\", height = " << size << ", width = " << size << " ];\n";
    }*/
    for (auto& i : result.vertexes)
    {
        v.push_back({ i.second.size(), i.first });
    }
    std::sort(v.begin(), v.end(), compare);
    for (auto& i : v)
    {
        double size = (i.first > 1) ?
                    (log(i.first) / log(log_base)) : 1.0;
        fout << '\t' << i.second << " [ label = \"" << i.first <<
                "\", height = " << size << ", width = " << size << " ];\n";
    }
    for (auto& i : result.connections)
    {
        for (auto& k : i.second.output)
        {
            for (unsigned n = 0; n < k.second; ++n)
                fout << '\t' << i.first << " -- " << k.first << "\n";
        }
    }
    fout << '}';
    fout.close();
}

void print(const result_clusterisation& result, std::ostream& out,
           double log_base)
{
    out << "digraph result {\n";
    out << "\tnode [shape = circle];\n";
    std::vector<std::pair<unsigned, unsigned>> v; // размер номер
    /*for (auto& i : result.vertexes)
    {
    double size = (i.second.size() > 1) ?
    (log(i.second.size()) / log(log_base)) : 1.0;
    fout << '\t' << i.first << " [ label = \"" << i.second.size() <<
    "\", height = " << size << ", width = " << size << " ];\n";
    }*/
    for (auto& i : result.vertexes)
    {
        v.push_back({ i.second.size(), i.first });
    }
    std::sort(v.begin(), v.end(), compare);
    for (auto& i : v)
    {
        double size = (i.first > 1) ?
                    (log(i.first) / log(log_base)) : 1.0;
        out << '\t' << i.second << " [ label = \"" << i.second << '/' << i.first <<
               "\", height = " << size << ", width = " << size << " ];\n";
    }
    for (auto& i : result.connections)
    {
        for (auto& k : i.second.output)
        {
            out << '\t' << i.first << " -> " << k.first <<
                   " [ label = \"" << k.second << "\" ];\n";
        }
    }
    out << '}';
}

void cl_elements(const result_clusterisation& result, std::ostream& out)
{
    out << result.clusters.size() << '\n';
    auto it = result.vertexes.begin();
    if (it != result.vertexes.end())
    {
        out << it->first << '(' << it->second.size() << ')';
        for (++it; it != result.vertexes.end(); ++it)
            out << ' ' << it->first << '(' << it->second.size() << ')';
    }
    out << '\n';
    for (auto& i : result.vertexes)
    {
        for (auto& k : i.second)
            out << k << '(' << i.first << ")\n";
    }
}

double cl_coef(const graph::iterator& it)
{
    double n = 2.0;

    unsigned k = it->second.in_d + it->second.out_d;
    return 2 * n / (k * (k - 1));
}

double assort(const graph& gr)
{
    double l = gr.edge_count(); // граф ориентированный
    double f_sum = 0, d_sum = 0, d_sqr = 0, f_sqr = 0, df_sum = 0;
    for (auto& i : gr)
    {
        for (auto& k : i.second.output)
        {
            auto it = gr.find(k.first);
            d_sum += i.second.out_d*k.second;
            f_sum += it->second.in_d*k.second;
            d_sqr += (i.second.out_d * i.second.out_d)*k.second;
            f_sqr += (it->second.in_d * it->second.in_d)*k.second;
            df_sum += (i.second.out_d * it->second.in_d)*k.second;
        }
    }
    double num = l * df_sum - d_sum*f_sum;
    double denum = sqrt((l*d_sqr - d_sum*d_sum)*(l*f_sqr - f_sum*f_sum));
    return num / denum;
}

std::map<unsigned, unsigned> diameter(const graph& gr)
{
    std::map<unsigned, unsigned> ret; // длина на количество путей такой длины
        for (auto& i : gr)
        {
            std::cout << i.first << std::endl;
            std::set<unsigned> used; // пройденные вершины
            used.insert(i.first);
            std::vector<std::set<unsigned>> id(1);
            id.back().insert(i.first);
            for (unsigned index = 0;; ++index)
            {
                id.push_back(std::set<unsigned>());
                for (auto& it : id[index])
                {
                    auto it1 = gr.find(it);
                    for (auto it2 = it1->second.output.upper_bound(it1->first);
                        it2 != it1->second.output.end(); ++it2)
                    {
                        if (used.find(it2->first) == used.end())
                        {
                            id[index + 1].insert(it2->first);
                            used.insert(it2->first);
                        }
                    }
                    for (auto it2 = it1->second.input.upper_bound(it1->first);
                        it2 != it1->second.input.end(); ++it2)
                    {
                        if (used.find(it2->first) == used.end())
                        {
                            id[index + 1].insert(it2->first);
                            used.insert(it2->first);
                        }
                    }
                }
                if (id[index + 1].empty())
                    break;
            }
            for (unsigned i = 1; i + 1 < id.size(); ++i)
            {
                auto it = ret.find(i);
                if (it != ret.end())
                    it->second += id[i].size();
                else
                    ret.insert({ i, id[i].size() });
            }
        }
        return ret;
}
