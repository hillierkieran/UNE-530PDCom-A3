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
        $(OBJDIR)mkRandomMatrix.o \
        $(OBJDIR)getMatrix.o \
        $(OBJDIR)matrix.o

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

a3: $(OBJDIR)a3.o $(OBJDIR)matrix_utils.o $(OBJDIR)convolution.o $(OBJDIR)matrix.o
	$(CC) -o $(OBJDIR)$@ $^ $(CFLAGS)

# Additional operations
clean:
	rm -f *~ $(SRCDIR)*.o $(OBJDIR)*

run:
	./$(OBJDIR)mkRandomMatrix input_matrix 6
	./$(OBJDIR)/getMatrix input_matrix 6
	mpirun -np 4 $(OBJDIR)a3 input_matrix output_matrix 3
	./$(OBJDIR)/getMatrix output_matrix 6

.PHONY: clean run all directories
