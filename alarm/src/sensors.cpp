#include "../include/sensors.h"
#include "../include/config.h"
#include <DHT.h>

// 11, темп влажность
static DHT dht(DHT_PIN, DHT_TYPE);

void sensors_init() {
    dht.begin();
}

void sensors_read(float* temperature, float* humidity) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h)) *humidity    = h;
    if (!isnan(t)) *temperature = t;
    // Если чтение не удается, сохраняются предыдущие значения.
}
