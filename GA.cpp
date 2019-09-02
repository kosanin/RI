//
// Created by petar on 11.8.19..
//
#include "GA.h"
#include <random>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <thread>

// inicijalna populacija sacinjena od m_population_size jediniki
GA::GA(int p, int n, float alpha)
    : alpha(alpha)
{
    // ucitavamo CAB skup podataka
    read_CAB();

    // definisemo inicijalnu populaciju
    for (int i = 0; i < m_population_size; ++i) {
       m_population.push_back(individual(p, n, C));
       m_population[i].evaluate_fitness(C, W, alpha);
    }


    // cuvamo trenutno najbolje resenje
    best_solution = m_population[0];

}

auto GA::population() const
    -> std::vector<individual>
{
    return m_population;
}


void GA::swap(std::vector<std::pair<bool, int>> &x, std::vector<std::pair<bool, int>> &y, size_t index)
{
    // jednopoziciono menjanje genetskog koda
    auto tmp = x[index];
    x[index] = y[index];
    y[index] = tmp;
}

std::pair<individual, individual> GA::crossover(const individual& p1, const individual& p2)
{
    // prepisujemo roditeljske gene
    std::vector<std::pair<bool, int>> c1_gc(p1.genetic_code());
    std::vector<std::pair<bool, int>> c2_gc(p2.genetic_code());

    // menjamo gene na sledecim u slucaju da:
    // parent1[i] = 0       parent1[j] = 1
    // parent2[i] = 1       parent2[j] = 0
    for(size_t i = 0, j = c1_gc.size() - 1; i <= j; ++i, --j) {
        if (!c1_gc[i].first && c2_gc[i].first) {
            swap(c1_gc, c2_gc, i);
        }

        if (c1_gc[j].first && !c2_gc[j].first) {
            swap(c1_gc, c2_gc, j);
        }
    }

    // ispravljanje genetickog koda, ako je potrebno
    genetic_code_correction(c1_gc, p1.hubs().size());
    genetic_code_correction(c2_gc, p1.hubs().size());

    return std::make_pair(
            individual(std::move(c1_gc)),
            individual(std::move(c2_gc)));
}

void GA::genetic_code_correction(std::vector<std::pair<bool, int>> &gc, int p)
{

    // racunamo broj jedinica u zapisu
    int ones = 0;
    for(auto x : gc) {
       ones += x.first;
    }

    bool flag;
    // ako je broj jedinica manji od p, trazimo 0 koje zamenjuemo u 1
    if (ones < p)       flag = false;
    // analaogno za slucaj kad ima visak jedinica
    else if (ones > p)  flag = true;
    else                return;

    while(ones != p) {
        // trazimo prvo pojavljivanje flag-a
        auto it = std::find_if(gc.begin(),gc.end(),[flag] (const auto& x) { return x.first == flag;});
        if (it != gc.end()) {
            (*it).first = !flag;
            ones += flag ? -1 : 1;
        }
    }
}


void GA::mutation(individual &i)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution ber(m_mutation_probability);
    std::uniform_int_distribution<> unif(0, i.hubs().size()-1);

    auto local_gc = i.genetic_code();
    // svaki element mutira sa verovatnocom m_mutation_probability
    std::transform(
            local_gc.begin(),
            local_gc.end(),
            local_gc.begin(),
            [&ber, &gen, &unif] (auto& elem) {
               if (ber(gen) && !elem.first) {
                   // ako dodje do mutacije
                   // povezujemo trenutni cvor sa proizvoljnim habom
                  return std::make_pair(elem.first, unif(gen));
               }
               return elem;
            });

    i.set_genetic_code(local_gc);

}

auto GA::roulette_sel(double total_fitness)
    -> individual
{
    // biramo random vrednost od 0 do total_fitness
    float val = std::rand() % (int)total_fitness;

    double current_sum = 0;
    for (auto x : m_population) {
        current_sum += x.fitness();

        // vracamo prvu jedinku koja ispuni uslov
        if (current_sum > val)
            return x;
    }
}

auto GA::selection() -> std::vector<individual>
{
    std::vector<individual> selected_ones;

    double total_fitness = 0;
    for(const auto& i : m_population)
        total_fitness += i.fitness();

    for(int i = 0; i < m_reproduction_size; ++i) {

        // shuffle populaciju kako ne bi neke jedinke imale prednost u selekciji
        if ( i % 10 == 0) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::shuffle(m_population.begin(), m_population.end(), gen);
        }
        selected_ones.push_back(roulette_sel(total_fitness));
    }

    return selected_ones;
}

auto GA::create_generation(const std::vector<individual>& selected) -> std::vector<individual>
{

    std::sort(
            m_population.begin(),
            m_population.end(),
            [] (const individual& a, const individual& b) { return a.fitness() < b.fitness();});

    // elitizam
    // prepisujemo 50 najboljih jedinki
    std::vector<individual> new_gen(m_population.begin(), m_population.begin() + 100);
    //std::vector<individual> new_gen;
    int new_gen_size = 0;

    while(new_gen_size < m_population_size - 100) {
        // biramo 2 random roditelja iz grupe odabrane za reprodukciju
        auto parent1 = selected[std::rand() % m_reproduction_size];
        auto parent2 = selected[std::rand() % m_reproduction_size];

        // ukrstanje roditelja
        auto children = crossover(parent1, parent2);

        // mutacija dobijenih jedinki
        mutation(children.first);
        mutation(children.second);

        // racunanje fitnesa jedinki
//        std::thread t1(&individual::evaluate_fitness, &children.first, std::ref(C), std::ref(W));
//        std::thread t2(&individual::evaluate_fitness, &children.second, std::ref(C), std::ref(W));
        children.first.evaluate_fitness(C, W, alpha);
        children.second.evaluate_fitness(C, W, alpha);
//        t1.join();
//        t2.join();

        // dodavanje u novu generaciju
        new_gen.push_back(children.first);
        new_gen.push_back(children.second);

        new_gen_size += 2;
    }

    return new_gen;
}


auto GA::fit() -> individual
{
    // brojac koliko puta uzastopno smo dobili isto resenje
    int num_same = 0;
    for (int i = 0; i < m_max_iters; ++i) {

        // ukoliko se u 100 iteracija nije promenlio resenje
        // stop
        if (num_same >= 200) {
            break;
        }

        // selekcija
        auto selected = selection();

        // stvaranje nove generacije
        m_population = create_generation(selected);

        // iterator na element sa minimalnom fitnesom
        auto it = std::min_element(
                m_population.begin(),
                m_population.end(),
                [] (const auto& a, const auto& b) { return a.fitness() < b.fitness();});


        // proveravamo da li posotji bolje resenje od trenutnog najboljeg
        if (best_solution.fitness() > (*it).fitness()) {
            best_solution = (*it);
            num_same = 0;
        }

        num_same++;

        if ( i % 10 == 0 ) {
            std::cout << "*******************************************\n";
            std::cout << i << ": najbolje resenje\n";
            std::cout << best_solution;
        }

    }
    return best_solution;
}


void GA::read_CAB()
{

    // konstruisemo matricu 25x25
    for(int i = 0; i < 25; ++i) {
        C.push_back(std::vector<double>(25));
    }

    for(int i = 0; i < 25; ++i) {
        W.push_back(std::vector<double>(25));
    }

    std::ifstream input("../CAB.txt");
    int i, j;
    float w, c;
    while(input >> i >> j >> w >> c) {
        W[i - 1][j - 1] = w;
        C[i - 1][j - 1] = c;
    }

    input.close();
}
