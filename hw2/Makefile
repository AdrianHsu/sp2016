#The following macros should be defined. 
TARGET1=big_judge 
TARGET2=judge
TARGET3=player 
OBJECT1=big_judge.o
OBJECT2=judge.o
OBJECT3=player.o
CC=gcc
LD=g++
LD_FLAGS=
#C_FLAGS= -g -O0
C_FLAGS=
# end of user configuration
#
ALL : $(TARGET1) 
 $(TARGET1) : $(OBJECT1) $(OBJECT2) $(OBJECT3)
	$(LD) $^ $(LD_FLAGS) -o $@
%.o : %.c
	$(CC) $(C_FLAGS) -c $<
.PHONY: clean
clean :
	-rm -f $(OBJECT1) $(OBJECT2) $(OBJECT3)
