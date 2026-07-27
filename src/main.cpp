#include "game.hpp"

int main() {
  GameOptions opts;
  opts.title = "Astro Rush";
  Game g = Game(opts);
  g.run();
  return 0;
}
