# jos

# Download

This repository is using an external repository called [josloader](https://github.com/ruth561/josloader) as a submodule. Please execute the following command first.

```
git clone --recurse-submodules https://github.com/ruth561/jos.git
```

# Dependencies

josloader is implemented as an EFI application. When using QEMU, ovmf is required, so please make sure it is installed:

```
sudo apt install ovmf
```

Additionally, install the following dependencies:

```
sudo apt install lld clang qemu-system
```

# Build and run JOS on QEMU

```
make qemu
```

# License

MIT
