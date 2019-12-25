# Brian Chrzanowski
# Tue Dec 24, 2019 17:25
#
# Brian's Raytracer Makefile

CC = gcc
LINKER = -lm
FLAGS = -g3 -Wall -march=native
TARGET = bray
SRC = src/bray.c src/math.c
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d) # one dependency file for each source

all: $(TARGET)

%.d: %.c
	@$(CC) $(FLAGS) $< -MM -MT $(@:.d=.o) >$@

%.o: %.c
	$(CC) -c $(FLAGS) -o $@ $<

-include $(DEP)

$(TARGET): $(OBJ)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJ) $(LINKER)

clean:
	rm $(TARGET) src/*.d src/*.o

