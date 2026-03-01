#!/usr/bin/env bash
set -euo pipefail

PREFIX="${PREFIX:-$HOME/.local}"
CSV_DIR="${CSV_RUNNER_DIR:-$HOME/.local/share/csv-runner}"
REMOVE_CSV_DIR=false

for arg in "$@"; do
  case "$arg" in
    --remove-csv-dir)
      REMOVE_CSV_DIR=true
      ;;
    *)
      echo "Unbekannte Option: $arg"
      echo "Verwendung: $0 [--remove-csv-dir]"
      exit 1
      ;;
  esac
done

PLUGIN_PATHS=(
  "$PREFIX/lib64/qt6/plugins/kf6/krunner/krunner_csvrunner.so"
  "$PREFIX/lib64/qt6/plugins/krunner/krunner_csvrunner.so"
  "$PREFIX/lib64/plugins/krunner/krunner_csvrunner.so"
)

REMOVED=0

for PLUGIN_PATH in "${PLUGIN_PATHS[@]}"; do
  if [[ -f "$PLUGIN_PATH" ]]; then
    rm -f "$PLUGIN_PATH"
    echo "Entfernt: $PLUGIN_PATH"
    REMOVED=1
  fi
done

if [[ "$REMOVED" -eq 0 ]]; then
  echo "Nichts zu entfernen in bekannten Pfaden."
fi

if [[ "$REMOVE_CSV_DIR" == true ]]; then
  if [[ -d "$CSV_DIR" ]]; then
    rm -rf "$CSV_DIR"
    echo "CSV-Ordner entfernt: $CSV_DIR"
  else
    echo "CSV-Ordner nicht vorhanden: $CSV_DIR"
  fi
else
  if [[ -d "$CSV_DIR" ]]; then
    printf "CSV-Ordner ebenfalls löschen? [%s] (y/N): " "$CSV_DIR"
    read -r answer || answer=""
    case "$answer" in
      y|Y|yes|YES)
        rm -rf "$CSV_DIR"
        echo "CSV-Ordner entfernt: $CSV_DIR"
        ;;
      *)
        echo "CSV-Ordner beibehalten: $CSV_DIR"
        ;;
    esac
  fi
fi

if command -v kquitapp6 >/dev/null 2>&1; then
  kquitapp6 krunner || true
fi

if command -v krunner >/dev/null 2>&1; then
  nohup krunner >/dev/null 2>&1 &
fi

echo "Deinstallation abgeschlossen."