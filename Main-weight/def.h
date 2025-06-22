// structure containing temperature and humidity value from dht sensor
struct DhtValues {
  uint8_t temp;
  uint8_t humidity;
};


struct SigFoxMessage {
  uint8_t temp;
  uint8_t humidity;
  uint8_t battery;
  int weight;
};
