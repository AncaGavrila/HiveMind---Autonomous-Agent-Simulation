#ifndef CITYMAP_H
#define CITYMAP_H


#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <queue>
#include "MapSystem.h"

using namespace std;

// Clasa CityMap- Gestioneaza harta si algoritmii de deplasare
class CityMap {
    int M, N;
    vector<string> grid;
    Position base{0, 0};

public:
    CityMap(int m, int n) : M(m), N(n) {}

    // Metoda de constructie a hartii
    void build(shared_ptr<IMapGenerator> gen, int numS, int numD, vector<Position>& clients) 
{
    // 1. incarcam harta
    gen->generate(grid, M, N);
    clients.clear();

    // 2. scanam harta
    // Acest pas este crucial pentru FileMapLoader
    bool hasPredefinedElements = false;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == 'D') {
                clients.push_back({i, j});
                hasPredefinedElements = true;
            } else if (grid[i][j] == 'S') {
                hasPredefinedElements = true;
            } else if (grid[i][j] == 'B') {
                base = {i, j};
            }
        }
    }

    
    if (!hasPredefinedElements) {
      
        for (int i = 0; i < numD; i++) {
            Position punctRandom;
            do { 
                punctRandom = {rand() % M, rand() % N}; 
            } while (grid[punctRandom.x][punctRandom.y] != '.');

            grid[punctRandom.x][punctRandom.y] = 'D';
            clients.push_back(punctRandom);
        }

       
        for (int i = 0; i < numS; i++) {
            Position punctStatie;
            do { 
                punctStatie = {rand() % M, rand() % N}; 
            } while (grid[punctStatie.x][punctStatie.y] != '.');

            grid[punctStatie.x][punctStatie.y] = 'S';
        }
        
        grid[0][0] = 'B'; 
    }

   
    if (!validateConnectivity()) {
     
        throw HiveMindException("Harta incarcata nu este valida (puncte inaccesibile)!");
    }
}
    // verifica daca punctele  sunt accesibile
    bool validateConnectivity() {
        queue<Position> coadaBFS;
        vector<vector<bool>> vizitat(M, vector<bool>(N, false));

        coadaBFS.push(base);
        vizitat[base.x][base.y] = true;

        while (!coadaBFS.empty()) {
            Position nodCurent = coadaBFS.front();
            coadaBFS.pop();

            // Vectori de deplasare pentru vecini
            int directieX[] = {-1, 1, 0, 0};
            int directieY[] = {0, 0, -1, 1};

            for (int i = 0; i < 4; i++) {
                int vecinaX = nodCurent.x + directieX[i];
                int vecinaY = nodCurent.y + directieY[i];

                // noua pozitie este valida, nu este obstacol
                if (vecinaX >= 0 && vecinaX < M && vecinaY >= 0 && vecinaY < N && 
                    !vizitat[vecinaX][vecinaY] && grid[vecinaX][vecinaY] != '#') {
                    
                    vizitat[vecinaX][vecinaY] = true;
                    coadaBFS.push({vecinaX, vecinaY});
                }
            }
        }

        // vedem daca am putut vizita toti clientii si statiile
        for (int i = 0; i <M; i ++) {
            for (int j = 0; j < N; j++) {
                if ((grid[i][j] == 'D' || grid[i][j] == 'S') && !vizitat[i][j]) 
                {
                    return false;
                }
            }
        }
        return true;
    }

    void displayMap() const {
        cout << "\n HARTA \n";
        for (const auto& randHarta : grid) {
            for (char celula : randHarta) cout << celula << " ";
            cout << "\n";
        }
       
    }

    //  drumul optim in functie de tipul agentului
    vector<Position> getPath(Position start, Position end, bool aerial) {
        if (start.esteEgal(end) == 1) return {};

        // Logica pentru drone (linie dreapta)
        if (aerial) {
            vector<Position> traseuDirect;
            Position pasCurent = start;
            while (pasCurent.esteDiferit(end) == 1) 
            {
                if (pasCurent.x < end.x) pasCurent.x++;
                else if (pasCurent.x > end.x) pasCurent.x--;
                else if (pasCurent.y < end.y) pasCurent.y++;
                else if (pasCurent.y > end.y) pasCurent.y--;
                traseuDirect.push_back(pasCurent);
            }
            return traseuDirect;
        }

        // Logica BFS pentru agenti terestri
        queue<pair<Position, vector<Position>>> coadaProcesare;
        coadaProcesare.push({start, {}});
        
        vector<vector<bool>> puncteVizitate(M, vector<bool>(N, false));
        puncteVizitate[start.x][start.y] = true;

        while (!coadaProcesare.empty()) {
            auto stareCurenta = coadaProcesare.front(); 
            coadaProcesare.pop();

            if (stareCurenta.first.esteEgal(end) == 1) return stareCurenta.second;

            int dx[] = {-1, 1, 0, 0};
            int dy[] = {0, 0, -1, 1};

            for (int i = 0; i < 4; i++) {
                int nX = stareCurenta.first.x + dx[i];
                int nY = stareCurenta.first.y + dy[i];

                if (nX >= 0 && nX < M && nY >= 0 && nY < N && 
                    !puncteVizitate[nX][nY]   &&   grid[nX][nY] != '#')
                     {
                    
                    puncteVizitate[nX][nY] = true;
                    vector<Position> noulDrum = stareCurenta.second;
                    noulDrum.push_back({nX, nY});
                    coadaProcesare.push({{nX, nY}, noulDrum});
                }
            }
        }
        return {};
    }

    Position findNearestStation(Position p)
     {
        Position statieOptima = base;
        int distantaMinima = 989;

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 'S' || grid[i][j] == 'B')
                 {
                    int d = abs(p.x - i) + abs(p.y - j);
                    if (d < distantaMinima) 
                    { 
                        distantaMinima = d; 
                        statieOptima = {i, j}; 
                    }
                }
            }
        }
        return statieOptima;
    }

    char getCell(Position p) const {
         return grid[p.x][p.y]; 
        }
    Position getBase() const {
         return base;
         }
   const vector<string>& getGrid()  { return grid; }
};

#endif