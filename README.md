# coevo1.x - model of co-evolution of species

### First version of individual based co-evolution model started ~1993-95

This was my first own serious simulation model. It was not published anywhere, 
but was used to obtain a government grant for young researchers that led to version 2. 
and latter's.
The code is written in very simple C ++ but it implements the main idea of the model. 
The next versions differed mainly in the increasingly complex visualization and 
complexity of the collected statistics.

The whole history of this research is available in a project at Research Gate.

___"Macroevolution & coevolution in CA & network like models"___

https://www.researchgate.net/project/Macroevolution-coevolution-in-CA-network-like-models

## Quick start

It needs __SYMSHELLLIGHT__ library in sister directory _SymShellLight/_ !

So go into your choice directory and run:

```console
  $ git clone https://github.com/borkowsk/SymShellLight.git
  $ git clone https://github.com/borkowsk/Coevo.git
```

Then go into _Coevo/_ directory, and, if you have *CMake* installed, run:

```console
  $ cmake .
  $ make
  $ ./coevo1
```
Or, if you have *Ninja* installed do better:

```console
$ mkdir Ninja-make
$ cd  Ninja-make
$ cmake ../ -G Ninja
$ ninja
$ ./coewo1
```

## Licencing

You can use this software freely for educational and research purposes, 
but if you feel that it would be appropriate to repay somehow, please 
finance me a big coffee :-)

COFFEE 
https://www.buymeacoffee.com/wborkowsk 
https://www.paypal.com/paypalme/wborkowsk

--------------------------------------------------------
Base directory: https://github.com/borkowsk/Coevo1_model