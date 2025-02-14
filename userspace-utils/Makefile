override _CFLAGS := -DSWCOMP_VERSION='"$(shell git describe --tags --always --dirty)"'
ifeq ($(DEBUG_FEATURES),1)
override _CFLAGS += -DDEBUG_FEATURES=1
endif
ifeq ($(INPUT_DEBUG),1)
override _CFLAGS += -DINPUT_DEBUG=1
endif

CFLAGS := -Wall -Wextra -Werror -Wno-unused-result -Wno-format-truncation -Wformat=2 -std=c89 --pedantic
CFLAGS_MENU := -I src/menu/include -fno-omit-frame-pointer
LDFLAGS :=

MENU_OBJ = $(patsubst src/%.c,build/%.o,$(shell find src/menu -name '*.c'))

.PHONY: all clean menu
all: menu
clean:
	rm -rf bin build
menu: bin/menu


bin/menu: $(MENU_OBJ)
	@mkdir -p $(@D)
	@$(info $S  LD $(patsubst build/menu/%.o,%.o,$<) ==> $(patsubst bin/%,%,$@))
	@$(CC) $(CFLAGS) $(CFLAGS_MENU) -o $@ $^ $(LDFLAGS) $(LDFLAGS_MENU)

build/menu/%.o: src/menu/%.c
	@mkdir -p $(@D)
	@$(info $S  CC $(patsubst src/menu/%.c,%.c,$<) ==> $(patsubst build/menu/%.o,%.o,$@))
	@$(CC) $(_CFLAGS) $(CFLAGS) $(CFLAGS_MENU) -c -o $@ $<
