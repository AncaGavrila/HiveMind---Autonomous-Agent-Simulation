
#include "HiveMindCore.h"
#include "Agent.h"
#include "Drona.h"
#include "Robot.h"
#include "Scooter.h"
#include "MapSystem.h"
#include "CityMap.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <queue>
#include <stdexcept>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <list>

using namespace std;

HiveMind* HiveMind::instance = nullptr;
class SimulationEngine{
    int Mx = 20, Ny = 20, maxTicks = 1000, spawnFreq = 10, totalPackages = 50, numS = 3, numD = 10;
    unique_ptr<CityMap> city;
    vector<Agent*> flota;
    vector<Position> clients;
    list<shared_ptr<Package>> packages;

public:
   ~SimulationEngine() {
    
    for (auto a : flota) {
        delete a; 
    }
    flota.clear();
    cout << "\n Memoria pentru flota a fost eliberata  " << endl;
}
void loadConfig(string path) 
{
    ifstream file(path);

    if (!file){
        throw HiveMindException("nu s a gasit fisierul simulation_setup.txt");
    }

    string line;
    string key;
    string mapMode ="PROCEDURAL";

    while (getline(file, line)) {
        if (line.empty() || line.find("//")== 0) 
        {
            continue;
        }

        stringstream ss(line);
        ss >> key;

        if (key =="MAP_SIZE:") {
            ss >> Mx >> Ny;
        }
        else if (key == "MAX_TICKS:") {
            ss >> maxTicks;
        }
        else if (key == "MAP_MODE:") {
            ss >> mapMode;
        }
        else if (key == "MAX_STATIONS:"){
            ss >> numS;
        }
        else if (key == "CLIENTS_COUNT:") {
             ss >> numD;
        }
        else if (key == "DRONES:") {
            int count;
            ss >> count;
            for (int i = 0; i < count; i++) 
            {

                Agent* d = new Drona(to_string(i)); 
                flota.push_back(d);
            }
        }
        else if (key == "ROBOTS:") {
            int count;
            ss >> count;
           for (int i = 0; i < count; i++)
        {
             Agent* r = new Robot(to_string(i));
             flota.push_back(r);
        }
        }
        else if (key == "SCOOTERS:") {
            int count;
            ss >> count;
            for (int i = 0; i < count; i++) 
            {
                Agent* s = new Scooter(to_string(i));
                flota.push_back(s);
            }
        }
        else if (key == "SPAWN_FREQUENCY:") {
            ss >> spawnFreq;
        }
        else if (key == "TOTAL_PACKAGES:") {
            ss >> totalPackages;
        }
    }

    shared_ptr<IMapGenerator> generator;

    if (mapMode == "FILE") {
        generator = make_shared<FileMapLoader>("map_debug.txt");
    } else {
        generator = make_shared<ProceduralGenerator>();
    }

    city = make_unique<CityMap>(Mx, Ny);
    city->build(generator, numS, numD, clients);
    city->displayMap();

    for (auto& agent : flota) {
        agent->setBase(city->getBase());
    }
}


    void run() {
        HiveMind* hm = HiveMind::getInstance();
        for (int t = 1; t <= maxTicks; t++) {
            hm->setTicksRulate(t);
          if (t % 10 == 0) {
    int pacheteActive = 0;
    for(auto& p : packages) if(!p->delivered) pacheteActive++;

    cout << "[Tick " << t << "] Profit: " << hm->getProfit() 
         << " , Pachete ramase: " << pacheteActive 
         << " , Status Agent 0: " << (int)flota[0]->getState() << endl;
}

            // 1. Spawn pachete
            if (t % spawnFreq == 0 && (int)packages.size() < totalPackages) {
                int deadlineTicks = 10 + rand() % 11;
              packages.push_back(make_shared<Package>(
        (int)packages.size(), 
        clients[rand() % clients.size()], 
        200 + rand() % 601, 
        t + deadlineTicks
    ));
      hm->incrementSpawned();
            }

            // 2. Penalizari Intarziere 
            for (auto& p : packages) {
                if (!p->delivered && !p->late && t > p->deadline) {
                    p->late = true;
                    hm->penaltyLate();
                }
            }

            // 3. Update Agenti
           for (Agent* a : flota) {
    // 1. Daca agentul e mort trecem peste el
    if (a->getState() == AgentState::DEAD) 
    {
        continue;
    }

    // 2. Daca se misca (nu e IDLE sau la incarcat), platim costul de operare
    if (a->getState() != AgentState::IDLE && a->getState() != AgentState::CHARGING) 
    {
        // Trimitem si tipul agentului pentru statistici 
        hm->addOpCost(a->getCost(), a->getTypeStr()); 
    }

    // 3. Verificam daca trebuie sa se intoarca acasa
    
    if (a->getState() == AgentState::RETURNING_TO_BASE) 
    {
        if (a->getPos().esteDiferit(city->getBase()) == 1) 
        {
            if (a->hasPath() == false) 
            {
                // Generam drumul inapoi catre baza
                vector<Position> drumCasa = city->getPath(a->getPos(), city->getBase(), a->isAerial());
                a->setPath(drumCasa, AgentState::RETURNING_TO_BASE);
            }
        }
    }

    // 4. Managementul bateriei (Cea mai apropiata statie)
    Position statieUrgenta = city->findNearestStation(a->getPos());
    vector<Position> drumS = city->getPath(a->getPos(), statieUrgenta, a->isAerial());
    
    int distS = drumS.size();
    int vitezaAgent = a->getViteza();
    int ticksPanaLaStatie = (distS + vitezaAgent - 1) / vitezaAgent;

    // Daca bateria e pe terminate il obligam sa mearga la incarcat
    if (a->getBaterie() < (ticksPanaLaStatie + 2) * a->getConsum()) 
    {
        if (a->getState() != AgentState::GOING_TO_CHARGE && a->getState() != AgentState::CHARGING) 
        {
            a->setPath(drumS, AgentState::GOING_TO_CHARGE);
        }
    }

    // 5. Executia miscarii si actualizarea starii
    a->step(city->getGrid());
    
    char celulaCurenta = city->getCell(a->getPos());
    a->updateStatus(celulaCurenta);

    // 6. Verificam daca a murit
    if (a->getState() == AgentState::DEAD) 
    {
        hm->penaltyDeath();
    }
}

            //  Alocare Pachete (HiveMind Strategy)
            for (auto& a : flota) {
              if (a->getState() == AgentState::DEAD || a->canCarryMore() ==0) 
    {
        continue;
    }

    
   
    if (a->getState() != AgentState::IDLE && a->getPos().esteDiferit(city->getBase()) ==1) 
    {
        continue;
    }

                shared_ptr<Package> bestP = nullptr;
                double maxScore = -99999;
                for (std::list<shared_ptr<Package>>::iterator it = packages.begin(); it != packages.end(); ++it)
                 {
                    auto& p = *it;
                    if (p->assigned ||p->delivered)
                     continue;
                    vector<Position> pathDest = city->getPath(a->getPos(), p->dest, a->isAerial());
                    if (pathDest.empty()) continue;
                    int ticksDus = (pathDest.size() + a->getViteza() - 1) / a->getViteza();

                    int distInapoi = city->getPath(p->dest, city->findNearestStation(p->dest), a->isAerial()).size();

                    int ticksIntors = (distInapoi + a->getViteza() - 1) / a->getViteza();

                    if (a->getBaterie() >= (ticksDus + ticksIntors + 1) * a->getConsum())
                    {
                        double urgency = (double)(maxTicks - p->deadline) / maxTicks;
                        double score = (p->reward * 1.5) - (ticksDus * a->getCost()) + (urgency * 800);
                        if (score > maxScore) { maxScore = score; bestP = p; }
                    }
                }
                if (bestP)
                {
                    a->pickUp(bestP); bestP->assigned = true;
                    a->setPath(city->getPath(a->getPos(), bestP->dest, a->isAerial()));
                }
            }
        }
        int livrate = 0, nelivrate = 0;
        for (auto& p : packages)
        { if (p->delivered) livrate++;
             else nelivrate++;
        }
        hm->setDelivered(livrate); hm->setUndelivered(nelivrate); hm->setSpawned(packages.size());
        ofstream out("simulation.txt");
         hm->stats(out);
         out.close();
    }
};

// TESTARE SI MAIN
void runUnitTests() {
    cout << "\n TESTE UNITARE  \n";

    int testsPassed = 0;
    int testsTotal = 0;

    // Test 1: Singleton + penalizari
    ////  metodele penalty modifica corect atributul privat 'profit'
    testsTotal++;
    HiveMind* h = HiveMind::getInstance();
    double profitInit = h->getProfit();
    h->penaltyDeath();
    h->penaltyUndelivered();
    if (h->getProfit() == profitInit - 700) {
        cout << "[OK] Penalizari HiveMind\n";
        testsPassed++;
    } else {
        cout << "[FAIL] Penalizari HiveMind\n";
    }

    // Test 2: Factory + tip agent
    // Confirmam ca AgentFactory instantiaza corect obiectele derivate 
    // si ca acestea pastreaza proprietatile specifice (isAerial).
    testsTotal++;
    auto drona = AgentFactory::create("DRONA", 1);
    auto robot = AgentFactory::create("ROBOT", 2);
    if (drona && robot && drona->isAerial() && !robot->isAerial()) {
        cout << "[OK] Factory agenti (aerian / sol)\n";
        testsPassed++;
    } else {
        cout << "[FAIL] Factory agenti\n";
    }

    // Test 3: Polimorfism (cost tick)

    testsTotal++;
    if (drona->getCost()== 15 && robot->getCost()== 1) {
        cout << "[OK] Costuri specifice agentilor\n";
        testsPassed++;
    } else {
        cout << "[FAIL] Costuri agenti\n";
    }


   
// Test 4: Consum baterie
testsTotal++;
auto scuter = AgentFactory::create("SCOOTER", 3);


vector<string> hartaTest = {"....", "....", "...."}; 


scuter->setPath({{1, 0}});

int batInitiala = scuter->getBaterie();


scuter->step(hartaTest); 

scuter->updateStatus('.');

if (scuter->getBaterie() < batInitiala) {
    cout << "[OK] Consum baterie la miscare\n";
    testsPassed++;
} else {
    cout << "[FAIL] Consum baterie\n";
}


    // Test 5: Incarcare la statie
    // Verificam daca agentul isi reface energia atunci cand se afla 
    // pe o celula de tip Statie ('S').
    testsTotal++;

    int batInainte = scuter->getBaterie();
    scuter->updateStatus('S');

    if (scuter->getBaterie() >= batInainte)
     {
        cout << "[OK] Incarcare baterie la statie\n";
        testsPassed++;
    } else {
        cout << "[FAIL] Incarcare baterie\n";
    }

    // Test 6: Harta conectata
    //detectarea drumurilor dintre Baza, 
    // Clienti si Statii intr-o configuratie procedurala.
    testsTotal++;
    CityMap testMap(5, 5);
    auto gen = make_shared<ProceduralGenerator>();
    vector<Position> clienti;
    testMap.build(gen, 1, 1, clienti);
    if (testMap.validateConnectivity()) 
    {
        cout << "[OK] Harta conectata (BFS)\n";
        testsPassed++;
    } else {
        cout << "[FAIL] Harta neconectata\n";
    }

    // Test 7: Exceptii la fisier inexistent
    testsTotal++;
    try {
        SimulationEngine e;
        e.loadConfig("fisier_inexistent.txt");
        cout << "[FAIL] Exceptie fisier\n";
    } catch (...) {
        cout << "[OK] Exceptie fisier configuratie\n";
        testsPassed++;
    }

    

    // Test 8  HiveMind singleton unic
    testsTotal++;
    if (HiveMind::getInstance() == h) {
        cout << "[OK] Singleton unic HiveMind\n";
        testsPassed++;
    } else {
        cout << "[FAIL] Singleton HiveMind\n";
    }

    cout << "\nRezultat teste: " << testsPassed << " / " << testsTotal << " teste trecute\n";
   }


int main() {
    srand(time(0));
    runUnitTests();

    HiveMind::getInstance()->reset();
    try{
        SimulationEngine e;
        e.loadConfig("simulation_setup.txt");
        e.run();
        cout << "Simulare incheiata, verificati simulation.txt\n";
    }
    catch (exception& ex) 
    { 
        cout << "Eroare: " << ex.what(); 
    }
    HiveMind::destroyInstance();
    return 0;
}

