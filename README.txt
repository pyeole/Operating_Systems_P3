Compiling the program-
1. Go into the program directory.
2. Run "make".
3. Two object files "master.o" and "player.o" will be created.
4. Two executables namely "master" and "player" will be created in the directory.

Executing the program-
1. First run the master program through following-
	$ ./master <port number> <number of players> <number of hops>

2. Start the players through following-
	$ ./player <master host name> <master port number>
