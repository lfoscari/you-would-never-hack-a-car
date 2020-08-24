Offroad pal
-----------

Utilizzando un lettore OBD II ed un GPS con un ESP8266 si vuole creare una dashboard di dati di viaggio consultabile tramite un sito web servito in locale.
I dati comprendono:
- Posizione corrente
- Vari dati sul veicolo (RPM, posizione acceleratore, quantità di carburante, etc)
- Dati sulla posizione (coordinate ed elevazione)

I dati sono disponibili il tempo reale sul sito e sono salvati su un servizio cloud. L'idea è nata intornoa ai tragitti in fuoristrada, per di avere dati sul comportamento del veicolo e informazioni precise di posizione, utili per ripetere eventualmente il percorso.

Sulla board deve anche essere presente uno shield per una SIM card, ad esempio SIM5360 o SIM7600. Questo per avere la posizione GPS e per poter salvare i dati sul cloud. In alternativa si può usare un telefono con tethering. La SIM si può comprare da https://www.thingsmobile.com/business/sim-card/standard-sim-card-2ff-3ff-4ff.

Per creare la dashboard di possono usare le websocket (?) oppure si possono scrivere i dati in un file che poi la dashboard leggerà ad intervalli regolari tramite una chiamata XHR. Per salvare i dati in un file sull'ESP è necessario abilitare un filesystem, la scelta ricade su LittleFS (https://github.com/ARMmbed/littlefs).

Fasi incrementali:
1. Configurare l'ESP8266 e il lettore OBD II per mostrare i dati su schermo.
2. Creare la dashboard con i dati e renderla disponibile via server.
3. Aggiungere la SIM card.
4. Salvare i dati su un servizio cloud.
5. Aggiungere il GPS e salvare anche i suoi dati.

Risorse interessanti:
- https://www.traccar.org Modern GPS Tracking Platform
- https://freematics.com/store/index.php?route=product/category&path=24 Telematics Kits
	In particolare https://freematics.com/store/index.php?route=product/product&path=24&product_id=87 in cui è specificato in modo come accoppiarlo con https://freematics.com/store/index.php?route=product/product&path=20&product_id=55 per avere il GPS
- http://arduinodev.com/hardware/obd-kit/ Esempio di OBD II Freematics con Arduino
- https://github.com/nostalgic-css/NES.css Framework CSS per la dashboard
