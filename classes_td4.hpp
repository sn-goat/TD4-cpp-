
#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
using gsl::span;
using namespace std;

class Acteur; class Item;

template <typename T>
class Liste{
public:
	Liste() = default;
	explicit Liste(int capaciteInitiale) :
		capacite_(capaciteInitiale),
		elements_(make_unique<shared_ptr<T>[]>(capacite_))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite_(autre.nElements_),
		nElements_(autre.nElements_),
		elements_(make_unique<shared_ptr<T>[]>(nElements_))
	{
		for (int i = 0; i < nElements_; ++i)
			elements_[i] = autre.elements_[i];
	}

	Liste(Liste<T>&&) = default;
	Liste<T>& operator= (Liste<T>&&) noexcept = default;

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements_ < capacite_);
		elements_[nElements_++] = std::move(element);
	}

	shared_ptr<T>& operator[] (int index) const { assert(0 <= index && index < nElements_); return elements_[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), nElements_); }

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_;
};

using ListeActeurs = Liste<Acteur>;

class Bibliotheque{
public:
    Bibliotheque() = default;
    void ajouterItem(unique_ptr<Item> item);
    void enleverItem(const unique_ptr<Item> item);
    shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
    const vector<unique_ptr<Item>>& obtenirItems();
    [[nodiscard]] int size() const { return items_.size(); }
    unique_ptr<Item>& operator[] (int index) { assert(0 <= index && index < size());  return items_[index]; }
    unique_ptr<Item> trouver(const function<bool(const Item&)>& critere) {
        for (auto& Item : span(items_))
            if (critere(*Item))
                return std::move(Item);
        return nullptr;
    }



private:
    vector<unique_ptr<Item>> items_;

};
class Item{
public:
    Item() = default;
    virtual ~Item() = default;
    virtual span<shared_ptr<Acteur>> obtenirActeurs() const {return {};};

private:
    string Titre, Annee;


};


class Film : public Item{
public:
    friend unique_ptr<Film> lireFilm(istream& fichier, Bibliotheque& Bibliotheque);
    span<shared_ptr<Acteur>> obtenirActeurs() const override;

private:
	string titre_, realisateur_;
	int anneeSortie_=0, recette_=0;
	ListeActeurs acteurs_;
};

class Livre : public Item{
public:
    friend unique_ptr<Livre> lireLivre(const string& fichier);

private:
    string titre_, auteur_;
    int annee_, millionsCopiesVendues_, nombresPages_;
};

class Acteur
{
public:
    friend shared_ptr<Acteur> lireActeur(istream& fichier, const Bibliotheque& bibliotheque);
    const string& obtenirNom() {return nom_;}

private:
	string nom_; int anneeNaissance_=0; char sexe_='\0';
};
