include config.mk

all: $(TARGET).a libLZ77_c.a
	ar -t $^
	gcc -o code2.$(EXTENSION) $(PATH_EXAMPLES)/code2.c $(CFLAGS) $(LINKER_FLAGS) 

$(TARGET).a: $(OBJECTS) 
	$(ARR) $(ARR_FLAGS) $@ $^
	ranlib $@

libLZ77_c.a:
	$(MAKE) -C ./$(PATH_LZ77_c) -f $(MAKE_NAME)

SimpleOfuscator.o: $(PATH_SRC)/SimpleOfuscator.c
	$(CC) $(CFLAGS) -c $^ -o $@

cleanobj:
	$(RM) $(RMFLAGS) $(OBJECTS)
	$(MAKE) -C ./$(PATH_LZ77_c) -f $(MAKE_NAME) cleanobj

cleanall: cleanobj
	$(RM) $(RMFLAGS) *.o *.a $(TARGET_DEBUG).a *.$(EXTENSION)
	$(MAKE) -C ./$(PATH_LZ77_c) -f $(MAKE_NAME) cleanall


.SILENT: clean cleanobj cleanall
.IGNORE: cleanobj cleanall
.PHONY:  cleanobj cleanall