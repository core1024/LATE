# LATE - Little Arduboy Timekiller Endeavour
A game that features several modes that fit into Arduboy memmory. I find them simple enough to code and pack together, so I'll be able to enjoy them on Arduboy too.

The game state is saved in the EEPROM when the pause menu is issued (usually by pressing [B]).

As of now, there's no support for RGB LED and sound.

Music is not planned at all.

## Tips and tricks:

Hold [Left] while pressing [A] in the menu for reset.
That way you can reset the current game state to start a new game (in the pause menu) or even reset the whole game data for the current mode (in the main menu), including high scores and any game state.

## Modes:
 * **Blocks Arcade** - Tetris clone. Graphics inspied by the Gameboy version. Gameplay features ghost piece, wall kicks and lock delay.
    ### Fetures
    - Enjoyable gameplay
    - Easy on the eyes graphics
    ### TODO:
    - Wall kicks sometimes fell wired
    - RGB/Sound signals for start, lock, line and game over.

 * **Blocks Puzzle** - A clone of 1010! by Gram Games.
    ### Fetures
    - Enjoyable gameplay
    - Easy on the eyes graphics
    ### TODO:
    - Performance tweaks.
    - Scoring systems? I need to verify with the origial for correctness.
    - Random generator. According to [this page](http://blog.coelho.net/games/2016/07/28/1010-game.html) 1010! uses uneven random distribution accross the pieces.
    - RGB/Sound signals for start, obstructed/placed block and game over.

## More modes?
I have plans to add more modes. Maybe even blockless. Something like "Stick Hero" by Ketchapp, "Flappy Bird" by dotGEARS or T-Rex Runner by... IDK maybe Google?

If it fits it sits :)
```
  /\**/\
 ( o_o  )_)
 ,(u  u  ,),
{}{}{}{}{}{}

```