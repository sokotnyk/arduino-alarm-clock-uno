#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

void sensors_init();
void sensors_read(float* temperature, float* humidity);

#endif // SENSORS_H