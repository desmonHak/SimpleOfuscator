include config.mk

all: $(TARGET).a
	ar -t $^
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(PATH_EXAMPLES)/code2.c $(LINKER_FLAGS) -o code2.$(EXTENSION)

$(TARGET).a: $(OBJECTS)
	$(ARR) $(ARR_FLAGS) $@ $^
	ranlib $@

SimpleOfuscator.o: $(PATH_SRC)/SimpleOfuscator.c
	$(CC) $(CFLAGS) -c $^ -o $@

cleanobj:
	$(RM) $(RMFLAGS) $(OBJECTS)
	$(MAKE) -C ./$(PATH_LZ77_c) -f $(MAKE_NAME) cleanobj

cleanall: cleanobj
	$(RM) $(RMFLAGS) *.o $(TARGET).a \
	$(TARGET_DEBUG).a *.$(EXTENSION) 
	$(MAKE) -C ./$(PATH_LZ77_c) -f $(MAKE_NAME) cleanall

.SILENT: clean cleanobj cleanall
.IGNORE: cleanobj cleanall
.PHONY:  cleanobj cleanall