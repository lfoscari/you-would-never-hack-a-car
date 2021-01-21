you-would-never-hack-a-car
--------------------------

Si vuole creare un dispositivo in grado di prelevare informazioni da una
macchina e di mostrarle all'utente. Un ELM327 collegato tramite OBD-II fornisce
i dati del veicolo. Un giroscopio e accelerometro MPU6050 fornisce
l'inclinazione laterale e frontale del veicolo.

Un ESP32 legge i dati dai sensori e li rende disponibili all'utente tramite una
dashboard costruita con Vue.js che sfrutta un canale socket tra ESP32 e client.