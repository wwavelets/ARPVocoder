/*
  ==============================================================================

    ModulationWrapper.cpp
    Created: 11 Jan 2022 6:43:06pm
    Author:  erich

  ==============================================================================
*/

#include "ModulationWrapper.h"
#include <map>
#include <vector>

class ModulationWrapper {
public:
    inline unsigned int getID() {
        return curid++;
    }
    inline void destroyID(const unsigned int&id) {
        mods.erase(id);
    }
    inline float getAmt(const unsigned int&id) {
        float res = 0;
        for (auto& mod : mods[id])
            res += modAmt[mod.first] * mod.second;
    }
private:
    std::map<unsigned int, std::map<unsigned int, float>> mods; // reciever -> modulator, amt
    std::map<unsigned int, std::map<unsigned int, float>> mods2; // modulator -> reciever, amt
    std::map<unsigned int, float> modAmt;
    unsigned int curid = 0;
};