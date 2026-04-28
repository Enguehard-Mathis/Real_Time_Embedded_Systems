#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <climits>

// Structure demandée pour décrire une tâche
struct Task {
    std::string name;
    double C;        // WCET
    int T;           // Période
    int D;           // Échéance
    int priority;    // Priorité RM (plus petit = plus prioritaire)
};

// Structure pour représenter une instance (Job)
struct Job {
    std::string taskName;
    int arrival;     // Date de réveil
    int deadline;    // Échéance absolue
    double duration; // Temps d'exécution
    int start = -1;  // Début réel
    int end = -1;    // Fin réelle
    int wait = 0;    // Temps d'attente
};

int main() {
    // Initialisation de l'ensemble de tâches
    std::vector<Task> tasks = {
        {"t1", 1.0, 10, 10, 1},
        {"t2", 3.0, 10, 10, 2},
        {"t3", 2.0, 20, 20, 3},
        {"t4", 2.0, 20, 20, 4},
        {"t5", 2.0, 40, 40, 5},
        {"t6", 2.0, 40, 40, 6},
        {"t7", 3.0, 80, 80, 7}
    };

    int hyperperiod = 80;

    // Vérification d'ordonnançabilité (U)
    double U = 0.0;
    for (const auto& t : tasks) {
        U += t.C / t.T;
    }
    std::cout << "Test d'utilisation : U = " << std::fixed << std::setprecision(4) << U 
              << (U <= 1.0 ? " -> ordonnancable" : " -> NON ordonnancable") << "\n\n";

    // Génération des jobs sur l'hyperpériode
    std::vector<Job> jobs;
    for (const auto& t : tasks) {
        for (int k = 0; k < hyperperiod / t.T; ++k) {
            Job j;
            j.taskName = t.name;
            j.arrival = k * t.T;
            j.deadline = (k + 1) * t.T;
            j.duration = t.C;
            jobs.push_back(j);
        }
    }

    // Simulation EDF non-préemptif
    int t = 0;
    int nb_finis = 0;
    int nb_jobs = jobs.size();

    while (nb_finis < nb_jobs) {
        int meilleur = -1;

        for (int i = 0; i < nb_jobs; ++i) {
            if (jobs[i].end != -1) continue;      // Déjà fini
            if (jobs[i].arrival > t) continue;    // Pas encore prêt

            // Logique EDF : échéance absolue la plus proche
            if (meilleur == -1 || jobs[i].deadline < jobs[meilleur].deadline) {
                meilleur = i;
            }
        }

        if (meilleur == -1) {
            // Aucun job prêt, on avance au prochain réveil
            int prochain = INT_MAX;
            for (int i = 0; i < nb_jobs; ++i) {
                if (jobs[i].end == -1 && jobs[i].arrival < prochain)
                    prochain = jobs[i].arrival;
            }
            t = prochain;
            continue;
        }

        // Exécution du job choisi
        jobs[meilleur].start = t;
        t += (int)jobs[meilleur].duration;
        jobs[meilleur].end = t;
        jobs[meilleur].wait = jobs[meilleur].start - jobs[meilleur].arrival;
        nb_finis++;
    }

    // Affichage des résultats
    // On trie par date de début pour l'affichage chronologique
    std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
        return a.start < b.start;
    });

    std::cout << std::left << std::setw(8) << "Job" << " | " 
              << std::setw(5) << "Rev" << " | " 
              << std::setw(5) << "DL" << " | " 
              << std::setw(6) << "Debut" << " | " 
              << std::setw(5) << "Fin" << " | " 
              << std::setw(7) << "Attente" << " | " << "Statut\n";
    std::cout << std::string(60, '-') << "\n";

    int attente_totale = 0;
    int echecs = 0;

    for (const auto& j : jobs) {
        bool rate = j.end > j.deadline;
        if (rate) echecs++;
        attente_totale += j.wait;

        std::cout << std::left << std::setw(8) << j.taskName << " | "
                  << std::setw(5) << j.arrival << " | "
                  << std::setw(5) << j.deadline << " | "
                  << std::setw(6) << j.start << " | "
                  << std::setw(5) << j.end << " | "
                  << std::setw(7) << j.wait << " | "
                  << (rate ? "RATE" : "ok") << "\n";
    }

    std::cout << "\nAttente totale   : " << attente_totale << "\n";
    std::cout << "Echeances ratees : " << echecs << "\n";

    return 0;
}