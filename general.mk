include config.mk

all: $(TARGET).a
	ar -t $^
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(PATH_EXAMPLES)/code2.c -L. -lSimpleOfuscator -o code2.$(EXTENSION)

$(TARGET).a: $(OBJECTS)
	$(ARR) $(ARR_FLAGS) $@ $^
	ranlib $@

SimpleOfuscator.o: $(PATH_SRC)/SimpleOfuscator.c
	$(CC) $(CFLAGS) -c $^ -o $@

cleanobj:
	$(RM) $(RMFLAGS) $(OBJECTS)

cleanall: cleanobj
	$(RM) $(RMFLAGS) $(TARGET).o $(TARGET).a \
	$(TARGET_DEBUG).a

.SILENT: clean cleanobj cleanall
.IGNORE: cleanobj cleanall
.PHONY:  cleanobj cleanall