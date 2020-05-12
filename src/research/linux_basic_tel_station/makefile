CC := gcc
CFLAGS := -g -Wall
LIBS := /usr/lib32/libc.so -
DEBUG_FOLDER := Debug/



EXECUTABLE := $(DEBUG_FOLDER)btel
SOURCES := $(wildcard *.c)
OBJECTS :=  $(addprefix $(DEBUG_FOLDER), $(patsubst %.asm, %.o, $(patsubst %.c, %.o, $(SOURCES))))

.PHONY: debug
all: $(EXECUTABLE)
	@echo $(OBJECTS)
	@echo $(SOURCES)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	
	
	
$(DEBUG_FOLDER)%.o: %.c
	@echo $@ " :" $<
	$(CC) $(CFLAGS) -c $< -MD -o $@


include $(wildcard *.d) 


clean:
	rm -rf $(DEBUG_FOLDER)*.o $(DEBUG_FOLDER)*.d $(DEBUG_FOLDER)$(EXECUTABLE)
