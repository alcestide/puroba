# puroba
![Standard](https://img.shields.io/badge/C_Standard-C11-00599C?style=for-the-badge&logo=c&logoColor=white)
![Compiler](https://img.shields.io/badge/Compiler-GCC-FFD21E?style=for-the-badge&logo=gnu-bash&logoColor=black)
![Optimization](https://img.shields.io/badge/Optimization-O2-orange?style=for-the-badge)
![UI-Lib](https://img.shields.io/badge/UI-ncursesw-FF6600?style=for-the-badge&logo=gnuterminal&logoColor=white)
![Build-Tool](https://img.shields.io/badge/Build-GNU_Make-gray?style=for-the-badge&logo=gnu&logoColor=white)

**Puroba** is a simple **terminal**-native **system** monitor/dashboard for Unix-like environments. 

It provides real-time tracking of **CPU**, **memory**, **disk**, and **network** metrics through a modular, multi-tab interface designed for clarity and speed.

<img width="1599" height="865" alt="image" src="https://github.com/user-attachments/assets/bbcb83d3-ad04-49fd-b50a-2a8dd0a0e515" />

# Installation
To install puroba, you will mainly need "ncursesw" development headers and make.

### Ubuntu/Debian
```bash
sudo apt install libncursesw5-dev build-essential
```

### Build
```Bash
git clone https://github.com/alcestide/puroba.git
cd puroba
make
```

### Run

```Bash
./build/puroba
```

## License

Released under [MIT](/LICENSE) by [@alcestide](https://github.com/alcestide).
