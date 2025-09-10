#include <iostream>

#include "Generator.h"
#include "Correlator.h"

int main()
{
    RandomGenerator gen;
    gen.config();

    uint32_t size = 100;
    std::vector<uint32_t> data;

    gen.generateBits(data, size);

    std::cout << "size: " << data.size() << std::endl;
    for (auto& it: data)
        std::cout << it << " ";
    std::cout << std::endl;

    return 0;
}