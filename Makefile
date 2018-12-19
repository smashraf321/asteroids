space_shooter: space_shooter.cpp
	g++ -o space_shooter space_shooter.cpp -lsfml-window -lsfml-graphics -lsfml-system
clean:
	rm -rf space_shooter
