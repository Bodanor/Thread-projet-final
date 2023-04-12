PRESENTATION=presentation

CC = g++-9 -DSUN -I$(PRESENTATION)
OBJS = $(PRESENTATION)/presentation.o 
PROGRAMS = dkjr

ALL: $(PROGRAMS)

dkjr:	dkjr.cpp $(OBJS)
	echo Creation de dkjr...
	$(CC) dkjr.cpp -o dkjr $(OBJS)  -lrt -lpthread -lSDL -Wall -Wextra -g

$(PRESENTATION)/presentation.o:	$(PRESENTATION)/presentation.c $(PRESENTATION)/presentation.h
		echo Creation de presentation.o ...
		$(CC) -c $(PRESENTATION)/presentation.c -Wall -Wextra -g
		mv presentation.o $(PRESENTATION)

clean:
	@rm -f $(OBJS) core

clobber:	clean
	@rm -f tags $(PROGRAMS) 
 

