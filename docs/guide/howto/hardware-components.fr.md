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
> driver qui tourne sur un Raspberry Pi (alimenté par les broches de
> `hardware-gpio`) tournera sur un microcontrôleur dès qu'un backend
> MCU existera (voir la
> [proposition amc-embedded](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md)).
> Rien ici n'est spécifique au Raspberry Pi, sauf le backend GPIO
> lui-même.

Tous les packages s'installent via l'index curé — `amc package add <nom>`
— et requièrent **amc ≥ 0.8.72** (dispatch d'interface). La colonne
**Composants pris en charge** liste les puces/dispositifs concrets
pilotés par chaque package.

## Fondation

| Package | Installation | Fournit |
|---|---|---|
| **hal** ([repo](https://github.com/amalgame-lang/amalgame-hal)) | `amc package add hal` | Les interfaces portables visées par chaque driver : `DigitalOut`, `DigitalIn`, `PwmOut`, `I2cBus`, `SpiBus`, `Clock`, `SerialPort`. |

## Backends de carte

Ils fournissent le HAL sur du vrai matériel. Choisissez celui de votre
carte ; les drivers ci-dessous ne s'en soucient pas.

| Package | Installation | Périphériques pris en charge |
|---|---|---|
| **hardware-gpio** ([repo](https://github.com/amalgame-lang/amalgame-hardware-gpio)) | `amc package add hardware-gpio` | Raspberry Pi 1→5 / SBC Linux via **libgpiod v2** : E/S numériques GPIO + fronts, I²C, SPI, PWM matériel, UART. Fournit les broches/bus HAL à tous les drivers. Voir le [how-to Raspberry Pi](raspberry-pi/README.md). |
| *Backend MCU* | *(roadmap)* | Exposera le même HAL sur microcontrôleurs nus — [amc-embedded](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md). |

## Sortie & actionneurs

| Package | Installation | Composants pris en charge |
|---|---|---|
| **hardware-led** ([repo](https://github.com/amalgame-lang/amalgame-hardware-led)) | `amc package add hardware-led` | LED simple (tout-ou-rien), LED RGB (PWM), ruban **APA102 / DotStar**, ruban **WS2812 / NeoPixel**. |
| **hardware-motor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-motor)) | `amc package add hardware-motor` | Servo / ESC, moteur CC (pont en H), moteur pas-à-pas 28BYJ-48, driver pas-à-pas **A4988**, relais, driver PWM 16 canaux **PCA9685**, buzzer piézo. |

## Entrée

| Package | Installation | Composants pris en charge |
|---|---|---|
| **hardware-input** ([repo](https://github.com/amalgame-lang/amalgame-hardware-input)) | `amc package add hardware-input` | Bouton-poussoir anti-rebond, encodeur rotatif. *(Un clavier matriciel arrive en v0.2.0, qui requiert amc ≥ 0.8.73.)* |

## Capteurs

| Package | Installation | Composants pris en charge |
|---|---|---|
| **hardware-sensor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-sensor)) | `amc package add hardware-sensor` | Distance **HC-SR04**, ADC SPI 8 canaux **MCP3008**, **BME280** température/pression/humidité, IMU 6 axes **MPU-6050**, lumière ambiante **BH1750**, courant/puissance **INA219**, ADC I²C 16 bits **ADS1115**, ADC cellule de charge **HX711**, thermocouple type K **MAX6675**. |
| **hardware-comms** ([repo](https://github.com/amalgame-lang/amalgame-hardware-comms)) | `amc package add hardware-comms` | Récepteur **GPS** NMEA via un `SerialPort` HAL. |

## Afficheurs

| Package | Installation | Composants pris en charge |
|---|---|---|
| **hardware-display** ([repo](https://github.com/amalgame-lang/amalgame-hardware-display)) | `amc package add hardware-display` | OLED **SSD1306** 128×64 (framebuffer + texte 5×7), LCD caractères **LCD1602** 16×2, driver 7-seg / matrice LED **MAX7219**, afficheur 4 digits 7-seg **TM1637**. |

## Expandeurs & I/O

| Package | Installation | Composants pris en charge |
|---|---|---|
| **hardware-io** ([repo](https://github.com/amalgame-lang/amalgame-hardware-io)) | `amc package add hardware-io` | Expandeur I²C 8 bits **PCF8574**, registre à décalage 8 bits **SN74HC595**, expandeur I²C 16 bits **MCP23017**. Chacun expose ses broches comme des `DigitalOut` + `DigitalIn` HAL, donc les drivers existants pilotent ses broches de façon transparente. |

## Horloge temps réel (RTC)

| Package | Installation | Composants pris en charge |
|---|---|---|
| **hardware-rtc** ([repo](https://github.com/amalgame-lang/amalgame-hardware-rtc)) | `amc package add hardware-rtc` | Horloge temps réel TCXO **DS3231**, horloge temps réel **DS1307** (en I²C), avec un helper `DateTime` lecture/écriture. |

## Contrôle & maths (Amalgame pur)

Pas de matériel propre — des briques pour les boucles de contrôle, indépendantes de la carte.

| Package | Installation | Fournit |
|---|---|---|
| **hardware-control** ([repo](https://github.com/amalgame-lang/amalgame-hardware-control)) | `amc package add hardware-control` | Régulateur PID, filtre complémentaire IMU, moyenne glissante, helpers de plage `map` / `clamp`. |

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
