# Composants matériels

La famille **`Amalgame.Hardware`** est un ensemble de petits packages
pour piloter de l'électronique réelle depuis Amalgame. Ils se
répartissent proprement en trois couches :

1. **Le HAL** (`amalgame-hal`) — des *interfaces* portables
   (`DigitalOut`/`DigitalIn`, `PwmOut`, `I2cBus`, `SpiBus`, `Clock`,
   `SerialPort`). Un driver s'écrit **une seule fois** contre elles.
2. **Un backend de carte** — implémente le HAL sur une carte donnée.
   Aujourd'hui : `amalgame-hardware-gpio` (Raspberry Pi / SBC Linux).
   Un backend MCU est sur la feuille de route.
3. **Les drivers** — composants (LEDs, moteurs, capteurs, afficheurs…)
   écrits contre le HAL, donc tournant sur *n'importe quel* backend
   sans modification.

> 🧩 **Le Pi aujourd'hui, le MCU demain — même code driver.** Comme
> les drivers visent le HAL et jamais une carte précise, le même code
> `amalgame-hardware-led` / `-motor` / `-sensor` qui tourne sur un
> Raspberry Pi (alimenté par les broches de `hardware-gpio`) tournera
> sur un microcontrôleur dès qu'un backend MCU existera (voir la
> [proposition amc-embedded](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md)).
> Rien ici n'est spécifique au Raspberry Pi, sauf le backend GPIO
> lui-même.

Tous les packages s'installent via l'index curé — `amc package add <nom>`
— et requièrent **amc ≥ 0.8.72** (dispatch d'interface).

## Fondation

| Package | Installation | Description |
|---|---|---|
| **hal** ([repo](https://github.com/amalgame-lang/amalgame-hal)) | `amc package add hal` | Interfaces d'abstraction matérielle portables : `DigitalOut`/`DigitalIn`, `PwmOut`, `I2cBus`, `SpiBus`, `Clock`, `SerialPort`. Le contrat que vise chaque driver. |

## Backends de carte

Ils fournissent le HAL sur du vrai matériel. Choisissez celui de votre
carte ; les drivers ci-dessous ne s'en soucient pas.

| Package | Installation | Description |
|---|---|---|
| **hardware-gpio** ([repo](https://github.com/amalgame-lang/amalgame-hardware-gpio)) | `amc package add hardware-gpio` | Backend Raspberry Pi 1→5 / SBC Linux via **libgpiod v2** : E/S numériques GPIO + fronts, I²C, SPI, PWM matériel, UART. Fournit les broches/bus HAL aux drivers. Voir le [how-to Raspberry Pi](raspberry-pi/README.md). |
| *Backend MCU* | — | Sur la feuille de route ([amc-embedded](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md)). Exposera le même HAL sur microcontrôleurs nus. |

## Sortie & actionneurs

| Package | Installation | Composants |
|---|---|---|
| **hardware-led** ([repo](https://github.com/amalgame-lang/amalgame-hardware-led)) | `amc package add hardware-led` | Drivers LED simple (tout-ou-rien) + LED RGB-PWM, par-dessus le HAL. |
| **hardware-motor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-motor)) | `amc package add hardware-motor` | Servo / ESC, moteur CC (pont en H), moteur pas-à-pas 28BYJ-48, et relais. |

## Entrée & capteurs

| Package | Installation | Composants |
|---|---|---|
| **hardware-input** ([repo](https://github.com/amalgame-lang/amalgame-hardware-input)) | `amc package add hardware-input` | Bouton-poussoir anti-rebond, encodeur rotatif. |
| **hardware-sensor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-sensor)) | `amc package add hardware-sensor` | Distance ultrason HC-SR04, ADC MCP3008, BME280 température / pression / humidité. |
| **hardware-comms** ([repo](https://github.com/amalgame-lang/amalgame-hardware-comms)) | `amc package add hardware-comms` | Drivers de comms série — GPS NMEA via un `SerialPort` HAL. |

## Afficheurs

| Package | Installation | Composants |
|---|---|---|
| **hardware-display** ([repo](https://github.com/amalgame-lang/amalgame-hardware-display)) | `amc package add hardware-display` | OLED SSD1306 128×64 (framebuffer + rendu texte) en I²C. |

## Expandeurs & I/O

| Package | Installation | Composants |
|---|---|---|
| **hardware-io** ([repo](https://github.com/amalgame-lang/amalgame-hardware-io)) | `amc package add hardware-io` | Expandeur d'E/S I²C 8 bits PCF8574 — chaque broche de l'expandeur est exposée comme un `DigitalOut` + `DigitalIn` HAL, donc les drivers existants pilotent ses broches de façon transparente. |

## Contrôle & maths (Amalgame pur)

Pas de matériel propre — des briques pour les boucles de contrôle, indépendantes de la carte.

| Package | Installation | Composants |
|---|---|---|
| **hardware-control** ([repo](https://github.com/amalgame-lang/amalgame-hardware-control)) | `amc package add hardware-control` | Régulateur PID, filtre complémentaire IMU, moyenne glissante, helpers `map` / `clamp`. |

---

## Assembler le tout

Un projet type choisit **un backend** + **les drivers nécessaires**.
Sur un Raspberry Pi, faire clignoter une LED via la couche driver :

```sh
amc package add hardware-gpio    # le backend Pi (fournit le HAL)
amc package add hardware-led     # le driver LED portable
```

Le driver est construit à partir d'une broche HAL fournie par le
backend, donc le même programme compilera sans changement sur un futur
backend MCU.

Nouveau sur tout ça ? Commencez par le **[how-to Raspberry Pi](raspberry-pi/README.md)**
— il mène de la carte SD vierge au pilotage GPIO, boutons et capteurs
I²C, pas à pas.
