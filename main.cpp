#include <iostream>
#include <fstream>
#include "individual.h"

#include "GA.h"


// funkcija koja izvrsava GA nad svim instancama CAB skupa podataka
void eval_all();

int main() {

    GA t(2, 25, 0.2);
    t.fit();

    return 0;
}

void eval_all()
{
    std::ofstream output("../rezultati.txt");
    for(int p = 2; p <= 4; ++p) {
        for(int n = 20; n <= 25; n+=5) {
            for(float alpha = 0.2; alpha <= 1.0; alpha += 0.2) {
                GA t(p, n, alpha);
                auto best = t.fit();
                output << n << " " << " " << p << " " << alpha << " " << best.fitness();
                output << "\n";
            }
        }
    }
    output.close();
}

