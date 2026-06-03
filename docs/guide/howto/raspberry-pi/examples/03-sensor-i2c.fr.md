# Scanner un bus de capteurs I²C

Presque tous les capteurs du Pi parlent **I²C** :
température/humidité, IMU, petits afficheurs OLED, expandeurs de
ports. Cet exemple est l'équivalent Amalgame de `i2cdetect -y 1` — il
ouvre le bus, sonde chaque adresse et affiche qui répond. Ensuite on
lit un registre d'un périphérique pour montrer l'aller-retour complet.

## Câblage

```
VCC capteur ── 3V3        SDA capteur ── GPIO2 (SDA1, broche 3)
GND capteur ── GND        SCL capteur ── GPIO3 (SCL1, broche 5)
```

L'I²C est un bus partagé à deux fils : plusieurs périphériques peuvent
donc pendre des mêmes broches SDA/SCL. Vérifiez que l'I²C est activé
(voir [préparation, étape 3](../01-setup.md#3--activer-les-interfaces-matérielles)).

## Code

```amalgame
namespace Demo

import Amalgame.Hardware

public class Program {
    // Les adresses I²C se lisent en hexa ; String_FromInt est décimal,
    // donc on formate un octet en deux chiffres hexa — Amalgame pur,
    // aucun C inline.
    private static string Hex2(v: int) {
        let digits: string = "0123456789abcdef"
        let hi: int = (v / 16) % 16
        let lo: int = v % 16
        return String_CharAt1(digits, hi) + String_CharAt1(digits, lo)
    }

    public static void Main(string[] args) {
        let busNum: int = 1
        let bus: I2c = new I2c(busNum)        // /dev/i2c-1
        if (!bus.IsOpen()) {
            Console.WriteLine("impossible d'ouvrir /dev/i2c-1 — I2C activé ? permissions ?")
            return
        }

        let found: List<int> = bus.Scan()     // sonde 0x03..0x77
        if (found.Count() == 0) {
            Console.WriteLine("aucun périphérique sur le bus " + String_FromInt(busNum))
        } else {
            Console.WriteLine(String_FromInt(found.Count()) + " périphérique(s) :")
            for addr in found {
                Console.WriteLine("  0x" + Program.Hex2(addr))
            }
        }

        bus.Close()
    }
}
```

## Lancer

```sh
amc build main.am -o i2c_scan
./i2c_scan
```

Vous verrez quelque chose comme :

```
1 périphérique(s) :
  0x68
```

Un MPU-6050 apparaît à `0x68`, un backpack LCD PCF8574 à `0x27`, de
nombreux OLED à `0x3c`, etc.

## Lire un registre

Une fois l'adresse connue, lisez ou écrivez ses registres. Par
exemple, pour réveiller un MPU-6050 (mettre à zéro son registre
`PWR_MGMT_1` en `0x6B`) puis lire en rafale 6 octets de données
accéléromètre :

```amalgame
bus.WriteReg(0x68, 0x6B, 0x00)             // réveille le périphérique
let raw: List<int> = bus.ReadBytes(0x68, 6) // octets haut+bas ACCEL_X/Y/Z
```

L'API I²C en un coup d'œil :

| Méthode | Usage |
|---|---|
| `new I2c(bus)` / `.IsOpen()` / `.Close()` | ouvrir `/dev/i2c-<bus>` |
| `.Scan()` → `List<int>` | lister les répondeurs, comme `i2cdetect` |
| `.ReadByte(addr)` / `.WriteByte(addr, v)` | octet brut unique |
| `.ReadReg(addr, reg)` / `.WriteReg(addr, reg, v)` | accès registre |
| `.ReadBytes(addr, n)` / `.WriteBytes(addr, list)` | rafale multi-octets |

Les octets traversent la frontière en `List<int>` (chacun `0..255`).

**Suivant :** [chenillard de LEDs →](04-led-bar.md)
