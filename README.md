# csv-runner

KRunner-Plugin (KDE Plasma 6, Wayland), das `key|value` aus allen CSV-Dateien in einem Ordner lädt.

Bei Treffer auf `key` wird abhängig von `value` ausgeführt:

1. `https://...` -> im Browser öffnen
2. `joplin://...` -> in Joplin öffnen
3. Mailadresse -> Mail-Dialog öffnen
4. `pass` -> Passwort aus `pass <key>` in Zwischenablage
5. `otp` -> OTP aus `pass otp <key>` in Zwischenablage
6. sonst -> `value` in Zwischenablage

## CSV-Format

Eine oder mehrere Dateien `*.csv` im Zielordner, pro Zeile:

```text
key|value
```

Es werden auch CSV-Dateien in Unterordnern des Zielordners berücksichtigt.

Beispiel:

```text
gh|https://github.com
notes|joplin://x-callback-url/openNote?id=123456
mail-team|team@example.org
bank/login|pass
bank/otp|otp
my-token|abc123
```

Zeilen mit `#` am Anfang werden ignoriert.

## CSV-Ordner

- Standard: `~/.local/share/csv-runner`
- Optional zusätzlicher Ordner per Umgebungsvariable: `CSV_RUNNER_DIR`
- Legacy-Fallback: `~/csv-runner` (falls der neue Standardordner nicht existiert)

Standardmäßig werden `~/.local/share/csv-runner` und `~/csv-runner` gescannt.
Ist `CSV_RUNNER_DIR` gesetzt, wird dieser Ordner zusätzlich mitgescannt.

CSV-Änderungen (neue/änderte Dateien) werden ohne Neuanmeldung bei der nächsten
KRunner-Abfrage berücksichtigt.

Beispiel:

```bash
export CSV_RUNNER_DIR="$HOME/secrets/csv-runner"
```

## Build (Fedora)

```bash
sudo dnf install -y \
	cmake extra-cmake-modules gcc-c++ \
	qt6-qtbase-devel \
	kf6-kconfig-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-krunner-devel

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix "$HOME/.local"
```

## Schnellstart

```bash
./install.sh
```

Optionales Zielverzeichnis:

```bash
PREFIX="$HOME/.local" ./install.sh
```

Deinstallieren:

```bash
./uninstall.sh
```

Dabei fragt das Skript optional, ob der CSV-Ordner mit gelöscht werden soll.

Ohne Rückfrage inkl. CSV-Ordner löschen:

```bash
./uninstall.sh --remove-csv-dir
```

Optionales Zielverzeichnis bei Deinstallation:

```bash
PREFIX="$HOME/.local" ./uninstall.sh
```

Beispiel-CSV liegt in `examples/sample.csv`.
`install.sh` legt den Standardordner automatisch an und kopiert `sample.csv` hinein,
falls noch keine Datei vorhanden ist.

Zum manuellen Testen z. B. nach `~/.local/share/csv-runner` kopieren:

```bash
mkdir -p "$HOME/.local/share/csv-runner"
cp examples/sample.csv "$HOME/.local/share/csv-runner/"
```

Danach KDE neu laden:

```bash
kquitapp6 krunner && krunner &
```

oder einmal aus-/einloggen.

Prüfen, ob der Runner geladen ist:

```bash
krunner --list | grep -i csv
```

Falls bei Installation in `~/.local` kein Treffer erscheint, muss `QT_PLUGIN_PATH`
für die Plasma-Sitzung gesetzt sein. `install.sh` legt dafür automatisch an:

```text
~/.config/plasma-workspace/env/csv-runner.sh
```

Danach einmal ab- und wieder anmelden.

`install.sh` setzt `QT_PLUGIN_PATH` zusätzlich direkt in die laufende User-Session
(systemd/DBus), sodass es meist sofort ohne Neu-Anmeldung funktioniert.

Manuell (falls nötig):

```bash
systemctl --user set-environment QT_PLUGIN_PATH="$HOME/.local/lib64/qt6/plugins"
dbus-update-activation-environment --systemd QT_PLUGIN_PATH="$HOME/.local/lib64/qt6/plugins"
kquitapp6 krunner || true
nohup krunner >/dev/null 2>&1 &
```

## Hinweise zu `pass` / `otp`

- Für `pass` muss das CLI-Tool `pass` installiert und entsperrbar sein.
- Für `otp` wird `pass-otp` erwartet (`pass otp <entry>`).