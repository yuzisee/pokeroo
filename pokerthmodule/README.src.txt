To build PokerTH 0.7.1 Win32 to use holdemDLL.dll as the AI engine for local games:
	1. Make sure you have installed PokerTH 0.7.1
	2. Download (and extract) the PokerTH 0.7.1 source.
	3. Apply the holdemDLL_pokerth.0.7.1.patchfile patch to the src directory within PokerTH-0.7.1-src
	4. Create a directory named pokerai, as a sibling of PokerTH-0.7.1-src, curl, zlib, etc.
	5. Copy holdemdll.lib into your newly created pokerai directory.
	6. Build PokerTH 0.7.1 with the newly patched source tree
		qmake pokerth_game.pro
		mingw32-make
	7. Copy your newly built pokerth.exe into your PokerTH installation directory (from release directory)
	8. Copy holdemdb.ini, holdemdll.dll, data (entire directory), into the PokerTH installation directory.
	9. Start PokerTH
	NOTE: The game will run a little slower due to increased processing demands (I prefer gamespeed 9 or 10)