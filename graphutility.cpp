#include "graphutility.h"

template<class _Iterator, class _Function>
void parallel_for_each(const _Iterator& begin, const _Iterator& end,
	const _Function& func, unsigned thread_count)
{
	std::thread** threads = new std::thread*[thread_count];
	for (unsigned id = 0; id < thread_count; ++id)
	{
		threads[id] = new std::thread([id, &thread_count, &begin, &end, &func](){
			auto it = begin;
			for (unsigned char i = 0; i < id && it != end; ++i)
				++it;
			while (it != end)
			{
				func(*it, id);
				for (unsigned char i = 0; i < thread_count && it != end; ++i)
					++it;
			}
		});
	}
	for (unsigned i = 0; i < thread_count; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}

template<class type, class _Function>
void parallel_for(const type& begin, const type& end, const type& step,
	const _Function& func, unsigned thread_count)
{
	std::thread** threads = new std::thread*[thread_count];
	for (unsigned id = 0; id < thread_count; ++id)
	{
		threads[id] = new std::thread([id, &step, &thread_count, &begin, &end, &func](){
			auto it = begin;
			for (unsigned char i = 0; i < id && it < end; ++i)
				it += step;
			while (it < end)
			{
				func(it, id);
				for (unsigned char i = 0; i < thread_count && it < end; ++i)
					++it;
			}
		});
	}
	for (unsigned i = 0; i < thread_count; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}

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
		gr.connect_by_index(i, temp);
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
				gr.merge_by_index(h, k);
			}
		}
	}
	return gr;
}

void top_sort::dfs(unsigned v)
{
	used[v] = true;
	for (auto& it : (*gr)[v].output)
	{
		if (it.first != v && !used[it.first])
			dfs(it.first);
	}
	ans.push_back(v);
}

std::map<unsigned, double> page_rank(const graph& gr, const double& c,
									 const double& delta)
{
	std::vector<double> rt(gr.size(), 1.0);
	top_sort srt;
	const std::vector<unsigned>& v_s = srt.sort(gr);
	double dlt;
	unsigned count = 0;
	std::vector<unsigned> empt;
	for (auto& i : gr)
	{
		if (gr[i.second].input.size() + gr[i.second].output.size() != 0)
			empt.push_back(i.first);
	}
	do
	{
		dlt = 0;
		for (auto& i : v_s)
		{
			double temp = 0, total = 0;
			for (auto& v : gr[i].input)
			{
				temp += rt[v.first] / (gr[v.first].out_d -
						gr[v.first].loop);
			}
			total += temp*c;
			temp = 0;
			for (auto& k : empt)
				temp += rt[k];
			total += c / gr.size()*temp;
			total += (1 - c) / gr.size();

			temp = rt[i];
			if (abs(temp - total) > dlt)
				dlt = std::abs(temp - total);
			rt[i] = total;
		}
	} while (dlt > delta);
	std::map<unsigned, double> ret;
	for (auto& i : gr)
		ret.insert({ i.first, rt[i.second] });
	return ret;
}

// Генерирует кластеризованый граф
// v_count - количество вершин, min_cl_size - минимальный размер кластера,
// max_cl_size- максимальный размер кластера, q_cl - вероятность наличия ребра в кластере
result_graph_generator clustered_graph(unsigned v_count, unsigned min_cl_size,
					  unsigned max_cl_size, double q_cl)
{
	if (min_cl_size > max_cl_size || max_cl_size > v_count ||
		min_cl_size <= 1)
	{
		throw(std::string("Wrong cl_size params"));
	}
	if (q_cl <= 0.5 || q_cl > 1)
	{
		throw(std::string("Wrong q_cl params"));
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
			throw(std::string("Wrong cl_size params"));
	}
	// генерим кластеры размерностью size[i] каждый.
	result_graph_generator ret;
	ret.clusters.resize(size.size());
	ret.vertexes.resize(v_count);
	for (unsigned i = 0; i < v_count; ++i)
		ret.gr.new_vertex(i);
	std::vector<std::pair<unsigned, unsigned>> clusters;
	for (unsigned b_begin = 0, cl = 0; cl < size.size(); ++cl)
	{
		unsigned b_end = b_begin + size[cl];
		clusters.push_back({ b_begin, b_end });
		for (unsigned i = b_begin; i < b_end; ++i)
		{
			ret.clusters[cl].g.push_back(i);
			ret.vertexes[i] = cl;
			for (unsigned j = i + 1; j < b_end; ++j)
			{
				if (q_def(gen) <= q_cl)
				{
					++ret.clusters[cl].aij_sum;
					ret.clusters[cl].di_sum += 2;
					if (in_out(gen) == 0)
						ret.gr.connect_by_index(j, i, 1);
					else
						ret.gr.connect_by_index(i, j, 1);
				}
			}
		}
		auto end = ret.gr.find(b_end);
		for (auto it = ret.gr.find(b_begin); it != end; ++it)
		{
			if (ret.gr[it->second].in_d + ret.gr[it->second].out_d > 0)
				continue;
			std::uniform_int_distribution<int> rand_v(b_begin, b_end - 1);
			unsigned r_v;
			do{
				r_v = rand_v(gen);
			} while (r_v == it->first);

			++ret.clusters[cl].aij_sum;
			ret.clusters[cl].di_sum += 2;
			auto it1 = ret.gr.find(r_v);
			if (in_out(gen) == 0)
				ret.gr.connect(it1, it, 1);
			else
				ret.gr.connect(it, it1, 1);
		}
		b_begin = b_end;
	}
	size.clear();
	// Зашумление
	for (unsigned i = 0; i < clusters.size(); ++i)
	{
		for (unsigned k = i + 1; k < clusters.size(); ++k)
		{
			for (unsigned it_f = clusters[i].first;
				 it_f < clusters[i].second; ++it_f)
			{
				for (unsigned it_s = clusters[k].first;
					 it_s < clusters[k].second; ++it_s)
				{
					if (q_def(gen) <= q_cl)
						continue;
					++ret.clusters[i].di_sum;
					++ret.clusters[k].di_sum;
					if (in_out(gen) == 0)
						ret.gr.connect_by_index(it_s, it_f, 1);
					else
						ret.gr.connect_by_index(it_f, it_s, 1);
				}
			}
		}
	}
	return ret;
}


// Коэффициент сластеризации
double cl_coef(const graph& gr)
{
	double trgl = 0, no_trgl = 0;
	for (auto& i : gr)
	{
		for (auto k = gr[i.second].input.upper_bound(i.second);
			 k != gr[i.second].input.end(); ++k)
		{
			if (gr[i.second].input.size() < gr[k->first].input.size())
			{
				for (auto l = gr[i.second].input.upper_bound(k->first);
					 l != gr[i.second].input.end(); ++l)
				{
					auto it_2 = gr[k->first].input.find(l->first);
					if (it_2 != gr[k->first].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].input.upper_bound(k->first);
					 l != gr[k->first].input.end(); ++l)
				{
					auto it_2 = gr[i.second].input.find(l->first);
					if (it_2 != gr[i.second].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}

			if (gr[i.second].output.size() < gr[k->first].input.size())
			{
				for (auto l = gr[i.second].output.upper_bound(k->first);
					 l != gr[i.second].output.end(); ++l)
				{
					auto it_2 = gr[k->first].input.find(l->first);
					if (it_2 != gr[k->first].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].input.upper_bound(k->first);
					 l != gr[k->first].input.end(); ++l)
				{
					auto it_2 = gr[i.second].output.find(l->first);
					if (it_2 != gr[i.second].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}

			if (gr[i.second].input.size() < gr[k->first].output.size())
			{
				for (auto l = gr[i.second].input.upper_bound(k->first);
					 l != gr[i.second].input.end(); ++l)
				{
					auto it_2 = gr[k->first].output.find(l->first);
					if (it_2 != gr[k->first].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].output.upper_bound(k->first);
					 l != gr[k->first].output.end(); ++l)
				{
					auto it_2 = gr[i.second].input.find(l->first);
					if (it_2 != gr[i.second].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}

			if (gr[i.second].output.size() < gr[k->first].output.size())
			{
				for (auto l = gr[i.second].output.upper_bound(k->first);
					 l != gr[i.second].output.end(); ++l)
				{
					auto it_2 = gr[k->first].output.find(l->first);
					if (it_2 != gr[k->first].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].output.upper_bound(k->first);
					 l != gr[k->first].output.end(); ++l)
				{
					auto it_2 = gr[i.second].output.find(l->first);
					if (it_2 != gr[i.second].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
		}

		for (auto k = gr[i.second].output.upper_bound(i.second);
			 k != gr[i.second].output.end(); ++k)
		{
			if (gr[i.second].input.size() < gr[k->first].input.size())
			{
				for (auto l = gr[i.second].input.upper_bound(k->first);
					 l != gr[i.second].input.end(); ++l)
				{
					auto it_2 = gr[k->first].input.find(l->first);
					if (it_2 != gr[k->first].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].input.upper_bound(k->first);
					 l != gr[k->first].input.end(); ++l)
				{
					auto it_2 = gr[i.second].input.find(l->first);
					if (it_2 != gr[i.second].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}

			if (gr[i.second].output.size() < gr[k->first].input.size())
			{
				for (auto l = gr[i.second].output.upper_bound(k->first);
					 l != gr[i.second].output.end(); ++l)
				{
					auto it_2 = gr[k->first].input.find(l->first);
					if (it_2 != gr[k->first].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].input.upper_bound(k->first);
					 l != gr[k->first].input.end(); ++l)
				{
					auto it_2 = gr[i.second].output.find(l->first);
					if (it_2 != gr[i.second].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}

			if (gr[i.second].input.size() < gr[k->first].output.size())
			{
				for (auto l = gr[i.second].input.upper_bound(k->first);
					 l != gr[i.second].input.end(); ++l)
				{
					auto it_2 = gr[k->first].output.find(l->first);
					if (it_2 != gr[k->first].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].output.upper_bound(k->first);
					 l != gr[k->first].output.end(); ++l)
				{
					auto it_2 = gr[i.second].input.find(l->first);
					if (it_2 != gr[i.second].input.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}

			if (gr[i.second].output.size() < gr[k->first].output.size())
			{
				for (auto l = gr[i.second].output.upper_bound(k->first);
					 l != gr[i.second].output.end(); ++l)
				{
					auto it_2 = gr[k->first].output.find(l->first);
					if (it_2 != gr[k->first].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
			else
			{
				for (auto l = gr[k->first].output.upper_bound(k->first);
					 l != gr[k->first].output.end(); ++l)
				{
					auto it_2 = gr[i.second].output.find(l->first);
					if (it_2 != gr[i.second].output.end())
						trgl += (unsigned)k->second * l->second * it_2->second;
				}
			}
		}
	}
	for (auto& i : gr)
	{
		double k = (gr[i.second].in_d + gr[i.second].out_d) - 2 * gr[i.second].loop;
		no_trgl += k*(k - 1);
	}
	return 6 * trgl / no_trgl;
}

graph cl_to_gr(const cluster& cl, const graph& parrent_graph)
{
	graph ret;
	for (auto& i : cl.g)
	{
		ret.new_vertex(i);
	}
	for (auto& i : ret)
	{
		auto it0 = parrent_graph.find(i.first); // вершина с тем же именем, но в parrent_graph
		ret.connect_by_index(i.second, i.second,
							 parrent_graph[it0->second].loop);
		for (auto& k : parrent_graph[it0->second].output)
		{
			auto it1 = ret.find(parrent_graph[k.first].name);
			if (it1 != ret.end())
				ret.connect_by_name(i.first, it1->first, k.second);
		}
	}
	return ret;
}

void print(const graph& gr, std::ostream& os)
{
	os << "graph gr {\n";
	for (auto& v : gr)
	{
		if (gr[v.second].in_d + gr[v.second].out_d == 0)
			os << v.first << '\n';
	}
	for (auto& v : gr)
	{
		for (unsigned k = 0; k < gr[v.second].loop; ++k)
			os << v.first << " -- " << v.first << '\n';
		for (auto& i : gr[v.second].output)
		{
			for (unsigned k = 0; k < i.second; ++k)
				os << v.first << " -- " << gr[i.first].name << '\n';
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
		v.push_back({i.second.size(), i.first});
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
		for (auto& k : result.connections[i.second].output)
		{
			for (unsigned n = 0; n < k.second; ++n)
				fout << '\t' << i.first << " -- " <<
						result.connections[k.first].name << "\n";
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
		v.push_back({i.second.size(), i.first});
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
		for (auto& k : result.connections[i.second].output)
		{
			out << '\t' << i.first << " -> " << result.connections[k.first].name <<
				   " [ label = \"" << k.second << "\" ];\n";
		}
	}
	out << '}';
}

// Ассортативность
double assort(const graph& gr)
{
	double l = gr.edge_count();
	double f_sum = 0, d_sum = 0, d_sqr = 0, f_sqr = 0, df_sum = 0;
	for (auto& i : gr)
	{
		for (auto& k : gr[i.second].output)
		{
			d_sum += (unsigned)gr[i.second].out_d*k.second;
			f_sum += (unsigned)gr[k.first].in_d*k.second;
			d_sqr += ((unsigned)gr[i.second].out_d * gr[i.second].out_d)*k.second;
			f_sqr += ((unsigned)gr[k.first].in_d * gr[k.first].in_d)*k.second;
			df_sum += ((unsigned)gr[i.second].out_d * gr[k.first].in_d)*k.second;
		}
	}
	double num = l * df_sum - d_sum*f_sum;
	double denum = sqrt((l*d_sqr - d_sum*d_sum)*(l*f_sqr - f_sum*f_sum));
	return num / denum;
}

// Ассортативность(вычисляется на gpu)
double assort_gpu(const std::vector<std::vector<unsigned>>& _in,
				  const std::vector<std::vector<unsigned>>& _out, unsigned edge_count)
{
	double f_sum = 0, d_sum = 0, d_sqr = 0, f_sqr = 0, df_sum = 0;

	double num = edge_count * df_sum - d_sum*f_sum;
	double denum = sqrt((edge_count*d_sqr - d_sum*d_sum)*
						(edge_count*f_sqr - f_sum*f_sum));
	return num / denum;
}

// Ассортативность для графа в виде списка рёбер
// gr - путь к текстовому файлу списка рёбер
double assort(const std::string& gr)
{
	std::unordered_map<unsigned, std::pair<unsigned, unsigned>> degs;
	std::fstream fin(gr);
	unsigned in, out, l = 0;
	std::string str;
	while (getline(fin, str))
	{
		int pos = str.find(" -- ");
		if (pos == -1)
			continue;
		in = atoi(str.substr(0, pos).c_str());
		auto it = degs.find(in);
		if (it != degs.end())
			++(it->second.first);
		else
			degs.insert({ in, { 1, 0 } });
		out = atoi(str.substr(pos + 4, str.size() - pos - 4).c_str());
		it = degs.find(out);
		if (it != degs.end())
			++(it->second.second);
		else
			degs.insert({ in, { 0, 1 } });
		++l;
	}
	fin.close();

	fin.open(gr);
	double f_sum = 0, d_sum = 0, d_sqr = 0, f_sqr = 0, df_sum = 0;
	while (getline(fin, str))
	{
		int pos = str.find(" -- ");
		if (pos == -1)
			continue;
		unsigned in_it = degs[atoi(str.substr(0, pos).c_str())].first;
		unsigned out_it = degs[atoi(str.substr(pos + 4,
											   str.size() - pos - 4).c_str())].second;
		d_sum += in_it;
		f_sum += out_it;
		d_sqr += in_it * in_it;
		f_sqr += out_it * out_it;
		df_sum += in_it * out_it;
	}
	double num = l * df_sum - d_sum*f_sum;
	double denum = sqrt((l*d_sqr - d_sum*d_sum)*(l*f_sqr - f_sum*f_sum));
	return num / denum;
}

// Диаметр графа(возвращает отображение длины пути на количество путей такой длины)
std::vector<unsigned> diameter(const graph& gr, unsigned ret_size)
{
	std::mutex resize_mut;
		unsigned t_count = std::thread::hardware_concurrency() * 2;
		std::vector<std::vector<unsigned>> ret(ret_size, std::vector<unsigned>(t_count, 0));
		parallel_for_each(gr.begin(), gr.end(),
			[&](std::pair<unsigned, unsigned> i, unsigned t_id)
		{
			//std::cout << i.second << std::endl;
			std::vector<bool> used(gr.size(), false); // пройденные вершины
			used[i.second] = true;
			std::queue<unsigned> id;
			id.push(i.second);
			for (unsigned index = 0;;)
			{
				std::queue<unsigned> new_id;
				while (!id.empty())
				{
					auto& it1 = gr[id.front()];
					id.pop();
					for (auto it2 = it1.output.upper_bound(i.second);
						it2 != it1.output.end(); ++it2)
					{
						if (!used[it2->first])
						{
							new_id.push(it2->first);
							used[it2->first] = true;
						}
					}
					for (auto it2 = it1.input.upper_bound(i.second);
						it2 != it1.input.end(); ++it2)
					{
						if (!used[it2->first])
						{
							new_id.push(it2->first);
							used[it2->first] = true;
						}
					}
				}
				++index;
				if (new_id.empty())
					break;
				else
				{
					if (ret.size() <= index)
					{
						std::lock_guard<std::mutex> lc(resize_mut);
						if (ret.size() <= index)
							ret.resize(index + 1, std::vector<unsigned>(t_count, 0));
					}
					ret[index][t_id] += new_id.size();
				}
				id = std::move(new_id);
			}
		}, t_count);
		std::vector<unsigned> result(ret.size(), 0);
		for (unsigned i = 0; i < result.size(); ++i)
			for (unsigned k = 0; k < t_count; ++k)
				result[i] += ret[i][k];
		return result;
}

double modular(const result_graph_generator& res)
{
	double sum1=0, sum2=0;
	double t= 1.0/(2*res.gr.edge_count());
	for(auto& i : res.clusters)
	{
		sum1 += i.aij_sum;
		sum2 += i.di_sum * i.di_sum;
	}
	return t*(2.0*sum1 - t*sum2);
}
