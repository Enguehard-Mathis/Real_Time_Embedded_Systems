#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <climits>

// Structure décrivant une tâche
struct Task {
    std::string name;
    double C;           // WCET
    int T;              // Période
    int D;              // Échéance
    int priority;       // Priorité RM
    bool isBackground;  // Vrai si la tâche peut être retardée (ex: tau5)
};

// Structure représentant une instance (Job)
struct Job {
    std::string taskName;
    int arrival;
    int deadline;
    double duration;
    int start = -1;
    int end = -1;
    int wait = 0;
    bool isBackground = false; // Hérité de la tâche parente
};

int main() {
    // Initialisation de l'ensemble de tâches (tau5 est en "background")
    std::vector<Task> tasks = {
        {"t1", 1.0, 10, 10, 1, false},
        {"t2", 3.0, 10, 10, 2, false},
        {"t3", 2.0, 20, 20, 3, false},
        {"t4", 2.0, 20, 20, 4, false},
        {"t5", 2.0, 40, 40, 5, true},  // <-- tau5 est marquée comme tâche de fond
        {"t6", 2.0, 40, 40, 6, false},
        {"t7", 3.0, 80, 80, 7, false}
    };

    int hyperperiod = 80;

    // Génération des jobs sur l'hyperpériode
    std::vector<Job> jobs;
    for (const auto& t : tasks) {
        for (int k = 0; k < hyperperiod / t.T; ++k) {
            Job j;
            j.taskName = t.name;
            j.arrival = k * t.T;
            j.deadline = (k + 1) * t.T;
            j.duration = t.C;
            j.isBackground = t.isBackground;
            jobs.push_back(j);
        }
    }

    // Ordonnancement EDF non-préemptif avec tâche de fond
    int t = 0;
    int nb_finis = 0;
    int nb_jobs = jobs.size();

    while (nb_finis < nb_jobs) {
        int meilleur = -1;

        // Étape A : Chercher le meilleur job PARMI LES TÂCHES CRITIQUES (non-background)
        for (int i = 0; i < nb_jobs; ++i) {
            if (jobs[i].end != -1) continue;           // Déjà fini
            if (jobs[i].arrival > t) continue;         // Pas encore prêt
            if (jobs[i].isBackground) continue;        // On ignore tau5 pour l'instant

            // EDF : échéance la plus proche
            if (meilleur == -1 || jobs[i].deadline < jobs[meilleur].deadline) {
                meilleur = i;
            }
        }

        // Étape B : Si aucun job critique n'est prêt, on regarde si tau5 est disponible
        if (meilleur == -1) {
            for (int i = 0; i < nb_jobs; ++i) {
                if (jobs[i].end == -1 && jobs[i].arrival <= t && jobs[i].isBackground) {
                    meilleur = i;
                    break;
                }
            }
        }

        // Étape C : Si absolument rien n'est prêt, on avance l'horloge
        if (meilleur == -1) {
            int prochain = INT_MAX;
            for (int i = 0; i < nb_jobs; ++i) {
                if (jobs[i].end == -1 && jobs[i].arrival < prochain) {
                    prochain = jobs[i].arrival;
                }
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

    // Tri chronologique et affichage
    std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
        return a.start < b.start;
    });

    std::cout << "Schedule 2 : EDF avec tau5 en tache de fond\n\n";
    std::cout << std::left << std::setw(8) << "Job" << " | " 
              << std::setw(5) << "Rev" << " | " 
              << std::setw(5) << "DL" << " | " 
              << std::setw(6) << "Debut" << " | " 
              << std::setw(5) << "Fin" << " | " 
              << std::setw(7) << "Attente" << " | " << "Statut\n";
    std::cout << std::string(62, '-') << "\n";

    int attente_totale = 0;
    int echecs_critiques = 0;
    int echecs_tau5 = 0;

    for (const auto& j : jobs) {
        bool rate = j.end > j.deadline;
        attente_totale += j.wait;

        if (rate && !j.isBackground) echecs_critiques++;
        if (rate && j.isBackground) echecs_tau5++;

        std::string statut = "ok";
        if (rate) {
            statut = j.isBackground ? "RATE tau5" : "RATE";
        }

        std::cout << std::left << std::setw(8) << j.taskName << " | "
                  << std::setw(5) << j.arrival << " | "
                  << std::setw(5) << j.deadline << " | "
                  << std::setw(6) << j.start << " | "
                  << std::setw(5) << j.end << " | "
                  << std::setw(7) << j.wait << " | "
                  << statut << "\n";
    }

    std::cout << "\nAttente totale          : " << attente_totale << "\n";
    std::cout << "Echeances ratees        : " << echecs_critiques << " (hors tau5)\n";
    std::cout << "Echecs tau5             : " << echecs_tau5 << "\n";
    std::cout << "Occupe / Creux          : 59 / 21 sur 80\n";

    return 0;
}
