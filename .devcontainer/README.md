# Wordclock development containers

VS Code offers two configurations when running **Dev Containers: Reopen in
Container**:

- **Wordclock Simulator (Linux/X11)** for native Linux and remote environments.
- **Wordclock Simulator (WSL2/WSLg)** when VS Code runs in WSL2. This forwards
  the WSLg X11, Wayland and PulseAudio sockets from `/mnt/wslg`.

Choose the configuration matching the host. Keeping the WSLg mount in its own
configuration prevents native Linux container startup from failing when
`/mnt/wslg` does not exist.

## SSH agent on Linux

The Linux variant mounts the host's `SSH_AUTH_SOCK` at the fixed path
`/tmp/ssh-agent`, because the host socket carries a random name that would
otherwise change under the container on every host login. Private key files are
never copied into the container.

Before opening the container, the host agent must be running and hold the key:

```sh
ssh-add -l
```

Inside the container the same command should list the same public identities. If
no agent is running, the mount would resolve to an empty `source=` and container
creation would fail on a docker error that never mentions SSH — an
`initializeCommand` checks for the socket first and says so instead.

The WSLg variant does not mount anything: it uses VS Code's built-in SSH-agent
forwarding.

## GitHub CLI

The image carries `gh`, so branches and pull requests can be tended from the
container instead of by hand in a browser.

It is a **pinned release `.deb`**, verified against its published SHA-256, rather
than the `universe` package. Ubuntu ships 2.46, whose `gh pr edit` still requests
the retired Projects-classic `projectCards` field and fails with a deprecation
error on every call — half of what `gh` would be here for. The upstream apt
repository would fix that as well, but adds a second source and its signing key to
verify behind a TLS-intercepting proxy, whereas the checksum of one file is
anchored in `Dockerfile` where it can be read. The cost is bumping `GH_VERSION`
and the checksums by hand:

```sh
V=$(gh api repos/cli/cli/releases/latest --jq .tag_name | tr -d v)
curl -sSL "https://github.com/cli/cli/releases/download/v$V/gh_${V}_checksums.txt" |
    grep -E 'linux_(amd64|arm64)\.deb$'
```

Only `amd64` and `arm64` carry a pinned checksum; any other architecture fails the
build with a message saying so rather than silently skipping `gh`.

`gh auth login` has to run **in a terminal** — the browser and device flows both
need to prompt. Choose SSH as the protocol for Git operations: the remote already
speaks SSH over port 443, and HTTPS to `github.com` is blocked in some networks
this is used from. `gh`'s own API calls go over HTTPS and honour `HTTPS_PROXY`,
which is why they work where `git push` over HTTPS does not.

The login lands in `~/.config/gh` and so does **not** survive a rebuild. It holds
a credential, which puts persisting it in a local variant — see below.

## Shared editor configuration

VS Code extensions, CMake settings and the toolchain sanity check live in the
local dev container feature in `shared/`, which every configuration references:

```jsonc
"features": { "./../shared": {} }
```

Feature metadata is merged with the referencing `devcontainer.json`, so a
variant can add its own extensions on top without repeating the shared list.

The shared settings install `clangd` and configure it to read CMake's
`build/compile_commands.json`; C/C++ Tools remains installed for debugging but
does not provide IntelliSense.

Feature metadata is baked into the image at build time, so editing
`shared/devcontainer-feature.json` requires **Dev Containers: Rebuild
Container**. Reopening the folder keeps the previous metadata, which looks like
a newly added extension being ignored. Entries in the `customizations` section
of a `devcontainer.json` take effect on reopen instead.

A `devcontainer.metadata` label in `Dockerfile` does *not* work for this:
clients read image metadata from the image the final `FROM` points at, and then
overwrite the label on the image built from this file. Extensions declared that
way are silently ignored.

## Site-specific variants

An additional configuration directory next to `linux/` and `wslg/` shows up as
another entry in the same picker, which is the place for settings that must not
be committed (internal base images, registry-hosted features, credentials). The
`Dockerfile` takes a `BASE_IMAGE` build argument for that purpose:

```jsonc
"build": {
    "dockerfile": "../Dockerfile",
    "context": "..",
    "args": { "BASE_IMAGE": "registry.example.internal/base:latest" }
}
```

Add such a directory to `.gitignore` to keep it local; `/.devcontainer/vector/`
is already listed there.

Anything that persists credentials belongs in such a variant rather than in the
committed ones. Persisting a tool's login across rebuilds, for instance, takes a
named volume plus one `chown`, because a volume whose target does not exist in
the image is created owned by root:

```jsonc
"mounts": [
    "source=wordclock-<tool>,target=/home/vscode/.<tool>,type=volume"
],
"postCreateCommand": "sudo chown vscode:vscode /home/vscode/.<tool>"
```
