EXECUTABLE	:= word-frequency-count

$(EXECUTABLE):
	g++ -g -Wall -pthread -std=c++11 -o $(EXECUTABLE) $(EXECUTABLE).cpp 

clean:
	-rm $(EXECUTABLE)
