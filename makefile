CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra
LDFLAGS := -lm

TARGET  := QuantumCircuitSim
SRCS    := main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)