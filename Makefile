EXECUTABLE	:= word-frequency-count

$(EXECUTABLE):
	g++ -g -Wall -std=c++11 -pthread -o $(EXECUTABLE) $(EXECUTABLE).cpp 

clean:
	-rm $(EXECUTABLE)
