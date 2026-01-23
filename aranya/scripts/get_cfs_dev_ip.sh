#!/usr/bin/env bash
set -euo pipefail

CONTAINER="cfs-dev"
NETWORK="cosmos-project_default"

if ! command -v docker >/dev/null 2>&1; then
  echo "Error: docker is not installed" >&2
  exit 2
fi

# Ensure the container exists and is running
if ! docker inspect -f '{{.State.Running}}' "$CONTAINER" >/dev/null 2>&1; then
  echo "Error: Container '$CONTAINER' not found or not running" >&2
  exit 1
fi

running=$(docker inspect -f '{{.State.Running}}' "$CONTAINER" 2>/dev/null || echo "false")
if [ "$running" != "true" ]; then
  echo "Error: Container '$CONTAINER' is not running" >&2
  exit 1
fi

# Query the container's IP on the specified network
ip=$(docker inspect -f "{{with index .NetworkSettings.Networks \"$NETWORK\"}}{{.IPAddress}}{{end}}" "$CONTAINER" 2>/dev/null || true)
if [ -z "$ip" ]; then
  echo "Error: Container '$CONTAINER' is not connected to network '$NETWORK' or has no IP" >&2
  exit 1
fi

echo "$ip"
