
override _CFLAGS := -DSWCOMP_VERSION='"$(shell git describe --tags --always --dirty)"' -DCLIENT_APP

CFLAGS := -Wall -Wextra -Werror -Wno-unused-result -Wno-format-truncation -Wformat=2 -std=c89 --pedantic
LDFLAGS :=

CLIENT_OBJ = $(patsubst ./%.c,../../build/client/%.o,$(shell find . -name '*.c'))

.PHONY: all client
all: client
client: ../../bin/swcomp-client


../../bin/swcomp-client: $(CLIENT_OBJ)
	@mkdir -p $(@D)
	@$(info $S  LD $(patsubst ../../build/client/%.o,%.o,$<) ==> $(patsubst ../../bin/%,%,$@))
	@$(CC) $(CFLAGS) $(CFLAGS_MENU) -o $@ $^ $(LDFLAGS) $(LDFLAGS_MENU)

../../build/client/%.o: %.c
	@mkdir -p $(@D)
	@$(info $S  CC $< ==> $(patsubst ../../build/client/%.o,%.o,$@))
	@$(CC) $(_CFLAGS) $(CFLAGS) $(CFLAGS_MENU) -c -o $@ $<
