//
// Created by petar on 11.8.19..
//

#ifndef RI_INDIVIDUAL_H
#define RI_INDIVIDUAL_H

#include <utility>
#include <iostream>
#include <vector>

class individual {

public:

    // konstruktori
    individual();
    individual(int p, int n, const std::vector<std::vector<double>>& C);
    individual(std::vector<std::pair<bool, int>>&& gc);

    // getteri
    auto genetic_code() const   -> std::vector<std::pair<bool, int>>;
    auto hubs() const           -> std::vector<int>;
    double fitness() const;

    // setteri
    void set_genetic_code(const std::vector<std::pair<bool, int>>&);
    void set_hubs(const std::vector<int>&);
    void set_fitness(float);

    // izracunavanje fitnes funkcije
    void evaluate_fitness(
            const std::vector<std::vector<double>>& C,
            const std::vector<std::vector<double>>& W,
            const float alpha);

    // ispisivanje jedinke na ekran
    friend std::ostream& operator<< (std::ostream& os, const individual& i);

private:

    // generisanje nasumicnog celog broja
    int generate_random_int(int min, int max);
    // generise random habova
    std::vector<int> generate_random_hubs(int p, int n);

    // vraca indeks najblizeg haba za prosledjeni ne-hab
    size_t closest_hub(const int non_hub, const std::vector<std::vector<double>>& C);


    std::vector<int> m_hubs;
    std::vector<std::pair<bool, int>> m_genetic_code;
    double m_fitness;

};


#endif //RI_INDIVIDUAL_H
