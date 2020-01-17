EXECUTABLE	:= word-frequency-count

$(EXECUTABLE):
	g++ -g -Wall -o $(EXECUTABLE) $(EXECUTABLE).cpp 

clean:
	-rm $(EXECUTABLE)
