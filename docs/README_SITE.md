Site publishing notes

This folder is prepared to be the GitHub Pages `docs/` content. To preview locally:

```bash
cd docs
python3 -m http.server 8000
# then open http://localhost:8000 in your browser
```

Recommended steps to publish to GitHub Pages:

1. Create a GitHub repo `owner/repo` and push your source code.
2. Commit the `docs/` folder into the repo and enable Pages from `docs/` in repository settings.

Or run `./deploy.sh gh` to push `docs/` to `gh-pages` branch of a repo (requires `gh` or network access).

Try page notes

- The Try page uses an in-browser emulator (v86) if you host `v86.js` and `v86.wasm` in `docs/assets/v86/`.
- If you cannot host v86, the Try page will offer an "Open in online emulator" button that launches the copy.sh/v86 site with your ISO URL.

CORS and range requests

- Ensure `docs/build/copperos.iso` is served with byte-range support and accessible over HTTPS for the in-browser emulator to mount it.
