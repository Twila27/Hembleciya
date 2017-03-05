Project: Hembleciya
Milestone: 04 (Final - Save/Load, Items, Features, Terrain)
Name: Benjamin D. Gibson
Details/Videos: http://bdgibson.com/hembleciya

//-----------------------------------------------------------------------------
Build Notes		
  1. The EXE will crash if these folders do not exist under the Run folder: 
    Data/
      Audio/
      Fonts/
      XML/
        Biomes/
        Factions/
        Features/
        Items/
        NPCs/
        Saves/
      
//-----------------------------------------------------------------------------
How to Use: Message Box Controls 	
	Tilde: Show/Hide Messages
	PageUp/PageDown: Scroll Messages

How to Use: Gameplay Controls
  Movement Option: Arrow Keys
  Movement Option: Numpad
		789
		4 6
		123
  Movement Option: Roguelike Standard (rest fingers on the H through L keys' row)
		YKU
		H L
		BJN
	A: Open/Close the '+' Doors
	R: Rest (+1 Health, Costs 2 Turns)
	Comma: Pick Up Item
		) - Weapon (Dreamcatchers)
		] - Armor (Guises)
		! - Potions
	Q: Quaff (Drink) Last Acquired Potion -- as UI not a project focus, inventory is just a LIFO stack.
	Period: Drop Last Acquired (Unequipped) Item
	S/F6: Save & Exit Game -- can close and reopen program and runs can be resumed, which erases the save data in roguelike style.
	Escape: Exit Without Saving -- use if you're at the Game Over screen!
  X: Mute Background Music (intentionally begins after first death, other SFX will still play)
    
How to Use: Developer Controls
	F1: Toggle Debug Info (GameState Value/Changes, A* Pathfinding Debug Visualization)
	F2: Toggle Invincibility
	F3: Pathfind Single-step
	F4: Pathfind Multi-step
	Tab: Change Procedural Generation Mode (on Quest Selection screen)
		- Auto: randomly steps each generation process in the XML blueprint for the selected quest-biome according to its maxSteps XML value.
		- Manual (Finite Steps): after selecting a quest-biome, allows user to 
			- Run a generator logic pass for the active generator process with Spacebar (Infinite Steps bypasses the maxSteps XML value).
			- Move between previous and next generation processes with the left and right arrow keys (currently only useful in #4 Sandbar biome).

A* Path Debug Visualization Color Legend
	* Blue = Open List
	* Red = Closed List
	* Green = Path from Start to Last "Active" Node Tested
	* Yellow = Goal
  
//-----------------------------------------------------------------------------
Resource Attributions
	Fonts							            BMFont Software
	Input Font						        http://input.fontbureau.com/
	Custom Generator Inspiration	Fritz Lang's "Metropolis" (1927)
	Game Theme Inspiration			  Lakota Sioux Tribe's "Hembleciya" (Vision Quest, lit. "crying for a dream")
	Sounds							          In Data/Audio/Attributions.txt
    
//-----------------------------------------------------------------------------
Special Postmortem Note: dream/amalgamate behaviors made EVERYTHING harder, 
    - e.g. swapping maps out back and forth meant features and items getting swapped over or stored on another map, 
  - e.g. save/load became a nightmare + amalgamate's fused entity IDs making restoring entity pointers less straightforward!
	
//-----------------------------------------------------------------------------
Quest Selection Guide
	1.) Cavernous Dreams of a Cellular Automaton
		- Generator: CellularAutomata ("GameOfLifeCaves")
		- Playable: Low
		- Motivation: modifications on the original Game of Life and Harward rules.
	2.) Conway's Game of Life Caves
		- Generator: CellularAutomata ("HarwardCaverns")
		- Ability to Play: Very Low
		- Motivation: primarily kept around out of the desire to demonstrate the origin and subsequent growth of the generator system.
	3.) Diagonal Dungeon (Prone to Break)
		- Generator: PrimHarwardGenerator ("Dungeon")
		- Playable: Very Low
		- Motivation: circular rooms, which prompted diagonal paths as straight paths did not connect correctly. 
			- Checks are too strict even after attempts to soften them, creating too few rooms.
	4.) Sandbars, Oceans, and Lava Pits (Terrain Types)
		- Generator: CellularAutomata ("HarwardCaverns") + CellularAutomata ("GameOfLifeCaves") + PrimHarwardGenerator ("Sandbar")
		- Playable: Moderate
		- Motivation: Combining generators together using the BiomeBlueprint framework.
	5.) Sleepwalking in the Metropolis
		- Generator: KruskalDartboardGenerator ("Metropolis")
		- Playable: High
	6.) Sleepwalking in the Metropolis (Hub)
		- Generator: KruskalDartboardGenerator ("MetropolisHub")
		- Playable: High
	7.) The Arena of Neverending Hallways
		- Generator: FromDataGenerator (see Data/XML/SecondMap.Map.xml, loaded by Data/XML/Biomes/FromDataTest.Biome.xml)
		- Playable: High
		
//-----------------------------------------------------------------------------
Old Notes on Generator Implementation for Context
	* Cellular Automaton Generator
		- Game of Life Caverns: 50-50 initialization of air/solid and Game of Life rules.
		- Harward Caverns: based on a modification of Prof. Harward's rules from lecture.
	* Dungeon Generators: 
		- Dungeon: Modified Non-dartboard Harward/Prim Algorithm
			//Uses diagonal halls to connect its circular rooms.
		- Sandbar Variant: I didn't like the results from the dungeon, so I decided to add post-processing to kill walls with few neighbors.
			But I happened to use water as the replacement type, to see which ones were being killed--giving it this fascinating look.
		- Goal was to create a less boxy, more organic feeling.
	~ Custom Generators: Dartboard/Kruskal Algorithm
		- "Metropolis": Dartboard Method from Class
		- "MetropolisHub": Dartboard Method Linking from Only Room #0 (forced to max room dimensions and the map center)
		- To create a boxy, inorganic or mechanical-industrial feeling to contrast with Dungeons.
	* From Data Generator
  
	note: doors exist only in metropolis map generators
  
//-----------------------------------------------------------------------------
Final Post-class Roadmap
	A1: Map Rescaling & Recentering
	A2: 10^x Random Path Generation for Performance Testing (Extra Credit)
	A4:
		- Consider an alternative game-coherent implementation for PickUpItem Behavior.
		- Save/Restore Turn Order Information (Extra Credit)
	Personal:
		- See the large blocks of notes for future development in the XML files, esp. NPC.xml and Items.xml.
		- Display the current attack and defense stats for the player at all times during the game.
		- Better key displays for the generation modes.
		* NPCBlueprint
			- "spawnRarity" attribute defaulting to 100% in constructor for how rarely it spawns.
				- Even better, a way to specify which NPCs spawn with which biomes.
			- "spawnLevel" attribute defaulting to 0 in constructor.
				- Checked against a Map's new m_level integral member which must surpass spawnLevel to allow it to spawn the type on it.
			- A more human-readable format for "traversableProperties" attribute.
				- Suggestion: blockedBy="walls,air,fire,diagonal,cardinal" slowedBy="..."	
		* ItemBlueprint
			- "spawnLevel", as above.
		- Add inhabitants="npcblueprintnames" in EnvironmentBlueprints to control which types can spawn in it.

//-----------------------------------------------------------------------------	
A4 Known Issues
	0. PickUpItem unimplemented because having dreams pick up items designed to be hostile to them didn't make sense for the game. 
		- Grade points somewhat compensated for by implementing Agent::GetBestWeapon() and Agent::GetBestArmor().
		- Especially with dream-breaking necessitating the destruction of items (copying them around was a headache enough).
	1. Save/Load with a dream map activated breaks colors.
		- Occurs because the non-dream color getter for the non-dream "real" glyphs will currently be invoked instead of the for-dreams-only parsedGlyph usage, because the dream is part of the real map at that point in time.
		- Loading a saved file bugs out if you save while being inside a dream map, load the file, kill the dreamer, trigger another dream, and try to save and load then (the second dreamer needs to immediately follow the first, so it might just not be reattaching to the real world map as the first dreamer dies).
	2. Combat currently takes the Agent's highest weapon and armor, rather than the sum of all such items they have equipped.
		- Because of how AttackData only takes one Item* each for weapon and armor.
	3. Uncommenting Agent::Update in NPC::Update to make NPCs take lava damage screws up untargeting, doesn't set targetEnemy to nullptr for others, and so we crash
	4. Because of how Inventory::EraseItem() and burning away in lava work, items may not be getting removed from theGame's entity list.
		- Therefore, they may still be getting saved out?
	5. From brief testing, in general, features appear to behave unexpectedly when dreams get created around them.
	
A3 Known Issues
	1. Some issues left for later with Behavior minutia: 
		- ChaseBehavior's max turns limit.
		- DreamBehavior, if you break out of a dream and are surrounded by solid tiles in the restored map, you're stuck!
			- Because right now this tends to cut the dream-making NPC off from others it could fuse Amalgamate, 
			perhaps in future don't trigger a dream map until m_visibleAgents[1]'s position would be within the dream map.
		- AmalgamateBehavior, need to find out how to access other behaviors to buff damage and blend dream maps during fusion.
	2. Right now when you die, nothing prevents remaining NPCs from running at the simulation delta, which can make things superfast.
		Not sure whether or not this is bad in design--if it were, note to self to just dial down the delta on player death or make it equal the "real" time's deltaSeconds.
	
A1 Known Issues
	1. The Dungeon generators do not always place a reasonable amount of rooms (too many forbid-on-overlap cases even after easing threshold).
	2. Smaller maps do not resize nor fit to the center of the screen.