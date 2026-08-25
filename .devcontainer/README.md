# Wordclock development containers

VS Code offers two configurations when running **Dev Containers: Reopen in
Container**:

- **Wordclock Simulator (Linux/X11)** for native Linux and remote environments.
- **Wordclock Simulator (WSL2/WSLg)** when VS Code runs in WSL2. This forwards
  the WSLg X11, Wayland and PulseAudio sockets from `/mnt/wslg`.

Choose the configuration matching the host. Keeping the WSLg mount in its own
configuration prevents native Linux container startup from failing when
`/mnt/wslg` does not exist.

## Logins that survive a rebuild

Two named volumes carry a login across a rebuild: `wordclock-codex` at `/home/vscode/.codex`
and `wordclock-gh` at `/home/vscode/.config/gh`. Without them both have to be entered again
by hand every time the container is built.

They are declared by the **shared feature**, so every variant gets them, because both tools
belong to every variant: the Codex extension is in the shared extension list, and every push
and pull request goes through `gh`. The `Dockerfile` creates both directories as the remote
user at mode 0700 first — an empty named volume takes its ownership from the path it covers,
so a path that does not exist in the image yields a volume owned by root that the tool cannot
write. 0700 rather than the package cache's 0755, because a credential is not a cache, and
`gh` refuses to read a config directory that others can enter.

Both volumes hold credentials: do not share, export or remove one unless the saved login
should be discarded.

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
need to prompt.

`gh` is pointed at `github.com` by the shared feature. The internal base image
sets `GH_HOST` to the company's own GitHub, and every call for this repository
would otherwise answer `HTTP 401` against a host that has never heard of it.

Which protocol to pick for Git operations depends on the network, and both fail
in ways that do not name the cause — see below.

The login lands in `~/.config/gh` and so does **not** survive a rebuild. It holds
a credential, which puts persisting it in a local variant — see below.

## Reaching GitHub: which transport works

Both transports exist and either can be the broken one, depending on the network
the container runs in. Neither says so plainly when it fails:

| Transport | What a failure looks like | What it means |
|---|---|---|
| SSH, port 22 | `ssh` hangs, `ssh-keyscan` prints a banner that is not GitHub's | the port is filtered |
| SSH, port 443 | `Connection reset by 140.82.121.36 port 443` | something between here and GitHub cuts the session |
| SSH, either port | `Host key verification failed` | only the known host key was missing — see below, this one is fixed |
| HTTPS | `TLS`/certificate errors, or a proxy error page | the intercepting proxy, or no credential helper |

Two commands tell them apart, and take a few seconds:

```sh
ssh -T -p 443 git@ssh.github.com          # "Hi <user>!" means SSH works
git ls-remote https://github.com/AndreasBur/Wordclock.git | head -1
```

Switch the remote to whichever answered:

```sh
git remote set-url origin ssh://git@ssh.github.com:443/AndreasBur/Wordclock.git
git remote set-url origin https://github.com/AndreasBur/Wordclock.git
```

HTTPS needs no separate login: `gh auth setup-git` installs `gh`'s credential
helper, so `git` uses the token that is already there. `gh`'s own API calls go
over HTTPS and honour `HTTPS_PROXY` either way, which is why `gh pr create` can
work in a container whose `git push` does not.

**The host key is in the image.** `~/.ssh` belongs to the container and is empty
after every rebuild, so the first SSH operation used to fail with `Host key
verification failed` — a message that reads like a missing permission and is
not one. `Dockerfile` writes GitHub's published ed25519 key into
`/etc/ssh/ssh_known_hosts` for `github.com`, `ssh.github.com` and
`[ssh.github.com]:443`, since an entry is keyed by host *and* port. It is public
data, not a secret: the fingerprint stands beside the key and `ssh-keygen -lf
/etc/ssh/ssh_known_hosts` prints it back for comparison with what GitHub
publishes. Fetching it with `ssh-keyscan` at build time would trust whatever
answered through the proxy, which is the thing a known host key prevents.

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

## PlatformIO for the ESP32 backend

The image carries PlatformIO in a virtual environment under `/opt/platformio`, on the
`PATH`, plus `python3` and `nodejs` — which is what
[`platform/esp32/test/run.sh`](../platform/esp32/test/run.sh) needs for the page embedding
and for the local server that puts both pages in front of it. So `pio run -d platform/esp32` works with nothing to install.

The **package cache is a named volume** at `~/.platformio`, declared by the shared feature.
The platform, the Xtensa toolchain and the Arduino core are about 1.5 GB and are downloaded
on first use; the volume is what keeps a container rebuild from fetching them again. An
empty named volume takes its ownership from the path it covers, so the Dockerfile creates
that path as the remote user first — otherwise PlatformIO cannot write into it.

Behind a **TLS-intercepting proxy** there is one trap that names neither the proxy nor the
cause. PlatformIO downloads with `requests` and hands it its own `certifi` bundle
explicitly, so it ignores `REQUESTS_CA_BUNDLE` and fails with
`CERTIFICATE_VERIFY_FAILED: self-signed certificate in certificate chain` on URLs that
`curl` and `git` fetch without complaint. The image appends its own trust store to that
bundle, which keeps verification on rather than turning it off.

That fix has to happen twice, because the platform packages build a *second* virtual
environment under `~/.platformio` at first use — and that one lives in the volume, not in
the image. `platformio-trust-store`, installed by the shared feature and run as its
`postStartCommand`, appends the store to any bundle under `~/.platformio` that does not
already carry it.

**Flashing is not covered.** The container can build; getting the USB serial adapter into
it is another matter, and under WSL2 it needs `usbipd-win` on the Windows side before the
container can see anything at all. Build here, flash from the host.

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

What must stay local is a **secret or an internal address** — a registry-hosted feature, an
internal base image, a token written into the file. A *mount* that persists a login is
neither: the line names a volume, the volume lives on the machine that created it, and
nothing about it reaches the repository. That is why the two login volumes above are in the
committed shared feature, where every variant gets them, rather than in a variant somebody
has to build for themselves.

Persisting a further tool's login is therefore one mount beside them and one line in the
`Dockerfile`, the way `~/.codex` and `~/.config/gh` are done. In a variant that cannot
change the image, the ownership is fixed after creation instead, because a volume whose
target does not exist in the image is created owned by root:

```jsonc
"mounts": [
    "source=wordclock-<tool>,target=/home/vscode/.<tool>,type=volume"
],
"postCreateCommand": "sudo chown vscode:vscode /home/vscode/.<tool>"
```
