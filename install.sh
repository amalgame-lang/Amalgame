# Dépendances
sudo apt install -y \\
    valac \\
    libglib2.0-dev \\
    libgee-0.8-dev \\
    meson \\
    ninja-build \\
    libgc-dev \\
    pkg-config

# Compiler le transpileur
git clone https://github.com/NeitsabTeguom/CODE.git
cd CODE
meson setup build
ninja -C build

# Vérifier
./build/codec --version
