#!/usr/bin/env bash
set -euo pipefail

SOCKET_PATH="/run/playit/playitd.sock"
RUNTIME_DIR="/run/playit"
LOG_DIR="/var/log/playit"
SECRET_PATH="/workspaces/Server-minecraft/playit/playit.toml"
DAEMON="/opt/playit/playitd"
AGENT="/opt/playit/agent"
PLAYIT_USER="playit"

if [[ "$(id -u)" -ne 0 ]]; then
  echo "Run as root: sudo bash start-playit.sh"
  exit 1
fi

for bin in "$DAEMON" "$AGENT"; do
  if [[ ! -x "$bin" ]]; then
    echo "Error: missing executable: $bin"
    exit 1
  fi
done

if ! getent passwd "$PLAYIT_USER" >/dev/null; then
  echo "Error: user '$PLAYIT_USER' does not exist"
  exit 1
fi

if ! getent group "$PLAYIT_USER" >/dev/null; then
  echo "Error: group '$PLAYIT_USER' does not exist"
  exit 1
fi

mkdir -p "$RUNTIME_DIR" "$LOG_DIR" "$(dirname "$SECRET_PATH")"
chown -R "$PLAYIT_USER":"$PLAYIT_USER" "$RUNTIME_DIR" "$LOG_DIR" "$(dirname "$SECRET_PATH")"

if [[ -e "$SOCKET_PATH" ]]; then
  rm -f "$SOCKET_PATH"
fi

echo "Starting playit daemon..."
runuser -u "$PLAYIT_USER" -- "$DAEMON" \
  --platform-docker \
  --secret-path "$SECRET_PATH" \
  --socket-path "$SOCKET_PATH" \
  -l "$LOG_DIR/playit.log" >/dev/null 2>&1 &

DAEMON_PID=$!

echo "Waiting for playit socket..."
for i in {1..30}; do
  if [[ -S "$SOCKET_PATH" ]]; then
    break
  fi

  if ! kill -0 "$DAEMON_PID" 2>/dev/null; then
    echo "Error: playitd exited early"
    tail -n 50 "$LOG_DIR/playit.log" || true
    exit 1
  fi

  sleep 1
done

if [[ ! -S "$SOCKET_PATH" ]]; then
  echo "Error: playitd did not create socket at $SOCKET_PATH"
  tail -n 50 "$LOG_DIR/playit.log" || true
  exit 1
fi

if [[ ! -f "$SECRET_PATH" ]]; then
  echo "Running playit setup..."
  runuser -u "$PLAYIT_USER" -- "$AGENT" --socket-path "$SOCKET_PATH" setup
else
  echo "Secret file already exists, skipping setup."
fi

echo "Playit is up."
echo "Socket: $SOCKET_PATH"