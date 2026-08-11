# Wordclock development containers

VS Code offers two configurations when running **Dev Containers: Reopen in
Container**:

- **Wordclock Simulator (Linux/X11)** for native Linux and remote environments.
- **Wordclock Simulator (WSL2/WSLg)** when VS Code runs in WSL2. This forwards
  the WSLg X11, Wayland and PulseAudio sockets from `/mnt/wslg`.

Choose the configuration matching the host. Keeping the WSLg mount in its own
configuration prevents native Linux container startup from failing when
`/mnt/wslg` does not exist.

Both configurations mount the named `wordclock-codex` volume at
`/home/vscode/.codex`. This preserves the Codex login and local configuration
when the development container is rebuilt. The volume contains credentials and
must not be shared or committed.

## SSH agent with Podman

The native Linux configuration bind-mounts the host's `SSH_AUTH_SOCK` at
`/tmp/ssh-agent` and sets the environment variable inside the container to that
path. The WSLg configuration instead relies on VS Code's built-in SSH-agent
forwarding.

The image also enforces the standard `1777` permissions on `/tmp` and creates
`/tmp/user`. This allows the non-root `vscode` user to create runtime files when
the Linux configuration derives `XDG_RUNTIME_DIR` from `/tmp/user/$(id -u)`.
Some Podman storage/filesystem combinations otherwise leave `/tmp` at `0755`.

Before opening the container, verify on the host that the agent is available
and contains the expected key:

```sh
test -S "$SSH_AUTH_SOCK" && ssh-add -l
```

After **Dev Containers: Rebuild and Reopen in Container**, `ssh-add -l` inside
the container should list the same public key identities. The private key files
themselves are not copied into the container. If the Linux configuration fails
to start, verify that `SSH_AUTH_SOCK` names an existing socket on the host.

## Shared editor configuration

VS Code extensions, CMake settings and the toolchain sanity check live in the
local dev container feature in `shared/`, which every configuration references:

```jsonc
"features": { "./../shared": {} }
```

Feature metadata is merged with the referencing `devcontainer.json`, so a
variant can add its own extensions on top without repeating the shared list.

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
be committed (internal base images, registry-hosted features). The `Dockerfile`
takes a `BASE_IMAGE` build argument for that purpose:

```jsonc
"build": {
    "dockerfile": "../Dockerfile",
    "context": "..",
    "args": { "BASE_IMAGE": "registry.example.internal/base:latest" }
}
```

Add such a directory to `.gitignore` to keep it local.
