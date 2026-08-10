# Wordclock development containers

VS Code offers two configurations when running **Dev Containers: Reopen in
Container**:

- **Wordclock Simulator (Linux/X11)** for native Linux and remote environments.
- **Wordclock Simulator (WSL2/WSLg)** when VS Code runs in WSL2. This forwards
  the WSLg X11, Wayland and PulseAudio sockets from `/mnt/wslg`.

Choose the configuration matching the host. Keeping the WSLg mount in its own
configuration prevents native Linux container startup from failing when
`/mnt/wslg` does not exist.

Shared VS Code extensions, CMake settings and lifecycle commands are stored in
the `devcontainer.metadata` label in `Dockerfile`. Dev Container clients merge
that metadata with either platform-specific configuration, so common settings
only need to be maintained once.

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
