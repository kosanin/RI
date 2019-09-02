//
// Created by petar on 11.8.19..
// Genetic Algorithm class
//

#ifndef RI_GA_H
#define RI_GA_H

#include <vector>
#include "individual.h"

class GA {
private:
    // parametri genetskog algoritma
    const int m_population_size = 150;
    const int m_max_iters = 500;
    const float m_mutation_probability = 0.05;
    const int m_reproduction_size = 150;

public:

    std::vector<std::vector<double>> C;
    std::vector<std::vector<double>> W;
    individual best_solution;

    // konstruktor
    GA(int p, int n);

    // operator selekcije
    auto selection() -> std::vector<individual>;

    auto create_generation(const std::vector<individual>& selected) -> std::vector<individual>;

    // operator ukrstanja
    std::pair<individual, individual> crossover(const individual&, const individual&);

    // operator mutacije
    void mutation(individual&);

    // minimizacija ciljne funkcije
    individual fit();

    std::vector<individual> population() const;

private:

    // funkcija za menjanje gena, koristi se u crossover operatoru
    void swap(
            std::vector<std::pair<bool, int>>& x,
            std::vector<std::pair<bool, int>>& y,
            size_t index);

    // crossover operator moze izbaciti nepravilne rezultate
    // genetic_code_correction ispravlja genetski kod td uvek bude p habova aktivno
    void genetic_code_correction(std::vector<std::pair<bool, int>>& gc, int p);

    // ucitavanje CAB fajla
    void read_CAB();

    // biranje jedinke roletskom selekcijom
    individual roulette_sel(double);

    std::vector<individual> m_population;
};


#endif //RI_GA_H
