## Name of Project

NAME = midi_controller

## Name of Midi output file

MIDI_FILE = output.mid

## Color for compilating (pink)

COLOR = \0033[1;35m

## List of Directories

INC_DIR = inc
OBJ_DIR = obj
SRC_DIR = src

# Add dirs here
MIDI_DIR = midi

## Compilating Utilities
FAST = -Ofast
# DEBUG = -g3 -fsanitize=address
FLAGS = -Wall -Wextra -Werror $(FAST) #$(DEBUG) -D_REENTRANT

INC = $(INC_DIR:%=-I./%)

CC = clang $(FLAGS) $(INC)

## List of Headers and C files 

INC_H = midi

MIDI_FT = midi_utilities

SRC_FT = main


## List of Utilities

SRC = $(SRC_FT:%=$(SRC_DIR)/%.c) \
	$(MIDI_FT:%=$(SRC_DIR)/$(MIDI_DIR)/%.c)

OBJ = $(SRC:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)

OBJ_DIRS = $(OBJ_DIR) \
	$(MIDI_DIR:%=$(OBJ_DIR)/%)

INCLUDES = $(INCLUDE_H:%=./$(INC_DIR)/%.h)

## Rules of Makefile

all: $(NAME)
	@echo "$(COLOR)$(NAME) \033[100D\033[40C\0033[1;30m[All OK]\0033[1;37m"

$(OBJ_DIRS):
	@mkdir -p $@
	@echo "$(COLOR)$@ \033[100D\033[40C\0033[1;32m[Created]\0033[1;37m"
	@#@echo "$(COLOR)Creating :\t\0033[0;32m$@\0033[1;37m"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDES)
	@$(CC) -c $< -o $@
	@echo "$(COLOR)$@ \033[100D\033[40C\0033[1;32m[Compiled]\0033[1;37m"

$(NAME): $(OBJ_DIRS) $(SRC) $(INCLUDES)
	@$(MAKE) -j $(OBJ)
	@echo "$(COLOR)Objects \033[100D\033[40C\0033[1;32m[Created]\0033[1;37m"
	@$(CC) $(OBJ) -o $@
	@echo "$(COLOR)$(NAME) \033[100D\033[40C\0033[1;32m[Created]\0033[1;37m"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(COLOR)Objects \033[100D\033[40C\0033[1;31m[Removed]\0033[1;37m"

fclean: clean
	@rm -f $(NAME)
	@echo "$(COLOR)$(NAME) \033[100D\033[40C\0033[1;31m[Removed]\0033[1;37m"

re: fclean all

run: coffee
	@echo ""
	@echo "$(COLOR)\"$(NAME)\" \033[100D\033[40C\0033[1;32m[Launched]\0033[1;37m"
	@./$(NAME) $(MIDI_FILE)

play: run
	@timidity $(MIDI_FILE)

rmmidi:
	@rm -f $(MIDI_FILE)
	@echo "$(COLOR)$(MIDI_FILE) \033[100D\033[40C\0033[1;31m[Removed]\0033[1;37m"

auto: play fclean rmmidi

define print_aligned_coffee
	@t=$(NAME); \
	l=$${#t};\
	i=$$((8 - l / 2));\
	echo "\0033[1;32m\033[3C\033[$${i}CAnd Your Program \"$(NAME)\" \0033[1;37m"
endef

coffee: all clean
	@echo ""
	@echo "                    {"
	@echo "                 {   }"
	@echo "                  }\0033[1;34m_\0033[1;37m{ \0033[1;34m__\0033[1;37m{"
	@echo "               \0033[1;34m.-\0033[1;37m{   }   }\0033[1;34m-."
	@echo "              \0033[1;34m(   \0033[1;37m}     {   \0033[1;34m)"
	@echo "              \0033[1;34m| -.._____..- |"
	@echo "              |             ;--."
	@echo "              |            (__  \ "
	@echo "              |             | )  )"
	@echo "              |   \0033[1;96mCOFFEE \0033[1;34m   |/  / "
	@echo "              |             /  / "
	@echo "              |            (  / "
	@echo "              \             | "
	@echo "                -.._____..- "
	@echo ""
	@echo ""
	@echo "\0033[1;32m\033[3C          Take Your Coffee"
	$(call print_aligned_coffee)

.PHONY: all clean fclean re run play auto rmmidi coffee