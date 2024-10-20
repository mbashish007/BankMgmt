# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Linker flags (for OpenSSL)
LDFLAGS = -lssl -lcrypto

# Source files
SRCS = main.c ./login/login.c ./utils/file/fileUtils.c ./sessions/sessions.c ./admin/admin.c ./controller/admin/adminController.c ./service/employee/employeeService.c

# Source files
SRCS1 = ./service/customer/customerService.c ./controller/customer/customerController.c ./controller/employee/employeeController.c ./controller/manager/mgrController.c ./utils/transaction/transaction.c ./utils/loan/loan.c

# Output executable name
TARGET = server

# Default target that builds the executable
all: $(TARGET)

# Compile the executable
$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(SRCS1) $(LDFLAGS)

# Clean up build artifacts
clean:
	rm -f $(TARGET)

# Phony target to prevent confusion with files of the same name
.PHONY: all clean
