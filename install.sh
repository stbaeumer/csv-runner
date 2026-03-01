#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX="${PREFIX:-$HOME/.local}"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build}"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j
cmake --install "$BUILD_DIR" --prefix "$PREFIX"

PLUGIN_PATH="$PREFIX/lib64/qt6/plugins/kf6/krunner/krunner_csvrunner.so"
ICON_DIR="$PREFIX/share/icons/hicolor/scalable/apps"
PLASMA_ENV_DIR="$HOME/.config/plasma-workspace/env"
PLASMA_ENV_FILE="$PLASMA_ENV_DIR/csv-runner.sh"
CSV_DIR="${CSV_RUNNER_DIR:-$HOME/.local/share/csv-runner}"
SAMPLE_SRC="$SCRIPT_DIR/examples/sample.csv"
SAMPLE_DST="$CSV_DIR/sample.csv"

if command -v kquitapp6 >/dev/null 2>&1; then
  kquitapp6 krunner || true
fi

if command -v krunner >/dev/null 2>&1; then
  nohup krunner >/dev/null 2>&1 &
fi

mkdir -p "$PLASMA_ENV_DIR"
cat > "$PLASMA_ENV_FILE" <<EOF
#!/usr/bin/env bash
export QT_PLUGIN_PATH="$PREFIX/lib64/qt6/plugins:\${QT_PLUGIN_PATH:-}"
EOF
chmod +x "$PLASMA_ENV_FILE"

mkdir -p "$CSV_DIR"
if [[ -f "$SAMPLE_SRC" && ! -f "$SAMPLE_DST" ]]; then
  cp "$SAMPLE_SRC" "$SAMPLE_DST"
fi

echo "Installiert nach: $PREFIX"
if [[ -f "$PLUGIN_PATH" ]]; then
  echo "Plugin: $PLUGIN_PATH"
else
  echo "Plugin nicht gefunden unter: $PLUGIN_PATH"
fi

echo "Icons in: $ICON_DIR"
ls -1 "$ICON_DIR" 2>/dev/null | grep -E '^(joplin|pass|otp|www|mail|csv-runner)\.svg$' || true
echo "Plasma-Env: $PLASMA_ENV_FILE"
echo "CSV-Ordner: $CSV_DIR"

echo "Wichtig: Für Alt+Leertaste/Plasma-KRunner einmal ab- und wieder anmelden."