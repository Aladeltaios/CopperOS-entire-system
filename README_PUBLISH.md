Publishing CopperOS release

Options to publish `build/copperos-release.tar.gz`:

1) Quick anonymous upload (transfer.sh)

- From your machine run:

```bash
bash publish.sh transfer
```

- If `curl` works and transfer.sh is reachable the script prints a public URL.

2) GitHub Releases (recommended for publishing releases)

- Option A: Use the `gh` CLI (recommended)
  - Install `gh`: https://cli.github.com/
  - Authenticate: `gh auth login`
  - Run: `bash publish.sh github` and follow prompts (enter `owner/repo` and tag like `v0.1`).

- Option B: Use `GITHUB_TOKEN` env var and GitHub API
  - Create a Personal Access Token with `repo` scope.
  - Export it: `export GITHUB_TOKEN=ghp_...`
  - Run: `bash publish.sh github` and follow prompts.

3) Manual uploads

- You can manually upload `build/copperos-release.tar.gz` from [build/copperos-release.tar.gz](build/copperos-release.tar.gz) to any file host (Google Drive, Dropbox, or GitHub Releases).

4) Checksums

- `build/SHA256SUMS` contains SHA-256 checksums for the included artifacts. Verify after download:

```bash
sha256sum -c build/SHA256SUMS
```

Security & notes
- Do NOT share your `GITHUB_TOKEN` publicly. If you want me to upload to GitHub on your behalf, provide the token privately and confirm.
- If your environment blocks outbound HTTP (corporate networks), transfer.sh may fail; use a machine with outbound internet or upload manually.
