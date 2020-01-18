EXECUTABLE	:= word-frequency-count

$(EXECUTABLE):
	g++ -g -Wall -pthread -o $(EXECUTABLE) $(EXECUTABLE).cpp 

clean:
	-rm $(EXECUTABLE)
