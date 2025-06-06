CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra
CPPFLAGS := -Iinclude

TARGET   := QuantumCircuitSim

SRCDIR   := src
SRCS     := \
    main.c \
    complex.c \
    gate.c \
    parser.c \
    loader.c \
    utils.c

SRCS := $(addprefix $(SRCDIR)/,$(SRCS))

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRCS) -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
