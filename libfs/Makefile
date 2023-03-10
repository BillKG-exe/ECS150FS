targets := libfs.a
obs     := fs.o disk.o

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror  -MMD
# debug: CFLAGS += -g

all: $(targets)

deps := $(pathsubst %.o,%.d,$(objs))
-include $(deps)

libfs.a: $(obs)
	ar rcs $@ $^	

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(targets) $(objs) $(deps)
