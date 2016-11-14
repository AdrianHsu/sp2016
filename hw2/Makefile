#The following macros should be defined. 
OBJECT1=big_judge
OBJECT2=judge
OBJECT3=player
CC=gcc
LD=g++
#LD_FLAGS=
#C_FLAGS= -g -O0
C_FLAGS=
# end of user configuration
#
ALL : big_judge judge player

big_judge: 
	$(CC) $(C_FLAGS) big_judge.c -o $(OBJECT1)
judge: 
	$(CC) $(C_FLAGS) judge.c -o $(OBJECT2)
player: 
	$(CC) $(C_FLAGS) player.c -o $(OBJECT3)
.PHONY: clean
clean :
	-rm -f $(OBJECT1) $(OBJECT2) $(OBJECT3)
