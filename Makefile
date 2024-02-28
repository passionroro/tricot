CC := g++
CFLAGS := -Wall -Wextra -Werror -std=c++11
SRC_DIR := srcs
OBJ_DIR := objs
TARGET := brainfuck
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

OPENCV_INCLUDE := -I/opt/homebrew/opt/opencv/include/opencv4
OPENCV_LIBS := `pkg-config --libs opencv4`
#OPENCV_LIBS := -L/opt/homebrew/opt/opencv/lib 

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OPENCV_LIBS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(OPENCV_INCLUDE) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
