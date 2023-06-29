# Tank game

A "precursor" to my game [sasma.io](https://github.com/xyl1t/sasma-io).

## How to run

Enter the `game/` directory and execute the following commands:

```shell
mkdir build
cd build
cmake ..
make
./game
```

Enjoy

## How this project is built

It uses [SDL2](https://www.libsdl.org/) for windowing and my basic graphics 
library called [sgl](https://github.com/xyl1t/sgl). I also made a crude 
implementation of [ECS](https://github.com/SanderMertens/ecs-faq). The game 
allocates a bunch of memory in the beginning and works just with it and doesn't 
do any further allocations.


## Hot reloading

Start the game

```shell
make game
./game
```

Edit the code in `src/game.c`. And when you want to see the changes run:

```shell
make game_code
```

The built shared library will be automatically loaded. In case you don't see
changes, that may be because the data has been loaded already. In that case
toggle debug mode with the `B` key and click `I` to re-initialize the game.

You can also inject code between reloads in the `inject_code_on_reload()`
function.

