#ifndef JNP1_ORGANISM_H
#define JNP1_ORGANISM_H

#include <concepts>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace {
using vitality_t = uint64_t;
using std::nullopt, std::optional, std::tuple, std::logic_error, std::get,
    std::equality_comparable;
}  // namespace

template <typename species_t, bool can_eat_meat, bool can_eat_plants>
requires equality_comparable<species_t> class Organism {
  const species_t species;
  const vitality_t vitality;

 public:
  constexpr vitality_t get_vitality() const {
    return vitality;
  }
  constexpr bool is_dead() const {
    return vitality == 0;
  }

  constexpr Organism(species_t const &species, uint64_t vitality)
      : species(species), vitality(vitality) {
  }

  constexpr const species_t &get_species() const {
    return species;
  }

  template <bool x, bool y>
  constexpr bool can_eat(const Organism<species_t, x, y> &that) const {
    if (can_eat_meat && !that.is_plant()) {
      return true;
    }
    if (can_eat_plants && that.is_plant()) {
      return true;
    }
    return false;
  }

  constexpr bool is_plant() const {
    return !can_eat_meat && !can_eat_plants;
  }

  template <bool can_other_eat_meat, bool can_other_eat_plants>
  constexpr bool are_species_equal(
      const Organism<species_t, can_other_eat_meat, can_other_eat_plants>
          &other) const {
    return (species == other.get_species() &&
            can_eat_meat == can_other_eat_meat &&
            can_eat_plants == can_other_eat_plants);
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

template <typename species_t>
using Carnivore = Organism<species_t, true, false>;
template <typename species_t>
using Omnivore = Organism<species_t, true, true>;
template <typename species_t>
using Herbivore = Organism<species_t, false, true>;
template <typename species_t>
using Plant = Organism<species_t, false, false>;

template <typename species_t, bool sp1_eats_m, bool sp1_eats_p, bool sp2_eats_m,
          bool sp2_eats_p>
constexpr tuple<Organism<species_t, sp1_eats_m, sp1_eats_p>,
                Organism<species_t, sp2_eats_m, sp2_eats_p>,
                optional<Organism<species_t, sp1_eats_m, sp1_eats_p>>>
encounter(Organism<species_t, sp1_eats_m, sp1_eats_p> organism1,
          Organism<species_t, sp2_eats_m, sp2_eats_p> organism2) {
  decltype(encounter(organism1, organism2)) nothing_happens = {
      organism1, organism2, nullopt};

  // 2. Nie jest możliwe spotkanie dwóch roślin.
  static_assert(!organism1.is_plant() || !organism2.is_plant());

  // 3. Spotkanie, w którym jedna ze stron jest martwa.
  if (organism1.is_dead() || organism2.is_dead()) {
    return nothing_happens;
  }

  // 4. Spotkanie dwóch zwierząt tego samego gatunku.
  if (organism1.are_species_equal(organism2)) {
    return {organism1,
            organism2,
            {{organism1.get_species(),
              (organism1.get_vitality() + organism2.get_vitality()) / 2}}};
  }

  // 5. Spotkanie organizmów, które nie potrafią się zjadać, nie przynosi
  // efektów.
  if (!organism1.can_eat(organism2) && !organism2.can_eat(organism1)) {
    return nothing_happens;
  }

  // 6. Spotkanie dwóch zwierząt, które potrafią się nawzajem zjadać.
  if ((!organism1.is_plant() && !organism2.is_plant()) &&
      (organism1.can_eat(organism2) && organism2.can_eat(organism1))) {
    bool organism1_dies = organism2.get_vitality() >= organism1.get_vitality();
    bool organism2_dies = organism1.get_vitality() >= organism2.get_vitality();
    return {
        (organism1_dies ? organism1.kill()
                        : organism1.add_vitality(organism2.get_vitality() / 2)),
        (organism2_dies ? organism2.kill()
                        : organism2.add_vitality(organism1.get_vitality() / 2)),
        nullopt};
  }

  // 7. Spotkanie roślinożercy lub wszystkożercy z rośliną skutkuje tym, że
  // roślina zostaje zjedzona.
  if (organism2.is_plant() && organism1.can_eat(organism2)) {
    return {organism1.add_vitality(organism2.get_vitality()), organism2.kill(),
            nullopt};
  }
  if (organism1.is_plant() && organism2.can_eat(organism1)) {
    return {organism1.kill(), organism2.add_vitality(organism1.get_vitality()),
            nullopt};
  }

  // 8. Spotkanie, w którym zdolność do konsumpcji zachodzi tylko w jedną
  // stronę.
  if (organism1.can_eat(organism2)) {
    if (organism2.get_vitality() >= organism1.get_vitality()) {
      return nothing_happens;
    }
    return {organism1.add_vitality(organism2.get_vitality() / 2),
            organism2.kill(), nullopt};
  }
  if (organism2.can_eat(organism1)) {
    if (organism1.get_vitality() >= organism2.get_vitality()) {
      return nothing_happens;
    }
    return {organism1.kill(),
            organism2.add_vitality(organism1.get_vitality() / 2), nullopt};
  }

  throw logic_error("Illegal state");
}

// Ponadto rozwiązanie powinno udostępniać szablon [o taki].
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
  return encounter_series(get<0>(encounter(organism1, organism2)), args...);
}

#endif  // JNP1_ORGANISM_H
