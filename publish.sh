#!/usr/bin/env bash
set -euo pipefail
RELEASE_FILE=build/copperos-release.tar.gz
if [ ! -f "$RELEASE_FILE" ]; then
  echo "Release file $RELEASE_FILE not found. Run make iso and package first." >&2
  exit 1
fi

upload_transfer() {
  if ! command -v curl >/dev/null 2>&1; then
    echo "curl required for transfer.sh upload" >&2
    return 1
  fi
  echo "Uploading to transfer.sh..."
  curl --progress-bar --upload-file "$RELEASE_FILE" "https://transfer.sh/$(basename $RELEASE_FILE)"
}

upload_github() {
  # Requires GITHUB_TOKEN env var with repo scope or 'gh' CLI authenticated
  if command -v gh >/dev/null 2>&1; then
    echo "Using gh CLI to create a release and upload asset"
    read -p "GitHub repo (owner/repo): " REPO
    read -p "Release tag (e.g. v0.1): " TAG
    gh release create "$TAG" "$RELEASE_FILE" --repo "$REPO" --title "$TAG" --notes "CopperOS release $TAG"
    return
  fi
  if [ -z "${GITHUB_TOKEN-}" ]; then
    echo "Set GITHUB_TOKEN env var with repo permissions or install/authorize gh CLI." >&2
    return 1
  fi
  read -p "GitHub repo (owner/repo): " REPO
  read -p "Release tag (e.g. v0.1): " TAG
  # Create release
  CREATE_URL="https://api.github.com/repos/$REPO/releases"
  DATA=$(printf '{"tag_name":"%s","name":"%s","draft":false,"prerelease":false}' "$TAG" "$TAG")
  echo "Creating release $TAG for $REPO..."
  RESP=$(curl -s -H "Authorization: token $GITHUB_TOKEN" -H "Content-Type: application/json" -d "$DATA" "$CREATE_URL")
  UPLOAD_URL=$(echo "$RESP" | grep -o '"upload_url": *"[^"]*"' | sed -E 's/"upload_url": *"([^"]+)"/\1/' | sed -E 's/\{\?name,label\}//')
  if [ -z "$UPLOAD_URL" ]; then
    echo "Failed to create release: $RESP" >&2
    return 1
  fi
  echo "Uploading asset..."
  curl -s -H "Authorization: token $GITHUB_TOKEN" -H "Content-Type: application/octet-stream" --data-binary "@$RELEASE_FILE" "$UPLOAD_URL?name=$(basename $RELEASE_FILE)" 
}

case "${1-}" in
  transfer)
    upload_transfer
    ;;
  github)
    upload_github
    ;;
  *)
    echo "Usage: $0 {transfer|github}" >&2
    exit 2
    ;;
esac
