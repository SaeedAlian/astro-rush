#pragma once

#include "collider.hpp"

/*
 * Check for collision between a and b.
 *
 * It will return false if it recognizes unknown collider types.
 *
 * @param a | first collider
 * @param b | second collider
 *
 * @return true if collision detected otherwise false
 */
bool collides(const Collider &a, const Collider &b);
