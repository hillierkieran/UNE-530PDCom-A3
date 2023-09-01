# Compiler settings
CC = mpicc
CFLAGS = -Wall -pedantic

# Directories
OBJDIR = build/
SRCDIR = src/

# Object files
OBJS =  $(OBJDIR)a3.o \
        $(OBJDIR)convolution.o \
        $(OBJDIR)matrix_utils.o \
        $(OBJDIR)matrix.o \
		$(OBJDIR)mpi_utils.o

# Main target
all: directories mkRandomMatrix getMatrix a3

# Rule for creating the object files
$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Rule for creating directories
directories: 
	mkdir -p $(OBJDIR)

# Compile targets
mkRandomMatrix: $(OBJDIR)mkRandomMatrix.o $(OBJDIR)matrix.o
	$(CC) -o $(OBJDIR)$@ $^ $(CFLAGS)

getMatrix: $(OBJDIR)getMatrix.o $(OBJDIR)matrix.o
	$(CC) -o $(OBJDIR)$@ $^ $(CFLAGS)

a3: $(OBJS)
	$(CC) -o $(OBJDIR)$@ $^ $(CFLAGS) -lm

# Additional operations
clean:
	rm -f *~ $(SRCDIR)*.o $(OBJDIR)*

run:
	./$(OBJDIR)mkRandomMatrix input_matrix 4 
	./$(OBJDIR)/getMatrix input_matrix 4
	mpirun -np 4 $(OBJDIR)a3 input_matrix output_matrix 2
	./$(OBJDIR)/getMatrix output_matrix 4

.PHONY: clean run all directories
