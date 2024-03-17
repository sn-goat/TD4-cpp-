
#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
using gsl::span;
using namespace std;

class Acteur; class Item; class Affichable; class Film; class Livre;

template <typename T>
class Liste {
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

class Bibliotheque {
public:
    Bibliotheque() = default;
    Bibliotheque(string nomFichierFilm, string nomFichierLivre);
    void ajouterItem(unique_ptr<Item> item);
    void enleverItem(const unique_ptr<Item>& item);
    shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
    const vector<unique_ptr<Item>>& obtenirItems();
    [[nodiscard]] int size() const { return items_.size(); }
    unique_ptr<Item>& operator[] (int index) { assert(0 <= index && index < size());  return items_[index]; }

    template<typename T>
    unique_ptr<Item> trouver(const T& critere) {
        for (auto& Item : span(items_))
            if (critere(*Item))
                return std::move(Item);
        return nullptr;
    }
    friend ostream& operator<<(ostream& os, const Bibliotheque& bibliotheque);



private:
    vector<unique_ptr<Item>> items_;

};

class Affichable {
public:
    virtual void afficher(ostream& os) const = 0;
    friend ostream& operator<< (ostream& os, const Affichable& affichable);
};

class Item: public Affichable {
public:
    void afficher(ostream& os) const override;
    Item() = default;
    virtual ~Item() = default;

    void afficherLingeSeparation(ostream& os) const;
    const string& obtenirTitre() const;
    virtual span<shared_ptr<Acteur>> obtenirActeurs() const { return {}; };
    friend unique_ptr<Film> lireFilm(istream& fichier, Bibliotheque& bibliotheque);
    friend unique_ptr<Livre> lireLivre(const string& fichier);

private:
    string titre_;
    int annee_ = 0;


};


class Film :virtual public Item{
public:
    Film() = default;
    void afficher(ostream& os) const override;
    friend unique_ptr<Film> lireFilm(istream& fichier, Bibliotheque& Bibliotheque);
    span<shared_ptr<Acteur>> obtenirActeurs() const override;
    friend ostream& operator<<(ostream& os, const Film& film);
    friend unique_ptr<Film> lireFilm(istream& fichier, Bibliotheque& bibliotheque);

private:
    string realisateur_;
    int recette_ = 0;
    ListeActeurs acteurs_;

};

class Livre :virtual public Item  {
public:
    Livre() = default;
    void afficher(ostream& os) const override;
    friend unique_ptr<Livre> lireLivre(const string& fichier);
    friend ostream& operator<<(ostream& os, const Livre& livre);


private:
    string  auteur_;
    int millionsCopiesVendues_, nombresPages_;
};

class FilmLivre : public Film, public Livre {
public:
    FilmLivre(const Film& film, const Livre& livre) : Item(film), Film(film), Livre(livre) { }
    void afficher(ostream& os) const override;
    friend ostream& operator<<(ostream& os, const FilmLivre& filmLivre);
};



class Acteur
{
public:
    friend shared_ptr<Acteur> lireActeur(istream& fichier, const Bibliotheque& bibliotheque);
    const string& obtenirNom() const { return nom_; }
    friend ostream& operator<< (ostream& os, const Acteur& acteur);

private:
    string nom_; int anneeNaissance_ = 0; char sexe_ = '\0';
};

