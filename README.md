<p align="center">
  Biggest meme on the world. And it hits. Somehow. <br>
  <img src="https://flat.badgen.net/badge/VAC/Undetected./green?icon=terminal"><br>
<img src="https://i.ibb.co/YjHMLYd/niggatoni.png"><br>v2
</p> <br>

### Feature list: <br>

- Desync
- Forced resolver
- Fixed Danger Zone crashes
- Remasterd Aimtux GUI ( made it more modern )

## How to install: <br>

Debian based: <br>
```sudo apt-get install cmake g++ gdb git libsdl2-dev zlib1g-dev```<br> 
  **If you get this error** `fatal error: xdo.h: No such file or directory` please install ```sudo apt install libxdo-dev```

Arch: <br>
```sudo pacman -S base-devel cmake gdb git sdl2 patchelf``` <br> 
**If you get this error:** `fatal error: xdo.h: No such file or directory` please install ```sudo pacman -S xdo xdotool``` 

Gentoo: <br>
```sudo emerge cmake dev-vcs/git gdb libsdl2 mesa``` <br> 
   **If you get this error** `fatal error: xdo.h: No such file or directory` please install ```sudo emerge x11/misc/xdo ```

openSUSE: <br> 
```sudo zypper install cmake gcc-c++ gdb git Mesa-libGL-devel SDL2-devel zlib-devel``` <br> 
  **If you get this error** `fatal error: xdo.h: No such file or directory` please install ```sudo zypper install xdotool ```

# Building:
```./build```

# Screenshots
<a href="https://ibb.co/Y7hMcPr"><img src="https://i.ibb.co/MD74cZX/image.png" alt="image" border="2"></a>



# Credits:
- xDylan#0376 - Backporting a fuckton of shit from LWSS/Fuzion
- LWSS/Fuzion for being a base
