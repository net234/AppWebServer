## Telechargement du source :arrow_right: [Main Branch](https://github.com/net234/AppWebServer)
## Text in english :arrow_right: [In English](README.md)

# AppWebServer beta version B02 (fonctionne avec BetaEvents.lib)

- Une simple librairie Arduino pour construire des applications IoT Basée sur des pages Web incluse dans un serveur local.
- C'est un serveur de fichier construit pour developper un interface utilisateur dynamic pour une application sur un ESP8266.
- AppWebServer gère un simple serveur Web connecté au WiFi local ou fonctionnant en autonome sans connexion à l'internet.
- Cette librairie utilise des fichiers Web standards (HTML CSSS javascript) avec un rafraichissement dynamic des données affichées (Genre ajax simplifié).

#### Cette version est construite et testée avec la plateforme Arduino ESP8266 Version(2.7.0) https://github.com/esp8266/Arduino (ou ultérieur)

## Contenu
 - [Comment Cela Fonctionne](#comment-cela-fonctionne)
 - [Demo Rapide](#demo-rapide)
 - [Releases](#releases)
 - [Contributions](#contributions)


## Comment Cela Fonctionne
- Des pages Web Standards sont enregistrées en mémoire flash sous forme de fichiers (LittleFS) elle sont affichées a la demande dans un navigateur classique.
- Des mots clefs spécifiques dans les pages Web permettent d'afficher des données dynamiques. Elles sont remplacée "à la volée".

## Demo Rapide
- Ouvrez le fichier "FirstDemo" dans votre IDE arduino IDE (Fichier -> Exemple -> AppWebServer).
- Dans le menu Outil -> Choisissez "Generic ESP8266 Module" (fonctionne avec la plupart des ESP8266)
- Dans le menu Outil -> erase flash "Sketch and WiFi Settings" (Seulement pour cette demo) 
- Dans le menu Outil -> builtIn led: "2"
- Dans le menu Outil -> Flash Size 4MB (FS:3MB...
- Téléchargez la partie site Web avec le menu Outil -> Esp8266 LittleFS Data Upload"
- Compilez et téléchargez Firstdemo.

- Connectez vous au WiFi "APPWEBDEMO" avec un ordinateur ou une tablette.
- Au bout de quelques secondes une fenêtre s'ouvre avec la page de configuration (si la page ne s'ouvre pas tapez http://10.10.10.10 dans votre navigateur)
- Entrez le nom et le mot de passe de votre WiFi local puis enregistrez (save).
- Connectez votre ordinateur à votre WiFI local WiFi puis tapez http://appwebdemo.local dans votre navigateur web préféré.
- Vous aurez accès à votre ESP8266 et vous pourrez allumer ou éteindre sa LED interne..
- Essayer d'autres exemple plus dynamiques dans le menu Fichier -> Exemple -> AppWebServer de votre IDE Arduino.


## Version https://github.com/net234/AppWebServer
##### betaversion B01  27/12/2020 - pre release of the lib Copyright 2020 Pierre HENRY net23@frdev.com 


fonctionne avec l'IDE Arduino version  1.6.5-1044-g170995a, built on Aug 10, 2015 of the ESP8266 Arduino library.


### Contributions
Rien de ceci n'aurai été fait sans l'incroyable aide de la communauté Arduino. Je ne vous remercierai jamais assez. Ceci est mon premier essais de développement en logiciel libre et je dois reconnaitre que c'est un univers inimaginable d'échanges et de créativité. Encore merci à tous pour ce dévouement extraordinaire.

Remerciement spécial à [tzapu](https://github.com/tzapu/WiFiManager) pour le travail sur [WiFimanager](https://github.com/tzapu/WiFiManager).  AppWebServer n'utilise pas la librairie WiFimanager mais des fonctionnalités très similaires y sont implantées.

Cette documentation est à finir

Il manque beaucoup de remerciements.

#### Inspiration
- http://www.esp8266.com/viewtopic.php?f=29&t=2520
