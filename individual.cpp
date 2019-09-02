//
// Created by petar on 11.8.19..
//

#include <iostream>
#include <random>
#include <algorithm>
#include "individual.h"
#include <fstream>


individual::individual()
{
}

individual::individual(std::vector<std::pair<bool, int>>&& gc)
    : m_genetic_code(gc)
{

    for(int i = 0; i < m_genetic_code.size(); ++i) {
        if (m_genetic_code[i].first)
            m_hubs.push_back(i+1);
    }

    m_fitness = 0.0;
}

int individual::generate_random_int(int min_val, int max_val)
{
    // generise brojeve iz U[min_val, max_val]
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> unif(min_val, max_val);

    return unif(gen);
}

std::vector<int> individual::generate_random_hubs(int p, int n)
{
    // puni vektor sa p brojeva iz U[1, n]
    std::vector<int> xs(p);
    std::generate(xs.begin(), xs.end(), [this, n] () { return generate_random_int(1, n); });

    std::sort(xs.begin(), xs.end());

    // resavanje problema duplikata
    auto it = xs.begin();
    while (true) {
        // pronalazimo prvi par duplikata
        it = std::adjacent_find(it, xs.end());
        if (it != xs.end()) {
            // za duplikate sa brojem n, jedan postavljamo na 1
            // i ponavljamo postupak pronalazenja duplikata jedinica
            if (*it == n) {
               *it = 1;
               std::sort(xs.begin(), it+1);
               it = xs.begin();
           } else {
                *it += 1;
                std::sort(it, xs.end());
           }
        } else {
            break;
        }
    }
    return xs;
}

individual::individual(int p, int n, const std::vector<std::vector<double>>& C)
    : m_hubs((generate_random_hubs(p, n)))
    , m_genetic_code(n)
{
    for (int i = 0; i < n; ++i) {
        // provera da li je trenutni cvor hub
        auto it = std::find(m_hubs.cbegin(), m_hubs.cend(), i+1);
        if (it != m_hubs.end())
            // ako je hub postavljamo na prvoj pozicij 1
            // druga nije bitna jer je svaki hub dodeljen sam sebi
            m_genetic_code[i] = std::make_pair(1, 0);
        else
            // resenje 1
            // povezivanje sa najblizim habom
            m_genetic_code[i] = std::make_pair(0, closest_hub(i, C));
    }
    // inicijalno postavljamo fitnes na nula
    // kasnije se fitness racuna u fji evaluate_fitness
    m_fitness = 0;
}

size_t individual::closest_hub(int non_hub, const std::vector<std::vector<double>>& C) {

    // trazimo indeks najblizeg haba za ne-hab
    size_t c = 0;
    float curr_shortest = C[non_hub][m_hubs[0] - 1];
    for(size_t i = 0; i < m_hubs.size(); ++i) {
        if (curr_shortest > C[non_hub][m_hubs[i] - 1]) {
            curr_shortest = C[non_hub][m_hubs[i] - 1];
            c = i;
        }
    }
    return c;
}

std::ostream& operator<< (std::ostream& os, const individual& i)
{
    os << "genetic code: ";
    std::for_each(
            i.m_genetic_code.cbegin(),
            i.m_genetic_code.cend(),
            [&os] (auto elem) { os << elem.first << elem.second << "|"; });

    os << "\n";
    os << "fitness: " << i.m_fitness << std::endl;
    os << "m_hubs: ";
    std::for_each(i.m_hubs.cbegin(), i.m_hubs.cend(), [&os] (auto h) { os << h << " "; });
    os << "\n";

    return os;
}

void individual::evaluate_fitness(
        const std::vector<std::vector<double>>& C,
        const std::vector<std::vector<double>>& W,
        const float alpha)
{
    m_fitness = 0.0;
    int n = m_genetic_code.size();

    for(int i = 0; i < n; ++i) {
        for(int j = i; j < n; ++j) {
            auto i_hub = m_hubs[m_genetic_code[i].second] - 1;
            auto j_hub = m_hubs[m_genetic_code[j].second] - 1;

            // preskacemo habove
            if (m_genetic_code[i].first || m_genetic_code[j].first)
                continue;

            //m_fitness += W[i][j] * (C[i][i_hub] + alpha * C[i_hub][j_hub] + C[j][j_hub]);
            m_fitness += C[i][i_hub] + alpha * C[i_hub][j_hub] + C[j][j_hub];
        }
    }
}

void individual::set_genetic_code(const std::vector<std::pair<bool, int>>& gc) {
   m_genetic_code = gc;
}
void individual::set_hubs(const std::vector<int>& h) {
   m_hubs = h;
}
void individual::set_fitness(float f) {
   m_fitness = f;
}

auto individual::genetic_code() const
    -> std::vector<std::pair<bool, int>>
{
    return m_genetic_code;
}
auto individual::hubs() const
    -> std::vector<int>
{
    return m_hubs;
}
double individual::fitness() const
{
    return m_fitness;
}

