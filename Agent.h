#ifndef AGENT_H
#define AGENT_H

#include "HiveMindCore.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <queue>

using namespace std;

// Reprezinta entitatea de baza pentru unitatile de livrare
class Agent {

protected:
    // Identificare 
    string id;
    string TypeStr;

    //  Localizare 
    Position pos{0, 0};
    Position basePos{0, 0};
    queue<Position> path;

    //  Parametri 
    int baterie, batMax;
    int consum, costTick;
    int viteza, capacitate;

    //Stare Interna 
    vector<shared_ptr<Package>> cargo;
    AgentState state = AgentState::IDLE;


public:
    // Constructor principal -mapare parametri pe atributele clasei
    Agent(string nume, string tip, int baterieMax, int vitezoman, int consumator, int cost, int cap)
        : id(nume), TypeStr(tip), baterie(baterieMax), batMax(baterieMax), viteza(vitezoman), consum(consumator), costTick(cost), capacitate(cap) {}


    // Destructor virtual -Asigura curatarea corecta a memoriei in ierarhii
    virtual ~Agent() {}

    // Metoda virtuala pura -Polimorfism pentru medii diferite (Aer/Sol)
    virtual bool isAerial() const = 0;

    // Getter pentru tipul agentului
    string getTypeStr() { 
        return TypeStr; 
    }

    //  disponibilitatea spatiului 
    bool canCarryMore() const { 
        return cargo.size() < (size_t)capacitate; 
    }

    // Adaugarea unui obiect in  transport
    void pickUp(shared_ptr<Package>  pachetNou)
    { 
        cargo.push_back(pachetNou); 
    }


    // Incarcarea unei rute noi si actualizarea starii
    void setPath(const vector<Position>& listaPuncte, AgentState stareNoua = AgentState::MOVING) {
        
        // Verificare stare  agent
        if (state == AgentState::DEAD) {
            throw HiveMindException("Incarcare ruta esuata: Agentul " + id + " este DEAD.");
        }

        // Golirea rutei anterioare
        while (!path.empty()) 
        {
            path.pop();
        }
        
        // Transferul noilor coordonate in coada de miscare
        for (const auto& punctCurent : listaPuncte)
        {
            path.push(punctCurent);
        }

        if (!path.empty()) {
            state = stareNoua;
        }
    }


    // Logica de deplasare bazata pe viteza specifica

void step(const vector<string>& grid) { 
    if (state == AgentState::DEAD || state == AgentState::IDLE || state == AgentState::CHARGING) return;

    int putereRamasa = this->viteza; 
    
    while(putereRamasa > 0 && !path.empty()) {
        Position urmatorulPas = path.front();

        if (!isAerial() && grid[urmatorulPas.x][urmatorulPas.y] == '#') { // 
            break; 
        }

        this->pos = urmatorulPas;
        path.pop();
        putereRamasa--;
        

        if (grid[pos.x][pos.y] == 'B' || grid[pos.x][pos.y] == 'S') 
        { 
            if (this->baterie < this->batMax * 0.1) break; 
        }
    }
}


    // Masina de stari: gestioneaza resursele si interactiunea cu mediul
    void updateStatus(char celulaHarta) {
        
        if (state == AgentState::DEAD) return;
        
        // Incarcare  in zonele de alimentare
        if (state == AgentState::IDLE && (celulaHarta == 'B' || celulaHarta == 'S')) {
            baterie = min(batMax, baterie + (batMax / 4));
            return;
        }

        // Scadere energie in timpul misiunilor 
        if (state == AgentState::MOVING || state == AgentState::GOING_TO_CHARGE || state == AgentState::RETURNING_TO_BASE) 
        {
            baterie -= consum;
        }

        // Protectie la epuizarea totala a bateriei
        if (baterie <= 0) {
            if (celulaHarta != 'B' && celulaHarta != 'S') 
            {
                state = AgentState::DEAD;
                return;
            } else {
                baterie = 1; // Mentinem agentul functional daca a ajuns la statie
            }
        }

        // Gestionare incarcare activa
        if (celulaHarta == 'B' || celulaHarta == 'S') 
        {
            if (baterie < batMax) 
            {
                state = AgentState::CHARGING;
                baterie = min(batMax, baterie + (batMax / 4));
            } else if (state == AgentState::CHARGING) 
            {
                // Decizie dupa terminarea incarcarii
                state = path.empty() ? AgentState::IDLE : AgentState::MOVING;
            }
        }

        // Verificare livrari la finalul rutei
        if (path.empty() && state != AgentState::CHARGING) 
        {
            
            for (auto iteratorPachet = cargo.begin(); iteratorPachet != cargo.end(); ) 
            {
                
                // Daca locatia curenta coincide cu destinatia pachetului
                if (pos.esteEgal((*iteratorPachet)->dest) == 1) {
                    (*iteratorPachet)->delivered = true;
                    HiveMind::getInstance()->addReward((*iteratorPachet)->reward);
                    iteratorPachet = cargo.erase(iteratorPachet);
                } else {
                    ++iteratorPachet;
                }
            }

            // Dupa finalizarea livrarilor, revenim la baza daca este cazul
            if (cargo.empty()) {
                if (pos.esteEgal(basePos) == 1) {
                    state = AgentState::IDLE;
                } else if (state != AgentState::GOING_TO_CHARGE)
                {
                    state = AgentState::RETURNING_TO_BASE;
                }
            }
        }
    }
   // Returneaza starea curenta a agentului 
AgentState getState() const { 
    return state; 
}

// Returneaza coordonatele X si Y actuale ale agentului
Position getPos() const { 
    return pos; 
}

// Returneaza cat costa operarea acestui agent per tick
int getCost() const { 
    return costTick; 
}

// Returneaza nivelul actual de energie al bateriei
int getBaterie() const { 
    return baterie; 
}

// Returneaza capacitatea maxima a bateriei 
int getBatMax() const { 
    return batMax; 
}

// Returneaza rata de consum a energiei per miscare
int getConsum() const { 
    return consum; 
}

// Returneaza viteza agentului (celule parcurge per tick)
int getViteza() const { 
    return viteza; 
}

// Returneaza numarul maxim de pachete pe care le poate transporta simultan
int getCapacitate() const { 
    return capacitate; 
}

// Verifica daca agentul are o ruta activa in coada de pathfinding
bool hasPath() const { 
    return !path.empty(); 
}


// Seteaza pozitia de start a agentului si locatia bazei de incarcare
void setBase(Position p)
{ 
    this->basePos = p; 
    this->pos = p; 
}
};
#endif