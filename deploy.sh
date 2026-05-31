#!/usr/bin/env bash
set -euo pipefail
# Deploy docs/ to GitHub Pages or upload to a static host.
# Usage: ./deploy.sh [method]
# Methods:
#  gh    - use gh CLI to create a Pages release (requires gh auth)
#  rsync - rsync to remote server (set RSYNC_DEST env var)
#  local - just start a local server to preview

method=${1-local}
case "$method" in
  gh)
    if ! command -v gh >/dev/null 2>&1; then echo "gh CLI required"; exit 1; fi
    read -p "Repo (owner/repo): " REPO
    read -p "Branch to publish gh-pages (default gh-pages): " BRANCH
    BRANCH=${BRANCH:-gh-pages}
    echo "Publishing docs/ to $REPO branch $BRANCH"
    # Create a temp worktree and push docs
    tmp=$(mktemp -d)
    git init "$tmp/repo" >/dev/null
    cp -r docs/* "$tmp/repo/"
    pushd "$tmp/repo" >/dev/null
    git add .
    git commit -m "Publish site" >/dev/null || true
    git branch -M $BRANCH
    git remote add origin "https://github.com/$REPO.git"
    git push -f origin $BRANCH
    popd >/dev/null
    rm -rf "$tmp"
    ;;
  rsync)
    if [ -z "${RSYNC_DEST-}" ]; then echo "Set RSYNC_DEST env var (user@host:/path)"; exit 1; fi
    rsync -avz docs/ "$RSYNC_DEST"
    ;;
  local)
    echo "Serving docs/ on http://localhost:8000"
    (cd docs && python3 -m http.server 8000)
    ;;
  *)
    echo "Usage: $0 {gh|rsync|local}"; exit 2
    ;;
esac
