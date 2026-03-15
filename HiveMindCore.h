#ifndef HIVEMINDCORE_H
#define HIVEMINDCORE_H


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <stdexcept>
#include <sstream>

using namespace std;


//  runtime_error din biblioteca standard C++
class HiveMindException : public runtime_error 
{
public:
    // Constructorul primeste un mesaj de eroare
    HiveMindException(const string& msg) 
        : runtime_error("HiveMind Error: " + msg) 
    {
        // Mesajul este trimis catre clasa parinte runtime_error
        // "HiveMind Error: " ca sa stim de unde vine eroarea
    }

};


// STRUCTURI DE BAZA

// Structura pentru coordonatele de pe harta
struct Position 
{
    int x;
    int y;

    // Metoda simpla pentru a verifica daca doua pozitii sunt la fel
  
    int esteEgal(Position alta) 
    {
        if (x == alta.x && y == alta.y) 
        {
            return 1; 
        }
        else 
        {
            return 0; 
        }
    }

    // Metoda pentru a verifica daca sunt diferite
    int esteDiferit(Position alta) 
    {
 
        if (esteEgal(alta)==0) 
        {
            return 1;
        }
        else 
        {
            return 0;
        }
    }
};

enum class AgentState { IDLE, MOVING, CHARGING, GOING_TO_CHARGE, RETURNING_TO_BASE, DEAD };

struct Package {
    int id;
    Position dest;
    int reward;
    int deadline;
    bool assigned = false;
    bool delivered = false;
    bool late = false;

    Package(int _id, Position _dest, int _rew, int _dead)
        : id(_id), dest(_dest), reward(_rew), deadline(_dead) {}
};


// SINGLETON: HIVEMIND 

class HiveMind {
private:
    static HiveMind* instance;
    double profit = 0;
    double totalRewards = 0;
    double totalOpCost = 0;
    int agentsDead = 0;
    int packagesLate = 0;
    int packagesUndelivered = 0;
    int packagesDelivered = 0;
    int totalSpawned = 0;
    int ticksRulate = 0;

    HiveMind() {}
    ~HiveMind() {}

public:
    static HiveMind* getInstance() 
    {
        if (!instance) instance = new HiveMind();
        return instance;
    }

    static void destroyInstance() {
        if (instance)
         {
            delete instance;
            instance = nullptr;
        }
    }
    void reset() {
    profit = 0;
    totalRewards = 0;
    totalOpCost = 0;
    packagesDelivered = 0;
    packagesLate = 0;
    packagesUndelivered = 0;
    agentsDead = 0;
    ticksRulate = 0;
    totalSpawned = 0;
    
}

    double getProfit() const { return profit; }

  // Metoda care adauga recompensa cand un pachet este livrat cu succes
    void addReward(int r) 
    { 
        profit = profit + r; 
        totalRewards = totalRewards + r; 
        packagesDelivered = packagesDelivered + 1; 
    }

    // Metoda pentru scaderea cheltuielilor de operare 
    void addOpCost(int c,string tip) 
    { 
        profit = profit - c; 
        totalOpCost = totalOpCost + c; 
    }

    // Incrementam numarul total de pachete care au aparut in simulare
    void incrementSpawned() 
    { 
        totalSpawned = totalSpawned + 1; 
    }

    // Actualizam timpul total de rulare
    void setTicksRulate(int t) 
    { 
        ticksRulate = t; 
    }

    // Penalizare mare daca pierdem un robot sau o drona (-500)
    void penaltyDeath() 
    { 
        profit = profit - 500; 
        agentsDead = agentsDead + 1; 
    }

    // Penalizare mica pentru intarzierea livrarii (-50)
    void penaltyLate() 
    { 
        profit = profit - 50; 
        packagesLate = packagesLate + 1; 
    }

    // Penalizare medie daca pachetul nu mai ajunge deloc la client (-200)
    void penaltyUndelivered() 
    { 
        profit = profit - 200; 
        packagesUndelivered = packagesUndelivered + 1; 
    }

    // Getter simplu pentru pachetele intarziate
    int getLateCount()
    { 
        return packagesLate; 
    }

    // Setteri folositi la finalul simularii pentru raport
    void setDelivered(int x) 
    { 
        packagesDelivered = x; 
    }

    void setUndelivered(int x) 
    { 
        packagesUndelivered = x; 
    }

    void setSpawned(int x) 
    { 
        totalSpawned = x; 
    }

    void stats(ostream& out) {
        double eficienta = (totalSpawned > 0) ? (packagesDelivered * 100.0 / totalSpawned) : 0;
        out << " RAPORT FINAL HIVEMIND \n" 
            << "Profit Net: " << profit << " credite\n"
            << "Ticks Rulate: " << ticksRulate << "\n"
            << "Eficienta Livrare: " << eficienta << "%\n"
            
            << "Rewarduri Totale (Brut): " << totalRewards << " credite\n"
            << "Costuri Operare Totale: " << totalOpCost << " credite\n"
            
            << "Pachete Livrate: " << packagesDelivered << "\n"
            << "Pachete Nelivrate: " << packagesUndelivered << "\n"
            << "Pachete Intarziate: " << packagesLate << "\n"
            << "Agenti morti: " << agentsDead << endl;
            out << "Total Pachete Spawned: " << totalSpawned << "\n";
    }
};

#endif