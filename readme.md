# NexOS

Petit système d'exploitation éducatif (projet pers.)

## Aperçu
NexOS est un noyau minimal en C/ASM pour i386 (mode protégé) avec :
- Bootloader en NASM et entrée 32-bit ([src/bootloader/kernel_entry.asm](src/bootloader/kernel_entry.asm)).
- IDT/PIC et gestion des interruptions.
- Pilote clavier AZERTY complet (Shift/AltGr, touches étendues E0).
- Pilote VGA text-mode pour affichage simple.
- Shell interactif avec commandes intégrées (`help`, `clear`, `echo`, `sum`, `time`, `uptime`, `sysinfo`, `reboot`, `stop`).
- Lecture de l'heure via le RTC CMOS ([src/kernel/time/time.c](src/kernel/time/time.c)).
- Petit gestionnaire de processus statique (registre/affiche des processus) dans [src/kernel/process](src/kernel/process).
- API d'impression `printf` minimale dans [src/kernel/libs/printf.c](src/kernel/libs/printf.c).

Le projet sert d'environnement d'apprentissage pour le développement bas niveau (boot, interruptions, drivers, shell).

## Structure du dépôt
- [src/bootloader](src/bootloader) — NASM boot + entry
- [src/kernel](src/kernel)
  - boot et `kernel_main.c`
  - drivers: `keyboard.c`, `vga.c` ([src/kernel/drivers](src/kernel/drivers))
  - idt: [src/kernel/idt](src/kernel/idt)
  - pic: [src/kernel/pic](src/kernel/pic)
  - libs: `print_*`, `scan.c`, `printf.c` ([src/kernel/libs](src/kernel/libs))
  - shell: [src/kernel/shell](src/kernel/shell)
  - time: RTC driver [src/kernel/time](src/kernel/time)
  - process: table minimale [src/kernel/process](src/kernel/process)
- [src/linker.ld](src/linker.ld) — script d'édition de liens
- [script.sh](script.sh) — script de build + lancement QEMU

## Fonctionnalités détaillées
- Clavier AZERTY
  - Mapping base, shift, AltGr.
  - Gestion des préfixes E0 pour touches étendues (flèches, Insert/Delete, touches Windows, Numpad Enter).
  - File circulaire pour buffer clavier accessible via `keyboard_getchar()`.
- Shell
  - Prompt `> `, édition de ligne basique (Backspace), echo, parsing d'arguments.
  - Commandes utiles: `help`, `clear`, `echo`, `sum n m`, `time`, `uptime`, `sysinfo`, `reboot`, `stop`.
- RTC
  - Lecture CMOS (ports 0x70/0x71), conversion BCD → binaire, gestion 12/24h.
- printf
  - `printf` minimal supportant `%d`, `%s`, `%c`, `%x`, `%%`.
- Process table
  - Table fixe, interface `proc_register()` pour enregistrer des processus visibles par `sysinfo`.

## Build & Run
Requis: `gcc` (i386 multilib), `nasm`, `ld`, `objcopy`, `qemu-system-i386`, `python3`.

Exécuter :

```bash
bash script.sh
```

Le script compile les modules, produit `out/os/nexOS.bin` et lance QEMU. Voir [script.sh](script.sh).

## Commandes du shell
- `help` — affiche l'aide
- `clear` / `cls` — efface l'écran
- `echo ...` — affiche le texte fourni
- `sum n m` — affiche la somme de deux entiers
- `time` — affiche l'heure lue depuis le RTC
- `uptime` — temps écoulé depuis le démarrage (approx. via RTC)
- `sysinfo` — affiche infos système (processus enregistrés, taille du kernel)
- `reboot` — tente un reset matériel via contrôleur clavier
- `stop` — met le CPU en halt

## Limitations connues
- Gestion mémoire: pas d'allocateur complet (pas de kmalloc/gestion de frames). `sysinfo` n'affiche pas la RAM libre totale.
- Pas d'ordonnanceur réel ni de processus préemptif; `proc_register()` ne crée pas d'exécution concurrente.
- Mapping des caractères accentués peut afficher différemment selon CP437; certains littéraux ont été remplacés par octets explicites.
- `reboot` utilise une méthode simple (`outb 0xFE`) qui fonctionne dans QEMU; on peut ajouter un fallback triple-fault si nécessaire.
- `uptime` est approximatif (calculé depuis le RTC au boot) — pour précision ajouter PIT/timer.

## TODO / Améliorations possibles
- Implémenter gestion mémoire (bitmap frames + `kmalloc`) et afficher `meminfo` dans `sysinfo`.
- Ajouter PIT-based jiffies/uptime et timers.
- Implémenter scheduler (processus/threads, context switch, préemption).
- Supporter exécution de binaires/utilitaires depuis un simple système de fichiers.
- Améliorer le mapping des caractères CP437 pour accents exacts.

## Contribution
- Fork & PR: modifications cohérentes et tests sous QEMU attendus.
- Style: garder le code simple, pas de dépendances libc ; functions freestanding.

## Licence
Voir le fichier `LICENSE` à la racine.

---

Si tu veux, je peux:
- ajouter une page `README` plus courte en anglais, ou
- implémenter l'une des TODOs (par ex. `meminfo` avec un allocator physique). Dis laquelle et je m'en occupe.