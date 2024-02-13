# PokeGame 327
## Version 1.03
### Creator: Jacob Garcia

PokeGame 327 is a game currently under development for ISU class COM S 327. It is a Pokemon-style game, and it is expected to be finished by May when the class concludes. Please see below sections for information about the game.

## World

The map is randomly generated each time with different roads and terrain.

- Mountains (%): Grey; denotes the world border.
- Trees (^): Grey; you cannot walk past them.
- Water (~): Blue; you cannot pass through.
- Short grass (.): Green; clearings where you can walk through without encountering Pokemon.
- Tall grass (:): Green; you can walk through tall grass but might be stopped by Pokemon.
- Roads (#): Yellow; they go from north to south and east to west. 
- PokeMarts (M): Magenta; one 2x2 PokeMart on each map.
- PokeCenters (C): same as PokeMart.

## Characters 

There are different types of characters. The player, NPCs, and Pokemon.

- PC (@): White; the player.

## Controls 

NOTE: This version of the game (1.03) only prints the paths then exits. 

There are only a few things you can do at the moment. Right now when you load in the game, type i to see available commands.
You can move in any of the cardinal directions or you can fly to a specific point on the map. 

## Running the Game

To run the game:

1. Navigate to the folder's directory in the terminal.
2. Type "make brun". This will build and run the game in the terminal.

### Updates

**1.03 (2/13/24)**
- Added Player Character (PC)
- Added Dijkstra's Algorithm to calculate distances from the PC
- Prints all paths to PC, which when printed will be green. 
- Seeds get saved in txt file for debugging purposes

**1.02 (2/2/24)**
- Added the World Map.
- Ability to move from map to map.
- Lined up paths to connect all maps with gates. 
- Closed paths at world borders. 
- Buildings are now not guaranteed to spawn everytime. Lower chances the farther you go out. 

**1.01 (1/25/24)**
- Initial creation.
- Map creation.