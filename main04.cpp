/* 
 * name: MazeSolver
 * description: Loads PNG 8 bit single channel gray scale [8UC1]file consisting of a maze; 
 * 				white pixels the paths, black pixels the walls.
 * 				The main rule of the program is, a white pixel oon the left of the border
 * 				or the top row of the image denotes the maze entrance.
 * 				Any pixel that is white on the far right of the maze or the bottom border
 * 				of the maze denotes an exit.  Each move is counted.
 * 
 * 				1/ First algorithm to solve maze is basic brute force method: bruteMove();
 * 
 * author: sloppyonefoot
 * contact: sloppyonefoot@gmail.com
 * compiled on: Kali linux-4.14.2 with: g++
 * build: make (runs Makefile in source directory)
 * created: 30/12/17
 * last modified: 02/12/17
 *
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace cv;
using namespace std;



#define COLOUR_MODE 0 		// 0 = greyscale, 1 = colour (assuming BGR)
#define MAZE_IMAGE "maze2-10x10.png"
#define ENTRANCE 0xff
#define WALL 0x00
#define STEPPER_COLOUR 0x00

const int PATH = 0xff, b = 0xff, g = 0xff, r = 0xff;
const unsigned int delay_for = 1000;

class Maze
{
	public:
		
		int width = 0, height = 0, x = height, y = height, colour = 0, **copyofMaze;
		string windowName;
		Scalar intensity;
		Mat image;
		
		Maze(string filename)
		{
			
			if ( openimage(filename) )
				cout << "Error opening file." << endl; // error message
			
						
			// Create a 2D array dynamically to suit image dimensions.
			int** map = new int*[y];
			
			for(int i = 0; i < y; ++i)
				map[i] = new int[x];
			
			copyofMaze = map;		// So we both point to the same mem location
			
			if ( mazemap(x, y, map) )
				cout << "Error creating the maze map." << endl;	// if we fuck up
			
		}
		
		int openimage(string fn)
		{
			
			// Read the image file
			image = imread(fn, COLOUR_MODE);    // Open image grey scale/colour
			
			// Check for failure
			if (image.empty())
			{
				cout << "Could not open or find the image" << endl;
				cin.get(); //wait for any key press
				return 1;
			}
			cout << "Image width, height: " << (x = image.cols) << " " << (y = image.rows) << endl;
			// windowName = fn; //Name of the window
			namedWindow("Maze Solver V0.01", WINDOW_AUTOSIZE); // Create a window
			imshow("Maze Solver V0.01", image); // Show our image inside the created window
			
		}

		int closeimage(void)
		{
			destroyWindow(windowName); //destroy the created window
			return 0;
		}
		
		int mazemap(int x, int y, int *map[])
		{
			// Prints values for debugging purposes but what 
			// this method does is map greyscale/colour 
			// values into the map[][] variable.	
			// int map[x][y] is public
		
			/* int** map = new int*[y];
			
			for(int i = 0; i < y; ++i)
				map[i] = new int[x];
			*/
			
			cout << "Begin the map: -" << endl;
				
			for ( int i = 0; i < y; i++ )
			{
				for ( int j = 0; j < x; j++ ) 
				{
					intensity = image.at<uchar>(i, j);      // in the order of y, x
					int colour = (int)intensity.val[0];
					map[i][j] = colour;
						
					cout << map[i][j] << "//" ;	// debug
					if ( j == 9 )
						cout << endl;					
				}	// eof j
			}	// eof i
			
			return 0;
		
		}
};

class Navigator
{
	public:
		int **map, height, width;
		Mat image;
		// The struct below contains the data of our Navigator object.
		struct information			
		{
			int id;					
			unsigned long number_moves;	// Helps keep track of the number of moves
			int x_location;		// position in maze
			int y_location;		// as above
		} info;
		
		Navigator(Maze theMaze)	// Constructor
		{
			static int creation_no = -1;		// This will keep track of our objects.
			creation_no++;						// They start at zero.
			map = theMaze.copyofMaze;			// get th map in this class
			height = theMaze.y;
			width = theMaze.x;
			image = theMaze.image;
			info.id = creation_no;	// init id
			info.x_location = 0;
			info.y_location = 0;
			info.number_moves = 0;
			
			// Now we locate the entrance.
			findEntrance(&info.x_location, &info.y_location);
		}
		
		// We need to check the top and left walls for a white pixel
		// representing the entrance, once the entrance is found, it is
		// closed by turning it's value black (0x00)
		void findEntrance(int *y_location, int *x_location)
		{
			for ( int row = 0; row < height; row++ )
			{
				if ( map[row][0] == ENTRANCE )
				{
					*x_location = row;	// swapped
					begin();
				}
			} // eof row

			for ( int col = 0; col < width; col++ )
			{
				if  ( map[0][col] == ENTRANCE )
				{
					*y_location = col;
					begin();
				}
			} // eof col
		}
	
	
		void begin()
		{
			cout << endl << "The entrance is located at x: " << info.x_location << ", y: "  << info.y_location << endl;
		}
		
		int bruteMove(void)
		{
			// Essentially we keep trying to move until will can in a
			// random fashion as our stepper is pretty stupid
			
			bool status = true;
			
			while ( status )
			{
				int m = gen_rand(4);	// Pick a number, any number!
				
				if ( m == 0 )
					status = right(); 	// Move right if possible
				if ( m == 1 )
					status = left();	// Move left if possible
				if ( m == 2 )
					status = down();	// Move down if possible
				if ( m == 3 ) 	
					status = up();		// move up (towards[0][x])
				
				// Each iteration is classed as an attempted move.
				info.number_moves++;
				displayStepper(image);
				usleep(delay_for);
			}	
			// When we have finally found a pixel to move too, we continue with
			// the program and print some stats.
						
			cout << "X: " << info.x_location << " Y: " << info.y_location << endl;
			
			if ( (info.number_moves % 100) == 0 )
				cout << "Move number: " << info.number_moves << endl;
			
			
			// Test if we've found the exit on the far right wall or bottom wall.
			if ( info.x_location == (height-1) || info.y_location == (width-1) ) 
			{
				cout << "Maze solved in " << info.number_moves << endl;
				cout << "Exit discoverd at x, y: " << info.x_location << ", " << info.y_location << endl;

				return 0;
			}
		
			return 1;
		}
		
		bool right(void)
		{
			// minus 1 from width so we can hit the boundy wall i.e. the exit should it be present
			if ( map[info.y_location][info.x_location + 1] == PATH ) 
			{
				info.x_location++;	// move right if it can			
				return false;
			}														
			else
				return true;
		}
		bool left(void)
		{
			// greater than zero so we don't hit the entrance (not allowed back out!) so we can hit the boundy wall i.e. the exit should it be present
			if ( map[info.y_location][info.x_location - 1] == PATH && info.x_location > 0 ) 
			{
				info.x_location--;	// move right if it can			
				return false;
			}														
			else
				return true;
		}
		bool down(void) // (Moves down in the maze be up in memory address of array element
		{
			// minus 1 from height so we can hit the boundy wall i.e. the exit should it be present
			if ( map[info.y_location + 1][info.x_location] == PATH ) 
			{
				info.y_location++;	// move right if it can			
				return false;
			}														
			else
				return true;
		}
		bool up(void)
		{
			// y greater than 0 idth so we dont hit the boundy wall
			if ( map[info.y_location - 1][info.x_location] == PATH && info.x_location > 0 ) 
			{
				info.y_location--;	// move right if it can			
				return false;
			}														
			else
				return true;
		}
	
		void displayStepper(Mat image)
		{
			Point pt =  Point(info.x_location, info.y_location);
			// MyLine(theMaze.image, info.x_location, info.y_location )
			// image.at<Vec3b>(Point(x,y)) = color
			image.at<Vec3b>(pt) = STEPPER_COLOUR;
			imshow("Maze Solver V0.01", image);
		}
	
	
		int gen_rand(int n)
		{
			unsigned int seed;
			
			FILE* urandom = fopen("/dev/urandom", "r");
			fread(&seed, sizeof(int), 1, urandom);
			fclose(urandom);
			srand(seed);

			return rand() % n;
		}
		
	
};

int main(int argc, char** argv)
{
		
	if ( argc <= 1 ) 
	{
		cout << endl << endl << "Please supply an image file, useage: mazesolver <image.png>" << endl << endl;
		exit(0);
		
	}
	
	Maze theMaze(argv[1]);
	
	
	Navigator stepper(theMaze);
	
	while ( stepper.bruteMove() )
	{
			// do nothing until stepper.move returns 0 indicating it
			// has finally found the entrance.
	}
	
	
	
	
	waitKey(0); // Wait for any keystroke in the window

	theMaze.closeimage();

	exit(0);
}
