#include "search_algorithm.h"

namespace gs {

const char* algoName(Algo a) {
  switch (a) {
    case Algo::EXHAUSTIVE: return "EXHAUSTIVE";
    case Algo::GRADIENT: return "GRADIENT";
    case Algo::SURGE_CAST: return "SURGE_CAST";
    default: return "?";
  }
}

const char* algoShortName(Algo a) {
  switch (a) {
    case Algo::EXHAUSTIVE: return "EXH";
    case Algo::GRADIENT: return "GRA";
    case Algo::SURGE_CAST: return "SUR";
    default: return "???";
  }
}

SearchAlgorithm* makeAlgorithm(Algo a) {
  // Doi tuong tinh: khong cap phat dong tren vi dieu khien.
  static ExhaustiveSearch exhaustive;
  static GradientSearch gradient;
  static SurgeCastSearch surge;
  switch (a) {
    case Algo::EXHAUSTIVE: return &exhaustive;
    case Algo::GRADIENT: return &gradient;
    case Algo::SURGE_CAST: return &surge;
    default: return &exhaustive;
  }
}

}  // namespace gs
