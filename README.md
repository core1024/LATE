# Arduino Mini Games

## Hardware:
 * 1.3" SH1106 OLED Display
 * (micro)SD card reader module
 * buttons

 ### Schematic:

 Buttons and display layout:

 ```
          ____
   [L]   |OLED|   [B]
 [D] [U] |UP->|
   [R]   |____|   [A]
```

All pins are defined in the beginning of the sketch. Look for `BTN_PIN_<X>` where `<X>` is the button name.

**NOTE:** *The buttons are hooked according to display orientation and latter rotated by `buttonsRotate()`.*

## Deps:

* U8G2 (Sketch -> Include Library -> Manage Libraries...)
* PetitFS (`git clone https://github.com/greiman/PetitFS.git`)

The SD card `CS` pin is defined inside `PetitFS` library.

```patch
diff --git a/src/pffArduino.h b/src/pffArduino.h
index 3eaeb4f..8f8ef3b 100644
--- a/src/pffArduino.h
+++ b/src/pffArduino.h
@@ -5,7 +5,7 @@
 #include "integer.h"

 // SD chip select pin
-#define SD_CS_PIN 10
+#define SD_CS_PIN 5

 // Use SPI SCK divisor of 2 if nonzero else 4.
 #define SPI_FCPU_DIV_2 1
diff --git a/src/pffconf.h b/src/pffconf.h
index c8ccdf1..03742de 100644
--- a/src/pffconf.h
+++ b/src/pffconf.h
@@ -10,12 +10,12 @@
 /---------------------------------------------------------------------------*/

 #define        _USE_READ       1       /* Enable pf_read() function */
-#define        _USE_DIR        1       /* Enable pf_opendir() and pf_readdir() function */
+#define        _USE_DIR        0       /* Enable pf_opendir() and pf_readdir() function */
 #define        _USE_LSEEK      1       /* Enable pf_lseek() function */
 #define        _USE_WRITE      1       /* Enable pf_write() function */

-#define _FS_FAT12      1       /* Enable FAT12 */
-#define _FS_FAT16      1       /* Enable FAT16 */
+#define _FS_FAT12      0       /* Enable FAT12 */
+#define _FS_FAT16      0       /* Enable FAT16 */
 #define _FS_FAT32      1       /* Enable FAT32 */
```

## Games (WIP):
 * bGun - No score, no speed increase
 * Tetris - Mostly compilant with <https://tetris.wiki/Tetris_(NES,_Nintendo)>
 * Snake - No speed increase, lame score
