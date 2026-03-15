#ifndef MAPSYSTEM_H
#define MAPSYSTEM_H

#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <queue>
#include "Drona.h"
#include "Robot.h"
#include "Scooter.h"

using namespace std;

class AgentFactory { 
public:
    static shared_ptr<Agent> create(string type, int id) {
    //convertim ID-ul numeric in string pentru constructorii agentilor
    string idStr = to_string(id);

    if (type=="DRONA") {
        // Alocam o drona noua
        return make_shared<Drona>(idStr);
    } 

    else if (type=="ROBOT") {
        // Alocam un robot nou
        return make_shared<Robot>(idStr);
    } 
    else if (type=="SCOOTER") {
        // Alocam un scuter nou
        return make_shared<Scooter>(idStr);
    }

    return nullptr;
}
};

// GENERARE Harta
class IMapGenerator { 
public:
    virtual void generate(vector<string>& grid, int& M, int& N) = 0;
    virtual ~IMapGenerator() {}
};

class ProceduralGenerator : public IMapGenerator { 
public:
    void generate(vector<string>& grid, int& M, int& N) override {
        grid.assign(M, string(N, '.'));
        for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) 
        {
                if ((rand() % 100) < 15) grid[i][j] = '#';
        }
        }
        grid[0][0] = 'B';
    }
};

class FileMapLoader : public IMapGenerator { 
    string filename;
public:
    FileMapLoader(const string& file) : filename(file) {}
    void generate(vector<string>& grid, int& M, int& N) override {
        ifstream f(filename);
        if (!f) throw HiveMindException("Nu se poate deschide fisierul de harta " + filename);
        f >> M >> N;
         grid.clear();
         grid.resize(M);
        for (int i = 0; i < M; i++) 
        {
            f >> grid[i];
            if ((int)grid[i].size()!= N) throw HiveMindException("Linie invalida in fisierul de harta");
        }
    }
};
#endif
