// Solutionnaire du TD3 INF1015 hiver 2024
// Par Francois-R.Boyer@PolyMtl.ca

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "classes_td4.hpp"      // Structures de données pour la collection de films en mémoire.

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <sstream>
#include <typeinfo>
#include <iterator>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).
using namespace std;
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
	T valeur{};
	fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
	return valeur;
}
#define erreurFataleAssert(message) assert(false&&(message)),terminate()
static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
	uint8_t entete = lireType<uint8_t>(fichier);
	switch (entete) {
	case enteteTailleVariableDeBase+0: return lireType<uint8_t>(fichier);
	case enteteTailleVariableDeBase+1: return lireType<uint16_t>(fichier);
	case enteteTailleVariableDeBase+2: return lireType<uint32_t>(fichier);
	default:
		erreurFataleAssert("Tentative de lire un entier de taille variable alors que le fichier contient autre chose à cet emplacement.");  //NOTE: Il n'est pas possible de faire des tests pour couvrir cette ligne en plus du reste du programme en une seule exécution, car cette ligne termine abruptement l'exécution du programme.  C'est possible de la couvrir en exécutant une seconde fois le programme avec un fichier films.bin qui contient par exemple une lettre au début.
	}
}

string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUintTailleVariable(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}



#pragma endregion//}



void Bibliotheque::ajouterItem(unique_ptr<Item> item) {
    items_.push_back(std::move(item));
}


const vector<unique_ptr<Item>>& Bibliotheque::obtenirItems() { return items_; }

void Bibliotheque::enleverItem(const unique_ptr<Item> item) {
    for (int i : range(size())) {  // Doit être une référence au pointeur pour pouvoir le modifier.
        if (items_[i] == item) {
            if (size() > 1)
                items_.erase(items_.begin() + i);
            return;
        }
    }
}


span<shared_ptr<Acteur>> Film::obtenirActeurs() const{
    return acteurs_.enSpan();
}

shared_ptr<Acteur> Bibliotheque::trouverActeur(const string& nomActeur) const
{
    for (auto&& item : span(items_)) {
        if(typeid(*item) == typeid(Film)) {
            for (const shared_ptr<Acteur> &acteur: item->obtenirActeurs()) {
                if (acteur->obtenirNom() == nomActeur)
                    return acteur;
            }
        }
    }
    return nullptr;
}


shared_ptr<Acteur> lireActeur(istream& fichier, const Bibliotheque& bibliotheque)
{
    Acteur acteur = {};
    acteur.nom_            = lireString(fichier);
    acteur.anneeNaissance_ = int(lireUintTailleVariable (fichier));
    acteur.sexe_           = char(lireUintTailleVariable(fichier));

    shared_ptr<Acteur> acteurExistant = bibliotheque.trouverActeur(acteur.nom_);
    if (acteurExistant != nullptr)
        return acteurExistant;
    else {
        cout << "Création Acteur " << acteur.nom_ << endl;
        return make_shared<Acteur>(std::move(acteur));  // Le move n'est pas nécessaire mais permet de transférer le texte du nom sans le copier.
    }
}


unique_ptr<Film> lireFilm(istream& fichier, Bibliotheque& bibliotheque)
{
    unique_ptr<Film> film = make_unique<Film>();
    film->titre_       = lireString(fichier);
    film->realisateur_ = lireString(fichier);
    film->anneeSortie_ = int(lireUintTailleVariable(fichier));
    film->recette_     = int(lireUintTailleVariable(fichier));
    auto nActeurs = int(lireUintTailleVariable(fichier));
    film->acteurs_ = ListeActeurs(nActeurs);  // On n'a pas fait de méthode pour changer la taille d'allocation, seulement un constructeur qui prend la capacité.  Pour que cette affectation fonctionne, il faut s'assurer qu'on a un operator= de move pour ListeActeurs.
    cout << "Création Film " << film->titre_ << endl;

    for ([[maybe_unused]] auto i : range(nActeurs)) {  // On peut aussi mettre nElements avant et faire un span, comme on le faisait au TD précédent.
        film->acteurs_.ajouter(lireActeur(fichier, bibliotheque));
    }

    return film;
}

unique_ptr<Livre> lireLivre(const string& fichier){
        unique_ptr<Livre> livre = make_unique<Livre>();

        vector<string> vectorStrings;
        stringstream ss(fichier);
        string tmp;
        while(getline(ss, tmp, '\t'))
        {
            tmp.erase(remove(tmp.begin(), tmp.end(), '\"'), tmp.end());
            vectorStrings.push_back(tmp);
        }

        livre->titre_ = vectorStrings[0];
        livre->annee_ = stoi(vectorStrings[1]);
        livre->auteur_ = vectorStrings[2];
        livre->millionsCopiesVendues_ = stoi(vectorStrings[3]);
        livre->nombresPages_ = stoi(vectorStrings[4]);
        cout << "Création Livre " << livre->titre_ << endl;

        return livre;

}

Bibliotheque creerItems(string nomFichierFilm, string nomFichierLivre){
    ifstream fichierFilm(nomFichierFilm, ios::binary);
    fichierFilm.exceptions(ios::failbit);
    int nElements = int(lireUintTailleVariable(fichierFilm));
    Bibliotheque bibliotheque;

    for ([[maybe_unused]] int i : range(nElements)) {
        bibliotheque.ajouterItem(lireFilm(fichierFilm, bibliotheque));
    }

    ifstream fichierLivre(nomFichierLivre);
    string ligne;
    while(getline(fichierLivre, ligne)) {
        bibliotheque.ajouterItem(lireLivre(ligne));
    }

    return  bibliotheque;



}


int main()
{
	#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
	#endif
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

    cout << "Création Bibliothèque" << ligneDeSeparation;
    Bibliotheque bibliotheque = creerItems("films.bin", "livres.txt" );




}
