#ifndef HEXAGON_H
#define HEXAGON_H
#include <Adafruit_NeoPixel.h>
#include "config.h"
#ifdef HEXAGONS
void hexagon_set(Adafruit_NeoPixel strip, int hexagon, int pixel, uint32_t color);
void hexagon_clear(Adafruit_NeoPixel strip, int hexagon, uint32_t color);
void hexagon_clear(Adafruit_NeoPixel strip, int hexagon, uint32_t color)
{
    int start = HEXAGONS[hexagon][0];
    int end = HEXAGONS[hexagon][1];
    for (int i = start; i < end; i++)
    {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

void hexagon_set(Adafruit_NeoPixel strip, int hexagon, int pixel, uint32_t color)
{
    int start = HEXAGONS[hexagon][0];
    int end = HEXAGONS[hexagon][1];
    int dt = end - start;
    strip.setPixelColor(dt + pixel, color);
    strip.show();
}
#endif
#endif