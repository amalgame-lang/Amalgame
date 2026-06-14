#!/usr/bin/env bash
# Launch code-server for the Amalgame IDE image.
#
#   PORT      bind port inside the container          (default 8080)
#   PASSWORD  if set → password auth; otherwise auth is disabled
#             (fine for a local `docker run`; set it when exposing
#              the container beyond localhost)
#   AMALGAME_WORKSPACE  workspace/folder to open
#                       (default /home/coder/amalgame.code-workspace)
set -euo pipefail

PORT="${PORT:-8080}"
WORKSPACE="${AMALGAME_WORKSPACE:-/home/coder/amalgame.code-workspace}"

if [ -n "${PASSWORD:-}" ]; then
    AUTH="password"
else
    AUTH="none"
fi

echo "amc: $(amc --version 2>/dev/null || echo '??')"
echo "code-server → http://localhost:${PORT}  (auth: ${AUTH})"

exec code-server \
    --bind-addr "0.0.0.0:${PORT}" \
    --auth "${AUTH}" \
    --disable-telemetry \
    --disable-update-check \
    "${WORKSPACE}"
