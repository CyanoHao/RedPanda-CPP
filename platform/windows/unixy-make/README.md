# Unixy (Cygwin) Make for Windows

## Build

```bash
podman run -it --rm \
  -v $PWD:/mnt -w /mnt \
  -v $PWD/xp/build:/tmp/build \
  -v $PWD/xp/cross:/usr/local \
  docker.io/amd64/ubuntu:16.04

# in container
./build.xp
```

## Maintenance

Due to the complexity of the build environment, the binary production is added to the repo and directly included in the distribution package. Thus reproducible build is required for everyone to verify the binaries are exactly built from source.

After any modification to source (including the build scripts, patches), you are expected to build at least twice to verify the build is reproducible -- `git add` after the first build, and then `git status` after the second build to verify no new changes are added.
