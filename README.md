# jos

# Dependencies

This repository is using an external repository called [josloader](https://github.com/ruth561/josloader) as a submodule. Please execute the following command first.

```
git submodule init
git submodule update
```

josloader is implemented as an EFI application. When using QEMU, ovmf is required, so please make sure to install it.

```
sudo apt install ovmf
```
