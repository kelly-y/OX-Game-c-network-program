# OX-Game-c-network-program

### Introduction
Game-Server and Game-Client are included to support OX games.

#### Directory **`server`**
1. `game.h`：Design a structure for OX games and Declare prototypes of game function.
2. `game.c`：Impelment game functions.
3. `server.c`：Implement TCP server which acts as a bridge between clients and maintain games.
4. `makefile`

#### Directory `client`
1. `client.c`：Log in to the server, contact with and play OX games with other players.
2. `makefile`

### Way to excute
`$ make`：Compile. <br>
`$ make exe`：Start the server or client. 【Server should be started first.】 <br>
`$ make clean`：Remove all files produced by `$ make` instruction. <br>

### Instruction
After log in to server, the following instructions can be enter：<br>
`List`：List online players who are not in games. <br>
`Invite [xxx]`：Invite a player to become game opponent. <br>
`Accept [xxx]`：Accept a invetation from other players. <br>
`Go [0-8]`：Place at specify lattice.

### Directions
First **enter account and password** to log in. <br>
Then list other online players. **Choose and invite a player**. <br>
When the invited player **accept the invitation, the game starts**. <br>
After the game over, new player can be chosen and invited. <br>
Players can invite many players, but only one of them become the game opponent. <br>

### Extra Functions
1. The server can support two games at the same time.
2. The player cannot be invited during a game.
3. Instruction `List` can show players who are online and not in a game, and it can be enter anytime.
