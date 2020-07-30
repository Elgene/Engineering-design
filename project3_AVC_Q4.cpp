#include <iostream>
#include "E101.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

using namespace std;

class ROBOT
{	
private:
	int v_left; 
	int v_right;
	int cam_tilt; 
	int dv;
	int n_black;
	int n_red;
	int n_green;
	int n_blue;
	int n_yellow;
	int numOfTurns = 0;
	int paperFlag = 0;
	int count = 0;
	double shapeError;
	double lineError;
	double kp = 0.05;
	const int cam_width = 320;
	const int cam_height = 240;
	const int v_left_go = 52;
	const int v_right_go = 43;
	bool hardTurn= false;
	bool reachedRed= false;
	bool reachedGreen= false;
	bool reachedBlue= false;
	bool finished= false;
	bool linePresent = true;
	bool shapePresent = true;
	
public:
	ROBOT(){};
	void setMotors();
	int initHardware();
	int measureLine();	
	int followLine();
	int OpenGate();
	int turnLeft();
	int turnRight();
	int lookLeft();
	int lookRight();
	int goToShape();
	int measureShape();
	int victoryDance();
	int redFlag();
	int run();
};

int ROBOT::OpenGate()
{   char server_addr[] = "130.195.6.196";
	connect_to_server(server_addr, 1024);
	char message[]= "Please";
	send_to_server(message);
	char password[24];
	receive_from_server(password);
	send_to_server(password);
	v_left = 63;				// fowards speed range from 49 -> 65 | left wheel motor 5						
    v_right = 30;				// fowards speed rande from 46 -> 30 | right wheel motor 1
    setMotors();
    sleep1(2000);				//go straight at max speed for 2 seconds, then find and follow line
	return 1;
}

int ROBOT::initHardware()
{	int err;
    cout<<"Start"<<endl;
    err = init(0);
    cout<<"After init() error: "<<err<<endl;
    v_left = 48;					// fowards speed range from 49 -> 65 | left wheel motor 5
    v_right = 47;					// fowards speed rande from 46 -> 30 | right wheel motor 1
	cam_tilt = 62;  				// 35->faces up......62->faces down
	dv = 0;
	setMotors();
	return err;
}

 void ROBOT::setMotors()
{   //cout<<"v_left: "<<v_left<<"     v_right: "<<v_right<<endl;
	set_motors(5,v_left);			// fowards speed range from 49 -> 65 | left wheel motor 5
	set_motors(1,v_right);			// fowards speed rande from 46 -> 30 | right wheel motor 1
	set_motors(3, cam_tilt);
	hardware_exchange();
}
 	
int ROBOT::measureLine()
{	int row = 120;
	int col = 0;
	double threshold = 70;
	take_picture();
	update_screen();
	linePresent= true;
	lineError = 0.0;
	n_black = 0;
	for( col = 0; col < cam_width; col++)
	{	if( get_pixel(row, col, 3) <= threshold)
		{	lineError += (col - cam_width/2.0);				//calculate line error
			n_black++;
		}
	}
	//cout <<"Number of Black Pixels: "<<n_black<<endl;
	if(n_black>300)											//if intersection ahead
	{	if(numOfTurns < 2)									//go left if number of left turns is less than 2
		{	turnLeft();
		}
		else                                                //go right if number of left turns has been reached
		{	turnRight();
		}
		numOfTurns++;
	}
	//cout<<"Number Of Turns: "<<numOfTurns<<endl;
	if(n_black < 10)
	{	linePresent = false;
		return -1;
	}
	linePresent = true;
	//cout<<"Line Error: "<<lineError<<endl;
	lineError = lineError/n_black;
	return 0;
}

int ROBOT::followLine()
{  	measureLine();
	if (hardTurn==true)
	{	hardTurn = false; 
		return 0; 
	}
	//cout<<"Line Present: "<<linePresent<<endl;
	if(linePresent==true)								//follow line
	{	dv = (int)(lineError*kp);
		v_left = v_left_go + dv;
		v_right = v_right_go + dv;
		//cout<<"Line Error: "<<lineError<<"   dv: "<<dv<<endl;
		setMotors();
	}
	else                                                //reverse
	{	//cout<<"Line is Missing"<<endl;
		v_left = 43;
		v_right = 52;
		setMotors();
		sleep1(100);
	}
	return 0;
}
 
int ROBOT::redFlag()
{ 	int n_red=0;
	int row = 120;
	int col = 0;
	for(col = 0; col < cam_width; col+=2)
	{	int red = get_pixel(row, col, 0);			//get amount of red in pixel
		int green = get_pixel(row, col, 1);			//get amount of green in pixel
		int blue = get_pixel(row, col, 2);			//get amount of blue in pixel
		//cout<<"Red: "<<red<<"     Green: "<<green<<"     Blue: "<<blue<<endl;
		if (green+blue<red-20)					//count number of red pixels mid screen
		{	n_red++;
		}
	}
	//cout<<"Number of Flags: "<<paperFlag<<endl;
	if (n_red > 20)										//flag count +1
	{	paperFlag++;
		v_left = 52;		// fowards speed range from 49 -> 65 | left wheel motor 5							
		v_right = 42;		// fowards speed rande from 46 -> 30 | right wheel motor 1
		setMotors();
		sleep1(1500);			//go straight for 1.5 seconds, then find line again
	}
	return 0;
}	

int ROBOT::turnLeft()	//turn 90 degrees to the left
{   v_left = 48;        // fowards speed range from 49 -> 65 | left wheel motor 5
    v_right = 35;  		// fowards speed rande from 46 -> 30 | right wheel motor 1
    setMotors();
    sleep1(2000);
    return 0;
}

int ROBOT::turnRight()	//turn 90 degrees to the right
{   v_left = 60;		// fowards speed range from 49 -> 65 | left wheel motor 5
    v_right = 47;		// fowards speed rande from 46 -> 30 | right wheel motor 1
    setMotors();
    sleep1(2000);
    return 0;
}

int ROBOT::measureShape()
{	take_picture();
	update_screen();
	int row = 120;
	int col = 0;
	shapePresent = 1;
	shapeError = 0.0;
	n_red = 0;
	n_green = 0;
	n_blue = 0;
	n_yellow = 0;
	for( col = 0; col < cam_width; col++)
	{	int red = get_pixel(row, col, 0);			//get amount of red in pixel
		int green = get_pixel(row, col, 1);			//get amount of green in pixel
		int blue = get_pixel(row, col, 2);			//get amount of blue in pixel
		//cout<<"Red: "<<red<<"     Green: "<<green<<"     Blue: "<<blue<<endl;
		if(reachedRed==false && reachedGreen==false && reachedBlue==false)											//looking for red		
		{	if(green+blue<red-20)		//is it a red pixel?
			{	n_red++;																							//add to the number of red pixels
				shapeError += (col - cam_width /2.0);																//calculate shape_error
			}
		}
		else if(reachedRed==true && reachedGreen==false && reachedBlue==false)										//looking for green		
		{	if(blue<red && blue*2<green)//is it a green pixel?		//// check values
			{	n_green++;																							//add to the number of green pixels
				shapeError += (col - cam_width /2.0);																//calculate shape_error
			}
		}
		else if(reachedRed==true && reachedGreen==true && reachedBlue==false)				//looking for blue	
		{	if(red<green && red<blue && red<100 && green>100 && blue>120 && green<150 && blue<170)		//is it a blue pixel?     //// check values
			{	n_blue++;																	//add to the number of blue pixels
				shapeError += (col - cam_width /2.0);										//calculate shape_error
			}
		}
		else if(reachedRed==true && reachedGreen==true && reachedBlue==true)				//looking for yellow		
		{	if(red>blue && green>blue && red>110 && green>110 && blue>40 && red<190 && green<190 && blue<100)			//is it a yellow pixel?			//// check values
			{	n_yellow++;																	//add to the number of yellow pixels
				shapeError += (col - cam_width /2.0);										//calculate shape_error
			}
		}	
	}
	if((n_red<5 && reachedRed==false) || (n_green<5 && reachedRed==true && reachedGreen==false) || (n_blue<5 && reachedGreen==true && reachedBlue==false) || (n_yellow<5 && reachedBlue==true))
	{	//cant see enough pixels of the right colour; shape isnt present
		shapePresent = false;
		return -1;
	}
	if(reachedRed==false && reachedGreen==false && reachedBlue==false)					//looking for red
	{	if(n_red>300)																	//if red shape is reached reverse for 1 second then look left
		{	reachedRed=true;
			v_left = 43;		// fowards speed range from 49 -> 65 | left wheel motor 5
			v_right = 52;		// fowards speed rande from 46 -> 30 | right wheel motor 1
			setMotors();
			sleep1(2000);
			lookLeft();
			return 0;
		}	
		shapePresent = true;
		//cout<<"Shape Error: "<<shapeError<<endl;
		shapeError = shapeError/n_red;
		return 0;	
	}
	else if(reachedRed==true && reachedGreen==false && reachedBlue==false)				//looking for green
	{	if(n_green>300)																	//if green shape is reached reverse for 1 second then look right
		{	reachedGreen=true;
			v_left = 43;
			v_right = 52;
			setMotors();
			sleep1(2000);
			lookRight();
			return 0;
		}
		shapePresent = true;
		//cout<<"Shape Error: "<<shapeError<<endl;
		shapeError = shapeError/n_green;
		return 0;	
	}
	else if(reachedRed==true && reachedGreen==true && reachedBlue==false)				//looking for blue
	{	if(n_blue>280)																	//if blue shape is reached reverse for 1 second then look left
		{	reachedBlue=true;
			v_left = 43;
			v_right = 52;
			setMotors();
			sleep1(2000);
			lookLeft();
			return 0;
		}
		shapePresent = true;
		//cout<<"Shape Error: "<<shapeError<<endl;
		shapeError = shapeError/n_blue;
		return 0;	
	}
	else if(reachedRed==true && reachedGreen==true && reachedBlue==true)				//looking for yellow
	{	if(n_yellow>300)																	//if yellow shape is reached reverse for 1 second then DANCE!!!!!!
		{	v_left = 43;
			v_right = 52;
			setMotors();
			sleep1(2000);
			victoryDance();
			count=10000001;
			return 0;
		}
		shapePresent = true;
		//cout<<"Shape Error: "<<shapeError<<endl;
		shapeError = shapeError/n_yellow;
		return 0;	
	}
	return 0;
}

int ROBOT::goToShape()
{	measureShape();
	//cout<<"Shape Present: " <<shapePresent<<endl;
	if(shapePresent==true)								//coloured shape in sight, go to it
	{	dv = (int)(shapeError*kp);
		v_left = v_left_go + dv;
		v_right = v_right_go + dv;
		//cout<<"Shape Error: "<<shapeError<<"dv: "<<dv<<endl;
		setMotors();
	}
	else                                                  //no present shape; look around to find shape
	{	//cout<<"cant see shape"<<endl;
		if(reachedRed==false && reachedGreen==false && reachedBlue==false)
		{	lookRight();
		}
		else if(reachedRed==true && reachedGreen==false && reachedBlue==false)
		{	lookLeft();
		}
		else if(reachedRed==true && reachedGreen==true && reachedBlue==false)
		{	lookRight();
		}
		else if(reachedRed==true && reachedGreen==true && reachedBlue==true)
		{	lookLeft();
		}	
	}
	return 0;
}

int ROBOT::lookLeft()	//turn a small amount to the left
{   v_left = 48;        // fowards speed range from 49 -> 65 | left wheel motor 5
    v_right = 43;  		// fowards speed rande from 46 -> 30 | right wheel motor 1
    setMotors();
    sleep1(700);
    return 0;
}

int ROBOT::lookRight()  //turn a small amount to the right
{   v_left = 52;		// fowards speed range from 49 -> 65 | left wheel motor 5
    v_right = 47;		// fowards speed rande from 46 -> 30 | right wheel motor 1
    setMotors();
    sleep1(700);
    return 0;
}

int ROBOT::victoryDance()  //hopefully this gets used...
{   v_left = 65;			// fowards speed range from 49 -> 65 | left wheel motor 5
    v_right = 65;			// fowards speed rande from 46 -> 30 | right wheel motor 1
    cam_tilt = 62;
    setMotors();
    sleep1(600);
    cam_tilt = 35;
    setMotors();
    sleep1(600);
    cam_tilt = 62;
    setMotors();
    sleep1(600);
    cam_tilt = 35;
    setMotors();
    sleep1(600);
    cam_tilt = 62;
    setMotors();
    sleep1(600);
    cam_tilt = 35;
    setMotors();
    sleep1(600);
    cam_tilt = 62;
    setMotors();
    sleep1(600);
    cam_tilt = 35;
    setMotors();
    sleep1(600);
    cam_tilt = 62;
    setMotors();
    sleep1(600);
    cam_tilt = 35;
    setMotors();
    sleep1(600);
    cam_tilt = 62;
    setMotors();
    sleep1(600);
    cam_tilt = 35;
    setMotors();
    sleep1(600);
    return 0;
}

int ROBOT::run()
{	initHardware();
	OpenGate();
	open_screen_stream();
	while(count < 10000000)
	{	if (paperFlag < 2)				//Q1, Q2, Q3
		{	//take_picture();
			//update_screen();
			followLine();
			redFlag();
			//cout<<"count: "<<count<<endl;
		}
		else if  (paperFlag > 1)                        //Q4
		{	if(cam_tilt==62)			//if just getting to Q4 look up
			{	cam_tilt = 35; 
				lookRight();
				//sleep1(1000);
			}	
			//take_picture();
			//update_screen();
			goToShape();
		}
		count++;		
	}
	close_screen_stream();
	stoph();
	return 0;
}

int main()
{
	ROBOT robot;
	robot.run();
}
 
	
	
	

