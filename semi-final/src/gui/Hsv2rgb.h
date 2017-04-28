#pragma once

#include <SFML/Graphics.hpp>


void RGBtoHSV(float fR, float fG, float fB, float& fH, float& fS, float& fV);
void HSVtoRGB(float fH, float fS, float fV, float& fR, float& fG, float& fB);

sf::Color HSVtoRGB(float fH, float fS, float fV);
