# PokeGame 327
## Version 1.04
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
- Hikers (h): Bold Red; Hikers path to the PC
- Rivals (r): Bold Red; Rivals also path to the PC
- Pacers (p): Bold Red; Pacers start with a direction and walk until they hit some terrain they cannot traverse, then they turn around and repeat, pacing back and forth.
- Wanderers (w): Bold Red; Wanderers never leave the terrain region they were spawned in. They have a direction and walk straight ahead to the edge of the terrain, whereupon they turn in a random direction and repeat.
- Sentries (s): Bold Red; Sentries don’t move, they just wait for the action to come to them.
- Explorers (e): Bold Red; Explorers move like wanderers, but they cross terrain type boundaries, only changing to a new, random direction when they reach an impassable terrain element

## Controls 

NOTE TO TAs: For this assignment, 1.04, use the command 'm' to start the movement process.

There are only a few things you can do at the moment. Right now when you load in the game, type i to see available commands.
You can move in any of the cardinal directions or you can fly to a specific point on the map. 

## Running the Game

To run the game:

1. Navigate to the folder's directory in the terminal.
2. Type "make brun". This will build and run the game in the terminal.

### Known Bugs

There are a few bugs I have discovered with my program that should be known just incase. The two seeds will show you the bugs. The first one
has an endless loop when trying to generate the first map. Second one has two buildings overlap eachother. Out of the probably thousand 
of times I have ran my program, I have NEVER had these issues again but I wanted to put them here just incase they show up.

- 1708631599 - Endless loop, cant place buildings
- 1708632100 - Buildings overlap

### Updates

**1.04 (2/23/24)**
- Added all the NPCs
- Used Dijkstra's Algorithm method to send Hiker and Rival NPCs to the player
- All NPCs are able to move
- Implemented priority queue for NPC movement.

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