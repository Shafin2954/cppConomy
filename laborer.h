#pragma once
#include "consumer.h"

class laborer : public consumer {
public:
    double skillLevel; // 0 - 1, affects wage
    double minWage; // daily wage

    laborer(int id, std::string name, int age, double skillLevel, double minWage) : consumer(id, name, age), skillLevel(skillLevel), minWage(minWage) {}
};