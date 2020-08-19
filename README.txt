Offroad pal
-----------

Utilizzando un lettore OBD II ed un GPS con un ESP8266 si vuole creare una dashboard di dati di viaggio consultabile tramite un sito web servito in locale.
I dati comprendono:
- Posizione corrente
- Vari dati sul veicolo (RPM, posizione acceleratore, inclinazione)
- Dati sulla posizione (coordinate ed elevazione)
- Altro?

I dati sono disponibili il tempo reale sul sito e sono salvati su un servizio cloud (da specificare) per poter essere interpretati in seguito.
L'idea è nata per i viaggi in fuoristrada, al fine di avere dati sul comportamento del veicolo e informazioni precise di posizione, al fine di ripetere il tragitto.

Sulla board deve anche essere presente uno shield per una SIM card, ad esempio SIM5360 o SIM7600. Questo per avere la posizione GPS e per poter salvare i dati sul cloud. In alternativa si può usare un telefono con tethering. La SIM si può comprare da https://www.thingsmobile.com/business/sim-card/standard-sim-card-2ff-3ff-4ff.

Risorse interessanti:
- https://www.traccar.org Modern GPS Tracking Platform
- https://freematics.com/store/index.php?route=product/category&path=24 Telematics Kits
	In particolare https://freematics.com/store/index.php?route=product/product&path=24&product_id=87 in cui è specificato in modo come accoppiarlo con https://freematics.com/store/index.php?route=product/product&path=20&product_id=55 per avere il GPS
- http://arduinodev.com/hardware/obd-kit/ Esempio di OBD II Freematics con Arduino

(Offroad HAL?)
