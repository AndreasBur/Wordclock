# Wordclock development containers

VS Code provides two configurations:

- **Linux/X11** for native Linux and remote Linux hosts.
- **WSL2/WSLg** for VS Code running in WSL2.

Both use the same image and shared editor configuration. Select the variant
matching the host, then run **Dev Containers: Rebuild and Reopen in Container**.

## Persistent Codex data

The named volume `wordclock-codex` is mounted at `/home/vscode/.codex` in both
variants. It preserves the Codex login and local configuration across rebuilds.
The volume contains credentials and must not be shared or committed.

## SSH agent on Linux

The Linux variant mounts the host's `SSH_AUTH_SOCK` at `/tmp/ssh-agent` and
keeps that stable path in VS Code terminals. Before opening the container, the
host agent must be running and contain the required key:

```sh
ssh-add -l
```

Inside the container, the same command should list the same public identities.
Private key files are never copied into the container. The WSLg variant uses VS
Code's built-in SSH-agent forwarding instead.

## Shared configuration

Extensions, CMake settings and the toolchain check live in `shared/`. Changes to
the local feature require a container rebuild.
