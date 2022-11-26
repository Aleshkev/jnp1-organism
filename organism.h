#ifndef JNP1_ORGANISM_H
#define JNP1_ORGANISM_H

#include <concepts>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tuple>

namespace {
using vitality_t = uint64_t;
}

// Celem zadania jest napisanie kodu do symulowania interakcji organizmów
// w środowisku naturalnym. Organizm jest reprezentowany przez instancję
// szablonu [o taką].
//
// Typ species_t określa gatunek. Typ ten powinien spełniać koncept
// std::equality_comparable, co powinno zostać sprawdzone podczas
// kompilowania. Wartości can_eat_plants oraz can_eat_meat określają
// odpowiednio, czy dany organizm potrafi jeść mięso lub rośliny.
template <std::equality_comparable species_t, bool can_eat_meat,
          bool can_eat_plants>
class Organism {
  const species_t species;
  const vitality_t vitality;

 public:
  // Każdy organizm powinien definiować metodę
  //
  //    uint64_t get_vitality();
  //
  // która zwraca witalność danego organizmu. Organizm, którego witalność wynosi
  // 0, uznaje się za martwy.
  constexpr vitality_t get_vitality() const {
    return vitality;
  }
  constexpr bool is_dead() const {
    return vitality == 0;
  }

  // Klasa Organism powinna udostępniać konstruktor ustawiający gatunek
  // i początkową witalność organizmu:
  //
  //    Organism(species_t const &species, uint64_t vitality);
  constexpr Organism(species_t const &species, uint64_t vitality)
      : species(species), vitality(vitality) {
  }

  // Powinno być możliwe poznanie gatunku organizmu:
  //
  //    const species_t &get_species();
  constexpr const species_t &get_species() const {
    return species;
  }

  // Ponadto obiekt typu Organism powinien mieć dodatkowe metody umożliwiające
  // eleganckie rozwiązanie zadania, o specyfikacji wybranej przez
  // rozwiązujących.

  template <bool x, bool y>
  constexpr bool can_eat(const Organism<species_t, x, y> &that) const {
    if (can_eat_meat && !that.is_plant())
      return true;
    if (can_eat_plants && that.is_plant())
      return true;
    return false;
  }
  // TODO: usunąć te funkcje jeśli naprawdę się nigdy nie przydadzą
  constexpr bool is_carnivore() const {
    return can_eat_meat && !can_eat_plants;
  }
  constexpr bool is_omnivore() const {
    return can_eat_meat && can_eat_plants;
  }
  constexpr bool is_herbivore() const {
    return !can_eat_meat && can_eat_plants;
  }
  constexpr bool is_plant() const {
    return !can_eat_meat && !can_eat_plants;
  }

  // Uznajemy, że dwa organizmy o różnych preferencjach żywieniowych są różnymi
  // gatunkami, nawet jeżeli porównanie species_t zwraca true.
  template <bool can_other_eat_meat, bool can_other_eat_plants>
  constexpr bool are_species_equal(
      const Organism<species_t, can_other_eat_meat, can_other_eat_plants>
          &other) const {
    return species == other.get_species() &&
           can_eat_meat == can_other_eat_meat &&
           can_eat_plants == can_other_eat_plants;
  }

  constexpr auto set_vitality(vitality_t new_vitality) const {
    return Organism(species, new_vitality);
  }

  constexpr auto add_vitality(vitality_t change) const {
    return set_vitality(get_vitality() + change);
  }

  constexpr auto kill() const {
    return set_vitality(0);
  }
};

// Rozwiązanie powinno udostępniać cztery typy organizmów będące specjalizacjami
// szablonu Organism
template <typename species_t>
using Carnivore = Organism<species_t, true, false>;
template <typename species_t>
using Omnivore = Organism<species_t, true, true>;
template <typename species_t>
using Herbivore = Organism<species_t, false, true>;
template <typename species_t>
using Plant = Organism<species_t, false, false>;

// Spotkanie powinno być realizowane przez szablon funkcji [o taki].
//
// Jej wynikiem jest krotka zawierająca kolejno: pierwotne organizmy
// zmodyfikowane w wyniku zdarzenia (jako nowe obiekty), w kolejności jak w
// argumentach, oraz opcjonalnie nowy organizm powstały w wyniku spotkania.
template <typename species_t, bool sp1_eats_m, bool sp1_eats_p, bool sp2_eats_m,
          bool sp2_eats_p>
constexpr std::tuple<Organism<species_t, sp1_eats_m, sp1_eats_p>,
                     Organism<species_t, sp2_eats_m, sp2_eats_p>,
                     std::optional<Organism<species_t, sp1_eats_m, sp1_eats_p>>>
encounter(Organism<species_t, sp1_eats_m, sp1_eats_p> organism1,
          Organism<species_t, sp2_eats_m, sp2_eats_p> organism2) {
  // Organizmy spotykają się według poniższych reguł. Reguły są aplikowane po
  // kolei, aż do znalezienia pierwszej, która rozstrzyga efekt spotkania. Wtedy
  // kończymy sprawdzanie, ignorując następne reguły.

  // 1. Możliwe jest spotkanie jedynie organizmów, których typ gatunku
  // (species_t) jest taki sam (wartości oczywiście nie muszą być takie same).
  // Przykładowo przy poniższych deklaracjach gazela może spotkać lwa, ale nie
  // psa. Próba spotkania organizmów o różnych typach powinna być wykrywana
  // podczas kompilowania i sygnalizować błąd.
  //
  //    Carnivore<std::string> lion("Panthera leo", 462);
  //    Herbivore<std::string> gazelle("Gazella dorcas", 130);
  //    Omnivore<uint64_t> dog(1, 15);
  //
  // [To zachodzi automatycznie bo nie ma specjalizacji funkcji dla różnych
  // species_t. Chodzi o *typ gatunku* a nie o *gatunek*.]

  // 2. Nie jest możliwe spotkanie dwóch roślin, gdyż są one niemobilne. Próba
  // spotkania dwóch roślin powinna być wykrywana podczas kompilowania
  // i sygnalizować błąd.
  static_assert(!organism1.is_plant() || !organism2.is_plant());

  // 3. Spotkanie, w którym jedna ze stron jest martwa, nie powoduje skutków.
  if (organism1.is_dead() || organism2.is_dead()) {
    return {organism1, organism2, std::nullopt};
  }

  // 4. Spotkanie dwóch zwierząt tego samego gatunku prowadzi do godów. Dla
  // uproszczenia zakładamy nierealistycznie, że wszystkie organizmy są
  // obojnacze i możliwe jest łączenie w pary dowolnych dwóch przedstawicieli
  // gatunku. Wynikiem jest dziecko - tego samego gatunku co rodzice i
  // witalności będącej średnią arytmetyczną witalności rodziców (zaokrągloną w
  // dół).
  if (organism1.are_species_equal(organism2)) {
    return {organism1,
            organism2,
            {{organism1.get_species(),
              (organism1.get_vitality() + organism2.get_vitality()) / 2}}};
  }

  // 5. Spotkanie organizmów, które nie potrafią się zjadać, nie przynosi
  // efektów.
  if (!organism1.can_eat(organism2) && !organism2.can_eat(organism1)) {
    return {organism1, organism2, std::nullopt};
  }

  // 6. Spotkanie dwóch zwierząt, które potrafią się nawzajem zjadać, prowadzi
  // do walki. Walkę wygrywa organizm o wyższej witalności. Organizm
  // przegrywający walkę ginie. Organizm, który wygrał, dodaje do swojej
  // witalności połowę (zaokrągloną w dół) witalności organizmu, który przegrał.
  // Gdy organizmy mają równą witalność, dochodzi do wyniszczającej walki: oba
  // organizmy zabijają się nawzajem.
  if ((!organism1.is_plant() && !organism2.is_plant()) &&
      (organism1.can_eat(organism2) && organism2.can_eat(organism1))) {
    bool organism1_dies = organism2.get_vitality() >= organism1.get_vitality();
    bool organism2_dies = organism1.get_vitality() >= organism2.get_vitality();
    return {
        (organism1_dies ? organism1.kill()
                        : organism1.add_vitality(organism2.get_vitality() / 2)),
        (organism2_dies ? organism2.kill()
                        : organism2.add_vitality(organism1.get_vitality() / 2)),
        std::nullopt};
  }

  // 7. Spotkanie roślinożercy lub wszystkożercy z rośliną skutkuje tym, że
  // roślina zostaje zjedzona. Zjadający zwiększa swoją witalność o witalność
  // rośliny, a witalność rośliny jest ustawiana na 0.
  if (organism2.is_plant() && organism1.can_eat(organism2)) {
    return {organism1.add_vitality(organism2.get_vitality()), organism2.kill(),
            std::nullopt};
  }
  if (organism1.is_plant() && organism2.can_eat(organism1)) {
    return {organism1.kill(), organism2.add_vitality(organism1.get_vitality()),
            std::nullopt};
  }

  // 8. Przy spotkaniu, w którym zdolność do konsumpcji zachodzi tylko w jedną
  // stronę, rozpatrujemy dwa przypadki. Jeśli „potencjalnie zjadany" ma
  // witalność nie mniejszą niż „potencjalnie zjadający", to nic się nie dzieje.
  // W przeciwnym przypadku zjadający dodaje do swojej witalności połowę
  // (zaokrągloną w dół) witalności zjedzonego, a zjedzony ginie.
  if (organism1.can_eat(organism2)) {
    if (organism2.get_vitality() >= organism1.get_vitality())
      return {organism1, organism2, std::nullopt};
    return {organism1.add_vitality(organism2.get_vitality() / 2),
            organism2.kill(), std::nullopt};
  }
  if (organism2.can_eat(organism1)) {
    if (organism1.get_vitality() >= organism2.get_vitality())
      return {organism1, organism2, std::nullopt};
    return {organism1.kill(),
            organism2.add_vitality(organism1.get_vitality() / 2), std::nullopt};
  }

  throw std::logic_error("Illegal state");
}

// Ponadto rozwiązanie powinno udostępniać szablon [o taki].
//
// gdzie Args również są typu Organism o tym samym species_t co organism1, ale
// być może odmiennych preferencjach żywieniowych, realizujący serię spotkań
// organizmu będącego pierwszym argumentem z organizmami podanymi na
// pozostałych argumentach, w kolejności od lewej do prawej. Wynikiem funkcji
// jest organism1 zmieniony poprzez wszystkie spotkania. Zmiany na pozostałych
// organizmach oraz ewentualne dzieci powstałe w wyniku spotkań nie są
// zwracane.
template <typename species_t, bool sp1_eats_m, bool sp1_eats_p,
          typename... Args>
constexpr Organism<species_t, sp1_eats_m, sp1_eats_p> encounter_series(
    Organism<species_t, sp1_eats_m, sp1_eats_p> organism1, Args... args);

// [Rekurencyjna implementacja.]
template <typename species_t, bool sp1_eats_m, bool sp1_eats_p>
constexpr Organism<species_t, sp1_eats_m, sp1_eats_p>
encounter_series(Organism<species_t, sp1_eats_m, sp1_eats_p> organism1) {
  return organism1;
}
template <typename species_t, bool sp1_eats_m, bool sp1_eats_p, bool sp2_eats_m,
          bool sp2_eats_p, typename... Args>
constexpr Organism<species_t, sp1_eats_m, sp1_eats_p>
encounter_series(Organism<species_t, sp1_eats_m, sp1_eats_p> organism1,
                 Organism<species_t, sp2_eats_m, sp2_eats_p> organism2,
                 Args... args) {
  return encounter_series(std::get<0>(encounter(organism1, organism2)),
                          args...);
}

#endif  // JNP1_ORGANISM_H
