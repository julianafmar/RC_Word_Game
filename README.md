# RC_Word_Game

The project compiles using the command "make". It's possible to clean the folder using the command "clean" (this will delete all files apart from the program and the files .txt since word_eng.txt is located in the root).
To execute the program you can use the command "./GS word_eng.txt" for the server and "./player" for the client.

The timeout time is a constant (TIMER_VALUE) that can be modified in the player and GS header files.

word_eng.txt contains the words and their respective hints. It's placed in the root directory.

Directory HINTS contains the images used as hints for the words in word_eng.txt.

Directory GAMES constains the players directories, which have the finished games of the respective player.

Directory SCORES contains the score files of finished games.