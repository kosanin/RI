#include <iostream>
#include <fstream>
#include "individual.h"

#include "GA.h"

int main() {
    GA t(2, 25, 0.2);
    t.fit();

    return 0;
}